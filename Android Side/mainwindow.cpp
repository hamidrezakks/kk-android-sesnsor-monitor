#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSensors>
#include <QTimer>
#include <QDesktopWidget>
#include <QRect>
#include <QtPositioning>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    isIpSet = false;
    stopRotate = false;
    //QScreen *display = new QScreen();
    //ui->textEdit->append(QString::number(display->availableSize().height()));
    QDesktopWidget awidget;
    QRect mainScreenSize = awidget.availableGeometry(awidget.primaryScreen());


    QRect textG,btn1G, btn2G, ipInG, portInG, btnCalG, sliderG, spinG;
    textG.setRect(0,0,mainScreenSize.width(),mainScreenSize.height()-555);
    ipInG.setRect(10,mainScreenSize.height()-435,mainScreenSize.width()/2 - 15,100);
    portInG.setRect(mainScreenSize.width()/2 + 5,mainScreenSize.height()-435,mainScreenSize.width()/2 - 15,100);
    btn1G.setRect(10,mainScreenSize.height()-325,mainScreenSize.width()/2 - 15,115);
    btn2G.setRect(mainScreenSize.width()/2 + 5,mainScreenSize.height()-325,mainScreenSize.width()/2 - 15,115);
    btnCalG.setRect(10,mainScreenSize.height()-200,mainScreenSize.width() - 20,115);
    sliderG.setRect(10,mainScreenSize.height()-545,mainScreenSize.width() -150,100);
    spinG.setRect(mainScreenSize.width()-130,mainScreenSize.height()-545,120,100);

    ui->textEdit->setGeometry(textG);
    ui->lineEdit->setGeometry(ipInG);
    ui->lineEdit_2->setGeometry(portInG);
    ui->pushButton->setGeometry(btn1G);
    ui->pushButton_2->setGeometry(btn2G);
    ui->pushButton_3->setGeometry(btnCalG);
    ui->horizontalSlider->setGeometry(sliderG);
    ui->spinBox->setGeometry(spinG);

    tiltSensor = new QTiltSensor(this);
    tiltSensor->start();

    gyroSensor = new QGyroscope(this);
    gyroSensor->start();

    lightSensor = new QLightSensor(this);
    lightSensor->start();

    rotateSensor = new QRotationSensor(this);
    rotateSensor->start();

    //gyroFilter = new QGyroscopeFilter();

    //intializing GPS
    source = QGeoPositionInfoSource::createDefaultSource(this);
    if (source) {
        connect(source, SIGNAL(positionUpdated(QGeoPositionInfo)),
             this, SLOT(updateGPS(QGeoPositionInfo)));
        source->startUpdates();
    }

    sensorTimer = new QTimer();
    connect(sensorTimer,SIGNAL(timeout()),this,SLOT(readSensors()));

    socket = new QUdpSocket(this);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete sensorTimer;
    delete tiltSensor;
    delete readingTilt;
    delete gyroSensor;
    delete readingGyro;
}

void MainWindow::on_pushButton_clicked()
{
   sensorTimer->start(ui->horizontalSlider->value());
   /*if(!isIpSet)
   {*/
       desIp.setAddress(ui->lineEdit->text());
       desPort = ui->lineEdit_2->text().toUInt();
       isIpSet = true;
   //}
   ui->textEdit->append("Started");
   ui->textEdit->append("Ip: "+desIp.toString()+" Port:"+QString::number(desPort));
}
void MainWindow::readSensors()
{
    QByteArray udpData;
    readingTilt = tiltSensor->reading();
    readingGyro = gyroSensor->reading();
    readingLight = lightSensor->reading();
    readingRotate = rotateSensor->reading();
    /*ui->textEdit->clear();
    ui->textEdit->append("\nxR:"+QString::number(readingTilt->xRotation())
                         +"\nyR:"+QString::number(readingTilt->yRotation()));

    udpData.append("\nxR:"+QString::number(readingTilt->xRotation())
                         +"\nyR:"+QString::number(readingTilt->yRotation()));

    ui->textEdit->append("\nxG:"+QString::number(readingGyro->x())
                         +"\n yG:"+QString::number(readingGyro->y())
                         +"\n zG:"+QString::number(readingGyro->z()));

    udpData.append("\nxG:"+QString::number(readingGyro->x())
                   +"\n yG:"+QString::number(readingGyro->y())
                   +"\n zG:"+QString::number(readingGyro->z()));

    ui->textEdit->append("\nlight:"+QString::number(readingLight->lux()));

    ui->textEdit->append("___________");
    */

    QString xmlData = "";

    QString rotateData = "";

    rotateData  += "<rotate><x>"+QString::number(readingRotate->x())+"</x>"
                +"<y>"+QString::number(readingRotate->y())+"</y>"
                +"<z>"+QString::number(readingRotate->z())+"</z></rotate>";

    xmlData += "<sensors>";
    if(!stopRotate)
        xmlData += rotateData;
    xmlData +="<light><lux>"+QString::number(readingLight->lux())+"</lux></light>"
            +"<tilt><x>"+QString::number(readingTilt->xRotation())+"</x>"
            +"<y>"+QString::number(readingTilt->yRotation())+"</y></tilt>"
            +"<gyro><x>"+QString::number(readingGyro->x())+"</x>"
            +"<y>"+QString::number(readingGyro->y())+"</y>"
            +"<z>"+QString::number(readingGyro->z())+"</z></gyro></sensors>";
    udpData.append(xmlData);

    socket->writeDatagram(udpData, desIp, desPort);
}

void MainWindow::updateGPS(const QGeoPositionInfo &info)
{
    if(isIpSet)
    {
        QByteArray udpData;
        /*ui->textEdit->clear();
        ui->textEdit->append("\nlong:"+QString::number(info.coordinate().longitude())
                             +"\nlati:"+QString::number(info.coordinate().latitude()));*/
        QString xmlData = "";
        xmlData += "<sensors><gps><long>"+QString::number(info.coordinate().longitude())+"</long>"
                +"<lati>"+QString::number(info.coordinate().latitude())+"</lati></gps></sensor>";

        udpData.append(xmlData);

        socket->writeDatagram(udpData, desIp, desPort);
    }

}

void MainWindow::on_pushButton_2_clicked()
{
    sensorTimer->stop();
    isIpSet = false;
    ui->textEdit->append("Stopped");
}

void MainWindow::on_pushButton_3_pressed()
{
    ui->pushButton_3->setText("Release, When You Rotated Your Phone!");

    QByteArray udpData;
    readingRotate = rotateSensor->reading();
    QString xmlData = "";
    xmlData += "<sensors><cals><x>"+QString::number(readingRotate->x())+"</x>"
            +"<y>"+QString::number(readingRotate->y())+"</y>"
            +"<z>"+QString::number(readingRotate->z())+"</z></cals></sensor>";

    udpData.append(xmlData);

    socket->writeDatagram(udpData, desIp, desPort);

    stopRotate = true;
}

void MainWindow::on_pushButton_3_released()
{
    QByteArray udpData;
    readingRotate = rotateSensor->reading();
    QString xmlData = "";
    xmlData += "<sensors><cale><x>"+QString::number(readingRotate->x())+"</x>"
            +"<y>"+QString::number(readingRotate->y())+"</y>"
            +"<z>"+QString::number(readingRotate->z())+"</z></cale></sensor>";

    udpData.append(xmlData);

    socket->writeDatagram(udpData, desIp, desPort);

    ui->pushButton_3->setText("Calibrated! Hold Again to Recalibrate.");

    stopRotate = false;

}

void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    sensorTimer->setInterval(value);
    ui->textEdit->append("Refresh rate: "+QString::number(value)+"msec");
    ui->spinBox->setValue(value);
}

void MainWindow::on_spinBox_editingFinished()
{
    //ui->horizontalSlider->setValue(ui->spinBox->value());
}

void MainWindow::on_spinBox_valueChanged(int arg1)
{
    ui->horizontalSlider->setValue(ui->spinBox->value());
}
