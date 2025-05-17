#include "qemotibirparser.h"


#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

qemotibirparser::qemotibirparser() {}



QStringList qemotibirparser::loadCsvFile(const QString &filePath)
{
    QStringList archivosGenerados;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "No se pudo abrir el archivo:" << filePath;
        return archivosGenerados;
    }

    QFileInfo fi(filePath);
    QString dir = fi.absolutePath();
    QString baseName = fi.completeBaseName();

    QTextStream in(&file);
    qint64 initialTimestamp = 0;
    bool firstTimestampFound = false;

    QMap<QString, double> freqMap = channelFreq.getAllFrequencies();

    struct Sample {
        double time;
        double value;
    };

    QMap<QString, QVector<Sample>> channelData;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList fields = line.split(',');
        if (fields.size() < 7) {
            qWarning() << "Línea con formato incorrecto:" << line;
            continue;
        }

        qint64 timestamp = fields[0].toLongLong();
        int numSamples = fields[2].toInt();
        QString channelID = fields[3];
        QStringList dataFields = fields.mid(6);

        if (dataFields.size() < numSamples) continue;

        if (!firstTimestampFound) {
            initialTimestamp = timestamp;
            firstTimestampFound = true;
        }

        double relativeTimeBase = static_cast<double>(timestamp - initialTimestamp) / 1000.0;

        if (!freqMap.contains(channelID)) continue;

        double freq = freqMap[channelID];
        if (freq <= 0.0) continue;

        double sampleInterval = 1.0 / freq;

        for (int i = 0; i < numSamples; ++i) {
            bool ok;
            double value = dataFields[i].toDouble(&ok);
            if (!ok) continue;

            double sampleTime = relativeTimeBase - (numSamples - 1 - i) * sampleInterval;
            channelData[channelID].append({ sampleTime, value });
        }
    }

    file.close();

    for (auto it = channelData.begin(); it != channelData.end(); ++it) {
        QVector<Sample> samples = it.value();
        std::sort(samples.begin(), samples.end(), [](const Sample &a, const Sample &b) {
            return a.time < b.time;
        });

        QString outFileName = QString("%1/%2_%3.CSV").arg(dir, baseName, it.key());
        QFile outFile(outFileName);
        if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "No se pudo escribir el archivo:" << outFileName;
            continue;
        }

        QTextStream outStream(&outFile);
        outStream << "time," << it.key() << "\n";
        for (const Sample &s : samples) {
            outStream << QString::number(s.time, 'f', 6) << "," << QString::number(s.value, 'f', 6) << "\n";
        }

        outFile.close();

        // Añadir archivo a la lista de retorno
        archivosGenerados << outFileName;
    }

    return archivosGenerados;
}
