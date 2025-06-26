#include "PainterWorker.h"
#include <QFile>
#include <QDebug>

PainterWorker::PainterWorker(QObject *parent) : QObject(parent) {}

void PainterWorker::process(const QString &filePath, int period)
{
    if (filePath.isEmpty() || period <= 0) {
        emit error("Invalid file path or period.");
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit error("Failed to open file: " + file.errorString());
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    const qint64 totalBits = data.size() * 8;
    if (totalBits == 0) {
        emit error("File is empty.");
        return;
    }

    const int width = period;
    const int height = static_cast<int>(totalBits / period);

    if (width <= 0 || height <= 0) {
        emit error("Calculated image size is invalid.");
        return;
    }

    // Создание изображения. Format_RGB32 быстрее для попиксельного доступа.
    QImage image(width, height, QImage::Format_RGB32);
    const QColor colorOne = QColor(Qt::green); // Цвет для '1' [cite: 4]
    const QColor colorZero = QColor(Qt::black); // Цвет для '0' [cite: 4]

    quint8 *fileData = (quint8*)data.constData();

    for (int y = 0; y < height; ++y) {
        QRgb *line = (QRgb *)image.scanLine(y);
        for (int x = 0; x < width; ++x) {
            qint64 currentBitPos = (qint64)y * width + x;
            qint64 byteIndex = currentBitPos / 8;
            int bitIndex = 7 - (currentBitPos % 8); // Читаем биты от старшего к младшему

            if (byteIndex >= data.size()) {
                break; // Выход, если данные кончились
            }

            bool bitIsSet = (fileData[byteIndex] >> bitIndex) & 1;

            line[x] = bitIsSet ? colorOne.rgb() : colorZero.rgb();
        }
    }

    emit finished(image);
}
