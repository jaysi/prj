#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFuture>
#include <QTextCursor>
#include <QGraphicsPixmapItem>
#include <QGraphicsWidget>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QShowEvent>
#include <QStyle>
#include <QAction>

#include "keypad.h"
#include "update.h"
#include "configdialog.h"
#include "finddialog.h"
#include "plotdialog.h"
#include "arraydialog.h"

#define Q_DEF_STAT_FILE "cute.stt"
#define Q_DEF_OUT_STAT_FILE "out.stq"
#define Q_DEF_EXP_STAT_FILE "exp.stq"
#define Q_DEF_MEXP_STAT_FILE    "mexp.stq"

#undef KEYPADMODE

namespace Ui {
class MainWindow;
}

#include <QtGui>
#include <QtCore/qstate.h>

class Pixmap : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
public:
    Pixmap(const QPixmap &pix)
        : QObject(), QGraphicsPixmapItem(pix)
    {
        setCacheMode(DeviceCoordinateCache);
    }
};

class Button : public QGraphicsWidget
{
    Q_OBJECT
public:
    Button(const QPixmap &pixmap, QGraphicsItem *parent = 0)
        : QGraphicsWidget(parent), _pix(pixmap)
    {
        setAcceptHoverEvents(true);
        setCacheMode(DeviceCoordinateCache);
    }

    QRectF boundingRect() const
    {
        return QRectF(-65, -65, 130, 130);
    }

    QPainterPath shape() const
    {
        QPainterPath path;
        path.addEllipse(boundingRect());
        return path;
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
    {
        bool down = option->state & QStyle::State_Sunken;
        QRectF r = boundingRect();
        QLinearGradient grad(r.topLeft(), r.bottomRight());
        grad.setColorAt(down ? 1 : 0, option->state & QStyle::State_MouseOver ? Qt::white : Qt::lightGray);
        grad.setColorAt(down ? 0 : 1, Qt::darkGray);
        painter->setPen(Qt::darkGray);
        painter->setBrush(grad);
        painter->drawEllipse(r);
        QLinearGradient grad2(r.topLeft(), r.bottomRight());
        grad.setColorAt(down ? 1 : 0, Qt::darkGray);
        grad.setColorAt(down ? 0 : 1, Qt::lightGray);
        painter->setPen(Qt::NoPen);
        painter->setBrush(grad);
        if (down)
            painter->translate(2, 2);
        painter->drawEllipse(r.adjusted(5, 5, -5, -5));
        painter->drawPixmap(-_pix.width()/2, -_pix.height()/2, _pix);
    }

signals:
    void pressed();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *)
    {
        emit pressed();
        update();
    }

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *)
    {
        update();
    }

private:
    QPixmap _pix;
};

class View : public QGraphicsView
{
public:    
    View(QGraphicsScene *scene) : QGraphicsView(scene) { }

protected:
    void resizeEvent(QResizeEvent *event)
    {
        QGraphicsView::resizeEvent(event);
        fitInView(sceneRect(), Qt::KeepAspectRatio);
    }
};

enum state{
    ST_CENTER,
    ST_RIGHT,
    ST_OUTLEFT,
    ST_CUSTOM
};

struct textanim{
    bool init;
    QGraphicsScene* scene;
    QList<Pixmap *> pixmaplist;
    QList<QPixmap* > qpixmaplist;
    QState *rootState;
    QList<QState *> statelist;
    QList<QTimer *> timerlist;
    QStateMachine* states;
    QAbstractTransition *trans;
    QParallelAnimationGroup *group;
    QList<QPropertyAnimation* > animlist;
    int in, out;//msec
    char* text;
};

//enum pipe_dir{PREAD, PWRITE};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setKeyPadClose();
    void appendToExp(QString);
    void deleteFromExp();
    void clearExp();

    struct session sess;

    void runExp(QString exp);
    //void runExp2(QString exp);

    QString strBackgroundColor, strFontColor;

    //void setAnimView(AnimView* v);

    //void setOutColor(int r, int g, int b);    
    void on_comboEdit_returnPressed();

private slots:

    void findText();

    void outNow();

    void appendOut(QString str, int r, int g, int b, bool bold, bool italic);        

    void on_toolKeyPad_clicked();

    void on_toolClear_clicked();

    void on_toolHelp_clicked();

    void on_toolRun_clicked();

//    void on_toolFork_clicked();

//    void on_toolForkList_clicked();

    void on_toolNext_clicked();

    void on_toolPrev_clicked();

    void on_toolComplete_clicked();

    void on_toolExit_clicked();

    void on_toolMode_clicked();

    void on_toolClearIn_clicked();

    void on_toolCopy_clicked();

    void on_toolDecimLess_clicked();

    void on_toolDecimMore_clicked();        

    void on_toolShut_clicked();    

    void on_toolConf_clicked();

    void on_toolHtmlHelp_clicked();

//    void on_toolCon_clicked();

    void on_toolAbout_clicked();

    void on_toolTextNumber_clicked();

    void on_toolMultiExp_clicked();

    void on_toolRunMultiExp_clicked();

    void on_toolClearMultiExp_clicked();

    void on_toolSaveMultiExp_clicked();

    void on_toolSaveAsMultiExp_clicked();

    void on_toolLoadMultiExp_clicked();

    void on_toolSaveOutAs_clicked();

    void on_toolLoadOut_clicked();

    void on_toolPlot_clicked();

    void on_toolAEdit_clicked();

    void on_toolSaveStat_clicked();

    void on_toolLoadStat_clicked();

    void on_comboEdit_editTextChanged(const QString &arg1);

private:
    //void InitAnim();
    Ui::MainWindow *ui;

    bool keypad;
    bool space1;
    int matchlistitem;
    int matchlistsize;
    char** matchlist;
    char* matchexp;
    int forkid;

    unsigned long nout;

    KeyPad* Keypad;
    ConfigDialog* Conf;
    void setKeyPad();

    int SetMatchList();

    Update up;

    bool firstrun;

    //animation
    bool graphicshown;
    bool sawme;
    void initGraph();
    void recalcTextAnimPos(struct textanim* tan);
    QList<QPixmap* >text2Graph(char* text);
    QGraphicsScene* scene;

    struct textanim ta_dbl;
    struct textanim ta_about_double;
    struct textanim ta_about_me;
    struct textanim ta_contact;

    int showTextAnim(struct textanim* ta, QGraphicsScene *scene, char* text, int in, int out);
    int deleteTextAnim(struct textanim* ta);

    bool forceexit;

    QString strStyleOut, strStyleIn;

    QAction* findAct;

    findDialog* findDlg;
    PlotDialog plotDlg;
    arrayDialog* aedDlg;

    QString MultiExpFilename;

    void setMultiExpFileName();
    int saveFile(QString filename, QString str);
    int saveFile(QString filename, QStringList strlist);
    QString loadFile(QString filename);
    QStringList loadFile2(QString filename);

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void closeEvent(QCloseEvent *);
    void moveEvent(QMoveEvent *event);
    void showEvent(QShowEvent* event);

};

#endif // MAINWINDOW_H
