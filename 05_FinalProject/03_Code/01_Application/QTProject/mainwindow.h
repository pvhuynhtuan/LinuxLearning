#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "RingMeter.h"
#include "cpuwindow.h"

#include "AppConfig.h"

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
    Ui::MainWindow *ui;
    RingMeter *pCpuMeter;
    RingMeter *pRamMeter;
};
#endif // MAINWINDOW_H
