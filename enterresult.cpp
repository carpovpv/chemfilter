#include "enterresult.h"
#include "ui_enterresult.h"

#include <QMessageBox>

EnterResult::EnterResult(QWidget *parent, const QString &d) :
    QDialog(parent),
    ui(new Ui::EnterResult)
{
    ui->setupUi(this);
    ui->txtValue->setText(d);

    connect(ui->btnOk, SIGNAL(clicked(bool)),
            this, SLOT(ok()));
    connect(ui->btnCancel, SIGNAL(clicked(bool)),
            this, SLOT(reject()));

    ui->txtValue->setFocus();
}

EnterResult::~EnterResult()
{
    delete ui;
}

void EnterResult::ok()
{
    nv = ui->txtValue->text().trimmed();
    if(nv.isEmpty())
    {
        int ret = QMessageBox::question(this, "Erase?", "The entered value is empty. Continue?",
                                        QMessageBox::Yes | QMessageBox::No);
        if(ret == QMessageBox::Yes)
            accept();
    }
    else
        accept();
}
