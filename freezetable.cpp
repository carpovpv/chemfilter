#include "freezetable.h"

#include <QHeaderView>
#include <QScrollBar>

FreezeTableWidget::FreezeTableWidget(QWidget * parent) : QTableView(parent)
{
      frozenTableView = new QTableView(this);
}
FreezeTableWidget::~FreezeTableWidget()
{
}
void FreezeTableWidget::init(QItemDelegate * first, QItemDelegate * second)
{
      //connect the headers and scrollbars of both tableviews together

      connect(horizontalHeader(), SIGNAL(sectionResized(int,int,int)), this,
            SLOT(updateSectionWidth(int,int,int)));
      connect(verticalHeader(),&QHeaderView::sectionResized, this,
            &FreezeTableWidget::updateSectionHeight);
      connect(frozenTableView->verticalScrollBar(), &QAbstractSlider::valueChanged,
            verticalScrollBar(), &QAbstractSlider::setValue);
      connect(verticalScrollBar(), &QAbstractSlider::valueChanged,
            frozenTableView->verticalScrollBar(), &QAbstractSlider::setValue);

      frozenTableView->setModel(model());

      frozenTableView->horizontalHeader()->setMinimumHeight(horizontalHeader()->height());
      frozenTableView->setFocusPolicy(Qt::NoFocus);
      frozenTableView->verticalHeader()->hide();
      frozenTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
      frozenTableView->raise();

      frozenTableView->setItemDelegateForColumn(0, first);
      frozenTableView->setItemDelegateForColumn(1, second);

      frozenTableView->setStyleSheet("QTableView { border: none;}");
      frozenTableView->setSelectionModel(selectionModel());

      for (int col = 2; col < model()->columnCount(); ++col)
            frozenTableView->setColumnHidden(col, true);

      frozenTableView->setColumnWidth(0, columnWidth(0));
      frozenTableView->setColumnWidth(1, columnWidth(1));

      frozenTableView->setAlternatingRowColors(true);
      frozenTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      frozenTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      frozenTableView->show();

      updateFrozenTableGeometry();

      setHorizontalScrollMode(ScrollPerPixel);
      setVerticalScrollMode(ScrollPerPixel);
      frozenTableView->setVerticalScrollMode(ScrollPerPixel);
}
void FreezeTableWidget::updateSectionWidth(int logicalIndex, int /* oldSize */, int newSize)
{
      if (logicalIndex < 2)
      {
            frozenTableView->setColumnWidth(logicalIndex, newSize);
            updateFrozenTableGeometry();
      }
}
void FreezeTableWidget::updateSectionHeight(int logicalIndex, int /* oldSize */, int newSize)
{
      frozenTableView->setRowHeight(logicalIndex, newSize);
}
void FreezeTableWidget::resizeEvent(QResizeEvent * event)
{
      QTableView::resizeEvent(event);
      updateFrozenTableGeometry();
}
QModelIndex FreezeTableWidget::moveCursor(CursorAction cursorAction,
                                          Qt::KeyboardModifiers modifiers)
{
      QModelIndex current = QTableView::moveCursor(cursorAction, modifiers);
      if (cursorAction == MoveLeft && current.column() > 1
              && visualRect(current).topLeft().x() < (frozenTableView->columnWidth(0) + frozenTableView->columnWidth(1)) )
      {
            const int newValue = horizontalScrollBar()->value() + visualRect(current).topLeft().x()
                                 - frozenTableView->columnWidth(0) - frozenTableView->columnWidth(1);
            horizontalScrollBar()->setValue(newValue);
      }
      return current;
}
void FreezeTableWidget::scrollTo (const QModelIndex & index, ScrollHint hint)
{
    if (index.column() > 1)
        QTableView::scrollTo(index, hint);
}
void FreezeTableWidget::updateFrozenTableGeometry()
{
      frozenTableView->setGeometry(verticalHeader()->width() + frameWidth(),
                                   frameWidth(), columnWidth(0) + columnWidth(1),
                                   viewport()->height()+horizontalHeader()->height());
}
