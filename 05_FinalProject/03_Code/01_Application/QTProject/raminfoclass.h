#ifndef RAMINFOCLASS_H
#define RAMINFOCLASS_H

#include <QFile>
#include <QStringList>
#include <QTextStream>
#include <QThread>
#include <QDebug>
#include <QRegularExpression>

#include "AppConfig.h"

class RamInfoClass
{
private:
    uint64_t ulRamTotal;
    uint64_t ulRamAvailable;
    uint64_t ulSwapTotal;
    uint64_t ulSwapFree;

    QString sRamFilePath;
    QString sRamTotalPrefix;
    QString sRamAvailablePrefix;
    QString sSwapTotalPrefix;
    QString sSwapFreePrefix;

public:
    RamInfoClass();
    RamInfoClass(QString lsRamTotalPrefix, QString lsRamAvailablePrefix, QString lsSwapTotalPrefix, QString lsSwapFreePrefix, QString lsRamFilePath);
    uint64_t getRamTotal();
    uint64_t getRamAvailable();
    uint64_t getSwapTotal();
    uint64_t getSwapFree();
    double CalculateRamUsage();
};

#endif // RAMINFOCLASS_H
