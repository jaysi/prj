#include "finddialog.h"
#include "ui_finddialog.h"

#include <QCloseEvent>

findDialog::findDialog(QWidget *parent, QString title, QString icon) :
    QDialog(parent),
    ui(new Ui::findDialog)
{
    ui->setupUi(this);
    strFind = tr("");
    setWindowTitle(title);
    setWindowIcon(QIcon(icon));
    if(title != tr("Find...")){
        ui->toolFindBack->hide();
    }
    forward = true;
}

findDialog::~findDialog()
{
    delete ui;
}

void findDialog::on_lineEdit_returnPressed()
{
    on_toolFind_clicked();
}

void findDialog::on_toolFind_clicked()
{
    forward = true;
    close();
}

void findDialog::closeEvent(QCloseEvent *event){
    strFind = ui->lineEdit->text();
    event->accept();
}

void findDialog::showEvent(QShowEvent *event){
    ui->lineEdit->setText(strFind);
    event->accept();
}

void findDialog::reject(){
    ui->lineEdit->setText(tr(""));
    close();
}

void findDialog::on_toolFindBack_clicked()
{
    forward = false;
    close();
}
