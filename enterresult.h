#ifndef ENTERRESULT_H
#define ENTERRESULT_H

#include <QDialog>

namespace Ui {
class EnterResult;
}

class EnterResult : public QDialog
{
    Q_OBJECT

public:
    explicit EnterResult(QWidget *parent, const QString &d);
    ~EnterResult();

    QString nv;

private:
    Ui::EnterResult *ui;
private slots:

    void ok();
};

#endif // ENTERRESULT_H
