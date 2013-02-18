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
    packet.cpp \
    netcommand.cpp \
    datainput.cpp \
    xbytearray.cpp \
    qhexedit.cpp \
    qhexedit_p.cpp \
    commands.cpp

HEADERS  += mainwindow.h \
    networkconnection.h \
    controllerwindow.h \
    packet-struct.h \
    packetstream.h \
    packet.h \
    netcommand.h \
    datainput.h \
    xbytearray.h \
    qhexedit.h \
    qhexedit_p.h \
    commands.h

FORMS    += mainwindow.ui \
    controllerwindow.ui \
    datainput.ui
