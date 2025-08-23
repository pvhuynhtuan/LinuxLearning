#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "cpuwindow.h"
#include "memorywindow.h"

#include "RingMeter.h"
#include "cpuinfoclass.h"
#include "raminfoclass.h"

#include "AppConfig.h"

// include the linux platform header section
#ifdef Q_OS_LINUX
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <net/if.h>
#include <ifaddrs.h>
#endif /* End of #ifdef Q_OS_LINUX */

// Specific define for CPU display
#define MAIN_FAST_TIMER_INTERVAL_MS         100
#define MAIN_SLOW_TIMER_INTERVAL_MS         1000

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onCPUMeterClicked();
    void onRAMMeterClicked();
    void onTimerExceedFast();
    void onTimerExceedSlow();

private:
    cpuwindow *cpuScreen;
    MemoryWindow *memoryScreen;
    Ui::MainWindow *ui;
    RingMeter *pCpuMeter;
    RingMeter *pRamMeter;
    CpuInfoClass *glTotalCpuInfo;
    RamInfoClass *gpRamInfo;
#ifdef Q_OS_LINUX
    QString gsMyIP;
    QString getCurrentIP();
#endif /* End of #ifdef Q_OS_LINUX */
};
#endif // MAINWINDOW_H
