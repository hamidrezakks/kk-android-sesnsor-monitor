#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QUrl>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QList>
#include <QPainter>

#include "glwidget.h"

struct gyroData {
    double x;
    double y;
    double z;
};

struct tiltData {
    double xRot;
    double yRot;
};

class GLWidget;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    bool receiveStart;
    GLWidget *glWidget;
    GLfloat rxdef, rydef, rzdef;
    //QPainter *painter;
    double defLong, defLati;
    double maxAmbient;

    QNetworkAccessManager manager;
    QNetworkReply *gmReply;

    void getGoogleStaticMap(double longitude, double latitude, int zoom);
    QUrl gmapURL;
    int gZoom;

    QPixmap *gPix;
    QPainter *gPainter;
    QList<gyroData> gyroLog;
    long gyroLogCount;

    QPixmap *tPix;
    QPainter *tPainter;
    QList<tiltData> tiltLog;
    long tiltLogCount;

    void resizeEvent(QResizeEvent * event );

private slots:
    void on_pushButton_clicked();
    void readyRead();

    void setGoogleMapImg(QNetworkReply* buffer);
    void setGoogleMapProgressBar(qint64 byteDl,qint64 totalByte);

    void setGyroLog(gyroData log);
    void drawGyroPlot(int max, int length);

    void setTiltLog(tiltData log);
    void drawTiltPlot(int max, int length);

    void on_horizontalSlider_sliderReleased();

private:
    Ui::MainWindow *ui;
    QUdpSocket *socket;
};

#endif // MAINWINDOW_H
