#include "painterworker.h"
#include <QFile>
#include <QDataStream>
#include <QColor>
#include <QDebug>

PainterWorker::PainterWorker(QObject *parent) : QObject(parent) {}

/**
 * @brief Запускает процесс генерации изображения из бинарного файла.
 * @param filePath Путь к бинарному файлу.
 * @param period Период (ширина) для отрисовки в битах.
 */
void PainterWorker::process(const QString &filePath, int period)
{
    if (filePath.isEmpty()) {
        emit error("Файл пуст. Требуется выбрать");
        return;
    }
    if (period <= 0) {
        emit error("Период должен быть пложительным числом");
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit error("Ошибка открытия файла: " + file.errorString());
        return;
    }

    // Чтение содержимого
    QByteArray fileBytes = file.readAll();
    file.close();

    const qint64 totalBits = fileBytes.size() * 8;
    if (totalBits == 0) {
        emit error("Файл пуст");
        return;
    }

    // Расчет размеров изображения
    const int width = period; // Ширина картинки - период
    const int height = static_cast<int>(totalBits / period); // Количество полных пакетов-строк

    if (width <= 0 || height <= 0) {
        emit error("Ошибка размера изображения. Требуется проверка периода.");
        return;
    }

    // Создание изображения и отрисовка
    QImage image(width, height, QImage::Format_RGB32);
    const QRgb colorOne = QColor(Qt::green).rgb();    // Цвет для бита '1'
    const QRgb colorZero = QColor(Qt::black).rgb();   // Цвет для бита '0'

    // Прямой указатель на данные для высокой скорости доступа
    const uchar *data = reinterpret_cast<const uchar*>(fileBytes.constData());

    // Проход по [пакету] будущего изображения:
    for (int y = 0; y < height; ++y) {
        // Указатель на начало текущей строки в QImage:
        QRgb *scanLine = reinterpret_cast<QRgb*>(image.scanLine(y));

        for (int x = 0; x < width; ++x) {
            // Рассчет абсолютной позиции текущего бита в потоке:
            qint64 bitPosition = (qint64)y * width + x;

            // Нахождение байта с нужным значением:
            qint64 byteIndex = bitPosition / 8;
            // Вычисление позиции бита внутри этого байта (7 - ... для MSB-first)
            int bitInByte = 7 - (bitPosition % 8); // Отрисовка: От старшего к младшему
            //int bitInByte = bitPosition % 8;         // Отрисовка: От младшего к старшему

            // Проверка на всякий - установлен ли бит? (1 или 0) //- // Отрисовка: От старшего к младшему
            bool isBitSet = (data[byteIndex] >> bitInByte) & 1;

            // Установка цвета пикселя:
            scanLine[x] = isBitSet ? colorOne : colorZero;
        }
    }
    // Отправка в главный поток для визуализации
    emit finished(image);
}
