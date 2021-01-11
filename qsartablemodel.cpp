#include "qsartablemodel.h"

#include <QMessageBox>
#include <QProgressDialog>
#include <QApplication>
#include <QDebug>

#include <openbabel/obconversion.h>
#include <openbabel/mol.h>
#include <openbabel/data.h>
#include <openbabel/op.h>
#include <openbabel/generic.h>

QsarTableModel::QsarTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{

}

QVariant QsarTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        return cols[section];
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

int QsarTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_data.size();
}

int QsarTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return cols.size();
}

QVariant QsarTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const int col = index.column();
    const int row = index.row();

    if(role == Qt::DisplayRole)
    {
        if( col < m_data[row].size() && col > 0)
            return m_data[ row ] [ col ];
    }
    else if(role == Qt::EditRole)
    {
        if( col < m_data[row].size() && col >= 0)
            return m_data[ row ] [ col ];
    }
    else if(role == Qt::CheckStateRole)
    {
        if(col == 0)
        {
            if(m_data[row][col].toString() == "Y")
               return Qt::Checked;
            return Qt::Unchecked;
        }
    }

    return QVariant();
}

Qt::ItemFlags QsarTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags fl = Qt::ItemIsEnabled;
    if(index.column() > 1)
        fl |= Qt::ItemIsSelectable;
    if(index.column() == 0)
        fl |= Qt::ItemIsUserCheckable;
    return fl;
}

bool QsarTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    const int row = index.row();
    const int col = index.column();

    if(col == 0)
    {
        int ch = value.toInt();
        m_data[row][col] = (ch == Qt::Checked ? "Y" : "N");
    }
    else
        m_data[row][col] = value;

    emit dataChanged(index, index);
    return true;
}

int QsarTableModel::addColumnModel(const QString &caption)
{
    cols << caption;
    insertColumns(cols.size() - 1, 1);

    return cols.size() -1;
}

bool QsarTableModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count - 1);
    // FIXME: Implement me!
    endInsertRows();

    return true;
}

bool QsarTableModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    beginInsertColumns(parent, column, column + count - 1);

    for(int i=0; i <m_data.size(); i++)
        for(int j=0; j< count; j++)
            m_data[i].push_back(QVariant());

    endInsertColumns();

    return true;
}

bool QsarTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count - 1);

    endRemoveRows();
    return true;
}

bool QsarTableModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    beginRemoveColumns(parent, column, column + count - 1);

    cols.removeAt(column);
    for(int i=0; i <m_data.size(); i++)
            m_data[i].removeAt(column);

    endRemoveColumns();

    return true;
}

void QsarTableModel::loadDataFromFile(const QString &fname, QWidget * parent)
{
    cols.clear();

    cols << ".." << "STRUCTURE" << "SMILES";

    std::ifstream sdf(fname.toUtf8().data(),
                      std::ifstream::in);

    if(!sdf.is_open())
    {
        QMessageBox::critical(parent, tr("Error"),
                              tr("Error opening file. Please, check path and permissions."));
        return;
    }

    OpenBabel::OBConversion conv(&sdf, &std::cout);
    if(conv.SetInAndOutFormats("SDF","SMILES"))
    {
        sdf.seekg(0, std::ios_base::end);
        off_t len = sdf.tellg();

        QProgressDialog * progress = new QProgressDialog(parent);

        progress->setWindowTitle(tr("SDF database"));
        progress->setLabelText(tr("Loading SDF database into memory...."));
        progress->setWindowModality(Qt::WindowModal);

        progress->setMaximum(len);
        progress->setMinimum(0);

        progress->setValue(0);
        progress->show();
        progress->activateWindow();
        progress->raise();

        sdf.seekg(0,std::ios_base::beg);

        mols.clear();
        std::vector< OpenBabel::OBGenericData * > prop;

        OpenBabel::OBMol mol;
        QSet<QString> unique;

        while(conv.Read(&mol))
        {
            off_t cur = sdf.tellg();
            progress->setValue(cur);

            progress->update();
            qApp->processEvents();

            if (progress->wasCanceled())
                  break;

            mol.DeleteHydrogens();
            mol.Center();

            std::stringstream os;
            conv.SetOutStream(&os);
            conv.Write(&mol);

            QString smiles = QString::fromStdString(os.str()).split("\t")[0];
            if(unique.contains(smiles))
                continue;

            unique.insert(smiles);
            mols.push_back(mol);

            QMap<int, QVariant> md;

            prop = mol.GetData();
            for(uint i =0; i< prop.size(); i++)
            {
                auto origin = prop[i]->GetOrigin();
                if(origin == OpenBabel::DataOrigin::local ||
                        origin == OpenBabel::DataOrigin::perceived)
                    continue;

                std::string skey = prop[i]->GetAttribute();
                QString key = QString::fromStdString(skey);

                key = key.replace(" ","_").trimmed();

                int col = cols.indexOf(key);
                if(col == -1)
                {
                    cols.push_back(key);
                    col = cols.size() - 1;
                }

                std::string value = prop[i]->GetValue();
                QString val = QString::fromStdString(value);

                bool ok = false;
                double d_val = val.toDouble(&ok);

                if(ok) md[col] = d_val;
                else md[col] = val;
            }

            int curmol = mols.size() - 1;

            QList<QVariant> record;
            record.push_back("N");
            record.push_back(curmol);
            record.push_back(smiles);

            for(int i=3; i< cols.size(); i++)
            {
                record.push_back(QVariant());
                if (md.contains( i ))
                    record[i] = md[i];
            }
            m_data.push_back(record);

        }

        progress->deleteLater();
    }

    for(int i=0; i< m_data.size(); i++)
        for(int j=m_data[i].size(); j< cols.size(); j++)
            m_data[i].push_back(QVariant());


    qDebug() << m_data.size();
    qDebug() << cols;

}
