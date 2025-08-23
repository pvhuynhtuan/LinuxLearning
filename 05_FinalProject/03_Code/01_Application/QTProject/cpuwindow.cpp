#include "cpuwindow.h"
#include "ui_cpuwindow.h"
// #include <QGraphicsOpacityEffect>
// #include <QPropertyAnimation>
#include <QThread>
#include <QDebug>

cpuwindow::cpuwindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::cpuwindow)
{
    ui->setupUi(this);



    // // Add fade-in effect
    // QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(this);
    // this->setGraphicsEffect(effect);

    // QPropertyAnimation *fadeIn = new QPropertyAnimation(effect, "opacity");
    // fadeIn->setDuration(500);  // milliseconds
    // fadeIn->setStartValue(0.0);
    // fadeIn->setEndValue(1.0);
    // fadeIn->setEasingCurve(QEasingCurve::OutCubic);
    // fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
    //series = new QLineSeries();

    gpCpuChart = ui->cCpuChart->chart();
    gpCpuChart->setMargins(QMargins(0, 0, 0, 0)); // smaller padding around plot
    gpCpuChart->setBackgroundBrush(QBrush(Qt::black));       // Solid color
    gpCpuChart->setBackgroundRoundness(0);                   // No rounded corners
    // gpCpuChart->setTitle("CPU Usage");
    gpCpuChart->removeAllSeries();

    QFont lLabelFont;
    lLabelFont.setPointSize(7);   // font size in points

    // Setting the axis:
    gpAxisX = new QValueAxis();
    gpAxisX->setLinePen(QColor(255, 170, 0)); // Line (axis) color
    gpAxisX->setGridLinePen(QPen(Qt::gray)); // Grid line color
    gpAxisX->setLabelsColor(QColor(255, 170, 0)); // Label (text) color
    gpAxisX->setLabelsVisible(false);
    gpAxisX->setLabelsFont(lLabelFont);
    gpAxisX->setRange(0, CPU_MAX_POINTS);
    gpCpuChart->addAxis(gpAxisX, Qt::AlignBottom);

    gpAxisY = new QValueAxis();
    gpAxisY->setLinePen(QColor(255, 170, 0)); // Line (axis) color
    gpAxisY->setGridLinePen(QPen(Qt::gray)); // Grid line color
    gpAxisY->setLabelsColor(QColor(255, 170, 0)); // Label (text) color
    gpAxisY->setLabelsFont(lLabelFont);
    gpAxisY->setRange(0, 100);
    gpCpuChart->addAxis(gpAxisY, Qt::AlignLeft);

    /*
     * Setting the overal CPU information
     * Note: In this time, the fixed paths are provided, need improve later if any
     */
    glTotalCpuInfo = new CpuInfoClass("cpu ", CPU_USAGE_PATH, CPU_LOAD_PATH, CPU_TEMPER_PATH);
    #ifdef Q_OS_LINUX
    glTotalCpuInfo->CalculateCpuUsage(); // dummy read to start initial data
    #endif

    int liCoreCount = QThread::idealThreadCount();
    if (liCoreCount > 0)
    {
        qDebug() << "Number of CPU cores:" << liCoreCount;
        for (int liIndex = 0; liIndex < liCoreCount; liIndex++)
        {
            // Create the object information of CPU core
            QString lsPrefix = QString("cpu%1").arg(liIndex);
            CpuInfoClass *loCpuInfo = new CpuInfoClass(lsPrefix, CPU_USAGE_PATH, CPU_LOAD_PATH, CPU_TEMPER_PATH);
            #ifdef Q_OS_LINUX
            loCpuInfo->CalculateCpuUsage(); // dummy read to start initial data
            #endif
            glCpusInfo.append(loCpuInfo);

            // Create the series of CPU core
            QLineSeries *lpSeries = new QLineSeries();

            lpSeries->setName(QString("%1").arg(liIndex));
            gpCpuChart->addSeries(lpSeries);
            lpSeries->attachAxis(gpAxisX);
            lpSeries->attachAxis(gpAxisY);

            glCpuSeries.append(lpSeries);  // store for later use
        }
    } else
    {
        qDebug() << "Unable to detect CPU core count.";
    }

    QTimer *lpTimerGraph = new QTimer(this);
    connect(lpTimerGraph, &QTimer::timeout, this,  &cpuwindow::onTimerExceedGraph);
    lpTimerGraph->start(CPU_GRAPH_TIMER_INTERVAL_MS);  // update every CPU_TIMER_INTERVAL_MS

    QTimer *lpTimerInfo = new QTimer(this);
    connect(lpTimerInfo, &QTimer::timeout, this,  &cpuwindow::onTimerExceedInfo);
    lpTimerInfo->start(CPU_INFO_TIMER_INTERVAL_MS);  // update every CPU_TIMER_INTERVAL_MS
}

cpuwindow::~cpuwindow()
{
    // Free memory
    glCpuSeries.clear();
    glCpusInfo.clear();
    delete gpCpuChart;
    delete ui;
}

void cpuwindow::on_btnBackButton_clicked()
{
    this->hide();
}

void cpuwindow::onTimerExceedGraph()
{
    static qint64 timeCounter = 0;
    qreal ldUsage;
    timeCounter++;

    for (int liIndex = 0; liIndex < glCpuSeries.size(); ++liIndex)
    {
        #ifdef Q_OS_LINUX
        ldUsage = glCpusInfo[liIndex]->CalculateCpuUsage();
        #elif defined(Q_OS_WIN)
        ldUsage = rand() % 100; // Fake data for now => to test on the window
        #endif

        //Checking the return value
        if (0 <= ldUsage)
        {
            glCpuSeries[liIndex]->append(timeCounter, ldUsage);
            if (CPU_MAX_POINTS < glCpuSeries[liIndex]->count())
            {
                glCpuSeries[liIndex]->removePoints(0, glCpuSeries[liIndex]->count() - CPU_MAX_POINTS);
            }
            else
            {
                // Do nothing
            }
        }
        else
        {
            // Do nothing
        }

    }

    // Keep only the last 60 points for each series
    if (timeCounter > CPU_MAX_POINTS)
    {
        gpAxisX->setRange(timeCounter - CPU_MAX_POINTS, timeCounter);
    }
}

void cpuwindow::onTimerExceedInfo()
{
    double ldLoadAvg, ldTotalCpuUsage, ldTemper;

    // Display the Load Average
    #ifdef Q_OS_LINUX
    ldLoadAvg = glTotalCpuInfo->CalculateLoadAverage();
    if (ldLoadAvg >= 0)
    {
        ui->lbLoadAvg->setText(QString::number(ldLoadAvg, 'f', 2));
    }
    else
    {
        // Do nothing
    }
    #elif defined(Q_OS_WIN)
    ldLoadAvg = rand() % 10; // Fake data for now => to test on the window
    ui->lbLoadAvg->setText(QString::number(ldLoadAvg, 'f', 2));
    #endif

    // Display the CPU usage
    #ifdef Q_OS_LINUX
    ldTotalCpuUsage = glTotalCpuInfo->CalculateCpuUsage();
    if (ldTotalCpuUsage >= 0)
    {
        ui->lbTotalCpuUsage->setText(QString::number(ldTotalCpuUsage, 'f', 2));
    }
    else
    {
        // Do nothing
    }
    #elif defined(Q_OS_WIN)
    ldTotalCpuUsage = rand() % 100; // Fake data for now => to test on the window
    ui->lbTotalCpuUsage->setText(QString::number(ldTotalCpuUsage, 'f', 2));
    #endif

    // Display the CPU temperature
    #ifdef Q_OS_LINUX
    ldTemper = glTotalCpuInfo->CalculateTemperature();
    if (ldTemper >= 0)
    {
        ui->lbTemperature->setText(QString::number(ldTemper, 'f', 2));
    }
    else
    {
        // Do nothing
    }
    ui->lbTemperature->setText(QString::number(ldTemper, 'f', 2));
    #elif defined(Q_OS_WIN)
    ldTemper = rand() % 100; // Fake data for now => to test on the window
    ui->lbTemperature->setText(QString::number(ldTemper, 'f', 2));
    #endif
}
