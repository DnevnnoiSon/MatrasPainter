TEMPLATE = lib
TARGET = MatrasPainterLib
CONFIG += staticlib

SOURCES += \
    painter.cpp

HEADERS += \
    painter.h

# Отдаем приоритет библиотекам Qt
QT += core gui
