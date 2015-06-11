#-------------------------------------------------
#
# Project created by QtCreator 2014-06-05T12:04:46
#
#-------------------------------------------------

QT       += core gui\
            gui\
            network\
            opengl widgets\
            xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = getUdp
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
        glwidget.cpp \
    phonemodel.cpp

HEADERS  += mainwindow.h \
        glwidget.h \
    phonemodel.h

FORMS    += mainwindow.ui

#target.path = $$[QT_INSTALL_EXAMPLES]/opengl/hellogl
INSTALLS += target

contains(QT_CONFIG, opengles.) {
    contains(QT_CONFIG, angle): \
        warning("Qt was built with ANGLE, which provides only OpenGL ES 2.0 on top of DirectX 9.0c")
    error("This example requires Qt to be configured with -opengl desktop")
}
