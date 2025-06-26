#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QThread>
#include "painterworker.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Слоты для обработки действий пользователя
    void onOpenFileActionTriggered();
    void onApplyButtonClicked();

    // Слоты для обработки сигналов от worker'а
    void onImageReady(const QImage &image);
    void onProcessingError(const QString &error);

signals:
    // Сигнал для запуска обработки в рабочем потоке
    void requestProcess(const QString &filePath, int period);

private:
    void setupConnections();
    void startProcessing();
    void updateScaledImage();

    Ui::MainWindow *ui;
    QThread workerThread;
    PainterWorker *worker;
    QString m_currentFilePath;

    // Поля для масштабирования:
    QImage m_originalImage;
    qreal m_scaleFactor = 1.0;
};
#endif // MAINWINDOW_H
