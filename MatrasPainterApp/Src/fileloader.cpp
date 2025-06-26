#include "fileloader.h"

FileLoader::FileLoader(QString filePath, QObject* parent)
    : QObject(parent),
    m_filePath(std::move(filePath))
{
// Авто-удаление после выполнения
    setAutoDelete(true);
}

void FileLoader::run()
{
    QFile file(m_filePath);

    if(!file.exists()) {
        // Сигнал об ошибке
        return;
    }

    if(!file.open(QIODevice::ReadOnly)) {
       // Сигнал об ошибке
        return;
    }

    auto fileData = std::make_unique<QByteArray>();

    // Чтение с буферизацией | буфер - 1MB
    constexpr qint64 bufferSize = 1 << 20;
    while(!file.atEnd()) {
        *fileData += file.read(bufferSize);
    }

    if(file.error() != QFile::NoError) {
        // Сигнал об ошибке
    } else {
        // Cигнал об успешном выполнении
        // Надо переместить данные
    }

}
