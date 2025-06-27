#include "painterworker.h"
#include <QFile>
#include <QDataStream>
#include <QColor>
#include <QDebug>

#include <algorithm>
#include <QElapsedTimer>

#include <QFileDevice>

PainterWorker::PainterWorker(QObject *parent) : QObject(parent) {}

void PainterWorker::process(const QString &filePath, int period)
{
    QElapsedTimer timer;
    timer.start();
    qint64 initTime = 0, readTime = 0, lutTime = 0, processTime = 0;

    if (filePath.isEmpty()) {
        emit error("Путь к файлу не может быть пустым");
        return;
    }
    if (period <= 0) {
        emit error("Период должен быть положительным числом");
        return;
    }
    initTime = timer.elapsed();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit error("Ошибка открытия файла: " + file.errorString());
        return;
    }

    const qint64 fileSize = file.size();
    if (fileSize == 0) {
        file.close();
        emit error("Файл пуст");
        return;
    }

    const uchar* data = file.map(0, fileSize);
    if (!data) {
        file.close();
        emit error("Не удалось отобразить файл в память: " + file.errorString());
        return;
    }
    readTime = timer.elapsed() - initTime;

    const qint64 totalBits = fileSize * 8;
    const int width = period;

    if (totalBits < period) {
        /// Добавлен const_cast
        file.unmap(const_cast<uchar*>(data));
        file.close();
        emit error("Данных в файле недостаточно для одной строки изображения");
        return;
    }
    const int height = totalBits / period;


    const qint64 max_pixels = 1'200'000'000;
    if (static_cast<qint64>(width) * height > max_pixels) {
        ///  Добавлен const_cast
        file.unmap(const_cast<uchar*>(data));
        file.close();
        QString errorMsg = QString("Изображение слишком велико (%1x%2). Выбери файл меньшего размера.")
                               .arg(width).arg(height);
        emit error(errorMsg);
        return;
    }

    if (width <= 0 || height <= 0) {
        /// Добавлен const_cast
        file.unmap(const_cast<uchar*>(data));
        file.close();
        emit error("Некорректные размеры изображения");
        return;
    }

    QImage image;
    try {
        image = QImage(width, height, QImage::Format_RGB32);
    } catch (const std::bad_alloc &) {
        /// Добавлен const_cast
        file.unmap(const_cast<uchar*>(data));
        file.close();
        emit error("Недостаточно памяти для создания изображения.");
        return;
    }

    if (image.isNull()) {
        file.unmap(const_cast<uchar*>(data));
        file.close();
        emit error("Не удалось создать QImage. Возможно, размеры слишком велики.");
        return;
    }

    const QRgb colors[2] = {QColor(Qt::black).rgb(), QColor(Qt::green).rgb()};

    struct BytePixels { QRgb pixels[8]; };
    std::array<BytePixels, 256> lut;
    for (uint32_t byte = 0; byte < 256; ++byte) {
        for (int bit = 0; bit < 8; ++bit) {
            lut[byte].pixels[bit] = colors[(byte >> (7 - bit)) & 1];
        }
    }
    lutTime = timer.elapsed() - readTime - initTime;

    uchar* imgBits = image.bits();
    const int scanLineBytes = image.bytesPerLine();

    const qint64 startProcess = timer.elapsed();
    for (int y = 0; y < height; y++) {
        QRgb* scanline = reinterpret_cast<QRgb*>(imgBits + y * scanLineBytes);
        const qint64 startBit = static_cast<qint64>(y) * width;
        const uchar* bytePtr = data + startBit/8;
        int bitOffset = 7 - (startBit % 8);
        int x = 0;

        while (x < width && bitOffset < 7) {
            scanline[x++] = colors[(*bytePtr >> bitOffset) & 1];
            if (--bitOffset < 0) {
                bitOffset = 7;
                bytePtr++;
            }
        }

        const int byteBlocks = (width - x) / 8;
        for (int i = 0; i < byteBlocks; ++i) {
            std::copy_n(lut[*bytePtr++].pixels, 8, scanline + x);
            x += 8;
        }

        while (x < width) {
            scanline[x++] = colors[(*bytePtr >> bitOffset) & 1];
            if (--bitOffset < 0) {
                bitOffset = 7;
                bytePtr++;
            }
        }
    }
    processTime = timer.elapsed() - startProcess;

    /// Добавлен const_cast
    file.unmap(const_cast<uchar*>(data));
    file.close();

    qDebug() << "========================================";
    qDebug() << "Производительность генерации изображения";
    qDebug() << "========================================";
    qDebug() << "Размер файла:" << fileSize / 1024 << "KB";
    qDebug() << "Размер изображения:" << width << "x" << height << "пикселей";

    qDebug() << "Общее время:" << timer.elapsed() << "ms";

    qDebug() << "----------------------------------------";
    qDebug() << "Инициализация:" << initTime << "ms";
    qDebug() << "Чтение файла (map):" << readTime << "ms";
    qDebug() << "Генерация LUT:" << lutTime << "ms";
    qDebug() << "Обработка пикселей:" << processTime << "ms";
    qDebug() << "----------------------------------------";
    qDebug() << "Скорость обработки:"
             << (static_cast<double>(width * height) / processTime) << "пикс/ms";
    qDebug() << "========================================";

    emit finished(image);
}
