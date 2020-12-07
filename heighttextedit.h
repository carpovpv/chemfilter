#ifndef HEIGHTTEXTEDIT_H
#define HEIGHTTEXTEDIT_H

#include <QPlainTextEdit>

class HeightTextEdit : public QPlainTextEdit
{
public:
    HeightTextEdit(QWidget * parent);

    QSize sizeHint() const;
};

#endif // HEIGHTTEXTEDIT_H
