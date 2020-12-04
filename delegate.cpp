#include "delegate.h"

#include <QPainter>
#include <QDebug>

#include <openbabel/obiter.h>
#include <openbabel/atom.h>
#include <openbabel/bond.h>
#include <openbabel/ring.h>
#include <openbabel/elements.h>

MolDelegate::MolDelegate(QObject *parent)
        : QItemDelegate(parent)
{
    colors["N"] = Qt::blue;
    colors["O"] = Qt::red;
    colors["Cl"] = Qt::green;
    colors["I"] = Qt::darkRed;
    colors["S"] = QColor("#84840a");
    colors["F"] = QColor("#ff8080");
    colors["Br"]= QColor("#c04000");
    colors["P"] = QColor("#ff8000");
    colors["S"] = QColor("#c9c73e");

    m_mdl = nullptr;
}

void MolDelegate::setModel(QsarTableModel *mdl)
{
    m_mdl = mdl;
}

float MolDelegate::GetAngleBetweenObjects(float x1,
                                        float y1,
                                        float x2,
                                        float y2) const
{
    float kx,ky;
    float t, a;

    kx=x2-x1;
    ky=y2-y1;
    if(kx==0)kx=0.00001f;
    t=ky/kx; if(t<0)t=t*-1;

    a=(float)(180*atan((float)t)/M_PI);

    if((kx<=0) && (ky>=0))a=180-a; else
    if((kx<=0) && (ky<=0))a=180+a; else
    if((kx>=0) && (ky<=0))a=359.99999f-a;

    return a;
}

void MolDelegate::drawDoubleLine(QPainter *painter,
                               float x1, float y1,
                               float x2, float y2) const
{
    const int of = 2;

    float dist = sqrt((x1-x2)*(x1-x2) + (y1-y2) * (y1-y2));
    float dx = (x2-x1);
    float dy = (y2-y1);

    if(dx == 0)
    {
        painter->drawLine(x1 - of, y1, x2 - of, y2);
        painter->drawLine(x1 + of, y1, x2 + of, y2);
    }
    else if(dy == 0)
    {
        painter->drawLine(x1, y1 - of, x2, y2 - of);
        painter->drawLine(x1, y1 + of, x2, y2 + of);
    }
    else
    {
        painter->save();

        painter->translate(x1, y1);
        painter->rotate(GetAngleBetweenObjects(x1,y1,x2,y2));

        painter->drawLine(0, of, dist, of);
        painter->drawLine(0, -of, dist, -of);

        painter->restore();
    }

}

void MolDelegate::drawTripleLine(QPainter *painter, float x1, float y1, float x2, float y2) const
{
    const int of = 2;

    float dist = sqrt((x1-x2)*(x1-x2) + (y1-y2) * (y1-y2));
    float dx = (x2-x1);
    float dy = (y2-y1);

    if(dx == 0)
    {
        painter->drawLine(x1 - of, y1, x2 - of, y2);
        painter->drawLine(x1 + of, y1, x2 + of, y2);
        painter->drawLine(x1, y1, x2, y2);

    }
    else if(dy == 0)
    {
        painter->drawLine(x1, y1 - of, x2, y2 - of);
        painter->drawLine(x1, y1 + of, x2, y2 + of);
        painter->drawLine(x1, y1, x2, y2);
    }
    else
    {

        painter->save();

        painter->translate(x1, y1);
        painter->rotate(GetAngleBetweenObjects(x1,y1,x2,y2));

        painter->drawLine(0, of, dist, of);
        painter->drawLine(0, -of, dist, -of);
        painter->drawLine(0, 0, dist, 0);

        painter->restore();
    }
}

void MolDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(m_mdl == nullptr) return;

    QRect rect = option.rect;

    int ind = index.data(Qt::DisplayRole).toInt();
    if(ind > m_mdl->mols.size())
        return;

    OBMol * mol = & m_mdl->mols[ind];
    if(!mol->Has2D())
        return;

    painter->save();

    //translate painter to current item
    painter->translate(rect.left(), rect.top());

    painter->setRenderHint(QPainter::Antialiasing);
    painter->fillRect(rect, Qt::white);

    float xmax, xmin;
    float ymax, ymin;

    xmax = xmin = ymax = ymin = 0;

    int i = 0;
    FOR_ATOMS_OF_MOL(atom, mol)
    {
        float x = atom->x();
        float y = atom->y();

        if(i == 0)
        {
            xmax = xmin = x;
            ymax = ymin = y;
        }
        else
        {
            if(xmax < x)
                xmax = x;
            if(ymax < y)
                ymax = y;
            if(xmin > x)
                xmin = x;
            if(ymin > y)
                ymin = y;
        }
        i++;

    }

    if(xmin == xmax || ymin == ymax)
    {
        qDebug() << "No coordinates.";

        painter->restore();
        return;
    }

    float xwin = rect.width() - 20;
    float ywin = rect.height() - 20;

    float xreal = (xmax - xmin);
    float yreal = (ymax - ymin);

    if(xreal >= yreal)
    {
        if(xwin >= ywin)
        {
            float k = floor(ywin / (yreal));
            float q = xreal * k;

            if(q < xwin)
                xwin = q;

            k = floor(xwin / (xreal));
            q = yreal * k;

            if(q < ywin)
                ywin = q;

        }
        else
        {
            float k = floor(xwin / (xreal));
            float q = yreal * k;

            if(q < ywin)
                ywin = q;

            k = floor(ywin / (yreal));
            q = xreal * k;

            if(q < xwin)
                xwin = q;

        }

    }
    else
    {
        if(xwin >= ywin)
        {
            float k = ywin / (ymax - ymin);
            xwin = (xmax - xmin) * k;
        }
        else
        {
            float k = xwin / (xmax - xmin);
            ywin = (ymax - ymin) * k;
        }
    }

    float dx = (rect.width() - xwin) / 2.0;
    float dy = (rect.height() - ywin) / 2.0;

    painter->translate(dx,dy);

    FOR_BONDS_OF_MOL(bond, mol)
    {
        OpenBabel::OBAtom * atom1 = bond->GetBeginAtom();
        OpenBabel::OBAtom * atom2 = bond->GetEndAtom();

        float x1 = atom1->x();
        float y1 = atom1->y();

        float x2 = atom2->x();
        float y2 = atom2->y();

        float cx1 = xwin + (xwin - 20) * (x1 -xmax) / (xmax - xmin);
        float cy1 = ywin + (ywin - 20) * (y1 -ymax) / (ymax - ymin);

        float cx2 = xwin + (xwin - 20) * (x2 -xmax) / (xmax - xmin);
        float cy2 = ywin + (ywin - 20) * (y2 -ymax) / (ymax - ymin);

        if(bond->GetBondOrder() == 1)
            painter->drawLine(cx1, cy1, cx2, cy2);
        else if(bond->IsAromatic())
            painter->drawLine(cx1, cy1, cx2, cy2);
        else if(bond->GetBondOrder() == 2)
            drawDoubleLine(painter, cx1, cy1, cx2, cy2);
        else if(bond->GetBondOrder() == 3)
             drawTripleLine(painter, cx1, cy1, cx2, cy2);

    }

    std::vector< OpenBabel::OBRing * > rings = mol->GetSSSR();
    for(uint i =0; i< rings.size(); ++i)
    {
        if(rings[i]->IsAromatic())
        {
            OpenBabel::vector3 center;
            OpenBabel::vector3 norm1;
            OpenBabel::vector3 norm2;

            rings[i]->findCenterAndNormal(center, norm1, norm2);

            float x = center.GetX();
            float y = center.GetY();

            float cx = xwin + (xwin - 20) * (x -xmax) / (xmax - xmin);
            float cy = ywin + (ywin - 20) * (y -ymax) / (ymax - ymin);

            float d = - 1;

            FOR_BONDS_OF_MOL(bond, mol)
            {
                if(rings[i]->IsMember(&*bond))
                {
                    OpenBabel::OBAtom * atom1 = bond->GetBeginAtom();
                    OpenBabel::OBAtom * atom2 = bond->GetEndAtom();

                    float x1 = atom1->x();
                    float y1 = atom1->y();

                    float x2 = atom2->x();
                    float y2 = atom2->y();

                    float cx1 = xwin + (xwin - 20) * (x1 -xmax) / (xmax - xmin);
                    float cy1 = ywin + (ywin - 20) * (y1 -ymax) / (ymax - ymin);

                    float cx2 = xwin + (xwin - 20) * (x2 -xmax) / (xmax - xmin);
                    float cy2 = ywin + (ywin - 20) * (y2 -ymax) / (ymax - ymin);

                    float a = cy1 - cy2;
                    float b = cx2 - cx1;
                    float c = cx1 * cy2 - cx2 * cy1;

                    float dist = sqrt(a*a + b*b);
                    float di = fabs(a*cx + b*cy + c) / dist;

                    if(d == -1)
                        d = di;
                    else
                    {
                        if(di < d)
                            d = di;
                    }

                }
            }

            d *= 0.8;

            painter->drawEllipse(cx-d,cy-d, d*2, d*2);
        }
    }

    FOR_ATOMS_OF_MOL(atom, mol)
    {
        float x = atom->x();
        float y = atom->y();

        float cx = xwin + (xwin - 20) * (x -xmax) / (xmax - xmin)-5;
        float cy = ywin + (ywin - 20) * (y -ymax) / (ymax - ymin)+5;

        int ci = atom->GetAtomicNum();
        if(ci != 6)
        {
            painter->save();

            painter->setBrush(QColor(Qt::white));
            painter->setPen(Qt::white);
            painter->drawEllipse(cx, cy-10, 10,10);

            QString atom = OBElements::GetSymbol(ci);
            if(colors.contains(atom))
                painter->setPen(colors[atom]);
            else
                painter->setPen(Qt::black);

            painter->drawText(cx,cy, atom);

            painter->restore();

        }
    }

    painter->restore();
}
