#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//QT
#include <QMainWindow>
#include <QDebug>
#include <QFileDialog>

//3rd party libs
#include <osgDB/WriteFile>
#include "gdal_priv.h"

//Local includes
//#include "GLWidget.h"
#include "xmlParser.h"
#include "AdapterWidget.h"
#include "osggislayer.h"
#include "osggistreeitem.h"
#include "filterstackdlg2.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    osg::ref_ptr<osg::Group> root ;
    ViewerQT *glWidget;
    /// controls if a filter property is changed to update the filter when unselected.
    bool filterChanged;

private slots:
    void on_actionEdit_Filter_Chain_triggered();
    void on_toolButton_clicked();
    void on_tableWidget_cellChanged(int row, int column);
    void on_treeWidget_itemSelectionChanged();
    void on_actionSave_triggered();
    void on_actionOpen_triggered();
};

#endif // MAINWINDOW_H
