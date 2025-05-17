/*
 * @class DoubleBuffer
 * @brief Clase genérica para almacenar datos en dos buffers, permitiendo escritura y lectura sin bloqueo.
 *
 * Esta clase permite escribir datos en un buffer inactivo mientras el otro buffer está disponible para lectura.
 * Se utiliza un mutex para sincronizar el acceso a los buffers y evitar condiciones de carrera.
 * Al llamar a `swapAndRead()`, los buffers se intercambian: el inactivo pasa a ser activo y se devuelve
 * el contenido del buffer previamente activo.
 *
 * @tparam T Tipo de dato que se almacenará en los buffers (por ejemplo, QString, QByteArray, etc.).
 */

#ifndef DOUBLEBUFFER_H
#define DOUBLEBUFFER_H

#include <QVector>
#include <QMutex>
#include <QMutexLocker>
#include <utility> // Para std::move

template <typename T>
class DoubleBuffer {
public:
    // Constructor: inicializa el buffer activo en el índice 0
    DoubleBuffer() : activeBuffer(0) {}

    /*
     * Escribe un elemento en el buffer inactivo.
     * Utiliza un mutex para asegurar que no haya acceso concurrente durante la escritura.
     *
     * @param data El dato a escribir en el buffer inactivo.
     */
    void write(const T &data) {
        QMutexLocker locker(&mutex);  // Bloquea el mutex para sincronización
        buffers[1 - activeBuffer].append(data);  // Escribe en el buffer inactivo
    }

    /*
     * Cambia los buffers y devuelve el contenido del buffer previamente activo para lectura.
     * Utiliza `std::move` para evitar copias de los datos.
     *
     * @return El contenido del buffer previamente activo.
     */
    QVector<T> swapAndRead() {
        QMutexLocker locker(&mutex);  // Bloquea el mutex para sincronización
        activeBuffer = 1 - activeBuffer;  // Cambia el buffer activo
        QVector<T> data = std::move(buffers[activeBuffer]);  // Mueve los datos al vector de retorno
        buffers[activeBuffer].clear();  // Limpia el buffer ahora inactivo
        return data;  // Retorna los datos movidos
    }

    /*
     * Verifica si hay datos en el buffer activo.
     *
     * @return true si el buffer activo tiene datos, false en caso contrario.
     */
    bool hasData() const {
        QMutexLocker locker(&mutex);  // Bloquea el mutex para sincronización
        return !buffers[activeBuffer].isEmpty();  // Verifica si el buffer activo tiene datos
    }

    /*
     * Obtiene el tamaño del buffer activo.
     *
     * @return El tamaño del buffer activo.
     */
    int activeBufferSize() const {
        QMutexLocker locker(&mutex);  // Bloquea el mutex para sincronización
        return buffers[activeBuffer].size();  // Retorna el tamaño del buffer activo
    }

private:
    QVector<T> buffers[2];   // Dos buffers (activo e inactivo)
    int activeBuffer;        // Índice del buffer activo (0 o 1)
    mutable QMutex mutex;    // Mutex para sincronización
};

#endif



