#include "CircularProgressBar.h"
#include <QPainter>
#include <QtMath>

CircularProgressBar::CircularProgressBar(QWidget *parent)
    : QWidget(parent), m_value(0) {
    setMinimumSize(100, 100);
}

void CircularProgressBar::setValue(int value) {
    m_value = qBound(0, value, 100); // clamp to 0–100
    update();
}

void CircularProgressBar::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int width = this->width();
    int height = this->height();
    int size = qMin(width, height) - 10;

    QRectF rect((width - size) / 2, (height - size) / 2, size, size);

    // Background circle
    painter.setPen(QPen(Qt::lightGray, 10));
    painter.drawEllipse(rect);

    // Progress arc
    painter.setPen(QPen(Qt::blue, 10, Qt::SolidLine, Qt::FlatCap));
    painter.drawArc(rect, 90 * 16, -m_value * 360 / 100 * 16);
}
