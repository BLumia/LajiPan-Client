#-------------------------------------------------
#
# Project created by QtCreator 2017-08-08T16:43:46
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qlajipan
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    filedownloader.cpp \
    requestsender.cpp \
    responsereceiver.cpp \
    filelistwidget.cpp \
    lajiutils.cpp \
    icucmodel.cpp \
    uploadform.cpp \
    downloadprogressitem.cpp \
    downloadform.cpp \
    filetcpdownloader.cpp \
    filehttpdownloader.cpp \
    fileuploader.cpp \
    filetcpblockingdownloader.cpp \
    blockingdownloader.cpp

HEADERS += \
        mainwindow.h \
    filedownloader.h \
    requestsender.h \
    responsereceiver.h \
    filelistwidget.h \
    lajiutils.h \
    icucmodel.h \
    uploadform.h \
    downloadprogressitem.h \
    downloadform.h \
    filetcpdownloader.h \
    filehttpdownloader.h \
    fileuploader.h \
    filetcpblockingdownloader.h \
    blockingdownloader.h

FORMS += \
        mainwindow.ui \
    uploadform.ui \
    downloadprogressitem.ui \
    downloadform.ui
