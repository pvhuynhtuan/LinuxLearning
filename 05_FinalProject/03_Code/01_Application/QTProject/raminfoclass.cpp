#include "raminfoclass.h"

RamInfoClass::RamInfoClass()
{
    ulRamTotal = 0;
    ulRamAvailable = 0;
    ulSwapTotal = 0;
    ulSwapFree = 0;

    sRamFilePath = RAM_USAGE_PATH;
    sRamTotalPrefix = RAM_TOTAL_PREFIX;
    sRamAvailablePrefix = RAM_AVAILABLE_PREFIX;
    sSwapTotalPrefix = SWAP_TOTAL_PREFIX;
    sSwapFreePrefix = SWAP_FREE_PREFIX;
}


RamInfoClass::RamInfoClass(QString lsRamTotalPrefix, QString lsRamAvailablePrefix, QString lsSwapTotalPrefix, QString lsSwapFreePrefix, QString lsRamFilePath)
{
    ulRamTotal = 0;
    ulRamAvailable = 0;
    ulSwapTotal = 0;
    ulSwapFree = 0;

    sRamFilePath = lsRamFilePath;
    sRamTotalPrefix = lsRamTotalPrefix;
    sRamAvailablePrefix = lsRamAvailablePrefix;
    sSwapTotalPrefix = lsSwapTotalPrefix;
    sSwapFreePrefix = lsSwapFreePrefix;
}

uint64_t RamInfoClass::getRamTotal()
{
    return ulRamTotal;
}

uint64_t RamInfoClass::getRamAvailable()
{
    return ulRamAvailable;
}

uint64_t RamInfoClass::getSwapTotal()
{
    return ulSwapTotal;
}

uint64_t RamInfoClass::getSwapFree()
{
    return ulSwapFree;
}

double RamInfoClass::CalculateRamUsage()
{
    static const QRegularExpression WHITESPACE("\\s+");
    uint64_t lulRamTotal, lulRamAvailable, lulSwapTotal, lulSwapFree;
    double ldReturnValue;

    // initializing with the invalid value
    ldReturnValue = -1.0;

    lulRamTotal = 0;
    lulRamAvailable = 0;
    lulSwapTotal = 0;
    lulSwapFree = 0;

    QFile loFile(this->sRamFilePath);

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
            if (lsLine.startsWith(this->sRamTotalPrefix))
            {
                // qDebug() << "lsLine: " << lsLine;
                QStringList lsParts = lsLine.split(WHITESPACE, Qt::SkipEmptyParts);
                if (lsParts.size() >= 2)
                {
                    lulRamTotal = lsParts[1].toULongLong();
                }
                else
                {
                    // missed ram total information
                    qDebug("missed ram total information");
                    break;
                }
            }
            else if (lsLine.startsWith(this->sRamAvailablePrefix))
            {
                QStringList lsParts = lsLine.split(WHITESPACE, Qt::SkipEmptyParts);
                if (lsParts.size() >= 2)
                {
                    lulRamAvailable = lsParts[1].toULongLong();
                }
                else
                {
                    // missed ram available information
                    qDebug("missed ram available information");
                    break;
                }
            }
            else if (lsLine.startsWith(this->sSwapTotalPrefix))
            {
                QStringList lsParts = lsLine.split(WHITESPACE, Qt::SkipEmptyParts);
                if (lsParts.size() >= 2)
                {
                    lulSwapTotal = lsParts[1].toULongLong();
                }
                else
                {
                    // missed swap total information
                    qDebug("missed swap total information");
                    break;
                }
            }
            else if (lsLine.startsWith(this->sSwapFreePrefix))
            {
                QStringList lsParts = lsLine.split(WHITESPACE, Qt::SkipEmptyParts);
                if (lsParts.size() >= 2)
                {
                    lulSwapFree = lsParts[1].toULongLong();
                }
                else
                {
                    // missed swap free information
                    qDebug("missed swap free information");
                    break;
                }
            }
            else
            {
                // Do nothing
            }
        }

        if ((0 != lulRamTotal) || (0 != lulRamAvailable))
        {
            this->ulRamTotal = lulRamTotal;
            this->ulRamAvailable = lulRamAvailable;
            this->ulSwapTotal = lulSwapTotal;
            this->ulSwapFree = lulSwapFree;

            ldReturnValue = ((double)(lulRamTotal - lulRamAvailable) * 100.0) / (double)lulRamTotal;
        }
        else
        {
            // Do nothing due to failed to read the data
        }


    }
    else
    {
        // Can't open the cpu information
        qDebug("Ram File path not found!");
    }

    return ldReturnValue;
}

