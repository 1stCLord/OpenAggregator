#-------------------------------------------------
#
# Project created by QtCreator 2013-06-13T23:51:23
#
#-------------------------------------------------

QT += core gui sql network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OpenAggregator
TEMPLATE = app


SOURCES += main.cpp\
        OpenAggregator.cpp

HEADERS  += OpenAggregator.h

FORMS    += OpenAggregator.ui

LIBS+=-Lc:/mysql/lib -llibmysql

