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
class QPushButton;
class QTreeWidget;
class QProgressBar;
class QToolButton;
class QListWidget;
class HistWidget;
class QComboBox;
class HeightTextEdit;

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
        return;

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

    QVector<class ModelInfo> models;
    QMap<QString, QTreeWidgetItem *> catItems;

    QStringList propsInEngine;

    volatile bool m_stop;

    void LoadModels();


    void createDockWindows();

    QPushButton *btnCalculate;
    QTreeWidget *lstModels;
    QProgressBar *prg;
    QPushButton *btnStopCalc;
    QToolButton *btnSelectAllModels;
    QToolButton *btnDeselectAllModels;
    void createDockModels();

    QToolButton * btnAddColumn;
    QToolButton * btnStats;
    QToolButton * btnDelete;
    QListWidget * lstCols;
    QLabel * lblProperty;
    QLabel * lblInfo;
    HistWidget * wdgHist;
    void createDockColumns();

    QToolButton *btnFilter;
    QComboBox *cmbFilter;
    HeightTextEdit * txtFilter;

    void createDockRule();

    QTableWidget * tblProps;
    void createDockProps();

    void closeEvent(QCloseEvent *event);

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

    void selectAllModels();
    void deselectAllModels();

    void listProps();
};

#endif // CHEMFILTER_H
