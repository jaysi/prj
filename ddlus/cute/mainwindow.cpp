#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "aboutdialog.h"
#include "finddialog.h"

#include <QCloseEvent>

#include <QClipboard>
#include <QApplication>
#include <QMessageBox>
//#include <QtTest/QTest>

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

#include <unistd.h>

#include "cutemsg.h"

/*extern "C" {int nt_convert(long double val, char* buf, size_t bufsize);}*/

int qsleep(unsigned long sec){
    return false_;
}

int init_clipboard(session *sess){
    sess->cursess->clp = QApplication::clipboard();
    return true_;
}

int qcp(struct session* sess, char* str){
    ((QClipboard*)sess->cursess->clp)->setText(QString::fromUtf8(str));
    return true_;
}

char* qclipstr(struct session* sess){            

    if(((QClipboard*)sess->cursess->clp)->text().toUtf8().length() <= MAXNUMLEN){
        return ((QClipboard*)sess->cursess->clp)->text().toUtf8().data();
    } else {
        return NULL;
    }

}

int intext_(struct session* sess, char* buf, size_t bufsize){
    QComboBox* in = (QComboBox*)sess->cursess->rd;
    if(in->currentText().length()){
        snprintf(buf, bufsize, "%s", in->currentText().toUtf8().data());
        return true_;
    } else {
        return false_;
    }
}

/* this is not a real loop, just inherited from the original double_loop()
 * and made it compatible with qt
 */
int qdouble_loop(struct session* sess){

    switch(get_exp(sess)){
    case true_:
        delete_exp(sess);
        switch(run_exp(sess)){
        case true_:
            break;
        default:
            sess->cursess->expbuf[0] = 0;
            break;
        }
        finish_exp(sess);
        break;
    case continue_:
        break;
    case false_:
        run_exp_ci(sess);
        finish_exp(sess);
        break;
    }

    //update view
    if(sess->cursess->flags & SESSF_EXIT){
        if(sess->cursess->nextchild){
           printw(MSG_BUSY, MSG_CHILDPROC);
           sess->cursess->flags &= ~SESSF_EXIT;
           return continue_;
        }
        destroy_lock();
        return false_;
    }

    return true_;

}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    graphicshown = false;
    forceexit = false;

    ui->setupUi(this);        

//    QVBoxLayout* layout = new QVBoxLayout(this);
//    QLayout *l = layout();
////    QVBoxLayout* l = new QVBoxLayout;

//    l->removeWidget(ui->frameTitle);
//    l->removeWidget(ui->frameOut);
//    l->removeWidget(ui->frameMultiExp);
//    l->removeWidget(ui->frameExp);

    ui->verticalLayout->removeWidget(ui->frameExp);
    ui->verticalLayout->removeWidget(ui->frameOut);
    ui->verticalLayout->removeWidget(ui->frameMultiExp);
    ui->verticalLayout->removeWidget(ui->frameTitle);

    QSplitter* spl = new QSplitter(Qt::Vertical);

    spl->addWidget(ui->frameOut);
    spl->addWidget(ui->frameMultiExp);

    QSizePolicy policy = spl->sizePolicy();
    policy.setHorizontalStretch(QSizePolicy::Expanding);
    policy.setVerticalStretch(QSizePolicy::Expanding);

    spl->setSizePolicy(policy);

    ui->verticalLayout->addWidget(ui->frameTitle);
    ui->verticalLayout->addWidget(spl);
    ui->verticalLayout->addWidget(ui->frameExp);

//    ui->frameTitle->setSizePolicy(policy);
//    ui->frameExp->setSizePolicy(policy);
//    l->addWidget(ui->frameTitle);
//    l->addWidget(spl);
//    l->addWidget(ui->frameExp);

//    setLayout(l);

    ta_dbl.init = false;
    ta_about_double.init = false;
    ta_about_me.init = false;
    ta_contact.init = false;

    initGraph();

    qRegisterMetaType<QString>("QString&");
    connect(&up, SIGNAL(updateOut(QString, int, int, int, bool, bool)),
            this, SLOT(appendOut(QString, int, int, int, bool, bool)));
    //qRegisterMetaType<fnum_t>("fnum_t&");
//    connect(&up, SIGNAL(updatePlot(fnum_t, fnum_t)),
//            this, SLOT(appendPlot(fnum_t, fnum_t)));

    Qt::WindowFlags flags = plotDlg.windowFlags();
    flags |= Qt::WindowMaximizeButtonHint|Qt::WindowMinimizeButtonHint;
#ifdef ANDROID
    flags |= Qt::WindowFullScreen;
#endif
    plotDlg.setWindowFlags(flags);

    aedDlg = new arrayDialog(0, this);

    aedDlg->setWindowFlags(flags);

    switch(init_lock()){

        case true_:
            break;

        case continue_:

        switch(QMessageBox::question(NULL, tr("Lock"), tr(MSG_LOCALLOCK),
                                     QMessageBox::Yes|QMessageBox::No)){

            case QMessageBox::Yes:
            break;
            default:
                exit(1);
                break;

        }

        case false_:
            QMessageBox::warning(NULL, tr("Lock"), tr(MSG_NOLOCKFILE));
            break;

    }

    init_double(&sess, ui->comboEdit, &up, &plotDlg, aedDlg);

    //ui->toolKeyPad->setIcon(QIcon(":/res/calculator.png"));
    ui->toolKeyPad->installEventFilter(this);
    ui->comboEdit->installEventFilter(this);
    ui->toolHelp->installEventFilter(this);
//    ui->toolRun->installEventFilter(this);
    ui->toolPrev->installEventFilter(this);
    ui->toolNext->installEventFilter(this);
    ui->toolComplete->installEventFilter(this);
    ui->toolExit->installEventFilter(this);
    ui->toolMode->installEventFilter(this);
    ui->toolCopy->installEventFilter(this);
    ui->graphicsView->installEventFilter(this);
    ui->toolShut->installEventFilter(this);
    ui->textEdit->installEventFilter(this);
    ui->textMultiExp->installEventFilter(this);
    //ui->frameMultiExp->installEventFilter(this);

#ifdef NOFORK
//    ui->toolFork->setDisabled(true);
//    ui->toolForkList->setDisabled(true);
#else
//    ui->toolFork->installEventFilter(this);
//    ui->toolForkList->installEventFilter(this);
#endif

    keypad = false;
    space1 = false;
    matchlistsize = 0;
    Keypad = new KeyPad(0, this);
    Conf = new ConfigDialog(0, this);
    setKeyPad();
#ifndef ANDROID
    Keypad->adjustSize();
#else
    Keypad->setWindowFlags(Qt::WindowFullScreen);
    Conf->setWindowFlags(Qt::WindowFullScreen);
#endif
    forkid = 0;

    firstrun = true;
    sawme = false;

    nout = 0;

    ui->comboEdit->setEditable(true);
    ui->comboEdit->insertItem(0, tr(""));

    ui->comboEdit->setFocus();

#ifdef KEYPADMODE
    this->hide();
    on_toolKeyPad_clicked();
#endif

    strBackgroundColor = tr("black");
    strFontColor = tr("green");

    ui->textEdit->setContextMenuPolicy(Qt::NoContextMenu);
    findAct = new QAction(trUtf8("&Find..."), this);
    connect(findAct, SIGNAL(triggered()), this, SLOT(findText()));

    findDlg = new findDialog;
//    findDlg->setWindowFlags(Qt::FramelessWindowHint);
    findDlg->setWindowFlags(Qt::FramelessWindowHint|
                            Qt::WindowSystemMenuHint|
                            Qt::WindowStaysOnTopHint|
                            Qt::MSWindowsFixedSizeDialogHint);

    //TODO: TEMPORARY
    //ui->textMultiExp->hide();
    ui->frameMultiExp->hide();
    ui->toolShut->hide();

//    ui->frameMultiExp->setUpdatesEnabled(false);
//    ui->frameOut->setUpdatesEnabled(false);

}

MainWindow::~MainWindow()
{    
    delete Keypad;
    delete Conf;
    delete ui;

}

void MainWindow::appendOut(QString str, int r, int g, int b, bool bold, bool italic){        

    QFont fnt = ui->textEdit->font();

    QColor qc(r,g,b);
    QColor prevc = ui->textEdit->textColor();
    QColor bgc;

    /*if(!ui->toolCon->isChecked())*/ ui->textEdit->setTextColor(qc);
    ///*else*/ ui->textEdit->setTextColor(QColor(strFontColor));

    if(sess.cursess->uiflags & SESS_UIF_HILIGHT){
        if(nout%2){
            /*if(!ui->toolCon->isChecked())*/ bgc = QColor(200,200,200);
        } else {
            /*if(!ui->toolCon->isChecked())*/ bgc = QColor(255,255,255);
        }
        ui->textEdit->setTextBackgroundColor(bgc);
    }

    fnt.setBold(bold);
    fnt.setItalic(italic);

    ui->textEdit->setCurrentFont(fnt);

    ui->textEdit->append(str);

    QTextCursor c = ui->textEdit->textCursor();
    c.movePosition(QTextCursor::End);

    ui->textEdit->setTextCursor(c);

    //qc = QColor(32, 32, 32);

    /*if(!ui->toolCon->isChecked())*/ ui->textEdit->setTextColor(prevc);
    fnt.setBold(false);
    fnt.setItalic(false);
    ui->textEdit->setCurrentFont(fnt);

//    if(gconf.hilight){
//        if(nout%2){
//            ui->textEdit->setTextBackgroundColor(bgcolor);
//        }
//    }

    nout++;

}

void MainWindow::on_comboEdit_returnPressed()
{

    if(!sawme){
        if(ui->comboEdit->currentText() == tr("show m3 demo")){
            ui->comboEdit->setCurrentIndex(0);
            ui->toolShut->show();
            return;
        }
    }

    if(firstrun && !ui->comboEdit->currentText().isEmpty()) firstrun = false;

    switch(qdouble_loop(&sess)){
        case true_:
        case continue_:
        break;
        case false_:
        exit(1);
        break;
    }

    ui->labelTime->setText(tr("%1").arg(sess.cursess->rt, 12, 'f', 10));

    if(sess.cursess->mode1 == SESSM1_ACC){
        ui->textEdit->setAlignment(Qt::AlignRight);
    } else {
        ui->textEdit->setAlignment(Qt::AlignLeft);
    }

    if(sess.cursess->mode1 == SESSM1_PROG){
            ui->textEdit->append(ui->comboEdit->currentText());
    }

    if(/*!ui->comboEdit->itemText(1).isEmpty() &&*/ ui->comboEdit->currentText() != ui->comboEdit->itemText(1)){
        ui->comboEdit->insertItem(1, ui->comboEdit->currentText());
    }

    if(!ui->comboEdit->itemText(0).isEmpty() && !ui->comboEdit->itemText(1).isEmpty()){
        ui->comboEdit->insertItem(0, tr(""));
    }
    ui->comboEdit->setCurrentIndex(0);

    //ui->comboEdit->clear();

    setKeyPad();

}

int MainWindow::SetMatchList(){
    struct variable* var;
    char* vname = NULL;
    char *start, *end;
    size_t vnamelen, varnamelen;
    struct tok_s* tk;
    int i;
    int j;
//    QStringList strList;
    //qDebug()<<tr("matchlistsize: %1").arg(matchlistsize);

    if(!matchlistsize){//create list and select first

        start = (char*)ui->comboEdit->currentText().toStdString().c_str();
        if(!strlen(start)) return 0;

        matchexp = (char*)malloc(strlen(start)+1);
        strcpy(matchexp, start);
        start = matchexp;
        end = start;

        //qDebug()<<tr("matchexp: %1, start: %2, end: %3").arg(matchexp).arg(start).arg(end);

        while(end){
            end = strpbrk(start, dbl_delimlist);
            if(!end){
                vname = start;
            } else {
                start = end + 1;
            }
        }

        //qDebug()<<tr("matchexp: %1, start: %2, vname: %3").arg(matchexp).arg(start).arg(vname);

        if(vname){

            if((vnamelen = strlen(vname))){

                //search vars
                for(var = sess.cursess->vfirst; var; var = var->next){
                    varnamelen = strlen(var->name);
                    if(!memcmp(vname, var->name, varnamelen<vnamelen?varnamelen:vnamelen)){
                        matchlistsize++;
                    }
                }

                //search toks
                j = 0;
                while((tk = tokidptr(&sess, (tok_t)j))){

                    if(tk->type == TOKT_SPEC || tk->type == TOKT_MATH_BASIC || tk->type == TOKT_NUMBER || tk->type == TOKT_MATH_LOGIC || tk->type == TOKT_STRING) goto endthis;

                    i = 0;
                    while(tk->form[i]){
                        varnamelen = strlen(tk->form[i]);
                        if(!memcmp(vname, tk->form[i], varnamelen<vnamelen?varnamelen:vnamelen)){
                            matchlistsize++;
                        }
                        i++;
                    }
                    endthis:
                    j++;
                }
           }

            //qDebug()<<tr("matchlistsize2: %1").arg(matchlistsize);

            if(matchlistsize){

                matchlistitem = 0;
                matchlist = (char**)malloc(sizeof(char**)*matchlistsize);

                //assign vars
                for(var = sess.cursess->vfirst; var; var = var->next){
                    varnamelen = strlen(var->name);
                    if(!memcmp(vname, var->name, varnamelen<vnamelen?varnamelen:vnamelen)){
                        //ui->textEdit->append(tr("%1: %2").arg(matchlistitem).arg(var->name));
                        if(matchlistitem > 0){
                            if(strcmp(matchlist[matchlistitem-1], var->name)){
                                matchlist[matchlistitem] = var->name;
//                                strList << var->name;
                                matchlistitem++;
                            } else {
                                matchlistsize--;
                            }

                        } else {
                            matchlist[matchlistitem] = var->name;
//                            strList << var->name;
                            matchlistitem++;
                        }
                    }
                }

                //assign toks
                j = 0;
                while((tk = tokidptr(&sess, (tok_t)j))){

                    if(tk->type == TOKT_SPEC || tk->type == TOKT_MATH_BASIC || tk->type == TOKT_NUMBER || tk->type == TOKT_MATH_LOGIC || tk->type == TOKT_STRING) goto endthisagain;

                    i = 0;
                    while(tk->form[i]){
                        varnamelen = strlen(tk->form[i]);
                        if(!memcmp(vname, tk->form[i], varnamelen<vnamelen?varnamelen:vnamelen)){
                            if(matchlistitem > 0){
                                if(strcmp(matchlist[matchlistitem-1], tk->form[i])){
                                    matchlist[matchlistitem] = tk->form[i];
//                                    strList << tk->form[i];
                                    matchlistitem++;
                                } else {
                                    matchlistsize--;
                                }

                            } else {
                                matchlist[matchlistitem] = tk->form[i];
//                                strList << tk->form[i];
                                matchlistitem++;
                            }
                        }
                        i++;
                    }
                    endthisagain:
                    j++;
                }

                matchlistitem = 0;

            }

            //qDebug()<<tr("matchlistitem: %1").arg(matchlistitem);

            *vname = 0;

            //qDebug()<<tr("vname: %1").arg(vname);

        }

//        QCompleter* qc = new QCompleter(strList);
//        ui->comboEdit->setCompleter(qc);
//        ui->textEdit->append(strList);

    } else {//browse list

        if(matchlistitem < matchlistsize - 1) matchlistitem++;
        else matchlistitem = 0;

    }

    if(matchlistsize){
        //qDebug()<<tr("i:%1, exp: %2, ml: %3, vname: %4, size: %5").arg(matchlistitem).arg(tr(matchexp)).arg(tr(matchlist[matchlistitem])).arg(tr(vname)).arg(matchlistsize);
        //ui->textEdit->append(tr("i:%1, exp: %2, ml: %3, vname: %4, size: %5").arg(matchlistitem).arg(tr(matchexp)).arg(tr(matchlist[matchlistitem])).arg(tr(vname)).arg(matchlistsize));

        ui->comboEdit->lineEdit()->setText(tr(matchexp)+tr(matchlist[matchlistitem]));

    }

    return 0;

}

bool MainWindow::eventFilter(QObject *obj, QEvent *event){

    QToolButton* tbtn;
    QComboBox* edit;
    QKeyEvent* key;
    QTextEdit* out;
    QMenu* menu;

    //static int baseY = -1, mY, mH, oH;

    if(obj->objectName() == tr("toolShut")){
        tbtn = dynamic_cast<QToolButton*>(obj);
        Q_ASSERT(tbtn);
        if(event->type() == QEvent::Enter){
            if(!sawme){
                tbtn->setIcon(QIcon(":/res/question-26.png"));
            } else {
                tbtn->setIcon(QIcon(":/res/cool-26.png"));
            }

        } else if(event->type() == QEvent::MouseButtonPress){
            if(!sawme){
                tbtn->setIcon(QIcon(":/res/wink-26.png"));
            } else {
                tbtn->setIcon(QIcon(":/res/tongue-26.png"));
            }
        } else if(event->type() == QEvent::MouseButtonRelease){
            tbtn->setIcon(QIcon(":/res/cool-26.png"));
        } else if(event->type() == QEvent::Leave){
            if(!sawme){
                tbtn->setIcon(QIcon("NULL"));
            }
        }

    } else if(obj->objectName() == tr("comboEdit")){
        edit = dynamic_cast<QComboBox*>(obj);
        Q_ASSERT(edit);
        if(event->type() == QEvent::KeyPress){
            key = dynamic_cast<QKeyEvent*>(event);
            Q_ASSERT(key);

            if(key->key() == Qt::Key_Tab){
                if(key->modifiers() == Qt::Key_Shift){//jump to out screen
                    //TODO: reverse match list
                } else {
                    //variable name completion
                    SetMatchList();
                }
            } else if(matchlistsize){
                free(matchlist);
                free(matchexp);
                matchlistsize = 0;
            }

            if(key->key() == Qt::Key_Space){
                if(!space1){
                    space1 = true;
                } else {
                    space1 = false;
                    on_comboEdit_returnPressed();
                    //ui->comboEdit->clear();
                    return true;
                }
            } else {
                space1 = false;
            }

            if(key->key() == Qt::Key_Return || key->key() == Qt::Key_Enter){
                on_comboEdit_returnPressed();
            }

//            if(key->key() == Qt::Key_Up){
//                //TODO: press ctrl+z
//                get_prev_hist(&sess, sess.cursess->expbuf, sess.cursess->expbufsize);
//                ui->comboEdit->lineEdit()->setText(trUtf8(sess.cursess->expbuf));
//            }
//            if(key->key() == Qt::Key_Down){
//                //TODO: press ctrl+y
//                get_next_hist(&sess, sess.cursess->expbuf, sess.cursess->expbufsize);
//                ui->comboEdit->lineEdit()->setText(trUtf8(sess.cursess->expbuf));
//            }
            if(key->key() == Qt::Key_Escape){
                ui->comboEdit->setCurrentIndex(0);
            }

            if(key->key() == Qt::Key_Up && key->modifiers() == Qt::ControlModifier){
                if(ui->textMultiExp->isVisible()) ui->textMultiExp->setFocus();
                else ui->textEdit->setFocus();
            }

            if(key->key() == Qt::Key_Left && key->modifiers() == Qt::ControlModifier){
                on_toolPrev_clicked();
            }

            if(key->key() == Qt::Key_Right && key->modifiers() == Qt::ControlModifier){
                on_toolNext_clicked();
            }

        }
    } else if(obj->objectName() == tr("graphicsView")){
        if(event->type() == QEvent::Resize){
            //ui->graphicsView->fitInView(ui->graphicsView->sceneRect(), Qt::KeepAspectRatio);
//            if(graphicshown){
                recalcTextAnimPos(&ta_dbl);
                if(ta_about_double.init){
                    recalcTextAnimPos(&ta_about_double);
                    showTextAnim(&ta_about_double, NULL, NULL, -1, 500);
                }
                if(ta_about_me.init){
                    recalcTextAnimPos(&ta_about_me);
                    showTextAnim(&ta_about_me, NULL, NULL, -1, 500);
                }
                showTextAnim(&ta_dbl, scene, NULL, 500, -1);
//            }
        }
    } else if(obj->objectName() == tr("textEdit")){
        out = dynamic_cast<QTextEdit*>(obj);
//        QContextMenuEvent* ev = new QContextMenuEvent(event);
        Q_ASSERT(out);
        if(event->type() == QEvent::ContextMenu){
            menu = out->createStandardContextMenu();
            menu->addSeparator();
            menu->addAction(findAct);
            menu->exec(QCursor::pos());
        } else if(event->type() == QEvent::KeyPress){
            key = dynamic_cast<QKeyEvent*>(event);
            switch(key->key()){
                case Qt::Key_F:
                    if(key->modifiers() == Qt::ControlModifier){
                        findText();
                    }
                    break;
            case Qt::Key_Plus:
            case Qt::Key_Equal:
                if(key->modifiers() == Qt::ControlModifier){
                    ui->textEdit->zoomIn();
                }
                break;
            case Qt::Key_Minus:
                if(key->modifiers() == Qt::ControlModifier){
                    ui->textEdit->zoomOut();
                }
                break;
            case Qt::Key_Down:
                if(key->modifiers() == Qt::ControlModifier){
                    if(ui->textMultiExp->isVisible()) ui->textMultiExp->setFocus();
                    else ui->comboEdit->setFocus();
                }
            default:
                break;
            }
        }
    } else if(obj->objectName() == tr("textMultiExp")){
        if(event->type() == QEvent::KeyPress){
            key = dynamic_cast<QKeyEvent*>(event);
            switch(key->key()){
                case Qt::Key_Enter:
                case Qt::Key_Return:
                    if(key->modifiers() == Qt::AltModifier){
                        on_toolRunMultiExp_clicked();
                    }
                    break;
            case Qt::Key_S:
                if(key->modifiers() == Qt::ControlModifier){
                    on_toolSaveMultiExp_clicked();
                }
            break;

            case Qt::Key_O:
                if(key->modifiers() == Qt::ControlModifier){
                    on_toolLoadMultiExp_clicked();
                }
            break;

            case Qt::Key_Down:
                if(key->modifiers() == Qt::ControlModifier){
                    ui->comboEdit->setFocus();
                    return true;
                }
            break;

            case Qt::Key_Up:
                if(key->modifiers() == Qt::ControlModifier){
                    ui->textEdit->setFocus();
                    return true;
                }
                break;

            default:
                break;
            }
        }
    }/* else if(obj->objectName() == tr("frameMultiExp")){

        switch(event->type()){
        case QEvent::Resize:
            return true;
            break;
        case QEvent::MouseButtonPress:
            baseY = ui->frameMultiExp->cursor().pos().y();
            //qDebug()<<"P"<<baseY;
            break;
        case QEvent::MouseButtonRelease:
            //ui->frameOut->adjustSize();
            //this->adjustSize();
            //qDebug()<<"RST";
            baseY = -1;
            break;
        case QEvent::MouseMove:
            //qDebug()<<"M"<<baseY;
            if(baseY > -1){

                int res = (baseY - ui->frameMultiExp->cursor().pos().y());

                if(res < 0 && ui->frameMultiExp->height() <= height()/5) break;
                if(res > 0 && ui->frameOut->height() <= height()/3) break;

                ui->frameMultiExp->resize(
                            ui->frameMultiExp->width(),
                            ui->frameMultiExp->height() + res/10);

                ui->frameOut->resize(ui->frameOut->width(),
                                     ui->frameOut->height() - res/10);

                ui->frameMultiExp->move(ui->frameMultiExp->x(), ui->frameMultiExp->y() - res/10);

            }
            break;
        default:
            break;

        }

    }*/

    return false;
}

void MainWindow::on_toolKeyPad_clicked()
{
    Qt::WindowFlags flags;
    if(!keypad){
        keypad = true;
        flags = Keypad->windowFlags();
        flags |= Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint;
        Keypad->setWindowFlags(flags);
        Keypad->show();
    } else {
        keypad = false;
        Keypad->hide();
    }
}

void MainWindow::setKeyPadClose(){
    keypad = false;
    //ui->toolKeyPad->setIcon(QIcon(":/res/calculator.png"));
    ui->toolKeyPad->setChecked(false);
}

void MainWindow::appendToExp(QString str){

    QString oldstr;

    oldstr = ui->comboEdit->currentText();
    oldstr.append(str);
    ui->comboEdit->lineEdit()->setText(oldstr);

}

void MainWindow::deleteFromExp(){
    QString oldstr;

    oldstr = ui->comboEdit->currentText();

    oldstr.chop(1);

    ui->comboEdit->lineEdit()->setText(oldstr);

}

void MainWindow::clearExp(){
    ui->comboEdit->clear();
}

void MainWindow::setKeyPad(){
    QString stat;
    QString last;
    char* helper;
    int has_err = 0;

    if(!memcmp(sess.cursess->outbuf, ANS_ICON, ICON_SIZE)){
        switch(sess.cursess->mode1){
        case SESSM1_ACC:
            last = QString(strstr(sess.cursess->outbuf, "= ")+2);
        break;
        default:
            if(sess.cursess->uiflags & SESS_UIF_DET){
                helper = strstr(sess.cursess->outbuf, ": ");
                if(helper){
                    last = QString(helper + 2);
                    last.truncate(last.indexOf(tr("\t")));
                }
            } else {
                helper = strstr(sess.cursess->outbuf, " ");
                if(!helper) helper = sess.cursess->outbuf;
                last = QString(helper);
            }
        break;
        }
    } else if(!memcmp(sess.cursess->outbuf, ERR_ICON, ICON_SIZE)){
        last = QString(sess.cursess->outbuf + ICON_SIZE);
        has_err = 1;
    } else if(!memcmp(sess.cursess->outbuf, WARN_ICON, ICON_SIZE)){
        last = QString(sess.cursess->outbuf + ICON_SIZE);
        has_err = 1;
    } else {
        last = QString("");
    }

    Keypad->setLast(last);

    if(has_err){
        //Keypad->setMode(tr("<font color=\"red\"><blink> [ ERROR ] </blink></font>"));
        Keypad->setMode(trUtf8(CMSG_MODEERR));
        return;
    }

    switch(sess.cursess->mode1){
        case SESSM1_ACC:
        stat = trUtf8(CMSG_MODEFIN);
        break;
        case SESSM1_PROG:
        stat = trUtf8(CMSG_MODEPROG);
        break;
        default:
        stat = trUtf8(CMSG_MODENORM);
        break;
    }

    if(sess.cursess->uiflags & SESS_UIF_DET){
        stat.append(trUtf8(CMSG_UIDET));
    } else {
        stat.append(trUtf8(CMSG_UINODET));
    }

    if(sess.cursess->uiflags & SESS_UIF_LOUD){
        stat.append(trUtf8(CMSG_UILOUD));
    } else {
        stat.append(trUtf8(CMSG_UISILENT));
    }

    if(sess.cursess->fileflags & SESS_FILEF_LINE){
        stat.append(trUtf8(CMSG_FILELINE));
    } else {
        stat.append(trUtf8(CMSG_FILEBULK));
    }

    if(sess.cursess->uiflags & SESS_UIF_ANS){
        stat.append(trUtf8(CMSG_UIANS));
    } else {
        stat.append(trUtf8(CMSG_UINOANS));
    }

    Keypad->setMode(stat);
}

void MainWindow::closeEvent(QCloseEvent *event){

    if(!forceexit){

        switch(QMessageBox::question(this, "Exit", "Do you want to save state?", QMessageBox::No|QMessageBox::Yes|QMessageBox::Cancel)){

            case QMessageBox::Yes:
            on_toolSaveStat_clicked();
            break;

        case QMessageBox::Cancel:
            event->ignore();
            return;

        default:
        break;
        }

    }

    Conf->close();
    Keypad->close();
//    aview->close();
    closesavefile(&sess);
    destroy_lock();
    event->accept();
}

void MainWindow::on_toolClear_clicked()
{
    ui->textEdit->clear();
}

void MainWindow::on_toolHelp_clicked()
{
    QString str = ui->comboEdit->currentText();

    if(!strpbrk(str.toUtf8().data(), dbl_delimlist) && tokid(&sess, str.toUtf8().data()) != TOK_INVAL){
        ui->comboEdit->lineEdit()->setText(tr("help %1;").arg(str));
        on_comboEdit_returnPressed();
        ui->comboEdit->clear();
    } else {
        ui->comboEdit->lineEdit()->setText(tr("help;"));
        on_comboEdit_returnPressed();
        ui->comboEdit->lineEdit()->setText(str);
    }

}

void MainWindow::on_toolRun_clicked()
{
    if(sess.cursess->mode1 != SESSM1_PROG){
        on_comboEdit_returnPressed();
    } else {
        on_comboEdit_returnPressed();
        ui->comboEdit->lineEdit()->setText(tr(";"));
        on_comboEdit_returnPressed();
    }
}

//void MainWindow::on_toolFork_clicked()
//{
//    QString str = ui->comboEdit->currentText();
//    ui->comboEdit->lineEdit()->setText(tr("fork(proc%1, \"%2\");").arg(forkid).arg(str));
//    ui->textEdit->append(tr("\"%1\" forked as \"proc%2\"").arg(str).arg(forkid));
//    on_comboEdit_returnPressed();
//    forkid++;
//}

//void MainWindow::on_toolForkList_clicked()
//{

//    runExp(tr("fork;"));

//}

void MainWindow::on_toolNext_clicked()
{
    get_prev_hist(&sess, sess.cursess->expbuf, sess.cursess->expbufsize);
    ui->comboEdit->lineEdit()->setText(tr(sess.cursess->expbuf));

}

void MainWindow::on_toolPrev_clicked()
{
    //TODO: press ctrl+y
    get_next_hist(&sess, sess.cursess->expbuf, sess.cursess->expbufsize);
    ui->comboEdit->lineEdit()->setText(tr(sess.cursess->expbuf));

}

void MainWindow::on_toolComplete_clicked()
{
    SetMatchList();
}

void MainWindow::on_toolExit_clicked()
{

    forceexit = true;

    on_toolSaveStat_clicked();

    QString str = ui->comboEdit->currentText();
    ui->comboEdit->lineEdit()->setText(tr("exit;"));    
    on_comboEdit_returnPressed();
    ui->comboEdit->lineEdit()->setText(str);
}

void MainWindow::on_toolMode_clicked()
{
    QString str;
    str = ui->comboEdit->currentText();
    switch(sess.cursess->mode1){
    case SESSM1_NR:
        ui->comboEdit->lineEdit()->setText(tr("mode fin;"));
        appendOut(trUtf8(CMSG_MODE).append(trUtf8(CMSG_MODEFIN)), 0, 0, 254, false, false);
        break;
    case SESSM1_ACC:
        ui->comboEdit->lineEdit()->setText(tr("mode prog;"));
        appendOut(trUtf8(CMSG_MODE).append(trUtf8(CMSG_MODEPROG)), 0, 0, 254, false, false);
        break;
    case SESSM1_PROG:
        ui->comboEdit->lineEdit()->setText(tr("$"));
        appendOut(trUtf8(CMSG_MODE).append(trUtf8(CMSG_MODENORM)), 0, 0, 254, false, false);
        break;
    }
    on_comboEdit_returnPressed();
    ui->comboEdit->lineEdit()->setText(str);

}

void MainWindow::on_toolClearIn_clicked()
{
    ui->comboEdit->clear();
    ui->comboEdit->insertItem(0, tr(""));
}

void MainWindow::on_toolCopy_clicked()
{
    QString str;
    str = ui->comboEdit->currentText();
    if(str.isEmpty()){
        ui->comboEdit->lineEdit()->setText(tr("cc;"));
    } else {
        if(strpbrk(str.toUtf8().data(), dbl_delimlist)){
            ui->comboEdit->lineEdit()->setText(tr("cc \"%1\";").arg(str));
        } else {
            ui->comboEdit->lineEdit()->setText(tr("cc %1;").arg(str));
        }
    }
    on_comboEdit_returnPressed();
    ui->comboEdit->lineEdit()->setText(str);

}

void MainWindow::runExp(QString exp){
    QString str = this->ui->comboEdit->currentText();
    this->ui->comboEdit->lineEdit()->setText(exp);
    on_comboEdit_returnPressed();
    this->ui->comboEdit->lineEdit()->setText(str);
}


void MainWindow::on_toolDecimLess_clicked()
{
    runExp(tr("sys(ui, \"decimal: -\");"));
    ui->textEdit->append(trUtf8(CMSG_DECIMAL).arg(sess.cursess->decim));
}

void MainWindow::on_toolDecimMore_clicked()
{
    runExp(tr("sys(ui, \"decimal: +\");"));
    ui->textEdit->append(trUtf8(CMSG_DECIMAL).arg(sess.cursess->decim));
}

void MainWindow::moveEvent(QMoveEvent *event){


//   if(aview) aview->move(QPoint(this->pos().x() + this->width(),
//                   this->pos().y())
//               );

    event->accept();
}

void MainWindow::recalcTextAnimPos(struct textanim* ta){

    QState* st;    

    //qreal baserightx = 0;//ui->graphicsView->rect().topRight().x();
    //-ui->graphicsView->rect().topRight().x();

    //qreal baseleftx = qreal(0);
    qreal baseleftx = qreal(0);
    qreal baserightx = qreal(ui->graphicsView->rect().topRight().x() - (ta->qpixmaplist.count())*ta->qpixmaplist[0]->width());

    //baserightx -= (ta->qpixmaplist.count()-1)*ta->qpixmaplist[0]->width();
    //qDebug()<<tr("L:%1, R: %2").arg(baseleftx).arg(baserightx);

    for (int i = 0; i < ta->pixmaplist.count(); ++i) {
        Pixmap *item = ta->pixmaplist.at(i);

        st = ta->statelist.at(ST_OUTLEFT);
        st->assignProperty(item, "pos", QPointF(baseleftx, 0));

        st = ta->statelist.at(ST_RIGHT);
        st->assignProperty(item, "pos", QPointF(baserightx+i*ta->qpixmaplist[0]->width() ,0));

        st = ta->statelist.at(ST_CENTER);
        st->assignProperty(item, "pos", QPointF());

    }

}

QList<QPixmap* >MainWindow::text2Graph(char* text){
    QList<QPixmap* >qpixmap_list;
    struct letterpix{
        char letter;
        char* res[21];
    } letterpic[] = {
        {'a',{":/res/char/a.png"}},
        {'b',{":/res/char/b.png"}},
        {'c',{":/res/char/c.png"}},
        {'d',{":/res/char/d.png"}},
        {'e',{":/res/char/e.png"}},
        {'f',{":/res/char/f.png"}},
        {'g',{":/res/char/g.png"}},
        {'h',{":/res/char/h.png"}},
        {'i',{":/res/char/i.png"}},
        {'j',{":/res/char/j.png"}},
        {'k',{":/res/char/k.png"}},
        {'l',{":/res/char/l.png"}},
        {'m',{":/res/char/m.png"}},
        {'n',{":/res/char/n.png"}},
        {'o',{":/res/char/o.png"}},
        {'p',{":/res/char/p.png"}},
        {'q',{":/res/char/q.png"}},
        {'r',{":/res/char/r.png"}},
        {'s',{":/res/char/s.png"}},
        {'t',{":/res/char/t.png"}},
        {'u',{":/res/char/u.png"}},
        {'v',{":/res/char/v.png"}},
        {'w',{":/res/char/w.png"}},
        {'x',{":/res/char/x.png"}},
        {'y',{":/res/char/y.png"}},
        {'z',{":/res/char/z.png"}},
        {'0',{":/res/char/0.png"}},
        {'1',{":/res/char/1.png"}},
        {'2',{":/res/char/2.png"}},
        {'3',{":/res/char/3.png"}},
        {'4',{":/res/char/4.png"}},
        {'5',{":/res/char/5.png"}},
        {'6',{":/res/char/6.png"}},
        {'7',{":/res/char/7.png"}},
        {'8',{":/res/char/8.png"}},
        {'9',{":/res/char/9.png"}},
        {' ',{":/res/char/space.png"}},
        {0,NULL},
     };

    struct letterpix* lp;

    for(;*text; text++){
        for(lp = letterpic; lp->letter; lp++){
            if(lp->letter == *text){
                QPixmap* pix = new QPixmap(lp->res[0]);
                qpixmap_list << pix;
            }
        }
    }

    return qpixmap_list;

}

void MainWindow::showEvent(QShowEvent* event){
#ifdef KEYPADMODE
    event->ignore();
#else
    event->accept();
#endif
}

void MainWindow::outNow(){

//    qDebug()<<tr("outNow!");

    ui->toolShut->hide();

    disconnect(ta_dbl.timerlist[0], SIGNAL(timeout()));

    int i;
    for(i = 0; i < ta_about_double.pixmaplist.count(); i++)
        ta_about_double.pixmaplist[i]->hide();

    for(i = 0; i < ta_about_me.pixmaplist.count(); i++)
        ta_about_me.pixmaplist[i]->hide();

    for(i = 0; i < ta_contact.pixmaplist.count(); i++)
        ta_contact.pixmaplist[i]->hide();

//    deleteTextAnim(&ta_about_double);
//    deleteTextAnim(&ta_about_me);


}

int MainWindow::deleteTextAnim(textanim *ta){
    QTimer* t;
    QPropertyAnimation* anim;
    QState* st;
    Pixmap* p;
    QPixmap* qp;
    int i;

    ta->states->stop();
    for(i = 0; i < ta->timerlist.count(); i++){
        t = ta->timerlist[i];
        disconnect(t, SIGNAL(timeout()));
        ta->timerlist.removeAt(i);
        delete t;
    }
    ta->timerlist.empty();

    qDebug()<<tr("1");

//    ta->trans ???

    ta->trans->removeAnimation(ta->group);

    ta->trans->deleteLater();

    ta->group->stop();

    delete ta->group;

    qDebug()<<tr("2");

    delete ta->states;

    for(i = 0; i < ta->statelist.count(); i++){
        st = ta->statelist[i];
        ta->statelist.removeAt(i);
        delete st;
    }
    ta->statelist.empty();

    delete ta->rootState;

    qDebug()<<tr("4");

    for(i = 0; i < ta->animlist.count(); i++){
        anim = ta->animlist[i];
        anim->stop();
        ta->animlist.removeAt(i);
        delete anim;
    }
    ta->animlist.empty();

    qDebug()<<tr("3");

    for(i = 0; i < ta->pixmaplist.count(); i++){
        p = ta->pixmaplist[i];
        ta->pixmaplist.removeAt(i);
        scene->removeItem(p);
        delete p;
    }
    ta->pixmaplist.empty();

    qDebug()<<tr("5");

    for(i = 0; i < ta->qpixmaplist.count(); i++){
        qp = ta->qpixmaplist[i];
        ta->qpixmaplist.removeAt(i);
        delete qp;
    }
    ta->qpixmaplist.empty();

    qDebug()<<tr("6");

    return 0;
}

int MainWindow::showTextAnim(textanim *ta, QGraphicsScene* scene, char *text, int in, int out){

    int i;
    QPixmap* qpix;
    QState* st;

    if(ta->init){
        if(in >= 0) ta->timerlist[0]->start(in);
        if(out >= 0){
            ta->timerlist[2]->start(out-100);
            ta->timerlist[1]->start(out);
        }
        return 0;
    }

    ta->in = in;
    ta->out = out;
    ta->text = NULL;

    ta->scene = scene;

    ta->qpixmaplist = text2Graph(text);

    for (i = 0; i < ta->qpixmaplist.count(); ++i) {
        qpix = ta->qpixmaplist.at(i);
        Pixmap *item = new Pixmap(*qpix);

        item->setOffset(-qpix->width()/2, -qpix->height()/2);
        //item->setOffset(ui->graphicsView->geometry().left(), ui->graphicsView->rect().height());

        item->setZValue(i);
        ta->pixmaplist << item;        
        scene->addItem(item);
    }

    ta->rootState = new QState;
    for(i = 0; i < ST_CUSTOM; i++){
        st = new QState(ta->rootState);
        ta->statelist << st;
    }  

    recalcTextAnimPos(ta);

    ta->states = new QStateMachine;
    ta->states->addState(ta->rootState);
    ta->states->setInitialState(ta->rootState);
    ta->rootState->setInitialState(ta->statelist[ST_CENTER]);

    ta->group = new QParallelAnimationGroup;
    for (int i = 0; i < ta->pixmaplist.count(); ++i) {
        QPropertyAnimation* anim = new QPropertyAnimation(ta->pixmaplist[i], "pos");
        anim->setDuration(ta->pixmaplist.count()*500 - i * 500);
        //anim->setEasingCurve(QEasingCurve::InOutExpo);
        anim->setEasingCurve(QEasingCurve::OutExpo);
//            if(i%2) anim->setEasingCurve(QEasingCurve::OutExpo);
//            else anim->setEasingCurve(QEasingCurve::OutBounce);
        ta->group->addAnimation(anim);
        ta->animlist << anim;
    }

    ta->init = true;

    //ta->trans = rootState->addTransition(ui->toolShut, SIGNAL(pressed()), leftState);

    QTimer* timer_start, *timer_end, *timer_mid;

    timer_start = new QTimer;
    ta->timerlist << timer_start;

    timer_start->start(in);
    timer_start->setSingleShot(true);
    ta->trans = ta->rootState->addTransition(timer_start, SIGNAL(timeout()), ta->statelist[ST_RIGHT]);
    ta->trans->addAnimation(ta->group);

    timer_end = new QTimer;
    ta->timerlist << timer_end;
    timer_end->setSingleShot(true);
    ta->trans = ta->rootState->addTransition(timer_end, SIGNAL(timeout()), ta->statelist[ST_OUTLEFT]);
    ta->trans->addAnimation(ta->group);

    timer_mid = new QTimer;
    ta->timerlist << timer_mid;
    timer_mid->setSingleShot(true);
    ta->trans = ta->rootState->addTransition(timer_mid, SIGNAL(timeout()), ta->statelist[ST_CENTER]);
    ta->trans->addAnimation(ta->group);

    if(out >= 0){

        timer_mid->start(out-100);
        timer_end->start(out);

    }

    ta->states->start();

    return 0;

}

void MainWindow::initGraph(){

        scene = new QGraphicsScene;
        ui->graphicsView->setStyleSheet("background: transparent");
        ui->graphicsView->setScene(scene);
        ui->graphicsView->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
        ui->graphicsView->setCacheMode(QGraphicsView::CacheBackground);
        ui->graphicsView->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

        showTextAnim(&ta_dbl, scene, "double", 500, -1);//a -1 means stay

        graphicshown = true;
}

void MainWindow::on_toolShut_clicked()
{

    char double_desc[] = {0x45,0x5a,0x56,0x14,0x51,0x59,0x42,0x5a,0x55,0x5f,0x1b,0x5f,0x5c,0x52,0x5c,0x35,0x2d,0x23,0x37,0x2b,0x37, 0x00};
    char my_name[] = {0x8,0x12,0x4c,0xf,0xf,0xd,0x11,0x1a,0x52,0x12,0x1f,0x1d,0x19,0x19,0x1c,0x10, 0x00};
    char contact[] = {0x8,0x2,0x1d,0x16,0xf,0x56,0x5b,0x49,0xb,0x1f,0x4c,0xa,0x3,0xe,0x19,0x1d,0x52,0x17,0x1b,0x1,0x56,0x14,0x17,0x14, 0x00};

    h311(double_desc, double_desc, '1', 21);
    h311(my_name, my_name, 'j', 16);
    h311(contact, contact, 'b', 24);

    //wait 60 seconds
//    timer1->start(1000*30);
//    connect(timer1, SIGNAL(timeout()), this, SLOT(ResetScene()));

    if(!sawme){

//        main
        showTextAnim(&ta_dbl, NULL, NULL, 50*1000, 500);
        showTextAnim(&ta_about_double, scene, double_desc, 500, 9500);
        showTextAnim(&ta_about_me, scene, my_name, 10000, 29500);
        showTextAnim(&ta_contact, scene, contact, 30000, 49500);

        //        todebug
//        showTextAnim(&ta_dbl, NULL, NULL, 5500, 500);
//        showTextAnim(&ta_about_double, scene, double_desc, 500, 2000);
//        showTextAnim(&ta_about_me, scene, my_name, 2500, 5000);

        connect(ta_dbl.timerlist[0], SIGNAL(timeout()), this, SLOT(outNow()));

        sawme = true;
    }

}

void MainWindow::on_toolConf_clicked()
{
    if(Conf->isHidden()){
        Conf->updateConf();
        Conf->show();
    } else Conf->hide();
}

//void MainWindow::updateLastNum(){
//    size_t len = bufsize;
//    char tmp[MAXNUMLEN*2];

//    //qDebug()<<tr("%1:%2").arg(len).arg(buf);

//    while (len) {
//        len--;
//        //qDebug()<<tr("buf[%1]:%2").arg(len).arg(buf[len]);
//        switch(buf[len]){
//        case '+': case '-': case '*': case '/': case '^': case '%':
//            ui->label->setText(QString(buf[len]));
//            ui->editLast->setText(QString(*(buf+len+1)?sep_buf(buf+len+1, tmp, MAXNUMLEN*2):""));
//            return;
//            break;
//        }

//    }
//}

void MainWindow::on_toolHtmlHelp_clicked()
{
    //ui->textEdit->setHtml(HTMLHLP);
    //Qt::Alignment al = ui->textEdit->alignment();

    QString str;
    str = tr("guide.");

    switch(sess.cursess->uilocale){
        case SESS_LOCALE_FA:
        str.append(tr("fa"));
        break;
    default:
        str.append(tr("en"));
        break;
    }

    QFile f(str);

    f.open(QFile::ReadOnly);
    if(f.isOpen()) str = trUtf8(f.readAll());
    else str = trUtf8(CMSG_NOHELP);
    //QMessageBox::information(this, "", tr("read %1 bytes").arg(str.size()));
    //ui->textEdit->setLayoutDirection(Qt::RightToLeft);
    ui->textEdit->append(str);
}

//void MainWindow::on_toolCon_clicked()
//{

//    if(ui->toolCon->isChecked()){
//        //sess.cursess->uiflags &= ~SESS_UIF_HILIGHT;
//        strStyleOut = ui->textEdit->styleSheet();
//        strStyleIn = ui->comboEdit->styleSheet();
//        ui->textEdit->setStyleSheet(tr("background-color: %1; color: %2;").arg(strBackgroundColor).arg(strFontColor));
//        ui->comboEdit->setStyleSheet(tr("background-color: %1; color: %2;").arg(strBackgroundColor).arg(strFontColor));
//    } else {
//        ui->textEdit->setStyleSheet(strStyleOut);
//        ui->comboEdit->setStyleSheet(strStyleIn);
//    }

//}

void MainWindow::on_toolAbout_clicked()
{
    aboutDialog aboutDlg;
    aboutDlg.exec();
}

void MainWindow::on_toolTextNumber_clicked()
{

//    if(sess.uilocale == SESS_LOCALE_FA) nt_convert(var_val(&sess, ANSVARNAME), buf, 4096);
//    else if(sess.uilocale == SESS_LOCALE_EN) nt_convert_en(var_val(&sess, ANSVARNAME), buf, 4096);

//    strNum = trUtf8(buf);
//    QApplication::clipboard()->setText(strNum);
//    ui->textEdit->append(trUtf8(CMSG_COPYCLIP).arg(strNum));

    runExp(tr("num2text(%1);").arg(ui->comboEdit->currentText().isEmpty()?ANSVARNAME:ui->comboEdit->currentText()));
    ui->textEdit->append(trUtf8(CMSG_COPYCLIP).arg(QApplication::clipboard()->text()));

    //qcp(&sess, buf);
}

void MainWindow::findText(){

    findDlg->exec();
    if(findDlg->strFind.isEmpty()) return;
    if(findDlg->forward){
        if(!ui->textEdit->find(findDlg->strFind)){
            QMessageBox::warning(this, tr("Find..."), tr("Could not find anything more in front."));
        }
    } else {
        if(!ui->textEdit->find(findDlg->strFind, QTextDocument::FindBackward)){
            QMessageBox::warning(this, tr("Find..."), tr("Could not find anything more in back."));
        }
    }

//    if(!ui->textEdit->find(findDlg->strFind)){
//        ui->textEdit->find(findDlg->strFind, QTextDocument::FindBackward);
//    }
}

void MainWindow::on_toolMultiExp_clicked()
{
    if(!ui->toolMultiExp->isChecked()){
        ui->frameMultiExp->hide();
        ui->textMultiExp->hide();
        ui->comboEdit->setFocus();
//        ui->toolRun->show();
//        ui->comboEdit->show();

    } else {
        ui->frameMultiExp->show();
        ui->textMultiExp->show();
        //ui->textMultiExp->activateWindow();
        //ui->textMultiExp->raise();
        ui->textMultiExp->setFocus();
//        ui->toolRun->hide();
//        ui->comboEdit->hide();

    }
}

void MainWindow::on_toolRunMultiExp_clicked()
{
    ui->comboEdit->lineEdit()->setText(ui->textMultiExp->toPlainText());
    on_comboEdit_returnPressed();
    if(ui->toolAutoClearMultiExp->isChecked()){
        ui->textMultiExp->clear();
    }
}

void MainWindow::on_toolClearMultiExp_clicked()
{
    ui->textMultiExp->clear();
}

void MainWindow::on_toolSaveMultiExp_clicked()
{
    if(MultiExpFilename.isEmpty()) on_toolSaveAsMultiExp_clicked();

    QFile file(MultiExpFilename);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)){
        MultiExpFilename.clear();
        setMultiExpFileName();
        return;
    }

    QTextStream out(&file);
    out << ui->textMultiExp->toPlainText();
    file.close();
    setMultiExpFileName();

}

void MainWindow::on_toolSaveAsMultiExp_clicked()
{
    //get file name
    QFileDialog fileDlg;

    fileDlg.setAcceptMode(QFileDialog::AcceptSave);
    fileDlg.setFileMode(QFileDialog::AnyFile);

    if(!fileDlg.exec()) return;

    MultiExpFilename = fileDlg.selectedFiles().at(0);

    on_toolSaveMultiExp_clicked();
}

void MainWindow::on_toolLoadMultiExp_clicked()
{
    QFileDialog fileDlg;

    fileDlg.setFileMode(QFileDialog::ExistingFile);

    if(!fileDlg.exec()) return;

    MultiExpFilename = fileDlg.selectedFiles().at(0);

    QFile file(MultiExpFilename);

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        MultiExpFilename.clear();
        setMultiExpFileName();
        return;
    }

    QTextStream in(&file);
    ui->textMultiExp->setText(in.readAll());
    file.close();
    setMultiExpFileName();

}

void MainWindow::setMultiExpFileName(){
    if(MultiExpFilename.isEmpty()) ui->textMultiExp->setToolTip(trUtf8("[ MultiLine Exporession ]"));
    else ui->textMultiExp->setToolTip(trUtf8("<html><head/><body><p>[ MultiLine Expression <span style=\" vertical-align:sub;\">%1</span> ]</p></body></html>").arg(MultiExpFilename));
    //else ui->textMultiExp->setToolTip(trUtf8("[ MultiLine Expression ] - File: %1").arg(MultiExpFilename));
}

void MainWindow::on_toolSaveOutAs_clicked()
{

    QFileDialog fileDlg;

    fileDlg.setAcceptMode(QFileDialog::AcceptSave);
    fileDlg.setFileMode(QFileDialog::AnyFile);

    if(!fileDlg.exec()) return;

    QString filename = fileDlg.selectedFiles().at(0);

    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)){
        return;
    }

    QTextStream out(&file);
    out << ui->textEdit->toPlainText();
    file.close();

}

void MainWindow::on_toolLoadOut_clicked()
{
    QFileDialog fileDlg;

    fileDlg.setFileMode(QFileDialog::ExistingFile);

    if(!fileDlg.exec()) return;

    QString filename = fileDlg.selectedFiles().at(0);

    QFile file(filename);

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        return;
    }

    QTextStream in(&file);
    ui->textEdit->append(in.readAll());
    file.close();

}

void MainWindow::on_toolPlot_clicked()
{
    runExp(trUtf8("plot(%1);").arg(ui->comboEdit->currentText()));
}

void MainWindow::on_toolAEdit_clicked()
{
    runExp(trUtf8("aedit(%1);").arg(ui->comboEdit->currentText()));
}

int MainWindow::saveFile(QString filename, QString str){
    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)){
        return 0;
    }

    QTextStream out(&file);
    out << str;
    file.close();

    return 1;

}

int MainWindow::saveFile(QString filename, QStringList strlist){
    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)){
        return 0;
    }

    QTextStream out(&file);

    for(int i = 0; i < strlist.size(); i++)
        out << strlist.at(i) << '\n';

    file.close();

    return 1;

}


void MainWindow::on_toolSaveStat_clicked()
{
    QStringList strlist;
    runExp(trUtf8("savestat(%1);").arg(Q_DEF_STAT_FILE));
    saveFile(trUtf8("%1").arg(Q_DEF_OUT_STAT_FILE), ui->textEdit->toHtml());
    saveFile(trUtf8("%1").arg(Q_DEF_MEXP_STAT_FILE), ui->textMultiExp->toHtml());

    for(int i = ui->comboEdit->count(); i > -1;  i--){
        strlist << ui->comboEdit->itemText(i);
    }

    saveFile(trUtf8("%1").arg(Q_DEF_EXP_STAT_FILE), strlist);

}

QString MainWindow::loadFile(QString filename){
    QString str;
    QFile file(filename);

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        return tr("");
    }

    QTextStream in(&file);    
    str = in.readAll();
    file.close();
    return str;
}

QStringList MainWindow::loadFile2(QString filename){
    QStringList strlist;
    QFile file(filename);

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        return strlist;
    }

    QTextStream in(&file);

    while(!in.atEnd()){
        strlist << in.readLine();
    }

    file.close();    
    return strlist;
}

void MainWindow::on_toolLoadStat_clicked()
{    
    runExp(trUtf8("load(%1);").arg(Q_DEF_STAT_FILE));
    ui->textEdit->setHtml(loadFile(tr("%1").arg(Q_DEF_OUT_STAT_FILE)));
    ui->textMultiExp->setHtml(loadFile(tr("%1").arg(Q_DEF_MEXP_STAT_FILE)));
    ui->comboEdit->clear();
    ui->comboEdit->insertItems(0, loadFile2(tr("%1").arg(Q_DEF_EXP_STAT_FILE)));
    ui->comboEdit->insertItem(0, tr(""));
}

void MainWindow::on_comboEdit_editTextChanged(const QString &arg1)
{    
    if(!arg1.length()) return;
    char* buf = (char*)malloc(strlen(arg1.toUtf8().data()));    
    int len;

    strcpy(buf, arg1.toAscii().data());

    len = strlen(buf)-1;

    while(len >= 0 && !strchr("0123456789", buf[len--]));
    *(buf + len + 1 + 1) = 0;
    if(len>0) while(len >= 0 && !strchr(dbl_delimlist, buf[len--]));

    ui->labelSep1000->setText(QString("%L1").arg(tr(buf + (len>0?len+2:0)).toDouble()));
    free(buf);
}
