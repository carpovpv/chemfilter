#ifndef LAP_CHECKBOX_H
#define LAP_CHECKBOX_H

#include <QtGui>
#include <QItemDelegate>
#include <QApplication>
#include <QtWidgets/QCheckBox>

class LAP_CheckBoxDelegate : public QItemDelegate
{
public:
    LAP_CheckBoxDelegate(QObject *parent = 0, QString states = "2")
        : QItemDelegate(parent)
    {
        m_states = states;
    }
    virtual void drawCheck(QPainter *painter, const QStyleOptionViewItem &option,
                           const QRect &, Qt::CheckState state) const
    {
        const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;

        QRect checkRect = QStyle::alignedRect(option.direction, Qt::AlignCenter,
            doCheck(option, option.rect, Qt::Checked).size(),
            QRect(option.rect.x() + textMargin, option.rect.y(),
            option.rect.width() - (textMargin * 2), option.rect.height()));
        QItemDelegate::drawCheck(painter, option, checkRect, state);
    }
    virtual bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                             const QModelIndex &index)
    {
        Q_ASSERT(event);
        Q_ASSERT(model);

        // make sure that the item is checkable
        Qt::ItemFlags flags = model->flags(index);
        if (!(flags & Qt::ItemIsUserCheckable) || !(flags & Qt::ItemIsEnabled))
            return false;
        // make sure that we have a check state
        QVariant value = index.data(Qt::CheckStateRole);
        if (!value.isValid())
            return false;
        // make sure that we have the right event type
        if (event->type() == QEvent::MouseButtonRelease) {
            const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
            QRect checkRect = QStyle::alignedRect(option.direction, Qt::AlignCenter,
                doCheck(option, option.rect, Qt::Checked).size(),
                QRect(option.rect.x() + textMargin, option.rect.y(),
                option.rect.width() - (2 * textMargin), option.rect.height()));
            if (!checkRect.contains(static_cast<QMouseEvent*>(event)->pos()))
                return false;
        } else if (event->type() == QEvent::KeyPress) {
            if (static_cast<QKeyEvent*>(event)->key() != Qt::Key_Space
                && static_cast<QKeyEvent*>(event)->key() != Qt::Key_Select)
                return false;
        } else {
            return false;
        }
        Qt::CheckState state = Qt::Unchecked;
        if(m_states=="3")
        {
            switch(value.toInt())
            {
                case Qt::Checked:
                    state = Qt::PartiallyChecked;
                    break;
                case Qt::PartiallyChecked:
                    state = Qt::Unchecked;
                    break;
                case Qt::Unchecked:
                    state = Qt::Checked;
                    break;
            }
        }
        else
        {
            if(value.toInt() == Qt::Checked)
                state = Qt::Unchecked;
            else
                state = Qt::Checked;
        }

        return model->setData(index, state, Qt::CheckStateRole);
    }

    virtual void drawFocus(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect) const
    {
        Q_UNUSED(rect);
        QItemDelegate::drawFocus(painter, option, option.rect);
    }
private:
    QString m_states;
};


#endif // LAP_CHECKBOX_H
