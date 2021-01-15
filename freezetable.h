#ifndef FREEZETABLE_H
#define FREEZETABLE_H

#include <QTableView>
#include <QItemDelegate>

class FreezeTableWidget : public QTableView {
     Q_OBJECT
public:
      FreezeTableWidget(QWidget *parent);
      ~FreezeTableWidget();
      void init(QItemDelegate *first, QItemDelegate *second);
      void hideNewlyAddedColumns();

protected:
      void resizeEvent(QResizeEvent *event) override;
      QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override;
      void scrollTo (const QModelIndex & index, ScrollHint hint = EnsureVisible) override;
private:
      QTableView *frozenTableView;
      void updateFrozenTableGeometry();
private slots:
      void updateSectionWidth(int logicalIndex, int oldSize, int newSize);
      void updateSectionHeight(int logicalIndex, int oldSize, int newSize);
};

#endif // FREEZETABLE_H
