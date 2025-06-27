#include "painterworker.h"
#include <QFile>
#include <QDataStream>
#include <QColor>
#include <QDebug>
#include <algorithm>
#include <QElapsedTimer>  // Для замера времени

PainterWorker::PainterWorker(QObject *parent) : QObject(parent) {}

void PainterWorker::process(const QString &filePath, int period)
{
    // Таймер для замера производительности
    QElapsedTimer timer;
    timer.start();
    qint64 initTime = 0, readTime = 0, lutTime = 0, processTime = 0;

    // Валидация параметров
    if (filePath.isEmpty()) {
        emit error("Путь к файлу не может быть пустым");
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
    const QByteArray fileBytes = file.readAll();
    file.close();
    readTime = timer.elapsed() - initTime;

    // Расчет параметров изображения
    const qint64 totalBits = fileBytes.size() * 8;
    if (totalBits == 0) {
        emit error("Файл пуст");
        return;
    }

    const int width = period;
    const int height = totalBits / period;
    if (width <= 0 || height <= 0) {
        emit error("Некорректные размеры изображения");
        return;
    }

    // Инициализация изображения
    QImage image(width, height, QImage::Format_RGB32);
    const QRgb colors[2] = {QColor(Qt::black).rgb(), QColor(Qt::green).rgb()};

    /// ОПТИМИЗАЦИЯ 1: Lookup-таблица ///
    struct BytePixels { QRgb pixels[8]; };
    std::array<BytePixels, 256> lut;
    for (uint32_t byte = 0; byte < 256; ++byte) {
        for (int bit = 0; bit < 8; ++bit) {
            lut[byte].pixels[bit] = colors[(byte >> (7 - bit)) & 1];
        }
    }
    lutTime = timer.elapsed() - readTime - initTime;

    // Параметры доступа к данным
    const uchar* data = reinterpret_cast<const uchar*>(fileBytes.constData());
    uchar* imgBits = image.bits();
    const int scanLineBytes = image.bytesPerLine();

    /// ОПТИМИЗАЦИЯ 2: Трехфазная обработка ///
    const qint64 startProcess = timer.elapsed();
    for (int y = 0; y < height; y++) {
        QRgb* scanline = reinterpret_cast<QRgb*>(imgBits + y * scanLineBytes);
        const qint64 startBit = static_cast<qint64>(y) * width;

        // Инициализация позиции
        const uchar* bytePtr = data + startBit/8;
        int bitOffset = 7 - (startBit % 8);
        int x = 0;

        // Фаза 1: Обработка невыровненных битов
        while (x < width && bitOffset < 7) {
            scanline[x++] = colors[(*bytePtr >> bitOffset) & 1];
            if (--bitOffset < 0) {
                bitOffset = 7;
                bytePtr++;
            }
        }

        // Фаза 2: Блочная обработка через LUT
        const int byteBlocks = (width - x) / 8;
        for (int i = 0; i < byteBlocks; ++i) {
            std::copy_n(lut[*bytePtr++].pixels, 8, scanline + x);
            x += 8;
        }

        // Фаза 3: Обработка остаточных битов
        while (x < width) {
            scanline[x++] = colors[(*bytePtr >> bitOffset) & 1];
            if (--bitOffset < 0) {
                bitOffset = 7;
                bytePtr++;
            }
        }
    }
    processTime = timer.elapsed() - startProcess;

    // Вывод результатов производительности
    qDebug() << "========================================";
    qDebug() << "Производительность генерации изображения";
    qDebug() << "========================================";
    qDebug() << "Размер файла:" << fileBytes.size() / 1024 << "KB";
    qDebug() << "Размер изображения:" << width << "x" << height << "пикселей";
    qDebug() << "Общее время:" << timer.elapsed() << "ms";
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
