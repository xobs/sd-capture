#-------------------------------------------------
#
# Project created by QtCreator 2013-02-06T15:51:19
#
#-------------------------------------------------

QT       += core gui network declarative

TARGET = sd-capture
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    networkconnection.cpp \
    controllerwindow.cpp \
    packet.cpp \
    netcommand.cpp \
    datainput.cpp \
    xbytearray.cpp \
    qhexedit.cpp \
    qhexedit_p.cpp \
    commands.cpp \
    byteswap.cpp

HEADERS  += mainwindow.h \
    networkconnection.h \
    controllerwindow.h \
    packet-struct.h \
    packet.h \
    netcommand.h \
    datainput.h \
    xbytearray.h \
    qhexedit.h \
    qhexedit_p.h \
    commands.h \
    byteswap.h

FORMS    += mainwindow.ui \
    controllerwindow.ui \
    datainput.ui
