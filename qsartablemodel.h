#ifndef QSARTABLEMODEL_H
#define QSARTABLEMODEL_H

#include <QAbstractTableModel>
#include <openbabel/mol.h>

class MolDelegate;
class QsarTableModel : public QAbstractTableModel
{
    Q_OBJECT

    friend class MolDelegate;

public:
    explicit QsarTableModel(QObject *parent = nullptr);

    enum ColType { Numeric, Date, String };

    QVector<OpenBabel::OBMol> mols;
    QStringList cols;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    int addColumnModel(const QString &caption);

    //QSAR
    void loadDataFromFile(const QString &fname, QWidget *parent);
    bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

private:

    QList< QList< QVariant> > m_data;

    // Add data:
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    // Remove data:
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
};

#endif // QSARTABLEMODEL_H
