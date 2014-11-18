#ifndef MYTREEWIDGET_H
#define MYTREEWIDGET_H

#include <QTreeWidget>

class MyTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit MyTreeWidget(QWidget *parent = 0);
    
    void scrollContentsBy(int dx, int dy);

    void wheelEvent(QWheelEvent *evt);

signals:
    void viewportChanged();
    
};

#endif // MYTREEWIDGET_H
