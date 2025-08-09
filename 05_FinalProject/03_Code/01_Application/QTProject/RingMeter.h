#ifndef RINGMETER_H
#define RINGMETER_H

#include <QWidget>
#define RINGMETER_MS    1

class RingMeter : public QWidget
{
    Q_OBJECT

signals:
    void onClicked();  // Custom signal for click action

protected:
    void mousePressEvent(QMouseEvent *event) override;

public:
    explicit RingMeter(QWidget *parent = nullptr);
    ~RingMeter();

    void setRange(double minValue, double maxValue);

    void setMinValue(double minValue);
    void setMaxValue(double maxValue);

    void setValue(double value);
    void setName(QString name);

    void setPrecision(int precision);
    void setScaleMajor(int scaleMajor);
    void setScaleMinor(int scaleMinor);
    void setStartAngle(int startAngle);
    void setEndAngle(int endAngle);

    void setAnimation(bool animation);
    void setAnimationStep(double animationStep);

    void setRingWidth(int ringWidth);
    void setRingStartPercent(int ringStartPercent);
    void setRingMidPercent(int ringMidPercent);
    void setRingEndPercent(int ringEndPercent);

    void setRingColorStart(const QColor &ringColorStart);
    void setRingColorMid(const QColor &ringColorMid);
    void setRingColorEnd(const QColor &ringColorEnd);

    void setPointerColor(const QColor &pointerColor);
    void setTextColor(const QColor &textColor);
    void setNameColor(const QColor &nameColor);

    virtual QSize sizeHint() const override final;

private Q_SLOTS:
    void updateValue();

private:
    virtual void paintEvent(QPaintEvent *event) override final;

    void drawRing(QPainter *painter);
    void drawScale(QPainter *painter);
    void drawScaleNum(QPainter *painter);
    void drawPointer(QPainter *painter);
    void drawValue(QPainter *painter);
    void drawName(QPainter *painter);

    double m_minValue;
    double m_maxValue;
    double m_value;
    QString m_name;
    int m_precision;
    int m_scaleMajor;
    int m_scaleMinor;
    int m_startAngle;
    int m_endAngle;
    bool m_animation;
    double m_animationStep;

    int m_ringWidth;
    int m_ringStartPercent;
    int m_ringMidPercent;
    int m_ringEndPercent;

    QColor m_ringColorStart;
    QColor m_ringColorMid;
    QColor m_ringColorEnd;
    QColor m_pointerColor;
    QColor m_textColor;
    QColor m_nameColor;

    bool m_reverse;
    double m_currentValue;
    QTimer *m_timer;
};

#endif // RINGMETER_H
