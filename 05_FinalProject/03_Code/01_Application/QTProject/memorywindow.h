#ifndef MEMORYWINDOW_H
#define MEMORYWINDOW_H

#include <QMainWindow>

#include "raminfoclass.h"
#include "StatusCircle.h"
#include "AppConfig.h"

// Specific define for CPU display
#define MEM_SLOW_TIMER_INTERVAL_MS          1000

namespace Ui {
class MemoryWindow;
}

class MemoryWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MemoryWindow(QWidget *parent = nullptr);
    ~MemoryWindow();

private slots:
    void on_btnMemBack_clicked();
    void onTimerExceedSlow();

private:
    Ui::MemoryWindow *ui;
    RamInfoClass *gpRamInfo;
    StatusCircle *pRamMeter;
    StatusCircle *pMemMeter;
};

#endif // MEMORYWINDOW_H
