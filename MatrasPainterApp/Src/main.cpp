#include "mainwindow.h"

#include <QApplication>
/**
 * @file main.cpp
 * @brief Главный исполняемый файл проекта.
 * @author Артём
 * @date 2025-06-26
 */
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
