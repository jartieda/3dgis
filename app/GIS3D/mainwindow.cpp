#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    filterChanged(false)
{
    ui->setupUi(this);
    glWidget = new ViewerQT;
    glWidget->setCameraManipulator(new osgGA::TrackballManipulator);
    //  ui->layout3d(glWidget);
    glWidget->setMinimumWidth(400);
    ui->layout3d->addWidget(glWidget);
    root = new osg::Group;

    glWidget->setSceneData(root);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_triggered()
{
    qDebug()<<"open";
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("Images (*.png *.xpm *.jpg *.tif *.ecw *.img *.shp)"));
    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dialog.exec()==QDialog::Accepted){
       fileNames = dialog.selectedFiles();
       QString filename;
       foreach(filename, fileNames)
       {
           if (filename.toLower().contains(".shp"))
           {
               osggisLayer *l=new osggisLayer;
               osg::ref_ptr<osg::Node> node;
               //FIXME Esto es un problema porque debemos pasar todos los terrenos no solo uno...
                node=l->addLayer(filename.toLocal8Bit().constData()).get();
                if (!node.valid())
                {
                    qDebug()<<"MainWindow: node not vaild";
                }
               //El nodo viejo para esta capa es el recién creado
               l->old_node=node;
               root->addChild(node);

               osggistreeItem *t = new osggistreeItem();
               t->setText(0,filename);
               t->setLayer(l);
               ui->treeWidget->insertTopLevelItem(0,t);

               //item->setText(fileNames.at(0).toLocal8Bit().constData());
               //ui->listWidget->addItem(item);
           }else{
               qDebug()<<"Error format not supported";
           }
       }
   }

}

void MainWindow::on_actionSave_triggered()
{
    qDebug()<<"open";
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    QStringList filters;
    filters << "3ds (*.3ds)"<<"Collada (*.dae)"<<"Obj (*.obj)"<<"OSG (*.osg)"<<"DXF (*.dxf)";
    dialog.setFilters(filters);
    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dialog.exec()==QDialog::Accepted){
        fileNames = dialog.selectedFiles();
        osgDB::writeNodeFile(*(root.get()), fileNames.at(0).toLocal8Bit().constData());
    }
}

void MainWindow::on_treeWidget_itemSelectionChanged()
{

    qDebug()<<ui->treeWidget->selectedItems()[0]->type();
    QTreeWidgetItem *item = ui->treeWidget->selectedItems()[0];
    if (item->type()>0)
    {
        osggistreeItem* ti = (osggistreeItem*) item->parent();
        osgGIS::FilterList  f = ti->getLayer()->graph->getFilters();
        //int n = f.size();
        //printf("%d \n",n);
        osgGIS::Filter *filtro = f.at(item->type()-1);//el menos uno es porque hemos usado el cero para otros menesters
        osgGIS::Properties p = filtro->getProperties();
        ui->tableWidget->setRowCount(p.size());
        ui->tableWidget->setColumnCount(2);
        int row =0;
        for (osgGIS::Properties::iterator i = p.begin();i != p.end();i++)
        {
            QTableWidgetItem *newItem = new QTableWidgetItem(QString(i->getName().c_str()));

            ui->tableWidget->setItem(row, 0, newItem);
            QTableWidgetItem *newItem2 = new QTableWidgetItem(QString(i->getValue().c_str()));

            ui->tableWidget->setItem(row, 1, newItem2);
            row++;
        }


    }
}

void MainWindow::on_tableWidget_cellChanged(int row, int )
{

}

void MainWindow::on_toolButton_clicked()
{

    //First Get Selected Layer
    qDebug()<<ui->treeWidget->selectedItems()[0]->type();
    QTreeWidgetItem *item = ui->treeWidget->selectedItems()[0];
    if (item->type()>0)
    {

        //The selection is for the filter, the layer of this filter is on level up.
        osggistreeItem* ti = (osggistreeItem*) item->parent();
        //Get a refrence for the selected filter
        osgGIS::FilterList  f = ti->getLayer()->graph->getFilters();
        osgGIS::Filter *filtro = f.at(item->type()-1);//el menos uno es porque hemos usado el cero para otros menesters
        for (int row = 0 ; row<ui->tableWidget->rowCount();row++)
        {
            filtro->setProperty( osgGIS::Property (ui->tableWidget->item(row,0)->text().toStdString(),
                                            ui->tableWidget->item(row,1)->text().toStdString()) );
            qDebug()<<"property " <<ui->tableWidget->item(row,0)->text()
                    <<": "<<ui->tableWidget->item(row,1)->text() ;
        }
        qDebug()<<ui->treeWidget->selectedItems()[0]->type();

        osg::ref_ptr<osg::Node> new_node=ti->getLayer()->compile();
        //reemplazamos en root el recién creado nodo, por el nodo viejo
        root->replaceChild(ti->getLayer()->old_node,new_node);

        //actualiza el nodo viejo
        ti->getLayer()->old_node=new_node;

    }

}

void MainWindow::on_actionEdit_Filter_Chain_triggered()
{
     QTreeWidgetItem *item = ui->treeWidget->selectedItems()[0];
     osggistreeItem* ti = (osggistreeItem*) item;
     FilterStackDlg2 *f = new FilterStackDlg2((osggistreeItem*) ui->treeWidget->selectedItems()[0]);
     if (f->exec()==QDialog::Accepted){
         osg::ref_ptr<osg::Node> new_node=ti->getLayer()->compile();
         //reemplazamos en root el recién creado nodo, por el nodo viejo
         root->replaceChild(ti->getLayer()->old_node,new_node);
          //actualiza el nodo viejo
         ti->getLayer()->old_node=new_node;
     }
}
