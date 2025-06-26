#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

/**
* @brief Главное окно приложения.
*
* Управляет интерфейсом, взаимодействием с пользователем
* и запуском процесса отрисовки в фоновом потоке.
*/
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

// Будет запускать процесс отрисовки в отдельном потоке, чтобы не блокировать GUI.

// Будет отвечать за взаимодействие с библиотекой MattressPainterLib для получения изображения.

// Будет отображать результат и обрабатывать возможные ошибки.

// Для считывания и работы с файлами нужен отдельный модуль
private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
