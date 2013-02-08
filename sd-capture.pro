#-------------------------------------------------
#
# Project created by QtCreator 2013-02-06T15:51:19
#
#-------------------------------------------------

QT       += core gui network

TARGET = sd-capture
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    networkconnection.cpp \
    controllerwindow.cpp \
    packetstream.cpp \
    packet.cpp

HEADERS  += mainwindow.h \
    networkconnection.h \
    controllerwindow.h \
    packet-struct.h \
    packetstream.h \
    packet.h

FORMS    += mainwindow.ui \
    controllerwindow.ui
