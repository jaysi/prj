#ifndef KEYPAD_H
#define KEYPAD_H

#include <QDialog>

#include <QCloseEvent>

namespace Ui {
class KeyPad;
}

class KeyPad : public QDialog
{
    Q_OBJECT

public:
    explicit KeyPad(QWidget *parent = 0);
    KeyPad(QWidget *parent, void* main);
    ~KeyPad();
    void setLast(QString);
    void setMode(QString);

private:
    void* main;
    Ui::KeyPad *ui;
    bool collapse, collapse1;
    char* buf;
    size_t bufsize;
    void updateView();
    void updateResults();

protected:
    void closeEvent(QCloseEvent*);
    void keyPressEvent(QKeyEvent* pe);
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void on_btn0_clicked();
    void on_btnDot_clicked();
    void on_btnAns_clicked();
    void on_btnRun_clicked();
    void on_btn1_clicked();
    void on_btn2_clicked();
    void on_btn3_clicked();
    void on_btnPlus_clicked();
    void on_btnMinus_clicked();
    void on_btn4_clicked();
    void on_btn5_clicked();
    void on_btn6_clicked();
    void on_btnMult_clicked();
    void on_btnDiv_clicked();
    void on_btn7_clicked();
    void on_btn8_clicked();
    void on_btn9_clicked();
    void on_btnDel_clicked();
    void on_btnAC_clicked();
    void on_btnSemiColon_clicked();
    void on_btnLT_clicked();
    void on_btnGT_clicked();
    void on_btnEQ_clicked();
    void on_btnNE_clicked();
    void on_btnAND_clicked();
    void on_btnOR_clicked();
    void on_btnAssign_clicked();
    void on_btnSpace_clicked();
    void on_btnColon_clicked();
    void on_btnBrOpen_clicked();
    void on_btnBrClose_clicked();
    void on_btnArrayOpen_clicked();
    void on_btnArrayClose_clicked();
    void on_btnGroupOpen_clicked();
    void on_btnGroupClose_clicked();
    void on_btnAuto_clicked();
    void on_btnHist_clicked();
    void on_btnHistload_clicked();
    void on_btnJalali_clicked();
    void on_btnGreg_clicked();
    void on_btnPrint_clicked();
    void on_btnVunlock_clicked();
    void on_btnArray_clicked();
    void on_btnAfill_clicked();
    void on_btnRecalc_clicked();
    void on_btnLoad_clicked();
    void on_btnSave_clicked();
    void on_btnVrm_clicked();
    void on_btnVrmall_clicked();
    void on_btnVexp_clicked();
    void on_btnVlist_clicked();
    void on_btnVstr_clicked();
    void on_btnVlock_clicked();
    void on_btnIf_clicked();
    void on_btnElseif_clicked();
    void on_btnElse_clicked();
    void on_btnCc_clicked();
    void on_btnCs_clicked();
    void on_btnLoop_clicked();
    void on_btnSinh_clicked();
    void on_btnCosh_clicked();
    void on_btnD2r_clicked();
    void on_btnR2d_clicked();
    void on_btnSqrt_clicked();
    void on_btnAbs_clicked();
    void on_btnAsin_clicked();
    void on_btnAcos_clicked();
    void on_btnAtan_clicked();
    void on_btnLn_clicked();
    void on_btnNround_clicked();
    void on_btnGday_clicked();
    void on_btnSin_clicked();
    void on_btnCos_clicked();
    void on_btnTan_clicked();
    void on_btnLog_clicked();
    void on_btnRound_clicked();
    void on_btnJday_clicked();
    void on_btnDown_clicked();
    void on_btnUp_clicked();
    void on_btnDel_2_clicked();
    void on_btnShow_clicked();
    void on_btnOn_clicked();
    void on_btnTr_clicked();
    void on_btnOff_clicked();
    void on_btnMode_clicked();
    void on_btnNr_clicked();
    void on_btnFin_clicked();
    void on_btnProg_clicked();
    void on_btnPower_clicked();
    void on_btnMod_clicked();
    void on_btnCollapse_clicked();
    void on_btnFlushf_clicked();
    void on_btnNormal_clicked();
    void on_btnLabel_clicked();
    void on_btnGoto_clicked();
    void on_btnReturn_clicked();
    void on_btnVren_clicked();
    void on_btnVcat_clicked();
    void on_btnSharp_clicked();
    void on_btnDollar_clicked();
    void on_btnCollapse1_clicked();
    void on_btnVDup_clicked();
    void on_btnView_clicked();
    void on_btnSys_clicked();
    void on_btnVer_clicked();
    void on_btnUi_clicked();
    void on_btnAt_clicked();
    void on_btnDCout_clicked();
    void on_btnBin_clicked();
    void on_btnHex_clicked();
    void on_btnAll_clicked();
};

#endif // KEYPAD_H
