#include "cpuinfoclass.h"

CpuInfoClass::CpuInfoClass()
{
    ulUser = 0;
    ulNice = 0;
    ulSystem = 0;
    ulIdle = 0;
    ulIowait = 0;
    ulIrq = 0;
    ulSoftirq = 0;
    ulSteal = 0;
    udLoadAverage = 0;
    udTemperature = 0;
    sPrefix = "cpu ";
    sCpuFilePath = "/proc/stat";
    sLoadFilePath = "/proc/loadavg";
    sTemperFilePath = "/sys/class/thermal/thermal_zone0/temp";
}

CpuInfoClass::CpuInfoClass(QString lPrefix, QString lCpuFilePath, QString lLoadFilePath, QString lTemperFilePath)
{
    ulUser = 0;
    ulNice = 0;
    ulSystem = 0;
    ulIdle = 0;
    ulIowait = 0;
    ulIrq = 0;
    ulSoftirq = 0;
    ulSteal = 0;
    udLoadAverage = 0;
    udTemperature = 0;
    sPrefix = lPrefix;
    sCpuFilePath = lCpuFilePath;
    sLoadFilePath = lLoadFilePath;
    sTemperFilePath = lTemperFilePath;
}

double CpuInfoClass::CalculateCpuUsage()
{
    static const QRegularExpression WHITESPACE("\\s+");

    uint64_t lulUser, lulNice, lulSystem, lulIdle, lulIowait, lulIrq, lulSoftirq, lulSteal;
    uint64_t lulPrevIdle, lulCurIdle;
    uint64_t lulPrevNonIdle, lulCurNonIdle;
    uint64_t lulPrevTotal, lulCurTotal;
    uint64_t lulTotalDiff, lulIdleDiff;

    QFile loFile(this->sCpuFilePath);

    // Read the file
    if (loFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        // Read entire file content at once
        QString lsData = loFile.readAll();
        loFile.close();

        // Split into lines (skip empty ones)
        QStringList lolines = lsData.split("\n", Qt::SkipEmptyParts);

        for (const QString &lsLine : lolines)
        {
            if (lsLine.startsWith(this->sPrefix))
            {
                // qDebug() << "lsLine: " << lsLine;
                lulSteal = 0;
                QStringList lsParts = lsLine.split(WHITESPACE, Qt::SkipEmptyParts);
                if (lsParts.size() >= 8)
                {
                    lulUser    = lsParts[1].toULongLong();
                    lulNice    = lsParts[2].toULongLong();
                    lulSystem  = lsParts[3].toULongLong();
                    lulIdle    = lsParts[4].toULongLong();
                    lulIowait  = lsParts[5].toULongLong();
                    lulIrq     = lsParts[6].toULongLong();
                    lulSoftirq = lsParts[7].toULongLong();
                    if (lsParts.size() > 8)
                    {
                        lulSteal = lsParts[8].toULongLong();
                    }
                    else
                    {
                        // Do nothing
                    }
                }
                else
                {
                    // missed cpu information
                    qDebug("Missed number of cpu information");
                    break;
                }

                // Calculate the ilde time
                lulPrevIdle = this->ulIdle + this->ulIowait;
                lulCurIdle = lulIdle + lulIowait;

                // Calculate the non-idle time
                lulPrevNonIdle = this->ulUser + this->ulNice + this->ulSystem + this->ulIrq + this->ulSoftirq + this->ulSteal;
                lulCurNonIdle = lulUser + lulNice + lulSystem + lulIrq + lulSoftirq + lulSteal;

                // Calculate the total time
                lulPrevTotal = lulPrevIdle + lulPrevNonIdle;
                lulCurTotal = lulCurIdle + lulCurNonIdle;

                // Calculate the different time
                lulTotalDiff = lulCurTotal - lulPrevTotal;
                lulIdleDiff = lulCurIdle - lulPrevIdle;

                // Store back the data
                this->ulUser = lulUser;
                this->ulNice = lulNice;
                this->ulSystem = lulSystem;
                this->ulIdle = lulIdle;
                this->ulIowait = lulIowait;
                this->ulIrq = lulIrq;
                this->ulSoftirq = lulSoftirq;
                this->ulSteal = lulSteal;

                if (lulCurTotal >= lulPrevTotal)
                {
                    return ((double)(lulTotalDiff - lulIdleDiff) / (double)lulTotalDiff) * 100.0;
                }
                else
                {
                    return -1; // Incorrect data
                }
            }
            else
            {
                // Do nothing
            }
        }
        // If the while loop is exit without any return => Can't find the cpu information
        qDebug() << "Prefix not found: " << this->sPrefix;
        return -1;
    }
    else
    {
        // Can't open the cpu information
        qDebug("File path not found!");
        return -1;
    }
}

double CpuInfoClass::CalculateLoadAverage()
{
    static const QRegularExpression WHITESPACE("\\s+");

    double ldLoadAverage;

    QFile loFile(this->sLoadFilePath);

    // Read the file
    if (loFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        // Read entire file content at once
        QString lsData = loFile.readAll();
        loFile.close();

        if (!lsData.isEmpty())
        {
            // qDebug() << "lsLine: " << lsData;

            QStringList lsParts = lsData.split(WHITESPACE, Qt::SkipEmptyParts);
            if (lsParts.size() >= 1)
            {
                // Get the first value only
                ldLoadAverage  = lsParts[0].toDouble();
            }
            else
            {
                // Do nothing
                ldLoadAverage = -1;
            }

            // Store back the data
            this->udLoadAverage = ldLoadAverage;

            return ldLoadAverage;
        }
        else
        {
            return -1; // Incorrect data
        }
    }
    else
    {
        // Can't open the cpu information
        qDebug("Load Average file path not found!");
        return -1;
    }
}

double CpuInfoClass::CalculateTemperature()
{
    uint64_t lulTemper;

    QFile loFile(this->sTemperFilePath);

    // Read the file
    if (loFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        // Read entire file content at once
        QString lsData = loFile.readAll();
        loFile.close();

        if (!lsData.isEmpty())
        {
            // qDebug() << "lsLine: " << lsData;

            lulTemper = lsData.toULongLong();

            if (lulTemper > 0)
            {
                this->udTemperature = (double)lulTemper / 1000.0;
                return this->udTemperature;
            }
            else
            {
                return -1;
            }
        }
        else
        {
            return -1; // Incorrect data
        }
    }
    else
    {
        // Can't open the cpu information
        qDebug("Temperature file path not found!");
        return -1;
    }
}
