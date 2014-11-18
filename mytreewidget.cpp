#include "mytreewidget.h"

MyTreeWidget::MyTreeWidget(QWidget *parent) :
    QTreeWidget(parent)
{
}

void MyTreeWidget::scrollContentsBy(int dx, int dy)
{
    emit viewportChanged();
    QTreeWidget::scrollContentsBy(dx,dy);
}

void MyTreeWidget::wheelEvent(QWheelEvent *evt)
{
    emit viewportChanged();
    QTreeWidget::wheelEvent(evt);
}
