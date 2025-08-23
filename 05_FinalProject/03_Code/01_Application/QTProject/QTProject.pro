QT       += core gui
QT       += charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    CircularProgressBar.cpp \
    RingMeter.cpp \
    StatusCircle.cpp \
    cpuinfoclass.cpp \
    cpuwindow.cpp \
    main.cpp \
    mainwindow.cpp \
    memorywindow.cpp \
    raminfoclass.cpp

HEADERS += \
    AppConfig.h \
    CircularProgressBar.h \
    RingMeter.h \
    StatusCircle.h \
    cpuinfoclass.h \
    cpuwindow.h \
    mainwindow.h \
    memorywindow.h \
    raminfoclass.h

FORMS += \
    cpuwindow.ui \
    mainwindow.ui \
    memorywindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    BackGround.qrc \
    button.qrc \
    footer.qrc \
    header.qrc \
    temper.qrc
