#include "newcolumn.h"
#include "ui_newcolumn.h"

#include <QMessageBox>
#include <QScriptEngine>
#include <QProgressDialog>
#include <QFileDialog>

#include <openbabel/obconversion.h>
#include <openbabel/mol.h>
#include <openbabel/data.h>
#include <openbabel/op.h>
#include <openbabel/generic.h>
#include<openbabel/fingerprint.h>

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
    connect(ui->btnSelectFile, SIGNAL(clicked(bool)),
            this, SLOT(selectFile()));
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

    QString sdfFile = ui->txtFile->text().trimmed();
    if(QFile::exists(sdfFile))
    {
        if(!script.isEmpty())
        {
            QMessageBox::warning(this, "Error", "You can calculate Tanimoto distance to structures in the file "
                                                "or evaluate a script, not both.");
            return;
        }

        OpenBabel::OBFingerprint *pFP = OpenBabel::OBFingerprint::FindFingerprint("FP2");
        if(!pFP)
        {
            QMessageBox::critical(this, "Error",
                                  "Check OpenBabel installation. FP2 plugin was not loaded.");
            return;
        }

        std::ifstream sdf(sdfFile.toUtf8().data(),
                          std::ifstream::in);

        if(!sdf.is_open())
        {
            QMessageBox::critical(this, tr("Error"),
                                  tr("Error opening file. Please, check path and permissions."));
            return;
        }

        OpenBabel::OBConversion conv(&sdf, NULL);
        if(conv.SetInAndOutFormats("SDF","SMILES"))
        {
            std::vector< std::vector<unsigned int> > fps;

            OpenBabel::OBMol mol;
            while(conv.Read(&mol))
            {
                std::vector<unsigned int > fp;
                pFP->GetFingerprint(&mol,fp);

                fps.push_back(fp);
            }

            if(fps.size() == 0)
            {
                QMessageBox::critical(this, "Error",
                                      "Cache of calculated fingerprints is zero-size.");
                return;
            }

            conv.SetInFormat("SMILES");

            int col = m_mdl->addColumnModel(varName);

            QProgressDialog * progress = new QProgressDialog("Calculating",
                                                             "Stop", 0, m_mdl->rowCount(), this);
            progress->setWindowModality(Qt::WindowModal);

            for(int row=0; row< m_mdl->rowCount(); row++)
            {
                QString smiles = m_mdl->index(row, 2).data().toString();

                std::stringstream is;
                is.str(smiles.toLatin1().data());

                conv.SetInStream(&is);
                double d = 0.0;

                if(conv.Read(&mol))
                {
                    std::vector<unsigned int> fp;
                    pFP->GetFingerprint(&mol,fp);

                    for(unsigned int i=0; i< fps.size(); i++)
                    {
                        double tanimoto = OpenBabel::OBFingerprint::Tanimoto(fp, fps[i]);
                        if(tanimoto > d)
                            d = tanimoto;
                    }

                    m_mdl->setData( m_mdl->index(row, col), d, Qt::DisplayRole);
                }

                progress->setValue(row);
                if (progress->wasCanceled())
                    break;
            }

            progress->setValue(m_mdl->rowCount());
            progress->deleteLater();

        }

        accept();
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

void NewColumn::selectFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                     QDir::homePath(),
                                                     tr("SDF files (*.sdf)"));
    ui->txtFile->setText(fileName);
}
