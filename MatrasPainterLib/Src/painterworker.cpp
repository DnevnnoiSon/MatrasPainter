#include "painterworker.h"
#include <QFile>
#include <QDataStream>
#include <QColor>
#include <QDebug>
#include <QElapsedTimer>

PainterWorker::PainterWorker(QObject *parent) : QObject(parent) {}

void PainterWorker::process(const QString &filePath, int period)
{
    QElapsedTimer timer;
    timer.start();
    qint64 initTime = 0, readTime = 0, lutTime = 0, processTime = 0;

    // Валидация параметров
    if (filePath.isEmpty()) {
        emit error("Файл пуст. Требуется выбрать");
        return;
    }
    if (period <= 0) {
        emit error("Период должен быть положительным числом");
        return;
    }
    initTime = timer.elapsed();

    // Загрузка файла
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit error("Ошибка открытия файла: " + file.errorString());
        return;
    }
    QByteArray fileBytes = file.readAll();
    file.close();
    readTime = timer.elapsed() - initTime;

    const qint64 totalBits = fileBytes.size() * 8;
    if (totalBits == 0) {
        emit error("Файл пуст");
        return;
    }

    // Расчет размеров изображения
    const int width = period;
    const int height = static_cast<int>(totalBits / period);
    if (width <= 0 || height <= 0) {
        emit error("Ошибка размера изображения. Требуется проверка периода.");
        return;
    }

    // Инициализация изображения
    QImage image(width, height, QImage::Format_RGB32);
    const QRgb colorOne = QColor(Qt::green).rgb();
    const QRgb colorZero = QColor(Qt::black).rgb();

    // LUT не используется в этой версии, но для совместимости вывода
    lutTime = 0;

    const uchar *data = reinterpret_cast<const uchar*>(fileBytes.constData());

    // Замер времени обработки пикселей
    const qint64 startProcess = timer.elapsed();

    // Основной цикл обработки
    for (int y = 0; y < height; ++y) {
        QRgb *scanLine = reinterpret_cast<QRgb*>(image.scanLine(y));

        for (int x = 0; x < width; ++x) {
            qint64 bitPosition = (qint64)y * width + x;
            qint64 byteIndex = bitPosition / 8;
            int bitInByte = 7 - (bitPosition % 8);
            bool isBitSet = (data[byteIndex] >> bitInByte) & 1;
            scanLine[x] = isBitSet ? colorOne : colorZero;
        }
    }

    processTime = timer.elapsed() - startProcess;
    const qint64 totalTime = timer.elapsed();

    // Вывод результатов в требуемом формате
    qDebug() << "========================================";
    qDebug() << "Производительность генерации изображения";
    qDebug() << "========================================";
    qDebug() << "Размер файла:" << fileBytes.size() / 1024 << "KB";
    qDebug() << "Размер изображения:" << width << "x" << height << "пикселей";
    qDebug() << "Общее время:" << totalTime << "ms";
    qDebug() << "----------------------------------------";
    qDebug() << "Инициализация:" << initTime << "ms";
    qDebug() << "Чтение файла:" << readTime << "ms";
    qDebug() << "Генерация LUT:" << lutTime << "ms";
    qDebug() << "Обработка пикселей:" << processTime << "ms";
    qDebug() << "----------------------------------------";
    qDebug() << "Скорость обработки:"
             << (static_cast<double>(width * height) / processTime) << "пикс/ms";
    qDebug() << "========================================";

    emit finished(image);
}
