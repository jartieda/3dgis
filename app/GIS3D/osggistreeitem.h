#ifndef OSGGISTREEITEM_H
#define OSGGISTREEITEM_H
//QT
#include <QTreeWidgetItem>
#include <QDebug>

//local
#include "osggislayer.h"

class osggistreeItem : public QTreeWidgetItem
{
    //Q_OBJECT
public:
    explicit osggistreeItem(QTreeWidgetItem *parent, const QStringList &strings, int type = Type);
    explicit osggistreeItem (int type = Type);
    void setLayer (osggisLayer *layer);
    osggisLayer *getLayer(){return l;}
private:
    osggisLayer *l;

};

#endif // OSGGISTREEITEM_H
