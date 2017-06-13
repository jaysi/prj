#include "keypad.h"
#include "ui_keypad.h"

#include "mainwindow.h"

#define mainw ((MainWindow*)main)
#define appnd(str) MACRO( if(bufsize >= mainw->sess.cursess->expbufsize){bufsize = 0; buf[0] = 0;} snprintf(buf + bufsize, mainw->sess.cursess->expbufsize - bufsize - 1, str); bufsize += strlen(str); mainw->appendToExp(tr(str)); updateView(); )

KeyPad::KeyPad(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KeyPad)
{
    ui->setupUi(this);
}

KeyPad::KeyPad(QWidget *parent, void* mainptr) :
    QDialog(parent),
    ui(new Ui::KeyPad)
{
    ui->setupUi(this);

    main = mainptr;
    collapse = false;
    collapse1 = false;
    buf = (char*)malloc(mainw->sess.cursess->expbufsize);
    buf[0] = 0;
    bufsize = 0;
    ui->btnCollapse->installEventFilter(this);
    ui->btnCollapse1->installEventFilter(this);

#ifdef KEYPADMODE
    on_btnCollapse_clicked();
#endif

}

KeyPad::~KeyPad()
{
    free(buf);
    delete ui;
}

void KeyPad::closeEvent(QCloseEvent *event){

    mainw->setKeyPadClose();
    event->accept();

}

void KeyPad::on_btn0_clicked()
{
    appnd(("0"));
}

void KeyPad::on_btnDot_clicked()
{
    appnd(("."));
}

void KeyPad::on_btnAns_clicked()
{
    appnd(("ans"));
}

void KeyPad::on_btnRun_clicked()
{
    mainw->on_comboEdit_returnPressed();
    buf[0] = 0;
    bufsize = 0;
}

void KeyPad::on_btn1_clicked()
{
    appnd(("1"));
}

void KeyPad::on_btn2_clicked()
{
    appnd(("2"));
}

void KeyPad::on_btn3_clicked()
{
    appnd(("3"));
}

void KeyPad::on_btnPlus_clicked()
{
    appnd(("+"));
}

void KeyPad::on_btnMinus_clicked()
{
    appnd(("-"));
}

void KeyPad::on_btn4_clicked()
{
    appnd(("4"));
}

void KeyPad::on_btn5_clicked()
{
    appnd(("5"));
}

void KeyPad::on_btn6_clicked()
{
    appnd(("6"));
}

void KeyPad::on_btnMult_clicked()
{
    appnd(("*"));
}

void KeyPad::on_btnDiv_clicked()
{
    appnd(("/"));
}

void KeyPad::on_btn7_clicked()
{
    appnd(("7"));
}

void KeyPad::on_btn8_clicked()
{
    appnd(("8"));
}

void KeyPad::on_btn9_clicked()
{
    appnd(("9"));
}

void KeyPad::on_btnDel_clicked()
{
    mainw->deleteFromExp();

    if(bufsize){
        bufsize--;
        buf[bufsize] = 0;
    }
}

void KeyPad::on_btnAC_clicked()
{
    mainw->clearExp();
    buf[0] = 0;
    bufsize = 0;
    updateView();

}

void KeyPad::on_btnSemiColon_clicked()
{
    appnd((";"));
}

void KeyPad::on_btnLT_clicked()
{
    appnd(("<"));
}

void KeyPad::on_btnGT_clicked()
{
    appnd((">"));
}

void KeyPad::on_btnEQ_clicked()
{
    appnd(("="));
}

void KeyPad::on_btnNE_clicked()
{
    appnd(("!"));
}

void KeyPad::on_btnAND_clicked()
{
    appnd(("&"));
}

void KeyPad::on_btnOR_clicked()
{
    appnd(("|"));
}

void KeyPad::on_btnAssign_clicked()
{
    appnd((":"));
}

void KeyPad::on_btnSpace_clicked()
{
    appnd((" "));
}

void KeyPad::on_btnColon_clicked()
{
    appnd((","));
}

void KeyPad::on_btnBrOpen_clicked()
{
    appnd(("("));
}

void KeyPad::on_btnBrClose_clicked()
{
    appnd((")"));
}

void KeyPad::on_btnArrayOpen_clicked()
{
    appnd(("["));
}

void KeyPad::on_btnArrayClose_clicked()
{
    appnd(("]"));
}

void KeyPad::on_btnGroupOpen_clicked()
{
    appnd(("{"));
}

void KeyPad::on_btnGroupClose_clicked()
{
    appnd(("}"));
}

void KeyPad::on_btnAuto_clicked()
{
    appnd(("auto "));
}

void KeyPad::on_btnHist_clicked()
{
    appnd(("hist;"));
}

void KeyPad::on_btnHistload_clicked()
{
    appnd(("histload "));
}

void KeyPad::on_btnJalali_clicked()
{
    appnd(("jalali("));
}

void KeyPad::on_btnGreg_clicked()
{
    appnd(("greg("));
}

void KeyPad::on_btnPrint_clicked()
{
    appnd(("pr("));
}

void KeyPad::on_btnVunlock_clicked()
{
    appnd(("vunlock("));
}

void KeyPad::on_btnArray_clicked()
{
    appnd(("array("));
}

void KeyPad::on_btnAfill_clicked()
{
    appnd(("fill("));
}

void KeyPad::on_btnRecalc_clicked()
{
    appnd(("recalc("));
}

void KeyPad::on_btnLoad_clicked()
{
    appnd(("load \""));
}

void KeyPad::on_btnSave_clicked()
{
    appnd(("save \""));
}

void KeyPad::on_btnVrm_clicked()
{
    appnd(("vrm("));
}

void KeyPad::on_btnVrmall_clicked()
{
    appnd(("vrmall;"));
}

void KeyPad::on_btnVexp_clicked()
{
    appnd(("vexp("));
}

void KeyPad::on_btnVlist_clicked()
{
    appnd(("vlist "));
}

void KeyPad::on_btnVstr_clicked()
{
    appnd(("vstr("));
}

void KeyPad::on_btnVlock_clicked()
{
    appnd(("vlock("));
}

void KeyPad::on_btnIf_clicked()
{
    appnd(("if("));
}

void KeyPad::on_btnElseif_clicked()
{
    appnd(("elseif("));
}

void KeyPad::on_btnElse_clicked()
{
    appnd(("else {"));
}

void KeyPad::on_btnCc_clicked()
{
    appnd(("cc "));
}

void KeyPad::on_btnCs_clicked()
{
    appnd(("cs "));
}

void KeyPad::on_btnLoop_clicked()
{
    appnd(("jump("));
}

void KeyPad::on_btnSinh_clicked()
{
    appnd(("sinh("));
}

void KeyPad::on_btnCosh_clicked()
{
    appnd(("cosh("));
}

void KeyPad::on_btnD2r_clicked()
{
    appnd(("d2r("));
}

void KeyPad::on_btnR2d_clicked()
{
    appnd(("r2d("));
}

void KeyPad::on_btnSqrt_clicked()
{
    appnd(("sqr("));
}

void KeyPad::on_btnAbs_clicked()
{
    appnd(("abs("));
}

void KeyPad::on_btnAsin_clicked()
{
    appnd(("asin("));
}

void KeyPad::on_btnAcos_clicked()
{
    appnd(("acos("));
}

void KeyPad::on_btnAtan_clicked()
{
    appnd(("atan("));
}

void KeyPad::on_btnLn_clicked()
{
    appnd(("ln("));
}

void KeyPad::on_btnNround_clicked()
{
    appnd(("~"));
}

void KeyPad::on_btnGday_clicked()
{
    appnd(("gday("));
}

void KeyPad::on_btnSin_clicked()
{
    appnd(("sin("));
}

void KeyPad::on_btnCos_clicked()
{
    appnd(("cos("));
}

void KeyPad::on_btnTan_clicked()
{
    appnd(("tan("));
}

void KeyPad::on_btnLog_clicked()
{
    appnd(("log("));
}

void KeyPad::on_btnRound_clicked()
{
    appnd(("round("));
}

void KeyPad::on_btnJday_clicked()
{
    appnd(("jday("));
}

void KeyPad::on_btnDown_clicked()
{
    appnd(("dn"));
}

void KeyPad::on_btnUp_clicked()
{
    appnd(("up"));
}

void KeyPad::on_btnDel_2_clicked()
{
    appnd(("del"));
}

void KeyPad::on_btnShow_clicked()
{
    appnd(("show"));
}

void KeyPad::on_btnOn_clicked()
{
    appnd(("on"));
}

void KeyPad::on_btnTr_clicked()
{
    appnd(("tr"));
}

void KeyPad::on_btnOff_clicked()
{
    appnd(("off"));
}

void KeyPad::on_btnMode_clicked()
{
    appnd(("mode "));
}

void KeyPad::on_btnNr_clicked()
{
    appnd(("nr"));
}

void KeyPad::on_btnFin_clicked()
{
    appnd(("fin;"));
}

void KeyPad::on_btnProg_clicked()
{
    appnd(("prog;"));
}

void KeyPad::setLast(QString str){
    ui->editLast->setText(str);
}

void KeyPad::setMode(QString str){
//    if(str == tr("[ ERROR ]")){
//    }
    ui->editStat->setText(str);
}

void KeyPad::keyPressEvent(QKeyEvent *pe){

//    bool update = true;

    switch(pe->key()){

        case Qt::Key_0:
            on_btn0_clicked();
        break;

        case Qt::Key_Period:
            on_btnDot_clicked();
        break;

        case Qt::Key_A:
            on_btnAns_clicked();
        break;

        case Qt::Key_AsciiCircum:
            on_btnPower_clicked();
        break;

        case Qt::Key_Percent:
            on_btnMod_clicked();
        break;

        case Qt::Key_1:
            on_btn1_clicked();
        break;

        case Qt::Key_2:
            on_btn2_clicked();
        break;

        case Qt::Key_3:
            on_btn3_clicked();
        break;

        case Qt::Key_Plus:
            on_btnPlus_clicked();
        break;

        case Qt::Key_Minus:
            on_btnMinus_clicked();
        break;

        case Qt::Key_4:
            on_btn4_clicked();
        break;

        case Qt::Key_5:
            on_btn5_clicked();
        break;

        case Qt::Key_6:
            on_btn6_clicked();
        break;

        case Qt::Key_Asterisk:
            on_btnMult_clicked();
        break;

        case Qt::Key_7:
            on_btn7_clicked();
        break;

        case Qt::Key_8:
            on_btn8_clicked();
        break;

        case Qt::Key_9:
            on_btn9_clicked();
        break;

        case Qt::Key_Escape:
            on_btnAC_clicked();
        break;

        case Qt::Key_Less:
            on_btnLT_clicked();
        break;

        case Qt::Key_Greater:
            on_btnGT_clicked();
            break;

    case Qt::Key_Equal:
        on_btnEQ_clicked();
        break;

    case Qt::Key_Exclam:
        on_btnNE_clicked();
        break;

    case Qt::Key_Ampersand:
        on_btnAND_clicked();
        break;

    case Qt::Key_Bar:
        on_btnOR_clicked();
        break;

    case Qt::Key_ParenLeft:
        on_btnBrOpen_clicked();
        break;

    case Qt::Key_ParenRight:
        on_btnBrClose_clicked();
        break;

    case Qt::Key_BraceLeft:
        on_btnGroupOpen_clicked();
        break;

    case Qt::Key_BraceRight:
        on_btnGroupClose_clicked();
        break;

    case Qt::Key_BracketLeft:
        on_btnArrayOpen_clicked();
        break;

    case Qt::Key_Colon:
        on_btnAssign_clicked();
        break;

    case Qt::Key_Comma:
        on_btnColon_clicked();
        break;

    case Qt::Key_BracketRight:
        on_btnArrayClose_clicked();
        break;

        case Qt::Key_Slash:
            on_btnDiv_clicked();
        break;

        case Qt::Key_Semicolon:
            on_btnSemiColon_clicked();
        break;

        case Qt::Key_Return:
        case Qt::Key_Enter:
//            update = false;
            on_btnRun_clicked();
            updateResults();
        break;

    case Qt::Key_Delete:
    case Qt::Key_Backspace:
        on_btnDel_clicked();
        updateView();
        break;

    case Qt::Key_Space:
        on_btnSpace_clicked();
        break;

    case Qt::Key_M:
        on_btnMode_clicked();
        break;

        case Qt::Key_C:
        on_btnCc_clicked();
        break;

        default:
           // qDebug()<<pe->key();
            return;
        break;
    }

//    if(update) {/*qDebug()<<"update";*/updateView();}
//    else {/*qDebug()<<"no update";*/updateResults();}

}

void KeyPad::on_btnPower_clicked()
{
    appnd(("^"));
}

void KeyPad::on_btnFlushf_clicked()
{
    appnd(("fflush;"));
}

void KeyPad::on_btnNormal_clicked()
{
    appnd(("normal;"));
}

void KeyPad::on_btnLabel_clicked()
{
    appnd(("label "));
}

void KeyPad::on_btnGoto_clicked()
{
    appnd(("goto "));
}

void KeyPad::on_btnReturn_clicked()
{
    appnd(("return;"));
}

void KeyPad::on_btnVren_clicked()
{
    appnd(("vren("));
}

void KeyPad::on_btnVcat_clicked()
{
    appnd(("vcat("));
}

void KeyPad::on_btnSharp_clicked()
{
    appnd(("#"));
}

void KeyPad::on_btnDollar_clicked()
{
    appnd(("$"));
}

void KeyPad::on_btnMod_clicked()
{
    appnd(("%%"));
    on_btnDel_clicked();
}

bool KeyPad::eventFilter(QObject *obj, QEvent *event){

    //QPushButton* btn = dynamic_cast<QPushButton*>(obj);
    QToolButton* tbtn;

    if(obj->objectName() == tr("btnCollapse")){
        tbtn = dynamic_cast<QToolButton*>(obj);
        Q_ASSERT(tbtn);
        if(event->type() == QEvent::Enter){
            if(!collapse) tbtn->setIcon(QIcon(":/res/collapse-26.png"));
            else tbtn->setIcon(QIcon(":/res/expand-26.png"));
        } else if(event->type() == QEvent::Leave){
            if(!collapse) tbtn->setIcon(QIcon(":/res/expand-26.png"));
            else tbtn->setIcon(QIcon(":/res/collapse-26.png"));
        }
    }

    return false;
}

void KeyPad::on_btnCollapse_clicked()
{

    if(!collapse) collapse = true;
    else collapse = false;

    if(collapse){
        ui->groupMathFunc->hide();
        ui->groupScriptFunc->hide();
        ui->groupArgs->hide();
        ui->btnCollapse1->setDisabled(true);
        this->adjustSize();
    } else {
        ui->groupMathFunc->show();
        ui->groupScriptFunc->show();
        ui->groupArgs->show();
        ui->btnCollapse1->setDisabled(false);
        this->adjustSize();
    }
}

void KeyPad::updateView(){

    //+-*/^%
    size_t len = bufsize;
    char tmp[MAXNUMLEN*2];

    //qDebug()<<tr("%1:%2").arg(len).arg(buf);

    while (len) {
        len--;
        //qDebug()<<tr("buf[%1]:%2").arg(len).arg(buf[len]);
        switch(buf[len]){
        case '+': case '-': case '*': case '/': case '^': case '%':
            ui->label->setText(QString(buf[len]));            
            ui->editLast->setText(QString(*(buf+len+1)?sep_buf(buf+len+1, tmp, MAXNUMLEN*2):""));
            return;
            break;
        }

    }

    ui->label->clear();
    //qDebug()<<buf;
    ui->editLast->setText(bufsize?sep_buf(buf, tmp, MAXNUMLEN*2):QString(buf));
    //qDebug()<<buf;

}

void KeyPad::updateResults(){
    ui->label->setText(QString('='));
}

void KeyPad::on_btnCollapse1_clicked()
{
    if(!collapse1) collapse1 = true;
    else collapse1 = false;

    if(collapse1){
        ui->groupScriptFunc->hide();
        ui->btnCollapse->setDisabled(true);
        this->adjustSize();
    } else {
        ui->groupScriptFunc->show();
        ui->groupMathFunc->show();
        ui->groupArgs->show();
        ui->btnCollapse->setDisabled(false);
        this->adjustSize();
    }
}

void KeyPad::on_btnVDup_clicked()
{
    appnd(("vdup("));
}

void KeyPad::on_btnView_clicked()
{
    appnd(("view \""));
}

void KeyPad::on_btnSys_clicked()
{
    appnd(("sys("));
}

void KeyPad::on_btnVer_clicked()
{
    appnd(("ver("));
}

void KeyPad::on_btnUi_clicked()
{
    appnd(("ui,"));
}

void KeyPad::on_btnAt_clicked()
{
    appnd(("@"));
}

void KeyPad::on_btnDCout_clicked()
{
    appnd(("\""));
}

void KeyPad::on_btnBin_clicked()
{
    appnd(("bin"));
}

void KeyPad::on_btnHex_clicked()
{
    appnd(("hex"));
}

void KeyPad::on_btnAll_clicked()
{
    appnd(("all"));
}
