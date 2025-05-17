/**
*  file ChannelFrequencies.h
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
*/

#ifndef CHANNELFREQUENCIES_H
#define CHANNELFREQUENCIES_H

#include <QString>
#include <QMap>

/*!
 * \class ChannelFrequencies.h
 * \brief Gestiona las frecuencias de muestreo asignadas a cada canal de datos del EmotiBit.
 *
 * Permite configurar y consultar frecuencias específicas para cada tipo de dato recibido, como movimiento,
 * temperatura, EDA o PPG, facilitando la correcta representación temporal de los datos adquiridos.
 * Los canales calculados también están definidos aquí.
 *
 * \see FormPlot, DoubleBuffer
 * \author Enrique Fuentes
 * \date 2025-04-24
 */

class ChannelFrequencies {
public:

    ChannelFrequencies();

    //  para obtener la frecuencia de un canal específico
    double getFrequency(const QString& channel) const;

    //  para verificar si un canal existe
    bool contains(const QString& channel) const;

    // para obtener todas las frecuencias
    QMap<QString, double> getAllFrequencies() const;

private:
    QMap<QString, double> channelFrequencies;
};

#endif // CHANNELFREQUENCIES_H
