#ifndef EMOTIBITPARSER_H
#define EMOTIBITPARSER_H

#include <QVector>
#include <QString>
#include "ChannelFrequencies.h"

// Estructura para almacenar cada muestra
struct Sample {
    double time;
    double value;
};

class EmotiBitParser {
public:
    // El constructor recibe la referencia a ChannelFrequencies
    EmotiBitParser(const ChannelFrequencies &freq);
    QVector<Sample> parseFile(const QString &filePath,
                              const QString &channelID,
                              qint64 referenceTimestamp);

private:
    const ChannelFrequencies &channelFreq;
};

#endif // EMOTIBITPARSER_H
