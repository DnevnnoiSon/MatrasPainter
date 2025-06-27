#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QThread>
#include "painterworker.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


/**
 * @class MainWindow
 * @brief Главное окно приложения для визуализации бинарных файлов.
 *
 * Управляет пользовательским интерфейсом, взаимодействует с рабочим потоком
 * для генерации изображений и отображает результат.
*/
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор класса MainWindow.
     * @param parent Родительский виджет, по умолчанию nullptr.
    */
    MainWindow(QWidget *parent = nullptr);

    /**
     * @brief Деструктор класса MainWindow.
     *
     * Корректно завершает рабочий поток.
    */
    ~MainWindow();

private slots:
    /**
     * @brief Слот, вызываемый при выборе файла через меню "Открыть".
     */
    void onOpenFileActionTriggered();
    /**
     * @brief Слот, вызываемый при нажатии на кнопку "Обновить".
     * Запускает процесс генерации изображения.
     */
    void onApplyButtonClicked();
    /**
     * @brief Слот для обработки готового изображения из рабочего потока.
     * @param image Сгенерированное изображение.
     */
    void onImageReady(const QImage &image);
    /**
     * @brief Слот для обработки сигнала об ошибке из рабочего потока.
     * @param error Текст ошибки.
     */
    void onProcessingError(const QString &error);
signals:
    /**
     * @brief Сигнал для запуска обработки в рабочем потоке.
     * @param filePath Путь к бинарному файлу.
     * @param period Период (ширина) изображения в битах.
     */
    void requestProcess(const QString &filePath, int period);

private:
    /**
     * @brief Настраивает все соединения сигналов и слотов.
     */
    void setupConnections();
    /**
     * @brief Инициирует запуск процесса генерации изображения.
     */
    void startProcessing();
    /**
     * @brief Обновляет отображаемое изображение с учетом текущего масштаба.
     */
    void updateScaledImage();

    Ui::MainWindow *ui;         ///< Указатель на объект UI, созданный в Qt Designer.
    QThread workerThread;       ///< Рабочий поток для выполнения генерации.
    PainterWorker *worker;      ///< Объект-исполнитель, который будет перемещен в workerThread.
    QString m_currentFilePath;  ///< Путь к последнему открытому файлу.

    /// Поля для масштабирования:
    QImage m_originalImage;     ///< Оригинальное, немасштабированное изображение.
    qreal m_scaleFactor = 1.0;  ///< Текущий фактор масштабирования.
};
#endif // MAINWINDOW_H

