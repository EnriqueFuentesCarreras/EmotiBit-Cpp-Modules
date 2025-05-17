#include "EmotiBitParser.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QStringList>
#include <algorithm>

EmotiBitParser::EmotiBitParser(const ChannelFrequencies &freq)
    : channelFreq(freq)
{
}

QVector<Sample> EmotiBitParser::parseFile(const QString &filePath,
                                          const QString &channelID,
                                          qint64 referenceTimestamp)
{
    QVector<Sample> samples;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "No se pudo abrir el archivo:" << filePath;
        return samples;
    }

    QTextStream in(&file);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList fields = line.split(',');
        if (fields.size() < 7) continue;

        qint64 timestamp = fields[0].toLongLong();  // ms
        int numSamples   = fields[2].toInt();
        QString cID      = fields[3];

        // Filtrar por canal
        if (cID != channelID)
            continue;

        // A partir de índice 6 están los valores
        QStringList dataFields = fields.mid(6);
        if (dataFields.size() < numSamples)
            continue;

        // Frecuencia del canal
        double freq = channelFreq.getFrequency(channelID);
        if (freq <= 0.0)
            continue;

        double sampleInterval = 1.0 / freq;

        // Usar el referenceTimestamp en lugar de un initialTimestamp local
        double relativeTimeBase = double(timestamp - referenceTimestamp) / 1000.0;

        // Procesar cada muestra
        for (int i = 0; i < numSamples; ++i) {
            bool ok;
            double val = dataFields[i].toDouble(&ok);
            if (!ok)
                continue;

            // Tiempo de la i-ésima muestra
            double sampleTime = relativeTimeBase - (numSamples - 1 - i) * sampleInterval;
            samples.append({ sampleTime, val });
        }
    }
    file.close();

    // Ordenar por tiempo
    std::sort(samples.begin(), samples.end(), [](const Sample &a, const Sample &b){
        return a.time < b.time;
    });

    return samples;
}
