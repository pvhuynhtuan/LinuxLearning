#ifndef CIRCULARPROGRESSBAR_H
#define CIRCULARPROGRESSBAR_H

#include <QWidget>

class CircularProgressBar : public QWidget {
    Q_OBJECT

public:
    explicit CircularProgressBar(QWidget *parent = nullptr);
    void setValue(int value); // 0 to 100

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int m_value;  // current progress
};

#endif // CIRCULARPROGRESSBAR_H
