#include "painterworker.h"
#include <QFile>
#include <QDataStream>
#include <QColor>
#include <QDebug>
#include <QElapsedTimer>
#include <QFileDevice>
#include <QImage>

PainterWorker::PainterWorker(QObject *parent) : QObject(parent) {}

void PainterWorker::process(const QString &filePath, int period)
{
    QElapsedTimer totalTimer; // Общий таймер для всей функции
    totalTimer.start();

    if (filePath.isEmpty()) {
        emit error("Путь к файлу не может быть пустым");
        return;
    }
    if (period <= 0) {
        emit error("Период должен быть положительным числом");
        return;
    }

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
    qDebug() << "Ход работы [1]: Файл отображен в память";

    const qint64 totalBits = fileSize * 8;
    const int width = period;

    if (totalBits < period) {
        file.unmap(const_cast<uchar*>(data));
        file.close();
        emit error("Данных в файле недостаточно для одной строки изображения");
        return;
    }
    const int height = totalBits / period;

    const qint64 max_pixels = 1'200'000'000;
    if (static_cast<qint64>(width) * height > max_pixels) {
        file.unmap(const_cast<uchar*>(data));
        file.close();
        QString errorMsg = QString("Изображение слишком велико (%1x%2). Выберите файл меньшего размера.")
                               .arg(width).arg(height);
        emit error(errorMsg);
        return;
    }

    if (width <= 0 || height <= 0) {
        file.unmap(const_cast<uchar*>(data));
        file.close();
        emit error("Некорректные размеры изображения");
        return;
    }
    qDebug() << "Ход работы [2]: Размеры изображения рассчитаны:" << width << "x" << height;

    QImage image;
    try {
        image = QImage(width, height, QImage::Format_RGB32);
    } catch (const std::bad_alloc &) {
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
    qDebug() << "Ход работы [3]: QImage успешно создан";

    const QRgb colors[2] = {QColor(Qt::black).rgb(), QColor(Qt::green).rgb()};

    struct BytePixels { QRgb pixels[8]; };
    std::array<BytePixels, 256> lut;
    for (uint32_t byte = 0; byte < 256; ++byte) {
        for (int bit = 0; bit < 8; ++bit) {
            lut[byte].pixels[bit] = colors[(byte >> (7 - bit)) & 1];
        }
    }
    qDebug() << "Ход работы [4]: Создана таблица поиска (LUT)";

    //=== Начало замера времени отрисовки ====//
    QElapsedTimer drawTimer;
    drawTimer.start();

    uchar* imgBits = image.bits();
    const int scanLineBytes = image.bytesPerLine();

    for (int y = 0; y < height; y++) {
        // Приведение 'y' к qint64 для предотвращения переполнения при умножении:
        QRgb* scanline = reinterpret_cast<QRgb*>(imgBits + static_cast<qint64>(y) * scanLineBytes);

        const qint64 startBit = static_cast<qint64>(y) * width;
        const uchar* bytePtr = data + startBit/8;
        int bitOffset = 7 - (startBit % 8);
        int x = 0;

        // Оптимизированная обработка пикселей:
        while (x < width && bitOffset < 7) {
            scanline[x++] = colors[(*bytePtr >> bitOffset) & 1];
            if (--bitOffset < 0) {
                bitOffset = 7;
                bytePtr++;
            }
        }

        const int byteBlocks = (width - x) / 8;
        for (int i = 0; i < byteBlocks; ++i) {
            if (bytePtr >= data + fileSize) {
                emit error(QString("Критическая ошибка: Попытка чтения за пределами файла в строке %1.").arg(y));
                file.unmap(const_cast<uchar*>(data)); // Освобождение ресурсов перед выходом
                file.close();
                return;
            }
            std::copy_n(lut[*bytePtr++].pixels, 8, scanline + x);
            x += 8;
        }

        while (x < width) {
            if (bytePtr >= data + fileSize) {
                emit error(QString("Критическая ошибка: Попытка чтения за пределами файла в строке %1.").arg(y));
                file.unmap(const_cast<uchar*>(data)); // Освобождение ресурсов перед выходом
                file.close();
                return;
            }
            scanline[x++] = colors[(*bytePtr >> bitOffset) & 1];
            if (--bitOffset < 0) {
                bitOffset = 7;
                bytePtr++;
            }
        }
    }

    //==== Конец замера времени отрисовки =====//
    const qint64 drawTimeMs = drawTimer.elapsed();
    qDebug() << "Ход работы [5]: Основной цикл обработки (отрисовка) завершен за" << drawTimeMs << "мс.";


    file.unmap(const_cast<uchar*>(data));
    file.close();
    qDebug() << "Ход работы [6]: Файл отключен от памяти и закрыт";

    const qint64 totalTimeMs = totalTimer.elapsed();
    qDebug() << "Ход работы [7]: Вся операция заняла" << totalTimeMs << "мс.";

    emit finished(image);
}
