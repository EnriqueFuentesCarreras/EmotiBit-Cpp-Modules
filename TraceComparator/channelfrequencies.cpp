#include "ChannelFrequencies.h"

// Constructor: inicializa las frecuencias por canal, numero de muestras por segundo
ChannelFrequencies::ChannelFrequencies() {

    // Motion: 25 muestras/segundo
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

    // canales calculados,
    channelFrequencies["HR"] = 1; // este es calculado a partir de PG
    channelFrequencies["SR"] = 1; // este es calculado
    channelFrequencies["SF"] = 1; // este es calculado
    channelFrequencies["SA"] = 1; // este es calculado
    channelFrequencies["BI"] = 1; // este es calculado intervalo entre latidos
    channelFrequencies["BV"] = 1;

}

// Obtiene la frecuencia de un canal específico
double ChannelFrequencies::getFrequency(const QString& channel) const {
    return channelFrequencies.value(channel, 0.0); // Retorna 0.0 si el canal no existe
}

// Verifica si el canal existe
bool ChannelFrequencies::contains(const QString& channel) const {
    return channelFrequencies.contains(channel);
}

// Obtiene todas las frecuencias
QMap<QString, double> ChannelFrequencies::getAllFrequencies() const {
    return channelFrequencies;
}


