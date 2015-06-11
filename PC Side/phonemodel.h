#ifndef PHONEMODEL_H
#define PHONEMODEL_H

#include <QObject>
#include <QColor>

class Patch;
struct Geometry;

class phonemodel : public QObject
{
public:
    explicit phonemodel(QObject *parent, int d = 64, qreal s = 1.0);
    ~phonemodel();
    void setColor(QColor c);
    void draw() const;
private:
    void buildGeometry(int d, qreal s);

    QList<Patch *> parts;
    Geometry *geom;
};

#endif // PHONEMODEL_H
