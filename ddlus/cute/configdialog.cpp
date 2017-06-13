#include "configdialog.h"
#include "ui_configdialog.h"
#include "mainwindow.h"

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{    
    ui->setupUi(this);    
}

ConfigDialog::ConfigDialog(QWidget *parent, void* mainw) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{
    QStringList list;
    main = mainw;
    ui->setupUi(this);
    list = QStyleFactory::keys();
    //qDebug()<<list;
    ui->comboStyle->insertItems(0, list);
    list = QColor::colorNames();
    //qDebug()<<list;
    ui->comboBackgroundColor->insertItems(0, list);
    ui->comboFontColor->insertItems(0, list);

}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

void ConfigDialog::updateConf(){

    ui->checkHilight->setChecked(((MainWindow*)main)->sess.cursess->uiflags & SESS_UIF_HILIGHT);
    ui->checkDetails->setChecked(((MainWindow*)main)->sess.cursess->uiflags & SESS_UIF_DET);
    ui->checkAns->setChecked(((MainWindow*)main)->sess.cursess->uiflags & SESS_UIF_ANS);
    ui->checkDecimFix->setChecked(((MainWindow*)main)->sess.cursess->uiflags & SESS_UIF_DECIMNOTR);
    ui->checkLoud->setChecked(((MainWindow*)main)->sess.cursess->uiflags & SESS_UIF_LOUD);
    ui->checkNoSep->setChecked(((MainWindow*)main)->sess.cursess->uiflags & SESS_UIF_NOSEP);

    if(((MainWindow*)main)->sess.cursess->uilocale == SESS_LOCALE_FA){ui->radioLocaleFarsi->setChecked(true);}
    else if(((MainWindow*)main)->sess.cursess->uilocale == SESS_LOCALE_EN){ui->radioLocaleEnglish->setChecked(true);}

    ui->comboBackgroundColor->setCurrentIndex(ui->comboBackgroundColor->findText(((MainWindow*)main)->strBackgroundColor));
    ui->comboFontColor->setCurrentIndex(ui->comboFontColor->findText(((MainWindow*)main)->strFontColor));

}

void ConfigDialog::on_checkHilight_clicked()
{
    if(((MainWindow*)main)->sess.cursess->uiflags & SESS_UIF_HILIGHT){
        ((MainWindow*)main)->sess.cursess->uiflags &= ~SESS_UIF_HILIGHT;
    } else {
        ((MainWindow*)main)->sess.cursess->uiflags |= SESS_UIF_HILIGHT;
    }
}

void ConfigDialog::on_checkDetails_clicked()
{
    if(((MainWindow*)main)->sess.cursess->uiflags & SESS_UIF_DET){
        ((MainWindow*)main)->sess.cursess->uiflags &= ~SESS_UIF_DET;
    } else {
        ((MainWindow*)main)->sess.cursess->uiflags |= SESS_UIF_DET;
    }
}

void ConfigDialog::on_checkLoud_clicked()
{
    if(((MainWindow*)main)->sess.cursess->uiflags & SESS_UIF_LOUD){
        ((MainWindow*)main)->sess.cursess->uiflags &= ~SESS_UIF_LOUD;
    } else {
        ((MainWindow*)main)->sess.cursess->uiflags |= SESS_UIF_LOUD;
    }
}

void ConfigDialog::on_checkAns_clicked()
{
    if(((MainWindow*)main)->sess.cursess->uiflags & SESS_UIF_ANS){
        ((MainWindow*)main)->sess.cursess->uiflags &= ~SESS_UIF_ANS;
    } else {
        ((MainWindow*)main)->sess.cursess->uiflags |= SESS_UIF_ANS;
    }
}

void ConfigDialog::on_checkDecimFix_clicked()
{
    if(((MainWindow*)main)->sess.cursess->uiflags & SESS_UIF_DECIMNOTR){
        ((MainWindow*)main)->sess.cursess->uiflags &= ~SESS_UIF_DECIMNOTR;
    } else {
        ((MainWindow*)main)->sess.cursess->uiflags |= SESS_UIF_DECIMNOTR;
    }
}

void ConfigDialog::on_checkNoSep_clicked()
{
    if(((MainWindow*)main)->sess.cursess->uiflags & SESS_UIF_NOSEP){
        ((MainWindow*)main)->sess.cursess->uiflags &= ~SESS_UIF_NOSEP;
    } else {
        ((MainWindow*)main)->sess.cursess->uiflags |= SESS_UIF_NOSEP;
    }
}


void ConfigDialog::on_radioLocaleFarsi_clicked()
{
    ((MainWindow*)main)->sess.cursess->uilocale = SESS_LOCALE_FA;
}

void ConfigDialog::on_radioLocaleEnglish_clicked()
{
    ((MainWindow*)main)->sess.cursess->uilocale = SESS_LOCALE_EN;
}

void ConfigDialog::on_comboBackgroundColor_currentIndexChanged(const QString &arg1)
{
    ((MainWindow*)main)->strBackgroundColor = arg1;
}

void ConfigDialog::on_comboFontColor_currentIndexChanged(const QString &arg1)
{
    ((MainWindow*)main)->strFontColor = arg1;
}

void ConfigDialog::on_comboStyle_currentIndexChanged(const QString &arg1)
{
    QApplication::setStyle(arg1);
}
