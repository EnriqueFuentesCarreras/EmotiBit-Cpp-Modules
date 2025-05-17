#ifndef QEMOTIBIRPARSER_H
#define QEMOTIBIRPARSER_H

#include <QString>
#include "ChannelFrequencies.h"

class qemotibirparser
{
public:
    qemotibirparser();
    QStringList loadCsvFile(const QString &filePath);

private:
    ChannelFrequencies channelFreq;
};

#endif
