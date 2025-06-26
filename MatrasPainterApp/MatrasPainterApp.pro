QT       += core gui widgets

CONFIG   += c++17

TARGET = MatrasPainterApp
TEMPLATE = app

SOURCES += \
    Src/main.cpp \
    Src/mainwindow.cpp

HEADERS += \
    Inc/mainwindow.h

FORMS += \
    ui/mainwindow.ui

#заголовочник либы:
INCLUDEPATH +=  $$PWD/Inc \
                $$PWD/../MatrasPainterLib/Inc
# Путь к либе для линковщика:
LIBS += -L$$OUT_PWD/../MatrasPainterLib -lMatrasPainterLib
