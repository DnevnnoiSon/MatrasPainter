#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Mattress Painter");

   // Перемещение битового отрисовщика в отдельный поток:
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
    connect(this, &MainWindow::requestProcess, worker, &PainterWorker::process);
    connect(worker, &PainterWorker::finished, this, &MainWindow::onImageReady);
    connect(worker, &PainterWorker::error, this, &MainWindow::onProcessingError);

    //  "Обновить" -> запуск обработки:
    connect(ui->applyButton, &QPushButton::clicked, this, &MainWindow::onApplyButtonClicked);

    // "Открыть файл" -> Выбор нужного файла:
    connect(ui->actionOpenFile, &QAction::triggered, this, &MainWindow::onOpenFileActionTriggered);

    // Удаление при завершении:
    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
}

void MainWindow::onOpenFileActionTriggered()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open Bitstream File", "", "Binary Files (*.bin);;All Files (*)");
    if (filePath.isEmpty()) {
        return;
    }

    m_currentFilePath = filePath;
    ui->filePathLabel->setText(QFileInfo(m_currentFilePath).fileName());

    // Автоматическое извлечение периода из имени файла
    QRegularExpression re("_(\\d+)\\.bin$");
    QRegularExpressionMatch match = re.match(m_currentFilePath);
    if (match.hasMatch()) {
        bool ok;
        int period = match.captured(1).toInt(&ok);
        if (ok) {
            ui->periodSpinBox->setValue(period);
        }
    }
    // Отрисовка файла с угаданным периодом:
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

    // Текущий статус внутрянки:
    ui->statusBar->showMessage("Обрабатывается, ждите...");
    ui->imageLabel->setText("Генерация изображения...");

    int period = ui->periodSpinBox->value();
    // Запуск обработки:
    emit requestProcess(m_currentFilePath, period);
}

//=============== Отображение получ. результата: =======================//
void MainWindow::onImageReady(const QImage &image)
{
    m_originalImage = image; // Сохраненип оригинала
    m_scaleFactor = 1.0;     // Сбрасываю масштаб к 100%
    updateScaledImage();     // Отображение картинки

    ui->statusBar->showMessage("Изображение успешно сгенерено", 5000);
}

void MainWindow::updateScaledImage()
{
    if (m_originalImage.isNull()) {
        return;
    }

    // Масштабирование оригинального изображения:
    QImage scaledImage = m_originalImage.scaled(m_originalImage.size() * m_scaleFactor,
    Qt::KeepAspectRatio,
    Qt::FastTransformation);

    ui->imageLabel->setPixmap(QPixmap::fromImage(scaledImage));
    // Приходится изменять размер QLabel -> чтобы QScrollArea поняла его новый размер
    ui->imageLabel->resize(scaledImage.size());
}

void MainWindow::onProcessingError(const QString &error)
{
    QMessageBox::critical(this, "Ошибка обработки:", error);
    ui->imageLabel->setText("Не получилось сгенерить файл. Чекай файл или период");
    ui->statusBar->showMessage("Ошибка.", 5000);
}
