/****************************************************************************
 * ChannelFrequencies.cpp
 *
 * Descripción: Esta clase gestiona las frecuencias de muestreo de diferentes
 * canales de datos, como los de movimiento, temperatura, EDA, PPG, etc.
 * Los canales calculados también están definidos aquí.
 *
 * Fecha: 2025-04-24
 ****************************************************************************/

#include "ChannelFrequencies.h"

// Constructor: inicializa las frecuencias por canal
ChannelFrequencies::ChannelFrequencies() {
    // Frecuencias de muestreo por canal

    // Movimiento: 25 muestras/segundo
    channelFrequencies["AX"] = 25.0;
    channelFrequencies["AY"] = 25.0;
    channelFrequencies["AZ"] = 25.0;
    channelFrequencies["GX"] = 25.0;
    channelFrequencies["GY"] = 25.0;
    channelFrequencies["GZ"] = 25.0;
    channelFrequencies["MX"] = 25.0;
    channelFrequencies["MY"] = 25.0;
    channelFrequencies["MZ"] = 25.0;

    // Temperatura: 7.5 muestras/segundo
    channelFrequencies["T1"] = 7.5;
    channelFrequencies["T0"] = 7.5;
    channelFrequencies["TH"] = 7.5;

    // EDA: 15 muestras/segundo
    channelFrequencies["EA"] = 15.0;
    channelFrequencies["EL"] = 15.0;
    channelFrequencies["ER"] = 15.0;

    // PPG: 25 muestras/segundo
    channelFrequencies["PI"] = 25.0;
    channelFrequencies["PR"] = 25.0;
    channelFrequencies["PG"] = 25.0;

    // Canales calculados:
    channelFrequencies["HR"] = 1; // Calculado a partir de PG
    channelFrequencies["SR"] = 1; // Canal calculado
    channelFrequencies["SF"] = 1; // Canal calculado
    channelFrequencies["SA"] = 1; // Canal calculado
    channelFrequencies["BI"] = 1; // Intervalo entre latidos (calculado)
    channelFrequencies["BV"] = 1; // Canal no identificado


}





// Obtiene la frecuencia de muestreo de un canal específico
/**
 * Devuelve la frecuencia de muestreo de un canal. Si el canal no existe,
 * retorna 0.0.
 *
 * @param channel El canal del cual obtener la frecuencia.
 * @return La frecuencia de muestreo del canal.
 */
double ChannelFrequencies::getFrequency(const QString& channel) const {
    return channelFrequencies.value(channel, 0.0); // Retorna 0.0 si el canal no existe
}






// Verifica si un canal existe
/**
 * Verifica si un canal específico está definido en las frecuencias.
 *
 * @param channel El canal a verificar.
 * @return true si el canal existe, false si no.
 */
bool ChannelFrequencies::contains(const QString& channel) const {
    return channelFrequencies.contains(channel);
}






// Obtiene todas las frecuencias de canales
/**
 * Obtiene un mapa de todas las frecuencias de los canales definidos.
 *
 * @return Un QMap que contiene todos los canales y sus frecuencias.
 */
QMap<QString, double> ChannelFrequencies::getAllFrequencies() const {
    return channelFrequencies;
}

