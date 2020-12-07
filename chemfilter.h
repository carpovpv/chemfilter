#ifndef CHEMFILTER_H
#define CHEMFILTER_H

#include <QMainWindow>
#include <QTextOption>
#include <QIcon>
#include <QVariant>
#include <QTableWidgetItem>
#include <QDebug>

#include <openbabel/mol.h>
#include <openbabel/groupcontrib.h>

#include "qsarsortmodel.h"

namespace Ui {
class ChemFilter;
}

using namespace OpenBabel;

class QLabel;
class QSettings;
class QTableWidgetItem;
class QTreeWidgetItem;
class QScriptEngine;
class QListWidgetItem;
class LAP_CheckBoxDelegate;
class MolDelegate;
class QScriptEngine;

struct Interval
{
    double low;
    double high;

    double percent;  //percent of drugbank coverage
};

class ModelInfo
{
public:
    QString dirName;

    QString category;
    QString name;
    QString var;
    QStringList fnames;  //filenames for the models

    QVector<struct Interval> intervals;
    ModelInfo()
    {

    }

    void printInfo()
    {
        qDebug() << "Category: " << category;
        qDebug() << "Name: " << name;
        qDebug() << "Number of intervals: " << intervals.size();
        for(int i=0; i< intervals.size(); i++)
            qDebug() << "\t" << intervals[i].percent << intervals[i].low << intervals[i].high;
        for(int i=0; i< fnames.size(); i++)
            qDebug() << "Model binary: " << fnames[i];
    }
};

class ChemFilter : public QMainWindow
{
    Q_OBJECT

public:
    explicit ChemFilter(QWidget *parent = 0);
    ~ChemFilter();

private:
    Ui::ChemFilter *ui;

    QLabel *stsInfo;
    QLabel *stsCenter;

    QSettings * settings;
    QString sdfFileName;

    QStringList header;
    OBSmartsPattern smarts;
    OBDescriptor* logP, *hbD, *hbA1;

    QIcon iconYes;
    QIcon iconNo;

    QsarSortModel * sortModel;
    LAP_CheckBoxDelegate * checkBoxDelegate;
    MolDelegate * molDelegate;
    QScriptEngine * engine;

    QVector<class ModelInfo> models;
    QMap<QString, QTreeWidgetItem *> catItems;

    QStringList propsInEngine;

    volatile bool m_stop;

    void LoadModels();

private slots:

    void LoadSdf();
    void exportSdf();
    void exportCSV();

    void calculate();
    void itemChanged(QListWidgetItem *it);

    void addNewColumn();
    void deleteColumn();

    void setUp();
    void setDown();

    void filter();
    void stopCalc();

    void clearStats();
    void analyzeProperty();

    void enterValue(QModelIndex ind);
    void save();

    void loadWhere();
};

#endif // CHEMFILTER_H
