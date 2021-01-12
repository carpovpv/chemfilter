#ifndef NEWCOLUMN_H
#define NEWCOLUMN_H

#include <QDialog>
#include <QScriptEngine>

#include "qsartablemodel.h"

namespace Ui {
class NewColumn;
}

class NewColumn : public QDialog
{
    Q_OBJECT

public:
    explicit NewColumn(QWidget *parent, QsarTableModel *mdl);
    ~NewColumn();

    const QString getVariable() const;

private:
    Ui::NewColumn *ui;
    QsarTableModel *m_mdl;
    QScriptEngine *engine;

    QString varName;
    QString script;

private slots:

    void ok();
    void selectFile();
};

#endif // NEWCOLUMN_H
