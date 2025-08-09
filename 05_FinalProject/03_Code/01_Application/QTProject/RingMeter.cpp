#include "RingMeter.h"

#include <QMouseEvent>
#include <qmath.h>
#include <QTimer>
#include <QPainter>

RingMeter::RingMeter(QWidget *parent)
    : QWidget(parent),
    m_minValue(0),
    m_maxValue(100),
    m_value(0),
    m_name("TBD"),
    m_precision(0),
    m_scaleMajor(10),
    m_scaleMinor(10),
    m_startAngle(40),
    m_endAngle(40),
    m_animation(true),
    m_animationStep(0.5),
    m_ringWidth(10),
    m_ringStartPercent(30),
    m_ringMidPercent(40),
    m_ringEndPercent(30),
    m_ringColorStart(0, 255, 127),
    m_ringColorMid(255, 255, 0),
    m_ringColorEnd(253, 107, 107),
    m_pointerColor(217, 217, 0),
    m_textColor(24, 188, 154),
    m_nameColor(255, 255, 255),
    m_reverse(false),
    m_currentValue(50)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(2 * RINGMETER_MS);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateValue()));
}

RingMeter::~RingMeter()
{
    if(m_timer->isActive())
    {
        m_timer->stop();
    }
}

void RingMeter::setRange(double minValue, double maxValue)
{
    if(minValue >= maxValue)
    {
        return;
    }

    m_minValue = minValue;
    m_maxValue = maxValue;

    if(m_value < m_minValue || m_value > m_maxValue)
    {
        setValue(m_value);
    }

    update();
}

void RingMeter::setMinValue(double minValue)
{
    setRange(minValue, m_maxValue);
}

void RingMeter::setMaxValue(double maxValue)
{
    setRange(m_minValue, maxValue);
}

void RingMeter::setValue(double value)
{
    if(value < m_minValue || value > m_maxValue)
    {
        return;
    }

    if(value > m_value)
    {
        m_reverse = false;
    }
    else if(value < m_value)
    {
        m_reverse = true;
    }

    m_value = value;

    if(!m_animation)
    {
        m_currentValue = value;
        update();
    }
    else
    {
        m_timer->start();
    }
}

void RingMeter::setName(QString name)
{
    m_name = name;

    if(!m_animation)
    {
        m_name = name;
        update();
    }
    else
    {
        m_timer->start();
    }
}

void RingMeter::setPrecision(int precision)
{
    if(precision <= 2 && m_precision != precision)
    {
        m_precision = precision;
        update();
    }
}

void RingMeter::setScaleMajor(int scaleMajor)
{
    if(m_scaleMajor != scaleMajor)
    {
        m_scaleMajor = scaleMajor;
        update();
    }
}

void RingMeter::setScaleMinor(int scaleMinor)
{
    if(m_scaleMinor != scaleMinor)
    {
        m_scaleMinor = scaleMinor;
        update();
    }
}

void RingMeter::setStartAngle(int startAngle)
{
    if(m_startAngle != startAngle)
    {
        m_startAngle = startAngle;
        update();
    }
}

void RingMeter::setEndAngle(int endAngle)
{
    if(m_endAngle != endAngle)
    {
        m_endAngle = endAngle;
        update();
    }
}

void RingMeter::setAnimation(bool animation)
{
    if(m_animation != animation)
    {
        m_animation = animation;
        update();
    }
}

void RingMeter::setAnimationStep(double animationStep)
{
    if(m_animationStep != animationStep)
    {
        m_animationStep = animationStep;
        update();
    }
}

void RingMeter::setRingWidth(int ringWidth)
{
    if(m_ringWidth != ringWidth)
    {
        m_ringWidth = ringWidth;
        update();
    }
}

void RingMeter::setRingStartPercent(int ringStartPercent)
{
    if(m_ringStartPercent != ringStartPercent)
    {
        m_ringStartPercent = ringStartPercent;
        update();
    }
}

void RingMeter::setRingMidPercent(int ringMidPercent)
{
    if(m_ringMidPercent != ringMidPercent)
    {
        m_ringMidPercent = ringMidPercent;
        update();
    }
}

void RingMeter::setRingEndPercent(int ringEndPercent)
{
    if(m_ringEndPercent != ringEndPercent)
    {
        m_ringEndPercent = ringEndPercent;
        update();
    }
}

void RingMeter::setRingColorStart(const QColor &ringColorStart)
{
    if(m_ringColorStart != ringColorStart)
    {
        m_ringColorStart = ringColorStart;
        update();
    }
}

void RingMeter::setRingColorMid(const QColor &ringColorMid)
{
    if(m_ringColorMid != ringColorMid)
    {
        m_ringColorMid = ringColorMid;
        update();
    }
}

void RingMeter::setRingColorEnd(const QColor &ringColorEnd)
{
    if(m_ringColorEnd != ringColorEnd)
    {
        m_ringColorEnd = ringColorEnd;
        update();
    }
}

void RingMeter::setPointerColor(const QColor &pointerColor)
{
    if(m_pointerColor != pointerColor)
    {
        m_pointerColor = pointerColor;
        update();
    }
}

void RingMeter::setTextColor(const QColor &textColor)
{
    if(m_textColor != textColor)
    {
        m_textColor = textColor;
        update();
    }
}

void RingMeter::setNameColor(const QColor &nameColor)
{
    if(m_nameColor != nameColor)
    {
        m_nameColor = nameColor;
        update();
    }
}

QSize RingMeter::sizeHint() const
{
    return QSize(200, 200);
}

void RingMeter::updateValue()
{
    if(!m_reverse)
    {
        if(m_currentValue >= m_value)
        {
            m_currentValue = m_value;
            m_timer->stop();
        }
        else
        {
            m_currentValue += m_animationStep;
        }
    }
    else
    {
        if(m_currentValue <= m_value)
        {
            m_currentValue = m_value;
            m_timer->stop();
        }
        else
        {
            m_currentValue -= m_animationStep;
        }
    }
    update();
}

void RingMeter::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    const int w = width();
    const int h = height();
    const int side = std::min(w, h);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.translate(w / 2, h / 2);
    painter.scale(side / 200.0, side / 200.0);

    drawRing(&painter);
    drawScale(&painter);
    drawScaleNum(&painter);
    drawPointer(&painter);
    drawValue(&painter);
    drawName(&painter);
}

void RingMeter::drawRing(QPainter *painter)
{
    int radius = 100;
    painter->save();

    QPen pen;
    pen.setCapStyle(Qt::FlatCap);
    pen.setWidthF(m_ringWidth);
    radius = radius - m_ringWidth;
    QRectF rect = QRectF(-radius, -radius, radius * 2, radius * 2);

    const double angleAll = 360.0 - m_startAngle - m_endAngle;
    const double angleStart = angleAll * m_ringStartPercent / 100;
    const double angleMid = angleAll * m_ringMidPercent / 100;
    const double angleEnd = angleAll * m_ringEndPercent / 100;

    pen.setColor(m_ringColorStart);
    painter->setPen(pen);
    painter->drawArc(rect, (270 - m_startAngle - angleStart) * 16, angleStart * 16);

    pen.setColor(m_ringColorMid);
    painter->setPen(pen);
    painter->drawArc(rect, (270 - m_startAngle - angleStart - angleMid) * 16, angleMid * 16);

    pen.setColor(m_ringColorEnd);
    painter->setPen(pen);
    painter->drawArc(rect, (270 - m_startAngle - angleStart - angleMid - angleEnd) * 16, angleEnd * 16);
    painter->restore();
}

void RingMeter::drawScale(QPainter *painter)
{
    int radius = 94;
    painter->save();

    QPen pen;
    pen.setColor(m_textColor);
    pen.setCapStyle(Qt::RoundCap);
    painter->rotate(m_startAngle);

    const int steps = (m_scaleMajor * m_scaleMinor);
    const double angleStep = (360.0 - m_startAngle - m_endAngle) / steps;

    const int indexStart = steps * m_ringStartPercent / 100 + 1;
    const int indexMid = steps * m_ringMidPercent / 100 - 1;
    const int indexEnd = steps * m_ringEndPercent / 100 + 1;
    int index = 0;

    for(int i = 0; i <= steps; ++i)
    {
        if(i % m_scaleMinor == 0)
        {
            if(index < indexStart)
            {
                pen.setColor(m_ringColorStart);
            }
            else if(index < (indexStart + indexMid))
            {
                pen.setColor(m_ringColorMid);
            }
            else if(index < (indexStart + indexMid + indexEnd))
            {
                pen.setColor(m_ringColorEnd);
            }

            index++;
            pen.setWidthF(1.5);
            painter->setPen(pen);
            painter->drawLine(0, radius - 13, 0, radius);
        }
        else
        {
            pen.setWidthF(0.5);
            painter->setPen(pen);
            painter->drawLine(0, radius - 5, 0, radius);
        }
        painter->rotate(angleStep);
    }
    painter->restore();
}

void RingMeter::drawScaleNum(QPainter *painter)
{
    constexpr int radius = 70;
    painter->save();
    painter->setPen(m_textColor);

    const double startRad = (360 - m_startAngle - 90) * (M_PI / 180);
    const double deltaRad = (360 - m_startAngle - m_endAngle) * (M_PI / 180) / m_scaleMajor;

    for(int i = 0; i <= m_scaleMajor; ++i)
    {
        const double sina = qSin(startRad - i * deltaRad);
        const double cosa = qCos(startRad - i * deltaRad);
        const double value = 1.0 * i * ((m_maxValue - m_minValue) / m_scaleMajor) + m_minValue;
        const QString &strValue = QString("%1").arg(value, 0, 'f', 0);
        const int textWidth = fontMetrics().horizontalAdvance(strValue);
        const int textHeight = fontMetrics().height();
        const int x = radius * cosa - textWidth / 2.0;
        const int y = -radius * sina + textHeight / 4.0;
        painter->drawText(x, y, strValue);
    }
    painter->restore();
}

void RingMeter::drawPointer(QPainter *painter)
{
    constexpr int radius = 62;
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(m_pointerColor);

    QPolygon pts;
    pts.setPoints(4, -5, 0, 0, -8, 5, 0, 0, radius);
    painter->rotate(m_startAngle);

    const double degRotate = (360.0 - m_startAngle - m_endAngle) / (m_maxValue - m_minValue) * (m_currentValue - m_minValue);
    painter->rotate(degRotate);
    painter->drawConvexPolygon(pts);
    painter->restore();
}


void RingMeter::drawValue(QPainter *painter)
{
    constexpr int radius = 100;
    constexpr int horizontalOffset = 30;
    painter->save();
    painter->setPen(m_textColor);

    QFont font;
    font.setPixelSize(18);
    painter->setFont(font);

    const QRectF textRect(-radius, -radius + horizontalOffset, radius * 2, radius * 2);
    painter->drawText(textRect, Qt::AlignCenter, QString("%1%").arg(m_currentValue, 0, 'f', m_precision));
    painter->restore();
}


void RingMeter::drawName(QPainter *painter)
{
    constexpr int radius = 100;
    constexpr int horizontalOffset = 80;
    painter->save();
    painter->setPen(m_nameColor);

    QFont font;
    font.setPixelSize(25);
    font.setBold(true);
    painter->setFont(font);

    const QRectF textRect(-radius, -radius + horizontalOffset, radius * 2, radius * 2);
    painter->drawText(textRect, Qt::AlignCenter, m_name);
    painter->restore();
}

void RingMeter::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit onClicked();  // Emit the custom signal
    }
    else
    {
        event->ignore();
    }
    QWidget::mousePressEvent(event);  // Call base class handler
}
