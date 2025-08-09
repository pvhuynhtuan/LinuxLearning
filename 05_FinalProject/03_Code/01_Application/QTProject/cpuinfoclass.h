#ifndef CPUINFOCLASS_H
#define CPUINFOCLASS_H

#include <QFile>
#include <QStringList>
#include <QTextStream>
#include <QThread>
#include <QDebug>
#include <QRegularExpression>

#include "AppConfig.h"

class CpuInfoClass
{
private:
    uint64_t ulUser;
    uint64_t ulNice;
    uint64_t ulSystem;
    uint64_t ulIdle;
    uint64_t ulIowait;
    uint64_t ulIrq;
    uint64_t ulSoftirq;
    uint64_t ulSteal;
    double udLoadAverage;
    double udTemperature;
    QString sPrefix;
    QString sCpuFilePath;
    QString sLoadFilePath;
    QString sTemperFilePath;

public:
    CpuInfoClass();
    CpuInfoClass(QString lPrefix, QString lCpuFilePath, QString lLoadFilePath, QString lTemperFilePath);
    double CalculateCpuUsage();
    double CalculateLoadAverage();
    double CalculateTemperature();
};

#endif // CPUINFOCLASS_H
