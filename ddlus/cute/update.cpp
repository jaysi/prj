#include "update.h"
#include "mainwindow.h"

//int plotxy(struct session *sess){

//    Update* up;
//    struct tok_s* x, *y;

//    x = poptok(sess);
//    if(!x){
//        printe(MSG_MOREARGS);
//        return false_;
//    }

//    y = poptok(sess);
//    if(!y){
//        printe(MSG_MOREARGS);
//        return false_;
//    }

//    up = (Update*)(sess->cursess->ui);

//    up->upPlot(x->val, y->val);

//    return true_;

//}

int outtext_(struct session* sess, char* buf){
    Update* up;

    up = (Update*)(sess->cursess->wr);

    if(!memcmp(buf, ERR_ICON, ICON_SIZE)){
        up->upOut(QString::fromUtf8(buf+ICON_SIZE), 255, 0, 0, false, false);
    } else if(!memcmp(buf, WARN_ICON, ICON_SIZE)) {
        up->upOut(QString::fromUtf8(buf+ICON_SIZE), 255, 0, 255, false, false);
    } else if(!memcmp(buf, INFO_ICON, ICON_SIZE)) {
        up->upOut(QString::fromUtf8(buf+ICON_SIZE), 0, 0, 255, false, false);
    } else if(!memcmp(buf, HELPBLOCK_ICON, ICON_SIZE)) {
        up->upOut(QString::fromUtf8(buf+ICON_SIZE), 32, 32, 32, true, false);
    } else if(!memcmp(buf, HELPINFO_ICON, ICON_SIZE)) {
        up->upOut(QString::fromUtf8(buf+ICON_SIZE), 32, 32, 32, true, false);
    } else if(!memcmp(buf, ANS_ICON, ICON_SIZE)) {
        up->upOut(QString::fromUtf8(buf+ICON_SIZE), 0, 0, 0, true, false);
    } else if(!memcmp(buf, RESULT_ICON, ICON_SIZE)) {
        up->upOut(QString::fromUtf8(buf+ICON_SIZE), 32, 32, 32, false, true);
    } else if(!memcmp(buf, INFOBLOCK_ICON, ICON_SIZE)) {
        up->upOut(QString::fromUtf8(buf+ICON_SIZE), 32, 32, 32, true, true);
    } else if(!memcmp(buf, NOP_ICON, ICON_SIZE)) {
        up->upOut(QString::fromUtf8(buf+ICON_SIZE), 32, 32, 32, false, false);
    } else {
        up->upOut(QString::fromUtf8(buf), 32, 32, 32, false, false);
    }

   return true_;
}

Update::Update(QObject *parent) :
    QThread(parent)
{

}

void Update::upOut(QString str, int r, int g, int b, bool bold, bool italic){
    emit updateOut(str, r, g, b, bold, italic);
    this->usleep(100);
}

void Update::upPlot(fnum_t x, fnum_t y){
    emit updatePlot(x, y);
}

void Update::upConf(QString text)
{
    emit updateConf(text);
}
