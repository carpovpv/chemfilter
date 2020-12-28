#include "histwidget.h"

#include <QPainter>
#include <QDebug>

HistWidget::HistWidget(QWidget *parent)
{
    setMinimumHeight(150);
}

void HistWidget::setData(QVector<double> *data,
                         double min,
                         double max,
                         double interval)
{
    m_data = data;
    m_min = min;
    m_max = max;
    m_interval = interval;

    m_h.clear();
    m_x.clear();

    const double dx = (m_max - m_min) / m_interval;
    for(int i=0 ; i< m_interval; i++)
    {
        m_x.push_back(m_min + i*dx);
        m_h.push_back(0);
    }

    for(int i=0; i< m_data->size(); i++)
    {
        double x = m_data->at(i);
        for(int j=0; j< m_x.size(); j++)
            if( x > m_x[j] && x <= (m_x[j] + dx))
                m_h[j]++;
    }

    y_max = m_h[0];
    for(int i=1; i< m_h.size(); i++)
        if(m_h[i] > y_max) y_max = m_h[i];

    repaint();
}

QSize HistWidget::sizeHint()
{
    return QSize(100, 150);
}

void HistWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QRect rc = rect();

    int left = rc.left() + 2;
    int right = rc.right() - 2;
    int top = rc.top() + 2;
    int bottom = rc.bottom() - 3;

    painter.setPen(QPen(Qt::blue));
    painter.drawLine(left, bottom, right+2, bottom);
    painter.drawLine(left, bottom, left, top);

    if(m_x.size() == 0)
        return;

    QPen pen(Qt::red);
    QBrush brush(Qt::red, Qt::DiagCrossPattern);
    painter.setPen(pen);
    painter.setBrush(brush);

    const int dx = (right - left) / m_interval;
    for(int i=0; i< m_x.size(); i++)
    {
        int y = (bottom - top) / 100.0 * (100.0 - 100.0 * m_h[i] / y_max) + top;
        if(y >= bottom)
            continue;

        painter.drawRect(i*dx + 3, y, (i == m_x.size() -1 ? (right - dx * (m_x.size() - 1)) : dx), bottom - y-1);
    }


}
