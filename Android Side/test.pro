#-------------------------------------------------
#
# Project created by QtCreator 2014-06-04T16:59:33
#
#-------------------------------------------------

QT       += core gui\
            sensors\
            gui\
            network\
            positioning


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = test
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

CONFIG += mobility
MOBILITY = 

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

#OTHER_FILES += \
 #   android/AndroidManifest.xml

