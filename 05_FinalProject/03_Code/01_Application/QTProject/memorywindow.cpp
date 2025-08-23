#include "memorywindow.h"
#include "ui_memorywindow.h"
#include <QTimer>
#include <QTime>
#include <QStorageInfo>

MemoryWindow::MemoryWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MemoryWindow)
{
    ui->setupUi(this);


    pRamMeter = findChild<StatusCircle*>("wRamCircle");
    pMemMeter = findChild<StatusCircle*>("wMemCircle");

    // for (const QStorageInfo &storage : QStorageInfo::mountedVolumes()) {
    //     qDebug() << "Device:" << storage.device();
    //     qDebug() << "Root path:" << storage.rootPath();
    //     qDebug() << "Name:" << storage.displayName();
    //     qDebug() << "File system type:" << storage.fileSystemType();
    //     qDebug() << "Is ReadOnly:" << storage.isReadOnly();
    //     qDebug() << "Bytes available:" << storage.bytesAvailable();
    //     qDebug() << "Bytes total:" << storage.bytesTotal();
    //     qDebug() << "-----";
    // }
    QStorageInfo root = QStorageInfo::root();
    ui->lbMemRootPath->setText(root.rootPath());
    ui->lbMemDevice->setText(root.device());
    ui->lbMemFileSysType->setText(root.fileSystemType());
    ui->lbMemTotal->setText(QString("%1").arg(root.bytesTotal() / 1024));
    ui->lbMemAvail->setText(QString("%1").arg(root.bytesAvailable() / 1024));


    QTimer *lpTimerSlow = new QTimer(this);
    connect(lpTimerSlow, &QTimer::timeout, this, &MemoryWindow::onTimerExceedSlow);
    lpTimerSlow->start(MEM_SLOW_TIMER_INTERVAL_MS);
}

MemoryWindow::~MemoryWindow()
{
    delete ui;
}

void MemoryWindow::on_btnMemBack_clicked()
{
    this->hide();
}



void MemoryWindow::onTimerExceedSlow()
{

}

