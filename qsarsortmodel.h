#ifndef QSARSORTMODEL_H
#define QSARSORTMODEL_H

#include <QSortFilterProxyModel>
#include <QScriptEngine>
#include <QMap>
#include <QRegExp>

class QsarSortModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    QsarSortModel(QObject *parent, QScriptEngine *eng);

    void setNewScript(const QString &t, const QMap<QString, int> & maps);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

private:

    QScriptEngine * engine;
    QString script;

    QRegExp expNumber;
    QMap<QString, int> m_maps;
};

#endif // QSARSORTMODEL_H
