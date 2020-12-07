#include "heighttextedit.h"

#include <QDebug>

HeightTextEdit::HeightTextEdit(QWidget * parent)
    :QPlainTextEdit(parent)
{

}

QSize HeightTextEdit::sizeHint() const
{
    QSize s = QPlainTextEdit::sizeHint();
    s.setHeight(60);

    return s;
}
