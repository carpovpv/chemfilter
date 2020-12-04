
#ifndef MOLDELEGATE
#define MOLDELEGATE

#include <QLineEdit>
#include <QItemDelegate>

#include <openbabel/mol.h>
#include <openbabel/obconversion.h>

#include "qsartablemodel.h"

using namespace OpenBabel;

class MolDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    MolDelegate(QObject *parent);
    void setModel(QsarTableModel * mdl);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:

    QsarTableModel * m_mdl;

    void drawDoubleLine(QPainter *painter,
                        float x1, float y1,
                        float x2, float y2) const ;
    void drawTripleLine(QPainter *painter,
                        float x1, float y1,
                        float x2, float y2) const;

    float GetAngleBetweenObjects(float x1,
                                 float y1,
                                 float x2,
                                 float y2) const;
    void setFileName(const QString &fileName);

    QMap<QString, QColor> colors;
};

#endif // MOLDELEGATE
