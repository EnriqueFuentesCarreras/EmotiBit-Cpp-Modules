
/** **************************************************************************
*
*  file DoubleBuffer.h
* @author Enrique
* @date Mayo 2025
*
* @details
* Proyecto desarrollado en C++ con Qt 6.7.2 para la UNED.
*
* - Framework: Qt (módulos: Network, Widgets, Core, etc.)
* - Compilador: MSVC / MinGW (según configuración)
* - Entorno: Windows 10/11
* - Herramientas de documentación: Doxygen, Graphviz
* - Licencia: MIT (si corresponde)
*
* @note Este proyecto forma parte del Trabajo de Fin de Grado.
*
 *
 * Descripción: Implementación de un doble buffer para almacenar datos de tipo T.
 * Utiliza dos buffers y un mutex para la sincronización, permitiendo escribir en
 * el buffer inactivo y leer desde el buffer activo de manera eficiente.
 *
 * Fecha: 2025-04-24
 ****************************************************************************/

#ifndef DOUBLEBUFFER_H
#define DOUBLEBUFFER_H

#include <QVector>
#include <QMutex>
#include <QMutexLocker>
#include <utility> // Para std::move




/*!
 * \file DoubleBuffer.h
 * \brief Implementación de un doble buffer para almacenamiento eficiente y seguro de datos entre hilos.
 *
 * Esta clase utiliza dos buffers internos y mecanismos de sincronización mediante QMutex para gestionar
 * simultáneamente la recepción y procesamiento de datos, garantizando rendimiento y seguridad en contextos multihilo.
 * Al utilizar dos búferes independientes, uno para escritura y otro para lectura,
 * permite que un hilo reciba y almacene datos sin interferir con otro hilo que los procesa,
 * evitando así condiciones de carrera y mejorando el rendimiento general.
 * El uso de QMutex y QMutexLocker hace que el acceso a los búferes esté correctamente sincronizado,
 * garantizando la consistencia y la integridad de los datos. Además,
 * la función swapAndRead mediante std::move, reduce copias innecesarias.
 * Hace posible la comunicación multihilo de manera fluida y segura,
 * permite que la aplicación maneje múltiples operaciones de red de forma simultánea y sin bloqueos.
 *
 * \see FormPlot, ChannelFrequencies
 * \author Enrique Fuentes
 * \date 2025-04-24
 */

template <typename T>
class DoubleBuffer {
public:
    DoubleBuffer() : activeBuffer(0) {}

    // Escribe un elemento en el buffer inactivo
    void write(const T &data) {
        QMutexLocker locker(&mutex);
        buffers[1 - activeBuffer].append(data);
    }


    void push_back(const T &data) {
        write(data);
    }

    void get(std::vector<T> &output) const {
        QMutexLocker locker(&mutex); // bloquea el mutex para evitar condiciones de carrera
        output.clear(); //  limpia el contenedor antes de llenarlo
        output.reserve(buffers[activeBuffer].size()); // Reserva espacio para evitar ajustar la cantidad de memoria
        for (const auto &item : buffers[activeBuffer]) {
            output.push_back(item);
        }
    }


    // Cambia los buffers, devuelve el contenido del buffer antes activo
    QVector<T> swapAndRead() {
        QMutexLocker locker(&mutex);
        // Cambia el buffer activo
        activeBuffer = 1 - activeBuffer;
        // Mueve los datos del buffer inactivo (ahora activo), evita copias
        QVector<T> data = std::move(buffers[activeBuffer]);
        // Limpia el buffer inactivo
        buffers[activeBuffer].clear();
        return data;
    }

    // Mira si hay datos en el buffer activo
    bool hasData() const {
        QMutexLocker locker(&mutex);
        return !buffers[activeBuffer].isEmpty();
    }

    // Obtiene el tamaño del buffer activo
    int activeBufferSize() const {
        QMutexLocker locker(&mutex);
        return buffers[activeBuffer].size();
    }

private:
    QVector<T> buffers[2];   // Doble buffer
    int activeBuffer;        // Índice del buffer activo (0 o 1)
    mutable QMutex mutex;    // Mutex para sincronización
};

#endif // DOUBLEBUFFER_H
