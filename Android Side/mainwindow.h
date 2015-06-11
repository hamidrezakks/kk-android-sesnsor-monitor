#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSensors>
#include <QTimer>
#include <QUdpSocket>
#include <QtPositioning>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QTiltSensor *tiltSensor;
    QTiltReading *readingTilt;
    QGyroscope *gyroSensor;
    QGyroscopeReading *readingGyro;
    QGyroscopeFilter *gyroFilter;
    QLightSensor *lightSensor;
    QLightReading *readingLight;
    QRotationSensor *rotateSensor;
    QRotationReading *readingRotate;
    QTimer *sensorTimer;

    QGeoPositionInfoSource *source;

    bool stopRotate;
    bool isIpSet;
    QUdpSocket *socket;
    QHostAddress desIp;
    quint16 desPort;

private slots:
    void on_pushButton_clicked();
    void readSensors();
    void updateGPS(const QGeoPositionInfo &info);

    void on_pushButton_2_clicked();

    void on_pushButton_3_pressed();

    void on_pushButton_3_released();

    void on_horizontalSlider_valueChanged(int value);

    void on_spinBox_editingFinished();

    void on_spinBox_valueChanged(int arg1);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
