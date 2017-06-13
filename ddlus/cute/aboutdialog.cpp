#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include "cutemsg.h"

aboutDialog::aboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::aboutDialog)
{
    ui->setupUi(this);

    QString aboutHtml;

    char buf[300];

    char myname[] = {0x69,0x4d,0x4f,0x4f,0x44,0x10,0x70,0x59,0x5b,0x5b,0x5b,0x52,0x5e, 0x00};//+
    char mycontact[] = {0x57,0x5f,0x46,0x33,0x28,0x73,0x70,0x4,0x22,0x2b,0x26,0x21,0x25,0x64,0x28,0x23,0x20, 0x00};//=
    char mysite[] = {0x5c,0x59,0x45,0x57,0x40,0x1a,0x52,0x59,0x58,0x5f,0x55,0x5f,0x15,0x5f,0x52,0x53,0x10,0x33,0x28,0x36,0x26,0x6b,0x31,0x34,0x26,0x21,0x25,0x25,0x2d,0x2d,0x20,0x2f,0x2b,0x3d,0x30,0x3c, 0x00};///

    h311(myname, myname, '+', 13);
    h311(mycontact, mycontact, '=', 17);
    h311(mysite, mysite, '/', 36);

    snprintf(buf, 300, MSG_ABOUT_ME, myname, mycontact, mysite);

    aboutHtml = tr("<pre><h4>") + trUtf8(MSG_ABOUT_HELP).right(strlen(MSG_ABOUT_HELP)-ICON_SIZE)+tr("</h4>")
                 + trUtf8(MSG_ABOUT_VER).right(strlen(MSG_ABOUT_VER)-ICON_SIZE)+tr("<br><br>")
                 + trUtf8(MSG_ABOUT_NOTES).right(strlen(MSG_ABOUT_NOTES)-ICON_SIZE)+tr("<br><br>")
                 + tr("<b>") + trUtf8(buf).right(strlen(buf)-ICON_SIZE)+tr("</b><br><br>")
                 + trUtf8(MSG_ABOUT_LIC).right(strlen(MSG_ABOUT_LIC)-ICON_SIZE)+tr("</pre>");

    ui->textEdit->setHtml(aboutHtml);
    ui->textEdit->adjustSize();
}

aboutDialog::~aboutDialog()
{
    delete ui;
}

void aboutDialog::on_btnOk_clicked()
{
    this->close();
}
