#ifndef PAINTER_H
#define PAINTER_H

#include <QImage>
#include <QByteArray>

/**
* @brief Библиотека для отрисовки битового потока.
*
* Класс предоставляет функциональность для преобразования
* битового потока в изображение.
*/
class Painter {
public:
    /**
     * @brief Генерирует изображение из битового потока.
     * @return QImage Сгенерированное изображение. 1 - зеленый, 0 - черный.
     */
    static QImage generateImage(/* данные и период */);
};

#endif // PAINTER_H
