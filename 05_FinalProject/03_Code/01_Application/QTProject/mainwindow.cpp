#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QTime>
#include <QDate>

#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , cpuScreen(nullptr)
    , memoryScreen(nullptr)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /*
     * Setting the overal CPU information
     * Note: In this time, the fixed paths are provided, need improve later if any
     */
    glTotalCpuInfo = new CpuInfoClass("cpu ", CPU_USAGE_PATH, CPU_LOAD_PATH, CPU_TEMPER_PATH);
    #ifdef Q_OS_LINUX
    glTotalCpuInfo->CalculateCpuUsage(); // dummy read to start initial data
    #endif

    gpRamInfo = new RamInfoClass();
    #ifdef Q_OS_LINUX
    gpRamInfo->CalculateRamUsage(); // dummy read to start initial data
    #endif

    pCpuMeter = findChild<RingMeter*>("meCPU");
    pCpuMeter->setName("CPU");
    pRamMeter = findChild<RingMeter*>("meRam");
    pRamMeter->setName("RAM");

    QTimer *lpTimerFast = new QTimer(this);
    connect(lpTimerFast, &QTimer::timeout, this, &MainWindow::onTimerExceedFast);
    lpTimerFast->start(MAIN_FAST_TIMER_INTERVAL_MS);

    QTimer *lpTimerSlow = new QTimer(this);
    connect(lpTimerSlow, &QTimer::timeout, this, &MainWindow::onTimerExceedSlow);
    lpTimerSlow->start(MAIN_SLOW_TIMER_INTERVAL_MS);

    connect(pCpuMeter, &RingMeter::onClicked, this, &MainWindow::onCPUMeterClicked);
    connect(pRamMeter, &RingMeter::onClicked, this, &MainWindow::onRAMMeterClicked);
}

MainWindow::~MainWindow()
{
    // Free memory
    delete cpuScreen;
    delete memoryScreen;
    delete pCpuMeter;
    delete pRamMeter;
    delete ui;
}

void MainWindow::onCPUMeterClicked()
{
    if (!cpuScreen) {
        cpuScreen = new cpuwindow(this);  // or nullptr for independent window
    }
    cpuScreen->show();
    cpuScreen->raise();
    cpuScreen->activateWindow();
}

void MainWindow::onRAMMeterClicked()
{
    if (!memoryScreen) {
        memoryScreen = new MemoryWindow(this);  // or nullptr for independent window
    }
    memoryScreen->show();
    memoryScreen->raise();
    memoryScreen->activateWindow();
}

void MainWindow::onTimerExceedFast()
{
    double ldLoadAvg, ldTotalCpuUsage, ldTemper;
    double ldRamUsage;
    #ifdef Q_OS_LINUX
    double ldRamTotal, ldRamAvailable;
    #endif

    // Display the CPU usage
    #ifdef Q_OS_LINUX
    ldTotalCpuUsage = glTotalCpuInfo->CalculateCpuUsage();
    if (ldTotalCpuUsage >= 0)
    {
        pCpuMeter->setValue(ldTotalCpuUsage);
    }
    else
    {
        // Do nothing
    }
    #elif defined(Q_OS_WIN)
    ldTotalCpuUsage = rand() % 100; // Fake data for now => to test on the window
    pCpuMeter->setValue(ldTotalCpuUsage);
    #endif

    // Display the Load Average
    #ifdef Q_OS_LINUX
    ldLoadAvg = glTotalCpuInfo->CalculateLoadAverage();
    if (ldLoadAvg >= 0)
    {
        ui->lbCpuInfo->setText(QString("Load Avg: %1").arg(ldLoadAvg, 0, 'f', 2));
    }
    else
    {
        // Do nothing
    }
    #elif defined(Q_OS_WIN)
    ldLoadAvg = rand() % 10; // Fake data for now => to test on the window
    ui->lbCpuInfo->setText(QString("Load Avg: %1").arg(ldLoadAvg, 0, 'f', 2));
    #endif

    // Display the cpu temperature
    #ifdef Q_OS_LINUX
    ldTemper = glTotalCpuInfo->CalculateTemperature();
    if (ldTemper >= 0)
    {
        ui->lbCpuTemp->setText(QString::number(ldTemper, 'f', 1));
    }
    else
    {
        // Do nothing
    }
    #elif defined(Q_OS_WIN)
    ldTemper = rand() % 100; // Fake data for now => to test on the window
    ui->lbCpuTemp->setText(QString::number(ldTemper, 'f', 1));
    #endif

    // Display the RAM usage
    #ifdef Q_OS_LINUX
    ldRamUsage = gpRamInfo->CalculateRamUsage();
    if (ldRamUsage >= 0)
    {
        pRamMeter->setValue(ldRamUsage);
    }
    else
    {
        // Do nothing
    }
    #elif defined(Q_OS_WIN)
    ldRamUsage = rand() % 100; // Fake data for now => to test on the window
    pRamMeter->setValue(ldRamUsage);
    #endif

    // Display the RAM data  
    #ifdef Q_OS_LINUX
    ldRamTotal = (double)gpRamInfo->getRamTotal() / 1024.0;
    ldRamAvailable = (double)gpRamInfo->getRamAvailable() / 1024.0;
    ui->lbRamInfo->setText(QString("RAM: %1/%2 MB").arg(ldRamTotal - ldRamAvailable, 0, 'f', 0).arg(ldRamTotal, 0, 'f', 0));
    #endif
}

void MainWindow::onTimerExceedSlow()
{
    ui->lbCurrentTime->setText(QTime::currentTime().toString());
    ui->lbCurrentDate->setText(QDate::currentDate().toString("dd/MM/yyyy"));

    #ifdef Q_OS_LINUX
    QString lsIp = getCurrentIP();
    if (!lsIp.isNull())
    {
        if (gsMyIP != lsIp)
        {
            qDebug() << "New IP address: " << lsIp;
            gsMyIP = lsIp;
            ui->lbMyIP->setText(lsIp);
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
    #endif /* End of #ifdef Q_OS_LINUX */
}

#ifdef Q_OS_LINUX
QString MainWindow::getCurrentIP()
{
    QString lsReturnValue = QString();
    struct ifaddrs *ifaddr, *ifa_index;
    char ip[INET_ADDRSTRLEN]; // The variable to store the ip as string

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("Can't get the list of interface");
        return lsReturnValue;
    }

    for (ifa_index = ifaddr; ifa_index != NULL; ifa_index = ifa_index->ifa_next)
    {
        if ((ifa_index->ifa_addr != NULL) && (ifa_index->ifa_addr->sa_family == AF_INET))
        {
            // Store the address from the interface
            struct sockaddr_in *addr = (struct sockaddr_in *)ifa_index->ifa_addr;

            // Convert the network address structure into a character string.
            inet_ntop(AF_INET, &addr->sin_addr, ip, INET_ADDRSTRLEN);

            // Store the my ip but ignore loop address
            if (strcmp(ifa_index->ifa_name, "lo") != 0)
            {
                // qDebug() << "My IP address of " << ifa_index->ifa_name << ": " << ip;
                // Free the memory which allocated by getifaddrs, close the file
                freeifaddrs(ifaddr);
                lsReturnValue = QString::fromUtf8(ip);
                return lsReturnValue;
            }
        }
        else
        {
            // Do nothing
        }
    }
    // Free the memory which allocated by getifaddrs, close the file
    freeifaddrs(ifaddr);
    return lsReturnValue;
}
#endif /* End of #ifdef Q_OS_LINUX */
