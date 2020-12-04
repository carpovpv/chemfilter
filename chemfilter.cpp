#include "chemfilter.h"
#include "ui_chemfilter.h"

#include <QLabel>
#include <QFileDialog>
#include <QSettings>
#include <QDir>
#include <QProgressDialog>
#include <QMessageBox>
#include <QThread>
#include <QDebug>
#include <QUuid>
#include <QInputDialog>
#include <QEventLoop>
#include <QMenu>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>

#include <openbabel/mol.h>
#include <openbabel/obconversion.h>
#include <openbabel/generic.h>
#include <openbabel/depict/depict.h>
#include <openbabel/op.h>
#include <openbabel/parsmart.h>

#include <QTableWidgetItem>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QScriptEngine>

#include <fstream>

#include "delegate.h"
#include "clearthread.h"
#include "qsartablemodel.h"
#include "transformermodel.h"
#include "lap_checkbox.h"
#include "newcolumn.h"
#include "enterresult.h"

using namespace OpenBabel;

double student(int freedom)
{
    //two-sided 95%
    static const double tbl []= {
        -1.0,
        12.71, 4.303, 3.182, 2.776, 2.571,
        2.447, 2.365, 2.306, 2.262, 2.228,
        2.201, 2.179, 2.160, 2.145, 2.131,
        2.120, 2.110, 2.101, 2.093, 2.086,
        2.080, 2.074, 2.069, 2.064, 2.060,
        2.056, 2.052, 2.048, 2.045, 2.042
    };

    if(freedom > 30)
        return 1.960;

    return tbl [freedom];
}

ChemFilter::ChemFilter(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ChemFilter)
{
    ui->setupUi(this);

    engine = new QScriptEngine(this);

    stsInfo = new QLabel(this);
    stsInfo->setText("Ready to use!");

    stsMsu = new QLabel(this);
    stsMsu->setText(tr("BIGCHEM"));

    stsCenter = new QLabel(this);
    stsCenter->setText(tr(""));

    ui->statusBar->addWidget(stsInfo);
    ui->statusBar->addWidget(stsCenter, 2);
    ui->statusBar->addWidget(stsMsu);

    iconYes = QIcon(":/images/ok.png");
    iconNo = QIcon(":/images/no.png");

    QThread *thread = new ClearThread();
    thread->start();

    sortModel  = new QsarSortModel(this, engine);
    checkBoxDelegate = new LAP_CheckBoxDelegate(this);
    molDelegate = new MolDelegate(this);

    settings = new QSettings(this);

    QMenu * exportMenu = new QMenu(this);
    QAction * actSdf = new QAction("Export SDF", this);
    connect(actSdf, SIGNAL(triggered(bool)),
            this, SLOT(exportSdf()));
    exportMenu->addAction(actSdf);

    QAction * actCsv = new QAction("Export CSV", this);
    connect(actCsv, SIGNAL(triggered(bool)),
            this, SLOT(exportCSV()));
    exportMenu->addAction(actCsv);

    ui->btnExportSDF->setMenu(exportMenu);

    clearStats();
    LoadModels();

    connect(ui->btnOpenSdf, SIGNAL(clicked()),
            this, SLOT(LoadSdf()));
    connect(ui->actExit, SIGNAL(triggered()),
            this, SLOT(close()));

    connect(ui->btnCalculate, SIGNAL(clicked(bool)),
            this, SLOT(calculate()));
    connect(ui->btnStopCalc, SIGNAL(clicked(bool)),
            this, SLOT(stopCalc()));

    connect(ui->btnAddColumn, SIGNAL(clicked()),
            this, SLOT(addNewColumn()));
    connect(ui->btnDelete, SIGNAL(clicked(bool)),
            this, SLOT(deleteColumn()));
    connect(ui->btnStats, SIGNAL(clicked(bool)),
            this, SLOT(analyzeProperty()));

    connect(ui->btnDown, SIGNAL(clicked(bool)),
            this, SLOT(setDown()));
    connect(ui->btnUp, SIGNAL(clicked(bool)),
            this, SLOT(setUp()));

    connect(ui->btnFilter, SIGNAL(clicked(bool)),
            this, SLOT(filter()));

    connect(ui->tblData, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(enterValue(QModelIndex)));
    connect(ui->btnSave, SIGNAL(clicked(bool)),
            this, SLOT(save()));
}

ChemFilter::~ChemFilter()
{
    delete ui;
}

void ChemFilter::LoadModels()
{
    QDir dir;
    dir.setPath(qApp->applicationDirPath() + "/models");
    dir.setFilter(QDir::Dirs | QDir::Hidden | QDir::NoSymLinks);

    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); ++i)
    {
        QString subFolder = list[i].baseName();
        if(subFolder.isEmpty())
            continue;

        struct ModelInfo inf;
        inf.dirName = qApp->applicationDirPath() + "/models/" + subFolder;

        QString infoFile = inf.dirName + "/info";
        QFile fp(infoFile);
        if(!fp.open(QIODevice::ReadOnly))
        {
            qDebug() << "info file is missing for " << subFolder << "?";
            continue;
        }

        QString data = fp.readAll();
        QStringList lst = data.split("\n");

        if(lst.size() > 3)
        {
            inf.category = lst[0].trimmed();
            inf.name = lst[1].trimmed();
            inf.var = lst[2].trimmed();

            for(int i=3; i < lst.size(); i++)
            {
                struct Interval interval;
                QStringList arr = lst[i].simplified().split(" ");
                if(arr.size() > 2)
                {
                    interval.percent = arr[0].toDouble();
                    interval.low = arr[1].toDouble();
                    interval.high = arr[2].toDouble();

                    inf.intervals.push_back(interval);
                }
            }
        }

        QDir modelDir;
        modelDir.setPath(inf.dirName);
        modelDir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

        QFileInfoList list2 = modelDir.entryInfoList();
        for (int i = 0; i < list2.size(); ++i)
        {
            QFileInfo fileInfo = list2.at(i);
            QString mdlName = fileInfo.absoluteFilePath();
            if(mdlName.endsWith(".trm"))
               inf.fnames << mdlName;
        }

        if(inf.fnames.size())
        {
            int idx = models.size();
            models.push_back(inf);

            QTreeWidgetItem * par = nullptr;
            if(catItems.contains(inf.category))
               par = catItems[inf.category];
            else
            {
                par = new QTreeWidgetItem(ui->lstModels, {inf.category});
                catItems[inf.category] = par;
            }

            QTreeWidgetItem *it = new QTreeWidgetItem(par, {inf.name});
            it->setCheckState(0, Qt::Unchecked);
            it->setData(0, Qt::UserRole, idx);

            inf.printInfo();
        }

    }

    ui->lstModels->sortItems(0, Qt::AscendingOrder);

    qDebug() << "Loaded " << models.size() << " models.";
}

void ChemFilter::LoadSdf()
{

    sdfFileName.clear();

    if(ui->tblData->model())
    {
        QSortFilterProxyModel * pmodel = static_cast<QSortFilterProxyModel*>
                (ui->tblData->model());
        if(pmodel)
           delete pmodel->sourceModel();
        molDelegate->setModel(nullptr);
    }

    QString sdfdir = settings->value("sdf-home-dir", QDir::homePath()).toString();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    sdfdir,
                                                    tr("SDF file (*.sdf *.txt)"));
    if(fileName.isEmpty())
        return;

    QsarTableModel * mdl = new QsarTableModel(this);
    mdl->loadDataFromFile(fileName, this);
    molDelegate->setModel(mdl);

    sdfFileName = fileName;

    sortModel->setSourceModel(mdl);

    ui->tblData->setModel(sortModel);
    ui->tblData->horizontalHeader()->setVisible(true);

    ui->tblData->setItemDelegateForColumn(0, checkBoxDelegate);
    ui->tblData->setItemDelegateForColumn(1, molDelegate);

    ui->tblData->setColumnWidth(0, 30);
    ui->tblData->setColumnWidth(1, 250);

    ui->lstCols->clear();
    for(int i = 3; i< mdl->cols.size(); i++)
    {
        QListWidgetItem *it = new QListWidgetItem(mdl->cols[i], ui->lstCols);
        it->setCheckState(Qt::Checked);
        it->setData(Qt::UserRole, i);
    }

    connect(ui->lstCols, SIGNAL(itemChanged(QListWidgetItem*)),
            this, SLOT(itemChanged(QListWidgetItem *)));

    int y = fileName.lastIndexOf("/");
    QString path = fileName.mid(0,y);
    settings->setValue("sdf-home-dir",path);

    stsInfo->setText(tr("Loaded %1 molecules.").arg(mdl->rowCount()));

    setDown();
}

void ChemFilter::exportSdf()
{
    QsarTableModel * mdl = static_cast<QsarTableModel*>( sortModel->sourceModel());
    if(! mdl )
    {
        QMessageBox::critical(this, "Error", "No data. Please load data into the table.");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                                QDir::homePath(),
                                tr("SDF (*.sdf)"));
    if(fileName.isEmpty())
        return;

    if(!fileName.endsWith(".sdf"))
        fileName += ".sdf";

    std::ofstream fn(fileName.toStdString());

    OpenBabel::OBConversion conv(NULL, &fn);
    conv.SetOutFormat("SDF");

    for(int i=0; i< sortModel->rowCount(); i++)
    {
        QModelIndex ind = sortModel->mapToSource( sortModel->index(i, 0) );

        if(mdl->data(ind, Qt::EditRole).toString() != "Y")
            continue;

        ind = sortModel->mapToSource( sortModel->index(i, 1) );
        int m = mdl->data(ind, Qt::DisplayRole).toInt();

        OpenBabel::OBMol mol = mdl->mols[m];
        auto data = mol.GetData();
        mol.DeleteData(data);

        for(int c = 3; c< mdl->cols.size(); c++)
        {
            OpenBabel::OBPairData  * data = new OpenBabel::OBPairData();
            data->SetAttribute(mdl->cols[c].toStdString());

            ind = sortModel->mapToSource( sortModel->index(i, c) );
            data->SetValue(mdl->data(ind, Qt::DisplayRole).toString().toStdString());

            data->SetOrigin(OpenBabel::userInput);
            mol.SetData(data);
        }

        conv.Write(&mol);
    }

    fn.close();
}

void ChemFilter::exportCSV()
{
    QsarTableModel * mdl = static_cast<QsarTableModel*>( sortModel->sourceModel());
    if(! mdl )
    {
        QMessageBox::critical(this, "Error", "No data. Please load data into the table.");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                                QDir::homePath(),
                                tr("CSV (*.csv)"));
    if(fileName.isEmpty())
        return;

    if(!fileName.endsWith(".csv"))
        fileName += ".csv";

    QFile data(fileName);
    if (!data.open(QFile::WriteOnly | QFile::Truncate))
    {
        QMessageBox::critical(this, "Error", "Unable to open the file for writing.");
        return;
    }

    QTextStream out(&data);

    auto escape = [&] (const QString &d)
    {
        QString a = d;
        a.replace("\"", "\"\"");

        return a;
    };

    for(int c = 2; c< mdl->cols.size(); c++)
    {
        out << "\"" << escape(mdl->cols[c]) << "\"";
        if(c < mdl->cols.size() -1)
            out << ",";
        else
            out << "\n";
    }

    for(int i=0; i< sortModel->rowCount(); i++)
    {
        QModelIndex ind = sortModel->mapToSource( sortModel->index(i, 0) );
        if(mdl->data(ind, Qt::EditRole).toString() != "Y")
            continue;

        for(int c = 2; c< mdl->cols.size(); c++)
        {
            ind = sortModel->mapToSource( sortModel->index(i, c) );
            out << "\"" << escape(mdl->data(ind, Qt::DisplayRole).toString()) << "\"";
            if(c < mdl->cols.size() -1)
                out << ",";
            else
                out << "\n";

        }
    }
}

void ChemFilter::calculate()
{
    QsarTableModel * mdl = static_cast<QsarTableModel*>( sortModel->sourceModel());
    if(! mdl )
    {
        QMessageBox::critical(this, "Error", "No data. Please load data into the table.");
        return;
    }

    ui->btnCalculate->setEnabled(false);
    ui->btnDelete->setEnabled(false);

    QList<QTreeWidgetItem *> items;
    for( int i = 0; i < ui->lstModels->topLevelItemCount(); ++i )
    {
       QTreeWidgetItem *item = ui->lstModels->topLevelItem( i );
       for(int j=0; j < item->childCount(); j++)
       {
           QTreeWidgetItem * child = item->child(j);
           if(child->data(0, Qt::CheckStateRole) == Qt::Checked)
               items.push_back(child);
       }
    }

    for(int m =0; m < items.size(); m++)
    {
        int idx = items[m]->data(0, Qt::UserRole).toInt();
        if(models.size() > 0 && idx >=0 && idx < models.size())
            qDebug() << "\t" << models[idx].name;
    }

    if(items.size() == 0)
    {
        ui->btnCalculate->setEnabled(true);
        ui->btnDelete->setEnabled(true);
        return;
    }

    ui->prg->setMaximum( items.size() * mdl->rowCount());

    long cnt = 0;
    m_stop = false;

    for(int m =0; m < items.size(); m++ )
    {
        int idx = items[m]->data(0, Qt::UserRole).toInt();
        QString var = models[idx].var;

        QString property = var;
        QString property_error = var + "_d";

        if(mdl->cols.contains( property) ||
           mdl->cols.contains( property_error))
        {
            QMessageBox::critical(this, "Error",
                                  "There is already a column with this name.");

            cnt += mdl->rowCount();
            ui->prg->setValue(cnt);
            continue;
        }

        int il = mdl->addColumnModel(property);
        int ih = mdl->addColumnModel(property_error);

        QListWidgetItem *it = new QListWidgetItem(property, ui->lstCols);
        it->setCheckState(Qt::Checked);
        it->setData(Qt::UserRole, il);

        it = new QListWidgetItem(property_error, ui->lstCols);
        it->setCheckState(Qt::Checked);
        it->setData(Qt::UserRole, ih);

        QVector<TransformerModel *> mdls;

        for(int f= 0; f < models[idx].fnames.size(); f++)
        {
            TransformerModel * sol = new TransformerModel(models[idx].fnames[f].toLocal8Bit().data());
            if(!sol->isGood())
                continue;

            mdls.push_back(sol);
        }

        if(mdls.size() == 0)
        {
            QMessageBox::critical(this, "Error", "Damage files or not enough memory!");

            ui->btnCalculate->setEnabled(true);
            ui->btnDelete->setEnabled(true);

            ui->prg->setMaximum( items.size() * mdl->rowCount());
        }
        else
        {
            for(int c=0; c< mdl->rowCount(); c++)
            {
                //smiles column
                QModelIndex ind = mdl->index(c, 2);

                QModelIndex idx_l = mdl->index(c, il);
                QModelIndex idx_h = mdl->index(c, ih);

                int max_n;
                std::set<std::string> ms;

                const std::string smiles = mdl->data(ind, Qt::DisplayRole).toString().toStdString();

                if(GetRandomSmiles(smiles, ms, max_n))
                {
                    QVector<double> results;
                    for(int m =0; m< mdls.size(); m++)
                    {
                        auto res = mdls[m]->predict(ms, max_n);
                        if(res.valid)
                        {
                            for(int i=0; i< res.size; i++)
                                results.append(res.value[i]);
                        }
                    }

                    double s = 0.0;

                    for(int i=0; i< results.size(); i++)
                        s += results[i];

                    double mean = s / results.size();
                    double sigma = 0.0;

                    for(int i=0; i< results.size(); i++)
                        sigma += (results[i] - mean) * (results[i] - mean);

                    double ci = student(results.size() - 1) * sqrt(sigma / (results.size() - 1)) / sqrt(results.size());

                    mdl->setData(idx_l, mean, Qt::DisplayRole);
                    mdl->setData(idx_h, fabs(round(2000.0 * ci / (mean + 1e-3))/10.0) , Qt::DisplayRole);

                }

                cnt++;
                ui->prg->setValue(cnt);
                qApp->processEvents();

                if(m_stop)
                    break;
            }

            for(int i=0; i< mdls.size(); i++)
               delete mdls[i];

            if(m_stop)
                goto fin;
        }
    }

fin:

    ui->btnCalculate->setEnabled(true);
    ui->btnDelete->setEnabled(true);
}

void ChemFilter::itemChanged(QListWidgetItem *it)
{
    if(it->checkState() == Qt::Unchecked)
        ui->tblData->hideColumn(it->data(Qt::UserRole).toInt());
    else if(it->checkState() == Qt::Checked)
        ui->tblData->showColumn(it->data(Qt::UserRole).toInt());
}

void ChemFilter::addNewColumn()
{
    QsarTableModel * mdl = static_cast<QsarTableModel*>( sortModel->sourceModel());
    if(! mdl )
    {
        QMessageBox::critical(this, "Error", "No data. Please load data into the table.");
        return;
    }

    NewColumn * a = new NewColumn(this, mdl, engine);
    if(a->exec())
    {
        QString varNam = a->getVariable();

        QListWidgetItem *it = new QListWidgetItem(varNam, ui->lstCols);
        it->setCheckState(Qt::Checked);
        it->setData(Qt::UserRole, mdl->cols.size()-1);

    }
    delete a;
}

void ChemFilter::deleteColumn()
{
    QsarTableModel * mdl = static_cast<QsarTableModel*>( sortModel->sourceModel());
    if(! mdl )
    {
        QMessageBox::critical(this, "Error", "No data. Please load data into the table.");
        return;
    }

    QList<QListWidgetItem*> items = ui->lstCols->selectedItems();
    if(items.size() == 0)
    {
        QMessageBox::warning(this, "Warning", "Please, select an item in the list widget (top right) "
                                              "which corresponds to the column you want to remove.");
        return;
    }

    int ret = QMessageBox::question(this, "Question",
                                    "Do you really want to delete selected property?",
                                    QMessageBox::Yes | QMessageBox::No);
    if(ret == QMessageBox::No)
        return;

    QVector<int> cols;
    for(int i=0; i< items.size(); i++)
        cols.push_back(items[i]->data(Qt::UserRole).toInt());

    std::sort(cols.begin(), cols.end());

    for(int i=cols.size()-1; i>=0; i--)
    {
        int col = cols[i];
        mdl->removeColumns(col, 1);
    }

    for(int i=0; i<items.size(); i++)
        delete items[i];

    //restore right order of cols
    for(int i=0; i< ui->lstCols->count(); i++)
        ui->lstCols->item(i)->setData(Qt::UserRole, i+3);
}

void ChemFilter::setUp()
{
    QsarTableModel * mdl = static_cast<QsarTableModel*>( sortModel->sourceModel());
    if(! mdl )
    {
        QMessageBox::critical(this, "Error", "No data. Please load data into the table.");
        return;
    }

    for(int i=0; i< sortModel->rowCount(); i++)
    {
        QModelIndex ind = sortModel->mapToSource( sortModel->index(i, 0) );
        mdl->setData( ind, 0, Qt::DisplayRole);
    }

}

void ChemFilter::setDown()
{
    QsarTableModel * mdl = static_cast<QsarTableModel*>( sortModel->sourceModel());
    if(! mdl )
    {
        QMessageBox::critical(this, "Error", "No data. Please load data into the table.");
        return;
    }

    for(int i=0; i< sortModel->rowCount(); i++)
    {
        QModelIndex ind = sortModel->mapToSource( sortModel->index(i, 0) );
        mdl->setData( ind, 2, Qt::DisplayRole);
    }
}

void ChemFilter::filter()
{
    QsarTableModel * mdl = static_cast<QsarTableModel*>( sortModel->sourceModel());
    if(! mdl )
    {
        QMessageBox::critical(this, "Error", "No data. Please load data into the table.");
        return;
    }

    QString script = ui->txtFilter->toPlainText().trimmed();
    QMap<QString, int> maps;

    //test run
    for(int i=3; i< mdl->cols.size(); i++)
    {
        engine->globalObject().setProperty(mdl->cols[i], "1");
        maps[mdl->cols[i]] = i;
    }

    QScriptValue res = engine->evaluate(script);
    if(engine->hasUncaughtException())
    {
        int line = engine->uncaughtExceptionLineNumber();
        QMessageBox::critical(this, "Error",
                              "Uncaught exception on line " + QString::number(line) + " : " + res.toString());
        return;
    }
    //end test run

    sortModel->setNewScript(script, maps);

    stsInfo->setText(tr("Loaded %1 molecules. Filtered %2").arg(mdl->rowCount()).arg(sortModel->rowCount()));
}

void ChemFilter::stopCalc()
{
    m_stop = true;
}

void ChemFilter::clearStats()
{
    ui->lblInfo->clear();
    ui->lblProperty->clear();
}

void ChemFilter::analyzeProperty()
{
    clearStats();

    QsarTableModel * mdl = static_cast<QsarTableModel*>( sortModel->sourceModel());
    if(! mdl )
    {
        QMessageBox::critical(this, "Error", "No data. Please load data into the table.");
        return;
    }

    QListWidgetItem * item = ui->lstCols->currentItem();
    if(item == nullptr)
    {
        QMessageBox::warning(this, "Warning", "Please select a column first.");
        return;
    }

    const int col = item->data(Qt::UserRole).toInt();

    QVector<double > d;
    for(int i=0; i< mdl->rowCount(); i++)
    {
        bool ok = false;
        QString s = mdl->index(i, col).data(Qt::DisplayRole).toString();
        s.replace(",",".").replace(" ", "");

        double v = s.toDouble(&ok);
        if(ok)
            d.push_back(v);
    }

    ui->lblProperty->setText(item->text());

    const int cnt = d.size();
    if(cnt > 0)
    {
         double * min = std::min_element(d.begin(), d.end());
         double * max = std::max_element(d.begin(), d.end());

         if (min == nullptr || max == nullptr)
             return;

         ui->lblProperty->setText(item->text() + " loaded <b>" + QString::number(cnt));
         ui->lblInfo->setText(tr("Min <b>%2</b>, Max <b>%3</b>").arg(*min).arg(*max));

         ui->wdgHist->setData(&d, *min, *max, 10);
    }

}

void ChemFilter::enterValue(QModelIndex ind)
{
    QsarTableModel * mdl = static_cast<QsarTableModel*>( sortModel->sourceModel());
    if(! mdl )
    {
        QMessageBox::critical(this, "Error", "No data. Please load data into the table.");
        return;
    }

    const int col = ind.column();
    if(col < 2)
        return;

    QModelIndex sind = sortModel->mapToSource( ind );
    QString d = mdl->data(sind, Qt::DisplayRole).toString();

    EnterResult * a = new EnterResult(this, d);
    if(a->exec())
    {
        d = a->nv;
        mdl->setData(sind, d, Qt::DisplayRole);
    }
    delete a;
}

void ChemFilter::save()
{
    QsarTableModel * mdl = static_cast<QsarTableModel*>( sortModel->sourceModel());
    if(! mdl )
    {
        QMessageBox::critical(this, "Error", "No data. Please load data into the table.");
        return;
    }

    if(sdfFileName.isEmpty())
        return;

    std::ofstream fn(sdfFileName.toStdString());

    OpenBabel::OBConversion conv(NULL, &fn);
    conv.SetOutFormat("SDF");

    for(int i=0; i< sortModel->rowCount(); i++)
    {
        QModelIndex ind = sortModel->mapToSource( sortModel->index(i, 0) );

        if(mdl->data(ind, Qt::EditRole).toString() != "Y")
            continue;

        ind = sortModel->mapToSource( sortModel->index(i, 1) );
        int m = mdl->data(ind, Qt::DisplayRole).toInt();

        OpenBabel::OBMol mol = mdl->mols[m];
        auto data = mol.GetData();
        mol.DeleteData(data);

        for(int c = 3; c< mdl->cols.size(); c++)
        {
            OpenBabel::OBPairData  * data = new OpenBabel::OBPairData();
            data->SetAttribute(mdl->cols[c].toStdString());

            ind = sortModel->mapToSource( sortModel->index(i, c) );
            data->SetValue(mdl->data(ind, Qt::DisplayRole).toString().toStdString());

            data->SetOrigin(OpenBabel::userInput);
            mol.SetData(data);
        }

        conv.Write(&mol);
    }

    fn.close();
}
