QT       += core gui widgets

CONFIG   += c++17

TARGET = MatrasPainterApp
TEMPLATE = app

INCLUDEPATH += Inc

SOURCES += \
    Src/main.cpp \
    Src/mainwindow.cpp

HEADERS += \
    Inc/mainwindow.h

FORMS += \
    ui/mainwindow.ui

# Указываем путь к библиотеке и линкуемся с ней
win32: LIBS += -L$$OUT_PWD/../MatrasPainterLib/release -lMatrasPainterLib
unix: LIBS += -L$$OUT_PWD/../MatrasPainterLib -lMatrasPainterLib

INCLUDEPATH += $$PWD/../MatrasPainterLib
DEPENDPATH += $$PWD/../MatrasPainterLib
