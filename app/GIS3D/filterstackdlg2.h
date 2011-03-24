#ifndef FILTERSTACKDLG2_H
#define FILTERSTACKDLG2_H

#include <QWidget>
#include <QMessageBox>

#include "ui_FilterStackDlg2.h"
#include "osggistreeitem.h"
#include <osgGIS/Registry>
#include <osgGIS/Filter>

class FilterStackDlg2 : public QDialog, public Ui::FilterStackDlg2
{
    Q_OBJECT

public:
    FilterStackDlg2( osggistreeItem *_l, QWidget *parent = 0);
    osggistreeItem *layer;

private:
    void fillListBoxs();
    bool PropertyChanged;
private slots:
    void on_AvailableFiltersList_customContextMenuRequested(QPoint pos);
    void on_tableWidget_cellChanged(int row, int column);
    void on_SelectedFilterList_currentRowChanged(int currentRow);
    void on_SelectedFilterList_currentItemChanged(QListWidgetItem* current, QListWidgetItem* previous);
    void on_AddTopButton_clicked();
    void on_RemoveButton_clicked();
    void on_buttonBox_rejected();
    void on_buttonBox_accepted();
};

#endif // FILTERSTACKDLG2_H
