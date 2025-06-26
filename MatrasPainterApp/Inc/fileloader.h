#ifndef FILELOADER_H
#define FILELOADER_H

#include <QObject>
#include <QRunnable>
#include <QFile>
#include <QByteArray>
#include <QString>
#include <memory>

/**
 * @class FileLoader
 * @brief Потокобезопасный загрузчик файлов для фоновой обработки [ бинарников ]
 *
 * Класс реализует асинхронную загрузку бинарных файлов в отдельном потоке.
 * После завершения загрузки автоматически удаляет себя.
 */
class FileLoader : public QObject, public QRunnable
{
    Q_OBJECT
public:
    /**
    * @brief Будет запускать процесс загрузки файла
    */
    explicit FileLoader(QString filePath, QObject* parent = nullptr);

    /**
    * @brief Будет запускать процесс загрузки файла
    */
    void run() override;
signals:
    /**
    * @brief Сигнал об успешной загрузке
    * @param data - Загруженные данные
    *
    * @note Срабатывает в потоке вызывающего объекта
    */
//    void finished(std::unique_ptr<QByteArray> data);

    /**
    * @brief Сигнал об ошибке загрузки
    * @param error - Текстовое описание ошибки
    *
    * @warning Срабатывает только при критических ошибках [файла]
    */
    void error(const QString& error);

private:
    QString m_filePath; ///< Путь к файлу для обращения
};

#endif // FILELOADER_H
