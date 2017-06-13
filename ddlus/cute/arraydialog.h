#ifndef ARRAYDIALOG_H
#define ARRAYDIALOG_H

#include <QDialog>

#include "../console/double.h"
#include "finddialog.h"

#include <QTableWidget>

namespace Ui {
class arrayDialog;
}

class arrayDialog : public QDialog
{
    Q_OBJECT

public:
    explicit arrayDialog(QWidget *parent = 0, void* qMain = 0);
    ~arrayDialog();
    void setArray(struct session* sess, char* name, int i, int j);
    void setCell(int i, int j, char* exp, fnum_t val, char* desc, char flags);
    void addArray(char* name);
    void clearArrayList();

private slots:
    void on_tableArray_cellChanged(int row, int column);

    void on_lineExp_returnPressed();

    void on_tableArray_cellActivated(int row, int column);

    void on_tableArray_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

    void on_toolRecalc_clicked();

    void on_comboArrays_currentIndexChanged(const QString &arg1);

    void on_toolComment_clicked();

    void on_toolExp_clicked();

    void on_toolSetDim_clicked();

    void on_toolNew_clicked();

    void on_comboCellColor_currentIndexChanged(const QString &arg1);

    void on_tableArray_itemSelectionChanged();

    void on_toolPaint_clicked();

private:
    Ui::arrayDialog *ui;
    void* main;
    struct session* sessptr;
    QString strName;
    int maxi, maxj;
    QTableWidgetItem* copy_item;
    int runExp(QString exp);
    void clearCell(QTableWidget* table);
    void setCellItem(QTableWidget* table, QTableWidgetItem* copy_item);
    QTableWidgetItem* cloneItem(QTableWidgetItem* src);
    void setComment(QTableWidget* table);

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event);
};

#endif // ARRAYDIALOG_H
