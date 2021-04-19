#-------------------------------------------------
#
# Project created by QtCreator 2021-04-19T22:20:24
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = QTGui_Assignment2
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h

FORMS    += mainwindow.ui
LIBS     += -lpaho-mqtt3c
LIBS     += -ljson-c
