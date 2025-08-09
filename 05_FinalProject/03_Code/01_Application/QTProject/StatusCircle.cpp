#include "StatusCircle.h"
#include <QTimer>
#include <QPainter>

StatusCircle::StatusCircle(QWidget *parent)
    : QWidget(parent),
    m_minValue(0),
    m_maxValue(100),
    m_value(30),
    m_precision(0),
    m_angle(40),
    m_usedColor(0, 0, 255),
    m_freeColor(255, 255, 255),
    m_rangeTextColor(137, 137, 137),
    m_valueTextColor(52, 155, 218),
    m_valueBackgroundColor(239, 239, 239),
    m_outBackgroundColor(233, 233, 233),
    m_centerBackgroundColorStart(45, 204, 112),
    m_centerBackgroundColorEnd(51, 152, 219),
    m_currentPercent(30),
    m_valuePercent(0)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(10 * 1);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateValue()));
}

StatusCircle::~StatusCircle()
{
    if(m_timer->isActive())
    {
        m_timer->stop();
    }
}

void StatusCircle::setRange(double minValue, double maxValue)
{
    if(minValue >= maxValue)
    {
        return;
    }

    if(m_timer->isActive())
    {
        m_timer->stop();
    }

    m_currentPercent = 0;
    m_valuePercent = 100 * (m_value - m_minValue) / (m_maxValue - m_minValue);
    m_minValue = minValue;
    m_maxValue = maxValue;

    if(m_value < minValue || m_value > maxValue)
    {
        setValue(m_value);
    }
}

void StatusCircle::setMinValue(double minValue)
{
    setRange(minValue, m_maxValue);
}

void StatusCircle::setMaxValue(double maxValue)
{
    setRange(m_minValue, maxValue);
}

void StatusCircle::setValue(double value)
{
    if(value < m_minValue)
    {
        m_value = m_minValue;
    }
    else if(value > m_maxValue)
    {
        m_value = m_maxValue;
    }
    else {
        m_value = value;
    }

    if(m_timer->isActive())
    {
        m_timer->stop();
    }

    m_currentPercent = 0;
    m_valuePercent = 100 * (m_value - m_minValue) / (m_maxValue - m_minValue);
    m_timer->start();
}

void StatusCircle::setAngle(double angle)
{
    if(m_angle != angle)
    {
        m_angle = angle;
        update();
    }
}

void StatusCircle::setPrecision(int precision)
{
    if(precision <= 3 && m_precision != precision)
    {
        m_precision = precision;
        update();
    }
}

void StatusCircle::setUnit(const QString &unit)
{
    if(m_unit != unit)
    {
        m_unit = unit;
        update();
    }
}

void StatusCircle::setUsedColor(const QColor &usedColor)
{
    if(m_usedColor != usedColor)
    {
        m_usedColor = usedColor;
        update();
    }
}

void StatusCircle::setFreeColor(const QColor &freeColor)
{
    if(m_freeColor != freeColor)
    {
        m_freeColor = freeColor;
        update();
    }
}

void StatusCircle::setRangeTextColor(const QColor &rangeTextColor)
{
    if(m_rangeTextColor != rangeTextColor)
    {
        m_rangeTextColor = rangeTextColor;
        update();
    }
}

void StatusCircle::setValueTextColor(const QColor &valueTextColor)
{
    if(m_valueTextColor != valueTextColor)
    {
        m_valueTextColor = valueTextColor;
        update();
    }
}

void StatusCircle::setValueBackgroundColor(const QColor &valueBackgroundColor)
{
    if(m_valueBackgroundColor != valueBackgroundColor)
    {
        m_valueBackgroundColor = valueBackgroundColor;
        update();
    }
}

void StatusCircle::setOutBackgroundColor(const QColor &outBackgroundColor)
{
    if(m_outBackgroundColor != outBackgroundColor)
    {
        m_outBackgroundColor = outBackgroundColor;
        update();
    }
}

void StatusCircle::setCenterBackgroundColorStart(const QColor &centerBackgroundColorStart)
{
    if(m_centerBackgroundColorStart != centerBackgroundColorStart)
    {
        m_centerBackgroundColorStart = centerBackgroundColorStart;
        update();
    }
}

void StatusCircle::setCenterBackgroundColorEnd(const QColor &centerBackgroundColorEnd)
{
    if(m_centerBackgroundColorEnd != centerBackgroundColorEnd)
    {
        m_centerBackgroundColorEnd = centerBackgroundColorEnd;
        update();
    }
}

QSize StatusCircle::sizeHint() const
{
    return QSize(200, 200);
}

void StatusCircle::updateValue()
{
    if(m_currentPercent >= m_valuePercent)
    {
        m_timer->stop();
        return;
    }

    m_currentPercent++;
    update();
}

void StatusCircle::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    const int w = width();
    const int h = height();
    const int side = std::min(w, h);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.translate(w / 2, h / 2);
    painter.scale(side / 200.0, side / 200.0);

    drawDial(&painter);
    drawBackgroundOut(&painter);
    drawBackgroundRound(&painter);
    drawBackgroundCenter(&painter);
    drawText(&painter);
}

void StatusCircle::drawDial(QPainter *painter)
{
    constexpr int radius = 95;
    constexpr double lineWidth = 2.5;
    painter->save();
    painter->rotate(m_angle);

    QPen pen = painter->pen();
    pen.setWidthF(lineWidth);
    pen.setCapStyle(Qt::RoundCap);

    const double rotate = (360 - (m_angle * 2)) * 1.0 / STATUS_CIRCLE_DIAL_NUM;

    pen.setColor(m_usedColor);
    painter->setPen(pen);

    int m_step = (STATUS_CIRCLE_DIAL_NUM * m_currentPercent) / 100;
    for(int i = 0; i < m_step; ++i)
    {
        painter->drawLine(0, radius, 0, radius / 1.2);
        painter->rotate(rotate);
    }

    pen.setColor(m_freeColor);
    painter->setPen(pen);

    for(int i = m_step; i < STATUS_CIRCLE_DIAL_NUM; ++i)
    {
        painter->drawLine(0, radius, 0, radius / 1.2);
        painter->rotate(rotate);
    }

    painter->restore();
}

void StatusCircle::drawBackgroundOut(QPainter *painter)
{
    constexpr int radius = 70;
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(m_outBackgroundColor);
    painter->drawEllipse(-radius, -radius, radius * 2, radius * 2);
    painter->restore();
}

void StatusCircle::drawBackgroundRound(QPainter *painter)
{
    constexpr int radius = 50;
    painter->save();

    painter->setPen(Qt::NoPen);
    painter->setBrush(m_centerBackgroundColorEnd);
    painter->drawEllipse(-radius, -radius, radius * 2, radius * 2);
    painter->restore();
}

void StatusCircle::drawBackgroundCenter(QPainter *painter)
{
    constexpr int radius = 30;
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(m_valueBackgroundColor);

    painter->drawEllipse(-radius, -radius, radius * 2, radius * 2);
    painter->restore();
}

void StatusCircle::drawText(QPainter *painter)
{
    constexpr int radius = 100;
    painter->save();

    double currentValue = m_currentPercent * ((m_maxValue - m_minValue) / 100) + m_minValue;

    if(currentValue > m_value)
    {
        currentValue = m_value;
    }

    const QString &strValue = QString("%1%2").arg(QString::number(currentValue, 'f', m_precision), m_unit);
    const QString &strMinValue = QString("%1%2").arg(m_minValue).arg(m_unit);
    const QString &strMaxValue = QString("%1%2").arg(m_maxValue).arg(m_unit);

    painter->setFont(QFont("Arial", 13));
    painter->setPen(QPen(m_valueTextColor));

    const QRectF textRect(-radius, -radius, radius * 2, radius * 2);
    painter->drawText(textRect, Qt::AlignCenter, strValue);

    painter->setFont(QFont("Arial", 8));
    painter->setPen(QPen(m_rangeTextColor));

    QSizeF size = painter->fontMetrics().size(Qt::TextSingleLine, strMinValue);
    painter->drawText(-radius / 2 - size.width() / 2 + 8, 80, strMinValue);
    size = painter->fontMetrics().size(Qt::TextSingleLine, strMaxValue);
    painter->drawText(radius / 2 - size.width() / 2 - 8, 80, strMaxValue);

    painter->restore();
}
