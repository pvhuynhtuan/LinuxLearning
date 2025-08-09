#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QTime>   // for QTime

#include <QMessageBox>

#if (APP_LINUX_CODE_ENABLE == STD_ON)
#include <time.h>
#endif /* End of #if (APP_LINUX_CODE_ENABLE == STD_ON) */

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , cpuScreen(nullptr)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    pCpuMeter = findChild<RingMeter*>("meCPU");
    pCpuMeter->setName("CPU");
    pRamMeter = findChild<RingMeter*>("meRam");
    pRamMeter->setName("RAM");

    #if (APP_LINUX_CODE_ENABLE == STD_ON)
    QTimer *timer = new QTimer(this);

    connect(timer, &QTimer::timeout, this, [=]()
    {
        time_t current_time;
        struct tm *local_time;
        // Get the current time
        current_time = time(NULL);
        if (current_time == ((time_t)-1)) {
            perror("time");
            return;
        }

        // Convert to local time
        local_time = localtime(&current_time);
        if (local_time == NULL) {
            perror("localtime");
            return;
        }

        // Format and print time and date
        char time_str[100];
        if (strftime(time_str, sizeof(time_str), "%H:%M:%S", local_time)) {
            // printf("Current Date and Time: %s\n", time_str);
            ui->lbCurrentTime->setText(QString::asprintf("%s", time_str));
        } else {
            fprintf(stderr, "strftime returned 0");
            return;
        }
    });
    
    timer->start(1000);  // update every second
    #else
    QTimer *lpTimerFast = new QTimer(this);
    connect(lpTimerFast, &QTimer::timeout, this, &MainWindow::onTimerExceedFast);
    lpTimerFast->start(MAIN_FAST_TIMER_INTERVAL_MS);

    QTimer *lpTimerSlow = new QTimer(this);
    connect(lpTimerSlow, &QTimer::timeout, this, &MainWindow::onTimerExceedSlow);
    lpTimerSlow->start(MAIN_SLOW_TIMER_INTERVAL_MS);
    #endif /* End of #if (APP_LINUX_CODE_ENABLE == STD_ON) */

    connect(pCpuMeter, &RingMeter::onClicked, this, &MainWindow::onCPUMeterClicked);
    connect(pRamMeter, &RingMeter::onClicked, this, &MainWindow::onRAMMeterClicked);
}

MainWindow::~MainWindow()
{
    // Free memory
    delete cpuScreen;
    delete pCpuMeter;
    delete pRamMeter;
    delete ui;
}

void MainWindow::onCPUMeterClicked()
{
    // pCpuMeter->setValue(50.5);
    // cpuwindow *pCpuWindow = new cpuwindow();
    // pCpuWindow->show();
    if (!cpuScreen) {
        cpuScreen = new cpuwindow(this);  // or nullptr for independent window
    }
    cpuScreen->show();
    cpuScreen->raise();
    cpuScreen->activateWindow();
}

void MainWindow::onRAMMeterClicked()
{
    pRamMeter->setValue(50.5);
}

void MainWindow::onTimerExceedFast()
{

}

void MainWindow::onTimerExceedSlow()
{
    ui->lbCurrentTime->setText(QTime::currentTime().toString());
}

