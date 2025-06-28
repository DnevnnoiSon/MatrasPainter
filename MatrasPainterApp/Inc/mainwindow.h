#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QThread>
#include <QScrollArea>
#include "painterworker.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @brief Класс MainWindow представляет собой главное окно приложения.
 *
 * Занимается обработкой взаимодействия с пользователем, управляет файловыми операциями,
 * отображает сгенерированное из битового потока изображение и предоставляет
 * элементы навигации, такие как масштабирование и панорамирование.
 * Обработка изображения вынесена в рабочий поток для сохранения отзывчивости интерфейса.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор класса MainWindow.
     * @param parent Родительский виджет, по умолчанию nullptr.
     */
    explicit MainWindow(QWidget *parent = nullptr);

    /**
     * @brief Деструктор класса MainWindow.
     */
    ~MainWindow();

signals:
    /**
     * @brief Сигнал для запроса обработки файла битового потока.
     * @param filePath Путь к входному файлу .bin.
     * @param period Значение периода для обработки.
     */
    void requestProcess(const QString &filePath, int period);

private slots:
    /// Обработчики действий и кнопок
    void onOpenFileActionTriggered();
    void onApplyButtonClicked();

    /// Слоты для обратных вызовов от рабочего потока
    void onImageReady(const QImage &image);
    void onProcessingError(const QString &error);

    /// Слоты для масштабирования и навигации
    void onZoomIn();
    void onZoomOut();
    void onZoomReset();

private:
    /**
     * @brief Устанавливются все соединения сигналов и слотов.
     */
    void setupConnections();

    /**
     * @brief Инициирует процесс обработки изображения.
     */
    void startProcessing();

    /**
     * @brief Обновляет отображаемое изображение на основе текущего масштаба.
     */
    void updateImageDisplay();

    /**
     * @brief Масштабирует изображение по заданному коэффициенту, сохраняя центр вида.
     * @param factor Коэффициент умножения для масштабирования (например, 1.25 для увеличения).
     */
    void scaleImage(double factor);

private:
    Ui::MainWindow *ui;
    QThread workerThread;
    PainterWorker *worker;

    QString m_currentFilePath;
    QImage m_originalImage;

    double m_scaleFactor = 1.0;  ///< Текущий уровень масштабирования, где 1.0 — это 100%.

    /// Минимально допустимый коэффициент масштабирования (например, 0.1 для 10%).
    static constexpr double MIN_SCALE_FACTOR = 0.1;
    /// Максимально допустимый коэффициент масштабирования (например, 10.0 для 1000%).
    static constexpr double MAX_SCALE_FACTOR = 10.0;
};

#endif // MAINWINDOW_H
