#-------------------------------------------------
#
# Project created by QtCreator 2011-09-15T23:31:31
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = demuxer
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp

QMAKE_CXXFLAGS += -fopenmp
LIBS += -lgomp
