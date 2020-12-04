#include "qsarsortmodel.h"
#include <QDebug>

QsarSortModel::QsarSortModel(QObject *parent, QScriptEngine *eng) : QSortFilterProxyModel(parent)
{
    engine = eng;
    expNumber.setPattern("(\\d+)");
}

void QsarSortModel::setNewScript(const QString &t, const QMap<QString, int> & maps)
{
    script = t;
    m_maps = maps;

    invalidateFilter();
}

bool QsarSortModel::filterAcceptsRow(int sourceRow,
                                     const QModelIndex &sourceParent) const
{
    if(script.isEmpty())
        return true;

    for(QMap<QString, int>::const_iterator it = m_maps.begin();
        it != m_maps.end(); ++it)
    {
        QModelIndex index = sourceModel()->index(sourceRow, it.value(), sourceParent);
        double val = sourceModel()->data(index).toDouble();
        engine->globalObject().setProperty(it.key(), val);
    }

    QScriptValue res = engine->evaluate(script);
    if(!engine->hasUncaughtException())
       return res.toBool();

    return true;
}

bool QsarSortModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    QVariant ld = sourceModel()->data(left);
    QVariant rd = sourceModel()->data(right);

    if(ld.type() == QVariant::Double && rd.type() == QVariant::Double)
        return ld.toDouble() <= rd.toDouble();

    if(ld.type() == QVariant::String || rd.type() == QVariant::String)
    {
         qlonglong ll;
         qlonglong lr;

         int pos_l = 0;
         if ((pos_l = expNumber.indexIn(ld.toString(), pos_l)) != -1)
         {
             ll = expNumber.cap(1).toLongLong();

             int pos_r = 0;
             if ((pos_r = expNumber.indexIn(rd.toString(), pos_r)) != -1)
             {
                 lr = expNumber.cap(1).toLongLong();
                 return ll < lr;
             }
         }

         return ld.toString() <= rd.toString();
    }

    return true;
}

void QsarSortModel::sort(int column, Qt::SortOrder order)
{
    if(column < 2) return;
    QSortFilterProxyModel::sort(column, order);
}
