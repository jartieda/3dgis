#include "osggistreeitem.h"

osggistreeItem::osggistreeItem(QTreeWidgetItem *parent, const QStringList &strings, int type) :
        QTreeWidgetItem(parent, strings, type )
{
}
osggistreeItem::osggistreeItem (int type):
QTreeWidgetItem(type )
{

}
void osggistreeItem::setLayer (osggisLayer *layer)
{
    l = layer;
    /** \todo borrar las capas antes de meter mas **/

    osgGIS::FilterList f;
    qDebug()<<"filter_stack:"<<l->graph->getFilters().size();
    f=l->graph->getFilters();
    //int n = f.size();
    //printf("%d \n",n);

    int n = 1;
    for (osgGIS::FilterList::iterator i = f.begin(); i != f.end(); i ++)
    {
        QTreeWidgetItem *newItem = new QTreeWidgetItem(this,n);
        qDebug()<<"inserting "<< i->get()->getFilterType().c_str()<< " type: "<<newItem->type();

        newItem->setText(0,i->get()->getFilterType().c_str());
        //newItem->setText(0,n);
        n++;
        //SelectedFilterList->insertItem(SelectedFilterList->count(), newItem);
    }

    /*const osgGIS::FilterFactoryMap *filterfactMap = osgGIS::Registry::instance()->getFilters();
    for ( osgGIS::FilterFactoryMap::const_iterator i = filterfactMap->begin(); i != filterfactMap->end(); i++)
    {
        QTreeWidgetItem *newItem = new QTreeWidgetItem;
        std::cout<<"inserting "<< i->first.c_str()<< std::endl;
        newItem->setText(i->first.c_str());
        AvailableFiltersList->insertItem(AvailableFiltersList->count(), newItem);
    }*/
}
