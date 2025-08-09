#ifndef STATUSCIRCLE_H
#define STATUSCIRCLE_H

#include <QWidget>
#include <string>

#define STATUS_CIRCLE_DIAL_NUM  50

class StatusCircle : public QWidget {
    Q_OBJECT

public:
    // Constructor and destructor
    explicit StatusCircle(QWidget *parent = nullptr);
    ~StatusCircle();

    // Public method
    void setColor(const QColor &color);
    void setRange(double minValue, double maxValue);

    void setMinValue(double minValue);
    void setMaxValue(double maxValue);

    void setValue(double value);

    void setAngle(double angle);

    void setPrecision(int precision);

    void setUnit(const QString &unit);

    void setUsedColor(const QColor &usedColor);
    void setFreeColor(const QColor &freeColor);

    void setRangeTextColor(const QColor &rangeTextColor);
    void setValueTextColor(const QColor &valueTextColor);

    void setValueBackgroundColor(const QColor &valueBackgroundColor);
    void setOutBackgroundColor(const QColor &outBackgroundColor);

    void setCenterBackgroundColorStart(const QColor &centerBackgroundColorStart);
    void setCenterBackgroundColorEnd(const QColor &centerBackgroundColorEnd);

    virtual QSize sizeHint() const override final;

private Q_SLOTS:
    void updateValue();

protected:
    //void paintEvent(QPaintEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override final;

    void drawDial(QPainter *painter);
    void drawBackgroundOut(QPainter *painter);
    void drawBackgroundRound(QPainter *painter);
    void drawBackgroundCenter(QPainter *painter);
    void drawText(QPainter *painter);

    double m_minValue;
    double m_maxValue;
    double m_value;
    int m_precision;

    double m_angle;
    QString m_unit;

    QColor m_usedColor;
    QColor m_freeColor;

    QColor m_rangeTextColor;
    QColor m_valueTextColor;

    QColor m_valueBackgroundColor;
    QColor m_outBackgroundColor;
    QColor m_centerBackgroundColorStart;
    QColor m_centerBackgroundColorEnd;

    double m_currentPercent;
    double m_valuePercent;
    QTimer *m_timer;

// private:
//     QColor m_color;
};

#endif // STATUSCIRCLE_H
