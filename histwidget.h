#ifndef HISTWIDGET_H
#define HISTWIDGET_H

#include <QObject>
#include <QWidget>
#include <QPaintEvent>

class HistWidget : public QWidget
{
    Q_OBJECT
public:
    HistWidget(QWidget * parent);
    void setData(QVector<double> * data,
                 double min,
                 double max,
                 double interval);

    QSize sizeHint();

protected:
    void paintEvent(QPaintEvent *);

private:

    QVector<double> * m_data;
    QVector<double> m_x;
    QVector<int> m_h;
    int m_interval;
    double m_min;
    double m_max;

    double y_max;
};

#endif // HISTWIDGET_H
