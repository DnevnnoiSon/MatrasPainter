TEMPLATE = lib
TARGET = MatrasPainterLib
CONFIG += staticlib

QT += gui

SOURCES += \
    Src/painterworker.cpp

HEADERS += \
    Inc/painterworker.h

PUBLIC_HEADERS += Inc/painterworker.h
INCLUDEPATH += $$PWD/Inc
