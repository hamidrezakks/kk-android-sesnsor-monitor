#include "phonemodel.h"

#include <QGLWidget>
#include <QMatrix4x4>
#include <QVector3D>

#include <qmath.h>


static const qreal phone_height = 1.28;
static const qreal phone_width = 0.709;
static const qreal phone_depth = 0.089;
static const qreal phone_curve_height = 0.1;
static const qreal phone_curve_width = 0.1;

struct Geometry
{
    QVector<GLushort> faces;
    QVector<QVector3D> vertices;
    QVector<QVector3D> normals;
    void appendSmooth(const QVector3D &a, const QVector3D &n, int from);
    void appendFaceted(const QVector3D &a, const QVector3D &n);
    void finalize();
    void loadArrays() const;
};

class Patch
{
public:
    enum Smoothing { Faceted, Smooth };
    Patch(Geometry *);
    void setSmoothing(Smoothing s) { sm = s; }
    void translate(const QVector3D &t);
    void rotate(qreal deg, QVector3D axis);
    void draw() const;
    void addTri(const QVector3D &a, const QVector3D &b, const QVector3D &c, const QVector3D &n);
    void addQuad(const QVector3D &a, const QVector3D &b,  const QVector3D &c, const QVector3D &d);

    GLushort start;
    GLushort count;
    GLushort initv;

    GLfloat faceColor[4];
    QMatrix4x4 mat;
    Smoothing sm;
    Geometry *geom;
};

static inline void qSetColor(float colorVec[], QColor c)
{
    colorVec[0] = c.redF();
    colorVec[1] = c.greenF();
    colorVec[2] = c.blueF();
    colorVec[3] = c.alphaF();
}

void Geometry::loadArrays() const
{
    glVertexPointer(3, GL_FLOAT, 0, vertices.constData());
    glNormalPointer(GL_FLOAT, 0, normals.constData());
}

void Geometry::finalize()
{
    for (int i = 0; i < normals.count(); ++i)
        normals[i].normalize();
}

void Geometry::appendSmooth(const QVector3D &a, const QVector3D &n, int from)
{
    // Smooth normals are achieved by averaging the normals for faces meeting
    // at a point.  First find the point in geometry already generated
    // (working backwards, since most often the points shared are between faces
    // recently added).
    int v = vertices.count() - 1;
    for ( ; v >= from; --v)
        if (qFuzzyCompare(vertices[v], a))
            break;

    if (v < from) {
        // The vertex was not found so add it as a new one, and initialize
        // its corresponding normal
        v = vertices.count();
        vertices.append(a);
        normals.append(n);
    } else {
        // Vert found, accumulate normals into corresponding normal slot.
        // Must call finalize once finished accumulating normals
        normals[v] += n;
    }

    // In both cases (found or not) reference the vertex via its index
    faces.append(v);
}

void Geometry::appendFaceted(const QVector3D &a, const QVector3D &n)
{
    // Faceted normals are achieved by duplicating the vertex for every
    // normal, so that faces meeting at a vertex get a sharp edge.
    int v = vertices.count();
    vertices.append(a);
    normals.append(n);
    faces.append(v);
}

Patch::Patch(Geometry *g)
   : start(g->faces.count())
   , count(0)
   , initv(g->vertices.count())
   , sm(Patch::Smooth)
   , geom(g)
{
    qSetColor(faceColor, QColor(Qt::darkGray));
}

void Patch::rotate(qreal deg, QVector3D axis)
{
    mat.rotate(deg, axis);
}

void Patch::translate(const QVector3D &t)
{
    mat.translate(t);
}

void Patch::draw() const
{
    glPushMatrix();
    glMultMatrixf(mat.constData());
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, faceColor);

    const GLushort *indices = geom->faces.constData();
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, indices + start);
    glPopMatrix();
}

void Patch::addTri(const QVector3D &a, const QVector3D &b, const QVector3D &c, const QVector3D &n)
{
    QVector3D norm = n.isNull() ? QVector3D::normal(a, b, c) : n;

    if (sm == Smooth) {
        geom->appendSmooth(a, norm, initv);
        geom->appendSmooth(b, norm, initv);
        geom->appendSmooth(c, norm, initv);
    } else {
        geom->appendFaceted(a, norm);
        geom->appendFaceted(b, norm);
        geom->appendFaceted(c, norm);
    }

    count += 3;
}

void Patch::addQuad(const QVector3D &a, const QVector3D &b,  const QVector3D &c, const QVector3D &d)
{
    QVector3D norm = QVector3D::normal(a, b, c);

    if (sm == Smooth) {
        addTri(a, b, c, norm);
        addTri(a, c, d, norm);
    } else {
        // If faceted share the two common vertices
        addTri(a, b, c, norm);
        int k = geom->vertices.count();
        geom->appendSmooth(a, norm, k);
        geom->appendSmooth(c, norm, k);
        geom->appendFaceted(d, norm);
        count += 3;
    }
}

static inline QVector<QVector3D> extrude(const QVector<QVector3D> &vertices, qreal depth)
{
    QVector<QVector3D> extr = vertices;
    for (int v = 0; v < extr.count(); ++v)
        extr[v].setZ(extr[v].z() - depth);
    return extr;
}

class Rectoid
{
public:
    void translate(const QVector3D &t)
    {
        for (int i = 0; i < parts.count(); ++i)
            parts[i]->translate(t);
    }

    void rotate(qreal deg, QVector3D axis)
    {
        for (int i = 0; i < parts.count(); ++i)
            parts[i]->rotate(deg, axis);
    }

    // No special Rectoid destructor - the parts are fetched out of this member
    // variable, and destroyed by the new owner
    QList<Patch *> parts;
};

class RectPrism : public Rectoid
{
public:
    RectPrism(Geometry *g, qreal width, qreal height, qreal depth);
};

RectPrism::RectPrism(Geometry *g, qreal width, qreal height, qreal depth)
{
    enum { bl, br, tr, tl };
    Patch *fb = new Patch(g);
    fb->setSmoothing(Patch::Faceted);

    // front face
    QVector<QVector3D> r(4);
    r[br].setX(width);
    r[tr].setX(width);
    r[tr].setY(height);
    r[tl].setY(height);
    QVector3D adjToCenter(-width / 2.0, -height / 2.0, depth / 2.0);
    for (int i = 0; i < 4; ++i)
        r[i] += adjToCenter;
    fb->addQuad(r[bl], r[br], r[tr], r[tl]);

    // back face
    QVector<QVector3D> s = extrude(r, depth);
    fb->addQuad(s[tl], s[tr], s[br], s[bl]);

    // side faces
    Patch *sides = new Patch(g);
    sides->setSmoothing(Patch::Faceted);
    sides->addQuad(s[bl], s[br], r[br], r[bl]);
    sides->addQuad(s[br], s[tr], r[tr], r[br]);
    sides->addQuad(s[tr], s[tl], r[tl], r[tr]);
    sides->addQuad(s[tl], s[bl], r[bl], r[tl]);

    parts << fb << sides;
}

class RectTorus : public Rectoid
{
public:
    RectTorus(Geometry *g, qreal iRad, qreal oRad, qreal depth, int numSectors);
};

RectTorus::RectTorus(Geometry *g, qreal iRad, qreal oRad, qreal depth, int k)
{
    QVector<QVector3D> inside;
    QVector<QVector3D> outside;
    for (int i = 0; i < k; ++i) {
        qreal angle = (i * 2 * M_PI) / k;
        inside << QVector3D(iRad * qSin(angle), iRad * qCos(angle), depth / 2.0);
        outside << QVector3D(oRad * qSin(angle), oRad * qCos(angle), depth / 2.0);
    }
    inside << QVector3D(0.0, iRad, 0.0);
    outside << QVector3D(0.0, oRad, 0.0);
    QVector<QVector3D> in_back = extrude(inside, depth);
    QVector<QVector3D> out_back = extrude(outside, depth);

    // Create front, back and sides as separate patches so that smooth normals
    // are generated for the curving sides, but a faceted edge is created between
    // sides and front/back
    Patch *front = new Patch(g);
    for (int i = 0; i < k; ++i)
        front->addQuad(outside[i], inside[i],
                       inside[(i + 1) % k], outside[(i + 1) % k]);
    Patch *back = new Patch(g);
    for (int i = 0; i < k; ++i)
        back->addQuad(in_back[i], out_back[i],
                      out_back[(i + 1) % k], in_back[(i + 1) % k]);
    Patch *is = new Patch(g);
    for (int i = 0; i < k; ++i)
        is->addQuad(in_back[i], in_back[(i + 1) % k],
                    inside[(i + 1) % k], inside[i]);
    Patch *os = new Patch(g);
    for (int i = 0; i < k; ++i)
        os->addQuad(out_back[(i + 1) % k], out_back[i],
                    outside[i], outside[(i + 1) % k]);
    parts << front << back << is << os;
}

phonemodel::phonemodel(QObject *parent, int divisions, qreal scale)
    : QObject(parent)
    , geom(new Geometry())
{
    buildGeometry(divisions, scale);
}

phonemodel::~phonemodel()
{
    qDeleteAll(parts);
    delete geom;
}

void phonemodel::setColor(QColor c)
{
    for (int i = 0; i < parts.count(); ++i)
        qSetColor(parts[i]->faceColor, c);
}

void phonemodel::buildGeometry(int divisions, qreal scale)
{

    scale = scale * 0.5;

    qreal cw;
    qreal ch;
    qreal pw = phone_width*scale;
    qreal ph = phone_height*scale;
    qreal pd = phone_depth*scale;
    qreal pcw = phone_curve_width*scale;
    qreal pch = phone_curve_height*scale;

    float div = 20;
    ch = pch/div;

    RectPrism phone(geom, pw, ph, pd);

    parts << phone.parts;

    RectPrism *curves[40];

    for(int i = 0; i < div; i++)
    {
        cw = pw - pcw*2;
        cw = cw + pcw*sqrt(qCos((M_PI_2/div)*(float)(i)))*2;
        //cw = cw + (pcw/div) * (i+1);
        curves[i*2] = new RectPrism(geom, cw, ch, pd );
        curves[i*2]->translate(QVector3D(0.0, ph/2 + ch*(i) , 0.0));
        curves[i*2+1] = new RectPrism(geom, cw, ch, pd );
        curves[i*2+1]->translate(QVector3D(0.0, -ph/2 - ch*(i) , 0.0));
        parts << curves[i*2]->parts << curves[i*2+1]->parts;
    }

    geom->finalize();
}

void phonemodel::draw() const
{
    geom->loadArrays();

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    for (int i = 0; i < parts.count(); ++i)
        parts[i]->draw();

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
}
