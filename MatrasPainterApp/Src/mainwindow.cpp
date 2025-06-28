#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <QLabel>
#include <QScrollBar>
#include <QtMath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Mattress Painter");
    ui->scrollArea->setBackgroundRole(QPalette::Dark);

    // QLabel будет содержать изображение и размещается внутри QScrollArea.
    auto *imageLabel = new QLabel;
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setAlignment(Qt::AlignCenter);

    ui->scrollArea->setWidget(imageLabel);

    // Перемещаем обработчик в отдельный поток.
    worker = new PainterWorker;
    worker->moveToThread(&workerThread);

    setupConnections();

    workerThread.start();
}

MainWindow::~MainWindow()
{
    workerThread.quit();
    workerThread.wait();
    delete ui;
}

void MainWindow::setupConnections()
{
    // Соединения с рабочим потоком
    connect(this, &MainWindow::requestProcess, worker, &PainterWorker::process);
    connect(worker, &PainterWorker::finished, this, &MainWindow::onImageReady);
    connect(worker, &PainterWorker::error, this, &MainWindow::onProcessingError);
    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);

    // Соединения с элементами интерфейса
    connect(ui->applyButton, &QPushButton::clicked, this, &MainWindow::onApplyButtonClicked);
    connect(ui->actionOpenFile, &QAction::triggered, this, &MainWindow::onOpenFileActionTriggered);
    connect(ui->zoomInButton, &QPushButton::clicked, this, &MainWindow::onZoomIn);
    connect(ui->zoomOutButton, &QPushButton::clicked, this, &MainWindow::onZoomOut);
    connect(ui->zoomResetButton, &QPushButton::clicked, this, &MainWindow::onZoomReset);
}

void MainWindow::onOpenFileActionTriggered()
{
    const QString filePath = QFileDialog::getOpenFileName(this, "Открыть файл", "", "Двоичные файлы (*.bin);;Все файлы (*)");
    if (filePath.isEmpty()) {
        return;
    }

    m_currentFilePath = filePath;
    ui->filePathLabel->setText(QFileInfo(m_currentFilePath).fileName());

    // Автоматически извлекаем период из имени файла вида "data_123.bin"
    const QRegularExpression re("_(\\d+)\\.bin$");
    const QRegularExpressionMatch match = re.match(m_currentFilePath);
    if (match.hasMatch()) {
        bool ok;
        int period = match.captured(1).toInt(&ok);
        if (ok) {
            ui->periodSpinBox->setValue(period);
        }
    }
    startProcessing();
}

void MainWindow::onApplyButtonClicked()
{
    startProcessing();
}

void MainWindow::startProcessing()
{
    if (m_currentFilePath.isEmpty()) {
        QMessageBox::warning(this, "Внимание!", "Сначала выберите файл!");
        return;
    }

    ui->statusBar->showMessage("Обрабатывается, ждите...");

    if (auto* label = qobject_cast<QLabel*>(ui->scrollArea->widget())) {
        label->setText("Генерация изображения...");
        label->setPixmap(QPixmap()); // Очищаем предыдущее изображение
    }

    emit requestProcess(m_currentFilePath, ui->periodSpinBox->value());
}

void MainWindow::onImageReady(const QImage &image)
{
    m_originalImage = image;
    ui->statusBar->showMessage("Изображение успешно сгенерировано", 5000);
    onZoomReset(); // Используем сброс зума для начального отображения
}

void MainWindow::onProcessingError(const QString &error)
{
    QMessageBox::critical(this, "Ошибка обработки", error);
    ui->statusBar->showMessage("Ошибка.", 5000);

    if (auto* label = qobject_cast<QLabel*>(ui->scrollArea->widget())) {
        label->setText("Не удалось сгенерировать изображение. Проверьте файл или период.");
    }
}

void MainWindow::updateImageDisplay()
{
    if (m_originalImage.isNull()) return;

    auto* imageLabel = qobject_cast<QLabel*>(ui->scrollArea->widget());
    if (!imageLabel) return;

    const QImage scaledImage = m_originalImage.scaled(
        m_originalImage.size() * m_scaleFactor,
        Qt::KeepAspectRatio,
        Qt::FastTransformation // Лучший выбор для пиксельной графики
        );

    imageLabel->setPixmap(QPixmap::fromImage(scaledImage));
    // Это ключевой вызов, чтобы QScrollArea обновила полосы прокрутки.
    imageLabel->resize(scaledImage.size());

    ui->statusBar->showMessage(
        QString("Размер: %1x%2 | Масштаб: %3%")
            .arg(m_originalImage.width())
            .arg(m_originalImage.height())
            .arg(qRound(m_scaleFactor * 100))
        );
}

/**
 * @brief Применяет коэффициент масштабирования и пытается сохранить центр вида.
 * @param factor Коэффициент масштабирования (например, 1.25 для увеличения).
 *
 * Эта функция содержит критические исправления стабильности:
 * 1. Ограничивает коэффициент масштабирования для предотвращения экстремальных значений.
 * 2. Обрабатывает деление на ноль, если полосы прокрутки отсутствуют.
 * 3. Корректно центрирует масштабирование на центре видимой области.
 */
void MainWindow::scaleImage(double factor)
{
    const double newScaleFactor = qBound(MIN_SCALE_FACTOR, m_scaleFactor * factor, MAX_SCALE_FACTOR);
    if (qFuzzyCompare(m_scaleFactor, newScaleFactor)) {
        return; // Масштаб не изменился [то есть предел]
    }

    // Сохраняем центральную точку видимой области до масштабирования
    QPointF scrollbarPos = {
        (double)ui->scrollArea->horizontalScrollBar()->value(),
        (double)ui->scrollArea->verticalScrollBar()->value()
    };
    QPointF viewportCenter = scrollbarPos + ui->scrollArea->viewport()->rect().center();
    QPointF targetPoint = viewportCenter / m_scaleFactor;

    m_scaleFactor = newScaleFactor;
    updateImageDisplay(); // Смена диапозона виджеты и прокрутки тумблера справа и снизу

    // Перечитвание новой позицию тумблеров прокрутки, чтобы сохранить целевую точку в центре
    QPointF newCenter = targetPoint * m_scaleFactor;
    ui->scrollArea->horizontalScrollBar()->setValue(qRound(newCenter.x() - ui->scrollArea->viewport()->width() / 2.0));
    ui->scrollArea->verticalScrollBar()->setValue(qRound(newCenter.y() - ui->scrollArea->viewport()->height() / 2.0));
}

void MainWindow::onZoomIn()
{
    scaleImage(1.25);
}

void MainWindow::onZoomOut()
{
    scaleImage(0.8);
}

void MainWindow::onZoomReset()
{
    m_scaleFactor = 1.0;
    updateImageDisplay();
    // Центририрование изображения после сброса
    ui->scrollArea->horizontalScrollBar()->setValue(0);
    ui->scrollArea->verticalScrollBar()->setValue(0);
}
