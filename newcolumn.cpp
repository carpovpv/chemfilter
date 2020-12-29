#include "newcolumn.h"
#include "ui_newcolumn.h"

#include <QMessageBox>
#include <QScriptEngine>
#include <QProgressDialog>

#include <QDebug>

NewColumn::NewColumn(QWidget *parent, QsarTableModel *mdl) :
    QDialog(parent),
    ui(new Ui::NewColumn)
{
    ui->setupUi(this);
    m_mdl = mdl;
    engine = new QScriptEngine(this);

    connect(ui->btnOk, SIGNAL(clicked(bool)),
            this, SLOT(ok()));
    connect(ui->btnCancel, SIGNAL(clicked(bool)),
            this, SLOT(reject()));
}

NewColumn::~NewColumn()
{
    delete ui;
}

const QString NewColumn::getVariable() const
{
    return varName;
}

void NewColumn::ok()
{
    varName = ui->txtVariable->text().trimmed();
    script = ui->txtScript->toPlainText();

    if(varName.isEmpty())
    {
        QMessageBox::warning(this, "Error",
                              "Please, provide a variable's name for the new column.");
        return;
    }

    if(m_mdl->cols.contains( varName ) )
    {
        QMessageBox::warning(this, "Error", "There is already a column with this name.\n"
                                             "Please, provide another name to continue.");

        return;
    }

    if(!script.isEmpty())
    {
        //test run
        for(int i=3; i< m_mdl->cols.size(); i++)
            engine->globalObject().setProperty(m_mdl->cols[i], "1");

        QScriptValue res = engine->evaluate(script);
        if(engine->hasUncaughtException())
        {
            int line = engine->uncaughtExceptionLineNumber();
            QMessageBox::critical(this, "Error",
                                  "Uncaught exception on line " + QString::number(line) + " : " + res.toString());
            return;
        }
    }
    //end test run

    int col = m_mdl->addColumnModel(varName);

    QProgressDialog * progress = new QProgressDialog("Calculating", "Stop", 0, m_mdl->rowCount(), this);
    progress->setWindowModality(Qt::WindowModal);

    for(int row=0; row< m_mdl->rowCount(); row++)
    {
        if(!script.isEmpty())
        {
            for(int i=3; i< m_mdl->cols.size(); i++)
                engine->globalObject().setProperty(m_mdl->cols[i], m_mdl->index(row, i).data().toDouble());

            QScriptValue res = engine->evaluate(script);
            if(!engine->hasUncaughtException())
                m_mdl->setData( m_mdl->index(row, col), res.toVariant(), Qt::DisplayRole);
        }

        progress->setValue(row);
        if (progress->wasCanceled())
            break;
    }

    progress->setValue(m_mdl->rowCount());
    progress->deleteLater();

    accept();
}
