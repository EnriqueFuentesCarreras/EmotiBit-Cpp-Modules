#ifndef CHANNELFREQUENCIES_H
#define CHANNELFREQUENCIES_H

#include <QString>
#include <QMap>

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

#endif
