#-------------------------------------------------
#
# Project created by QtCreator 2013-09-03T15:15:10
#
#-------------------------------------------------

DEFINES += QGIT_TEST_BUILD=1

include(../src/src.pro)

QT       += widgets testlib

TARGET = test_diff
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += \
    $$PWD/test_diff.cpp

cache()
