#include "filterstackdlg2.h"

FilterStackDlg2::FilterStackDlg2(osggistreeItem *_l, QWidget *parent)
    : QDialog(parent)

{
    setupUi(this);
  /*QObject::connect(AddTopButton, SIGNAL(clicked()),
        this, SLOT(on_AddTopButton_clicked()));
    QObject::connect(buttonBox, SIGNAL(accepted()),
        this, SLOT(on_buttonBox_accepted()));*/
    layer = _l;
    fillListBoxs();

}

void FilterStackDlg2::fillListBoxs()
{
    osgGIS::FilterList f;
    std::cout<<"filter_stack"<<layer->getLayer()->graph->getFilters().size()<<std::endl;
    f=layer->getLayer()->graph->getFilters();
    int n = f.size();
    printf("%d \n",n);
    SelectedFilterList->clear();
    AvailableFiltersList->clear();

    for (osgGIS::FilterList::iterator i = f.begin(); i != f.end(); i ++)
    {
        QListWidgetItem *newItem = new QListWidgetItem;
        std::cout<<"inserting "<< i->get()->getFilterType().c_str()<< std::endl;
        newItem->setText(i->get()->getFilterType().c_str());
        SelectedFilterList->insertItem(SelectedFilterList->count(), newItem);
    }

    const osgGIS::FilterFactoryMap *filterfactMap = osgGIS::Registry::instance()->getFilters();
    for ( osgGIS::FilterFactoryMap::const_iterator i = filterfactMap->begin(); i != filterfactMap->end(); i++)
    {
        QListWidgetItem *newItem = new QListWidgetItem;
        std::cout<<"inserting "<< i->first.c_str()<< std::endl;
        newItem->setText(i->first.c_str());
        AvailableFiltersList->insertItem(AvailableFiltersList->count(), newItem);
    }

}
void FilterStackDlg2::on_buttonBox_accepted()
{
    this->accept();
}

void FilterStackDlg2::on_buttonBox_rejected()
{
    this->reject();
}

void FilterStackDlg2::on_AddTopButton_clicked()
{
    if (AvailableFiltersList->currentItem()!= NULL)
    {
        osg::ref_ptr<osgGIS::Filter> f = osgGIS::Registry::instance()->createFilterByType( AvailableFiltersList->currentItem()->text().toLocal8Bit().constData());

        layer->getLayer()->graph->insertFilter( f.get(), SelectedFilterList->currentIndex().row() );
        fillListBoxs();
    }else{
        QMessageBox msgBox;
        msgBox.setText("current item is null.");
        msgBox.exec();
    }
}

void FilterStackDlg2::on_RemoveButton_clicked()
{
    if ( SelectedFilterList->currentItem()!=NULL )
    {
        layer->getLayer()->graph->removeFilter(SelectedFilterList->currentIndex().row() );
        fillListBoxs();
    }else{
        QMessageBox msgBox;
        msgBox.setText("current item is null.");
        msgBox.exec();
    }

}

void FilterStackDlg2::on_SelectedFilterList_currentItemChanged(QListWidgetItem* current, QListWidgetItem* previous)
{

}

void FilterStackDlg2::on_SelectedFilterList_currentRowChanged(int currentRow)
{
    if (currentRow>=0)
    {
        osgGIS::FilterList flist = layer->getLayer()->graph->getFilters();
        osgGIS::FilterList::iterator it = flist.begin();
        it += currentRow;

        std::cout<<"currenRow:"<<currentRow<<" from "<<flist.size()<<std::endl;
        osgGIS::Filter *fil = it-> get();

        osgGIS::Properties props = fil->getProperties();
        int row = 0;
    //    for( osgGIS::Properties::iterator itprop = it->get()->getProperties().begin(); itprop != it->get()->getProperties().end(); itprop++ )
        std::cout<<"number of props:"<< props.size()<<std::endl;
        tableWidget->setRowCount(props.size());
        for (osgGIS::Properties::iterator itprop = props.begin() ; itprop != props.end(); itprop++)
        {
            std::cout<<"prop: "<<itprop->getName().c_str()<<": "<<itprop->getValue().c_str()<<std::endl;

            QTableWidgetItem *newItem = new QTableWidgetItem(itprop->getName().c_str());
            tableWidget->setItem(row, 0, newItem);
            QTableWidgetItem *newItem2;

            if (itprop->getValue()=="")
                newItem2 = new QTableWidgetItem("");
            else
                newItem2 = new QTableWidgetItem(itprop->getValue().c_str());

            tableWidget->setItem(row, 1, newItem2);
            row ++;
        }
    }
}

void FilterStackDlg2::on_tableWidget_cellChanged(int row, int column)
{
    PropertyChanged=true;

    if ((tableWidget->currentRow()>=0)&&(column==1)){
        if ((row>=0)&&(row<tableWidget->rowCount())){

            osgGIS::FilterList flist = layer->getLayer()->graph->getFilters();
            osgGIS::FilterList::iterator it = flist.begin();
            it += SelectedFilterList->currentRow();

            std::cout<<"currenRow:"<<tableWidget->currentRow()<<" from "<<flist.size()<<std::endl;
            osgGIS::Filter *fil = it-> get();

            fil->setProperty( osgGIS::Property (tableWidget->item(row,0)->text().toStdString(),tableWidget->item(row,1)->text().toStdString()) );
            std::cout<<"property " <<tableWidget->item(row,0)->text().toStdString()<<": "<<tableWidget->item(row,1)->text().toStdString() <<std::endl;

        }
    }
}

void FilterStackDlg2::on_AvailableFiltersList_customContextMenuRequested(QPoint pos)
{
    QListWidgetItem * item = AvailableFiltersList->itemAt(pos);

}
