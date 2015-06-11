#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtWidgets>
#include <QtOpenGL>
#include <QDomDocument>
#include <QPainter>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include "glwidget.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //connect(parent,SIGNAL())
    rxdef = rydef = rzdef = 0;
    defLong = defLati = 0;
    maxAmbient = 20;
    gZoom = 15;
    gyroLogCount = 0;
    tiltLogCount = 0;
    receiveStart = false;

    glWidget = new GLWidget(this);

    glWidget->setGeometry(5,5,400,400);
    glWidget->setToolTip("openGL");

    //connect(glWidget,SIGNAL())
    glWidget->setLightAmbient(0.5);
    glWidget->updateWGL();

    ui->label->setGeometry(5,410,400,195);
    ui->label_2->setGeometry(410,410,400,195);
    ui->label_3->setGeometry(410,5,400,400);


    ui->label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->label_2->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->label_3->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    connect(&manager,SIGNAL(finished(QNetworkReply*)),this,SLOT(setGoogleMapImg(QNetworkReply*)));
    //connect(gmReply,SIGNAL(downloadProgress(qint64,qint64)),this,SLOT(setGoogleMapProgressBar(qint64,qint64)));

    ui->progressBar->hide();
    //ui->progressBar->show();

    QPixmap pix(ui->label->width(),ui->label->height());
    pix.fill(QColor(19, 79, 90));
    QPainter painter(&pix);

    painter.setPen(QColor(255,255,255,64));
    for(int i=0; i < ui->label->width(); i += 20)
    {
        painter.drawLine(QPoint(i,0),QPoint(i,ui->label->height()));
    }
    for(int i=-19+ui->label->height()%20; i < ui->label->height(); i += 20)
    {
        painter.drawLine(QPoint(0,i),QPoint(ui->label->width(),i));
    }
    ui->label->setPixmap(pix);
    ui->label_2->setPixmap(pix);

    QPixmap mPix(ui->label_3->width(),ui->label_3->height());
    mPix.fill(QColor(185, 185, 185));
    QPainter mPainter(&mPix);
    mPainter.setFont(QFont("Arial"));
    mPainter.setPen(QColor(50,50,50));
    mPainter.drawText(QRect((ui->label_3->width()-200)/2,(ui->label_3->height()-100)/2,200,100),"Google Maps",QTextOption(Qt::AlignCenter));
    ui->label_3->setPixmap(mPix);

    gPix = new QPixmap(400,200);
    gPainter = new QPainter(gPix);

    tPix = new QPixmap(400,200);
    tPainter = new QPainter(tPix);

    setWindowTitle("Sensor Monitoring By KK");
}

MainWindow::~MainWindow()
{
    delete ui;
    delete glWidget;
}

void MainWindow::resizeEvent (QResizeEvent * event )
{
    int height = event->size().height(), width = event->size().width();
    QRect glGeo, mapGeo, mapSliderGeo, mapPBGeo, plot1Geo, plot2Geo, btnGeo, portTextGeo;
    int controlHeight = 45;
    glGeo.setRect(5,5,(width-15)/2,(height-controlHeight)*2/3);
    mapGeo.setRect((width-15)/2+10,5,(width-15)/2,(height-controlHeight)*2/3);
    mapSliderGeo.setRect((width-15)/2+15,5,(width-15)/2-10,20);
    mapPBGeo.setRect((width-15)/2+15,(height-controlHeight)*2/3-20,(width-15)/2-10,20);
    plot1Geo.setRect(5,(height-controlHeight)*2/3+10,(width-15)/2,(height-controlHeight)/3);
    plot2Geo.setRect((width-15)/2+10,(height-controlHeight)*2/3+10,(width-15)/2,(height-controlHeight)/3);
    btnGeo.setRect(5,height-(controlHeight-12),200,30);
    portTextGeo.setRect(210,height-(controlHeight-13),100,27);
    glWidget->setGeometry(glGeo);
    ui->label_3->setGeometry(mapGeo);
    ui->horizontalSlider->setGeometry(mapSliderGeo);
    ui->progressBar->setGeometry(mapPBGeo);
    ui->label->setGeometry(plot1Geo);
    ui->label_2->setGeometry(plot2Geo);
    ui->pushButton->setGeometry(btnGeo);
    ui->lineEdit->setGeometry(portTextGeo);

    if(!receiveStart)
    {
        QPixmap pix(ui->label->width(),ui->label->height());
        pix.fill(QColor(19, 79, 90));
        QPainter painter(&pix);

        painter.setPen(QColor(255,255,255,64));
        for(int i=0; i < ui->label->width(); i += 20)
        {
            painter.drawLine(QPoint(i,0),QPoint(i,ui->label->height()));
        }
        for(int i=-19+ui->label->height()%20; i < ui->label->height(); i += 20)
        {
            painter.drawLine(QPoint(0,i),QPoint(ui->label->width(),i));
        }
        ui->label->setPixmap(pix);
        ui->label_2->setPixmap(pix);

        QPixmap mPix(ui->label_3->width(),ui->label_3->height());
        mPix.fill(QColor(185, 185, 185));
        QPainter mPainter(&mPix);
        mPainter.setFont(QFont("Arial"));
        mPainter.setPen(QColor(50,50,50));
        mPainter.drawText(QRect((ui->label_3->width()-200)/2,(ui->label_3->height()-100)/2,200,100),"Google Maps",QTextOption(Qt::AlignCenter));
        ui->label_3->setPixmap(mPix);
    }
    else
        getGoogleStaticMap(defLong,defLati,gZoom);
}

void MainWindow::on_pushButton_clicked()
{
    socket = new QUdpSocket(this);
    // The most common way to use QUdpSocket class is
    // to bind to an address and port using bind()
    // bool QAbstractSocket::bind(const QHostAddress & address,
    // quint16 port = 12345, BindMode mode = DefaultForPlatform)

    socket->bind(ui->lineEdit->text().toUInt(),QUdpSocket::ShareAddress);
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    receiveStart = true;
}

void MainWindow::readyRead()
{
    // when data comes in
    QByteArray buffer;
    buffer.resize(socket->pendingDatagramSize());
    QHostAddress sender;
    quint16 senderPort;

    socket->readDatagram(buffer.data(), buffer.size(),&sender, &senderPort);
    QDomDocument doc("streamdoc");

    doc.setContent(buffer);

    // print out the element names of all elements that are direct children
    // of the outermost element.
    QDomElement docElem = doc.documentElement();

    QDomNode n = docElem.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if(!e.isNull()) {
            if(e.tagName() == "rotate")
            {
                glWidget->setXRotation((-90+e.elementsByTagName("x").at(0).firstChild().nodeValue().toFloat()+rxdef)*16);
                glWidget->setYRotation((e.elementsByTagName("y").at(0).firstChild().nodeValue().toFloat()+rydef)*16);
                glWidget->setZRotation((e.elementsByTagName("z").at(0).firstChild().nodeValue().toFloat()+rzdef)*16);
                glWidget->updateWGL();
            }
            if(e.tagName() == "light")
            {
                glWidget->setLightAmbient((e.elementsByTagName("lux").at(0).firstChild().nodeValue().toFloat()/maxAmbient)*4 + 1);
                if(e.elementsByTagName("lux").at(0).firstChild().nodeValue().toFloat() > maxAmbient)
                    maxAmbient = e.elementsByTagName("lux").at(0).firstChild().nodeValue().toFloat();
                //qDebug() << e.elementsByTagName("lux").at(0).firstChild().nodeValue();
                glWidget->updateWGL();
            }
            if(e.tagName() == "gps")
            {
                getGoogleStaticMap(e.elementsByTagName("long").at(0).firstChild().nodeValue().toDouble(),
                                   e.elementsByTagName("lati").at(0).firstChild().nodeValue().toDouble(),
                                   gZoom);
                defLati = e.elementsByTagName("lati").at(0).firstChild().nodeValue().toDouble();
                defLong = e.elementsByTagName("long").at(0).firstChild().nodeValue().toDouble();
            }
            if(e.tagName() == "cals")
            {
                qDebug() << e.elementsByTagName("x").at(0).firstChild().nodeValue();
                rxdef += e.elementsByTagName("x").at(0).firstChild().nodeValue().toDouble();
                qDebug() << e.elementsByTagName("y").at(0).firstChild().nodeValue();
                rydef += e.elementsByTagName("y").at(0).firstChild().nodeValue().toDouble();
                qDebug() << e.elementsByTagName("z").at(0).firstChild().nodeValue();
                rzdef += e.elementsByTagName("z").at(0).firstChild().nodeValue().toDouble();
            }
            if(e.tagName() == "cale")
            {
                qDebug() << e.elementsByTagName("x").at(0).firstChild().nodeValue();
                rxdef -= e.elementsByTagName("x").at(0).firstChild().nodeValue().toDouble();
                qDebug() << e.elementsByTagName("y").at(0).firstChild().nodeValue();
                rydef -= e.elementsByTagName("y").at(0).firstChild().nodeValue().toDouble();
                qDebug() << e.elementsByTagName("z").at(0).firstChild().nodeValue();
                rzdef -= e.elementsByTagName("z").at(0).firstChild().nodeValue().toDouble();
            }
            
            // Plot Data
            
            if(e.tagName() == "gyro")
            {
                gyroData temp;
                temp.x = e.elementsByTagName("x").at(0).firstChild().nodeValue().toDouble();
                temp.y = e.elementsByTagName("y").at(0).firstChild().nodeValue().toDouble();
                temp.z = e.elementsByTagName("z").at(0).firstChild().nodeValue().toDouble();
                setGyroLog(temp);
            }

            if(e.tagName() == "tilt")
            {
                tiltData temp;
                temp.xRot = e.elementsByTagName("x").at(0).firstChild().nodeValue().toDouble();
                temp.yRot = e.elementsByTagName("y").at(0).firstChild().nodeValue().toDouble();
                setTiltLog(temp);
            }
        }
        n = n.nextSibling();
    }
}

void MainWindow::getGoogleStaticMap(double longitude, double latitude, int zoom)
{
    QString gurl = QString("http://maps.googleapis.com/maps/api/staticmap?center=%1,%2&zoom=%3&size=%4x%5&sensor=false")
            .arg(QString::number(latitude), QString::number(longitude), QString::number(zoom),
                 QString::number(ui->label_3->width()), QString::number(ui->label_3->height()));
    qDebug()<<gurl;
    //gmapURL.setUrl(gurl);
    QUrl myUrl(gurl);
    QNetworkRequest request(myUrl);
    //QNetworkAccessManager manager;
    gmReply = manager.get(request);
    connect(gmReply,SIGNAL(downloadProgress(qint64,qint64)),this,SLOT(setGoogleMapProgressBar(qint64,qint64)));
    ui->progressBar->show();
    //connect(&manager,SIGNAL(finished(QNetworkReply*)),this,SLOT(setGoogleMapImg(QNetworkReply*)));
}

void MainWindow::setGoogleMapProgressBar(qint64 byteDl, qint64 totalByte)
{
    ui->progressBar->setMaximum(totalByte);
    ui->progressBar->setValue(byteDl);
}

void MainWindow::setGoogleMapImg(QNetworkReply *buffer)
{
    QByteArray data = buffer->readAll();
    buffer->deleteLater();
    QPixmap pixmap(ui->label_3->width(),ui->label_3->height());
    pixmap.loadFromData(data);
    ui->label_3->setPixmap(pixmap.scaled(ui->label_3->width(),ui->label_3->height(),Qt::IgnoreAspectRatio));
    ui->progressBar->hide();
    ui->progressBar->setValue(0);
}

void MainWindow::on_horizontalSlider_sliderReleased()
{
    gZoom = ui->horizontalSlider->value();
    getGoogleStaticMap(defLong, defLati, gZoom);
}

void MainWindow::setGyroLog(gyroData log)
{
    gyroLog.prepend(log);
    if(gyroLog.count() > 5000)
        gyroLog.removeLast();
    drawGyroPlot(400, gyroLog.count() );

    gyroLogCount++;
}

void MainWindow::drawGyroPlot(int max, int length)
{
    int scale = 768;
    int height = 200, width = 400;
    int start = 0;
    if(length > max)
        start = max - 1;
    else
        start = length - 1;
    //QPixmap pix(width,height);
    int xZero = height/2;
    gPix->fill(QColor(19, 79, 90));
    //QPainter painter(gPix);

    gPainter->setPen(QColor(255,255,255,64));
    if(max < length)
    {
        for(int i=(max - (gyroLogCount-1))%20-20; i < width; i += 20)
        {
            gPainter->drawLine(QPoint(i,0),QPoint(i,height));
        }
    }
    else
    {
        for(int i=0; i < width; i += 20)
        {
            gPainter->drawLine(QPoint(i,0),QPoint(i,height));
        }
    }
    for(int i=-19+height%20; i < height; i += 20)
    {
        gPainter->drawLine(QPoint(0,i),QPoint(width,i));
    }

    QPolygon polyX1,polyX2,polyY1,polyY2,polyZ1,polyZ2;

    for(int i = start; i >= 0; i--)
    {
        polyX1<<QPoint(start - i+1,(-gyroLog.at(i).x * xZero)/scale + xZero);
        polyX2<<QPoint(start - i+1,(-gyroLog.at(i).x * xZero)/scale + xZero);

        polyY1<<QPoint(start - i+1,(-gyroLog.at(i).y * xZero)/scale + xZero);
        polyY2<<QPoint(start - i+1,(-gyroLog.at(i).y * xZero)/scale + xZero);

        polyZ1<<QPoint(start - i+1,(-gyroLog.at(i).z * xZero)/scale + xZero);
        polyZ2<<QPoint(start - i+1,(-gyroLog.at(i).z * xZero)/scale + xZero);

    }
    QPainterPath path;
    path.addPolygon(polyX1);
    QPen penX1(QBrush(QColor(255,0,0,128),Qt::SolidPattern), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    gPainter->setPen(penX1);
    gPainter->drawPath(path);

    path = QPainterPath();
    path.addPolygon(polyX2);
    QPen penX2(QBrush(QColor(255,0,0),Qt::SolidPattern), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    gPainter->setPen(penX2);
    gPainter->drawPath(path);

    path = QPainterPath();
    path.addPolygon(polyY1);
    QPen penY1(QBrush(QColor(255,255,0,128),Qt::SolidPattern), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    gPainter->setPen(penY1);
    gPainter->drawPath(path);

    path = QPainterPath();
    path.addPolygon(polyY2);
    QPen penY2(QBrush(QColor(255,255,0),Qt::SolidPattern), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    gPainter->setPen(penY2);
    gPainter->drawPath(path);

    path = QPainterPath();
    path.addPolygon(polyZ1);
    QPen penZ1(QBrush(QColor(0,255,0,128),Qt::SolidPattern), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    gPainter->setPen(penZ1);
    gPainter->drawPath(path);

    path = QPainterPath();
    path.addPolygon(polyZ2);
    QPen penZ2(QBrush(QColor(0,255,0),Qt::SolidPattern), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    gPainter->setPen(penZ2);
    gPainter->drawPath(path);

    ui->label->setPixmap(gPix->scaled(QSize(ui->label->width(),ui->label->height()),Qt::IgnoreAspectRatio));
}

void MainWindow::setTiltLog(tiltData log)
{
    tiltLog.prepend(log);
    if(gyroLog.count() > 5000)
        tiltLog.removeLast();
    drawTiltPlot(400, gyroLog.count() );

    tiltLogCount++;
}

void MainWindow::drawTiltPlot(int max, int length)
{
    int scale = 128;
    int height = 200, width = 400;
    int start = 0;
    if(length > max)
        start = max - 1;
    else
        start = length - 1;
    //QPixmap pix(width,height);
    int xZero = height/2;
    tPix->fill(QColor(19, 79, 90));
    //QPainter painter(&pix);

    tPainter->setPen(QColor(255,255,255,64));
    if(max < length)
    {
        for(int i=(max - (tiltLogCount - 1))%20-20; i < width; i += 20)
        {
            tPainter->drawLine(QPoint(i,0),QPoint(i,height));
        }
    }
    else
    {
        for(int i=0; i < width; i += 20)
        {
            tPainter->drawLine(QPoint(i,0),QPoint(i,height));
        }
    }
    for(int i=-19+height%20; i < height; i += 20)
    {
        tPainter->drawLine(QPoint(0,i),QPoint(width,i));
    }

    QPolygon polyX1,polyX2,polyY1,polyY2;

    for(int i = start; i >= 0; i--)
    {
        polyX1<<QPoint(start - i+1,(-tiltLog.at(i).xRot * xZero)/scale + xZero);
        polyX2<<QPoint(start - i+1,(-tiltLog.at(i).xRot * xZero)/scale + xZero);

        polyY1<<QPoint(start - i+1,(-tiltLog.at(i).yRot * xZero)/scale + xZero);
        polyY2<<QPoint(start - i+1,(-tiltLog.at(i).yRot * xZero)/scale + xZero);
    }
    QPainterPath path;
    path.addPolygon(polyX1);
    QPen penX1(QBrush(QColor(255,0,0,128),Qt::SolidPattern), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    tPainter->setPen(penX1);
    tPainter->drawPath(path);

    path = QPainterPath();
    path.addPolygon(polyX2);
    QPen penX2(QBrush(QColor(255,0,0),Qt::SolidPattern), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    tPainter->setPen(penX2);
    tPainter->drawPath(path);

    path = QPainterPath();
    path.addPolygon(polyY1);
    QPen penY1(QBrush(QColor(255,255,0,128),Qt::SolidPattern), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    tPainter->setPen(penY1);
    tPainter->drawPath(path);

    path = QPainterPath();
    path.addPolygon(polyY2);
    QPen penY2(QBrush(QColor(255,255,0),Qt::SolidPattern), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    tPainter->setPen(penY2);
    tPainter->drawPath(path);

    ui->label_2->setPixmap(tPix->scaled(QSize(ui->label_2->width(),ui->label_2->height()),Qt::IgnoreAspectRatio));
}
