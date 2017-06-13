#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>

namespace Ui {
class ConfigDialog;
}

class ConfigDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ConfigDialog(QWidget *parent = 0);
    explicit ConfigDialog(QWidget *parent = 0, void* mainw = 0);
    ~ConfigDialog();
    void updateConf();

private slots:
    void on_checkHilight_clicked();

    void on_checkDetails_clicked();

    void on_checkLoud_clicked();

    void on_checkAns_clicked();

    void on_checkDecimFix_clicked();

    void on_checkNoSep_clicked();

    void on_radioLocaleFarsi_clicked();

    void on_radioLocaleEnglish_clicked();

    void on_comboBackgroundColor_currentIndexChanged(const QString &arg1);

    void on_comboFontColor_currentIndexChanged(const QString &arg1);

    void on_comboStyle_currentIndexChanged(const QString &arg1);

private:
    Ui::ConfigDialog *ui;
    void* main;

};

#endif // CONFIGDIALOG_H
