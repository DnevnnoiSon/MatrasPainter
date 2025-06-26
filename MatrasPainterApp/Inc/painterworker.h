#ifndef PAINTERWORKER_H
#define PAINTERWORKER_H

#include <QObject>
#include <QString>
#include <QImage>

/**
 * @brief Выполняет генерацию изображения из бинарного файла в рабочем потоке.
 *
 * Класс читает битовый поток из файла и формирует QImage на основе
 * заданного периода (ширины изображения). Работает асинхронно.
 */
class PainterWorker : public QObject
{
    Q_OBJECT

public:
    explicit PainterWorker(QObject *parent = nullptr);

public slots:
    /**
     * @brief Запускает процесс генерации изображения.
     * @param filePath Путь к бинарному файлу.
     * @param period Период (ширина) для отрисовки в битах.
     */
    void process(const QString &filePath, int period);

signals:
    /**
     * @brief Сигнал, испускаемый по завершении успешной генерации.
     * @param image Сгенерированное изображение.
     */
    void finished(const QImage &image);

    /**
     * @brief Сигнал об ошибке в процессе работы.
     * @param error Текст ошибки.
     */
    void error(const QString &error);
};

#endif // PAINTERWORKER_H
