#ifndef PLOTDIALOG_H
#define PLOTDIALOG_H

#include <QDialog>
#include <QPainter>
#include <QPaintEvent>

#include "../console/double.h"

namespace Ui {
class PlotDialog;
}

class PlotDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PlotDialog(QWidget *parent = 0);
    ~PlotDialog();
    void addPt(fnum_t x, fnum_t y);
    int plot();

private:
    QPainter* painter;
    Ui::PlotDialog *ui;

    QList<fnum_t> xlist;
    QList<fnum_t> ylist;

    qreal sx, sy;

protected:
    virtual void paintEvent(QPaintEvent *);
private slots:
    void on_toolZoomOut_clicked();
    void on_toolZoomIn_clicked();
    void on_toolZoomNr_clicked();
};

#endif // PLOTDIALOG_H
