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

# 1. Путь к заголовочным файлам библиотеки
INCLUDEPATH +=  $$PWD/Inc \
                $$PWD/../MatrasPainterLib/Inc

# 2. Путь к библиотеке для линковщика (улучшенная версия)


# $$OUT_PWD - это папка сборки для MatrasPainterApp.
LIB_PATH = $$OUT_PWD/../MatrasPainterLib

win32 {
    # Конфигурация для Windows

    CONFIG(debug, debug|release) {
        # Debug: ищем библиотеку с суффиксом 'd' в папке 'debug'
        LIBS += -L$$LIB_PATH/debug -lMatrasPainterLib
    } else {
        # Release: ищем стандартную библиотеку в папке 'release'
        LIBS += -L$$LIB_PATH/release -lMatrasPainterLib
    }
} else:unix {
    # Конфигурация для Linux и других Unix-подобных систем
    LIBS += -L$$LIB_PATH -lMatrasPainterLib
}
