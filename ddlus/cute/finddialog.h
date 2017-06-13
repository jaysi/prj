#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>

namespace Ui {
class findDialog;
}

class findDialog : public QDialog
{
    Q_OBJECT

public:
    explicit findDialog(QWidget *parent = 0, QString title = tr("Find..."), QString icon = tr(":/res/search-26.png"));
    ~findDialog();
    QString strFind;
    bool forward;

private slots:
    void on_toolFind_clicked();

    void on_lineEdit_returnPressed();

    void reject();

    void on_toolFindBack_clicked();

private:
    Ui::findDialog *ui;
protected:
    void closeEvent(QCloseEvent *);
    void showEvent(QShowEvent *);
};

#endif // FINDDIALOG_H
