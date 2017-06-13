
#include "plotdialog.h"
#include "ui_plotdialog.h"

#include <QDebug>

//this is not thread safe version, see update.cpp for main.
int plotxy(struct session *sess){

    int i, i2, j;

    PlotDialog* p = (PlotDialog*)sess->cursess->ui;
    if(!p){
        printe(MSG_IMPLEMENT, "plot");
        return false_;
    }

    struct tok_s* arrayx, *arrayy;

    arrayy = poptok(sess);
    if(!arrayy){
        printe(MSG_MOREARGS, "plot");
        return false_;
    }

    arrayx = poptok(sess);
    if(!arrayx){
        printe(MSG_MOREARGS, "plot");
        return false_;
    }

    if(arrayx->id != TOK_ARRAY || arrayy->id != TOK_ARRAY){
        printe(MSG_FORMAT, "not array.");
        return false_;
    }

    if(!array_limits(sess, arrayx->str, &i, &j)){
        printe(MSG_FAIL, "array limits (x).");
        return false_;
    }

    if(!array_limits(sess, arrayy->str, &i2, &j)){
        printe(MSG_FAIL, "array limits (y).");
        return false_;
    }

//    if(i != i2){
//        printe(MSG_SIZEMIS, arrayx->str, i, arrayy->str, i2);
//        return false_;
//    }

    i = 0;
    for(i = 0; i < i2+1; i++){
        p->addPt(array_val(sess, arrayx->str, i, -1), array_val(sess, arrayy->str, i, -1));
    }

    p->plot();

    return true_;

}


PlotDialog::PlotDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PlotDialog)
{
    ui->setupUi(this);    
    sx = 1.0; sy = 1.0;
    painter = new QPainter(this);

}

PlotDialog::~PlotDialog()
{
    delete ui;
}

void PlotDialog::addPt(fnum_t x, fnum_t y){
    qDebug()<<"added x = "<<(double)x<<" y = "<<(double)y;
    xlist << x;
    ylist << y;
}

#define SX(val) (val/sx)
#define SY(val) (val/sy)

int PlotDialog::plot(){
    //QPolygonF poly;
    QLineF line;
    fnum_t i;
    inum_t j;
    if(isHidden()) show();
    if(painter->isActive()){                

        QTransform trans = painter->transform();
        trans.scale(sx, sy);
        painter->setTransform(trans);

        QRect rect = painter->viewport();

        painter->eraseRect(rect);

        painter->setPen(Qt::blue);

        painter->drawLine(SX(rect.width()/2), SY(0), SX(rect.width()/2), SY(rect.height()));
        painter->drawLine(SX(0), SY(rect.height()/2), SX(rect.width()), SY(rect.height()/2));

        painter->setPen(Qt::red);

        for(i = 0; i < SX(rect.width()); i+=5*sx){
            line.setLine(i, SY(rect.height()/2), i, SY(rect.height()/2)+SY(1));
            painter->drawLine(line);
        }

        for(i = 0; i < SY(rect.height()); i+=5*sy){
            line.setLine(SX(rect.width()/2), i, SX(rect.width()/2)-SX(1), i);
            painter->drawLine(line);
        }

        painter->setPen(Qt::black);

        QPointF* pt = new QPointF[xlist.count()];

        for(j = 0; j < xlist.count(); j++){
            //poly << QPointF(xlist.at(i)+SX(rect.width()/2), -ylist.at(i)+SY(rect.height()/2));
            //painter->drawPolyline(poly.data(), xlist.count());
            pt[j] = QPointF(xlist.at(j)+SX(rect.width()/2), -ylist.at(j)+SY(rect.height()/2));
        }

        painter->drawPolyline(pt, xlist.count());

        delete pt;

    }

    ui->labelScale->setText(tr("%1").arg(sx));

    return 0;
}

void PlotDialog::paintEvent(QPaintEvent *event){
    if(!painter->isActive()) painter->begin(this);

    painter->setRenderHint(QPainter::Antialiasing);
    plot();

    if(painter->isActive()) painter->end();

}

void PlotDialog::on_toolZoomOut_clicked()
{
    if(sx <= .3) return;
    sx-=.1;
    sy-=.1;
    this->repaint();
}

void PlotDialog::on_toolZoomIn_clicked()
{
    if(sx >= 10) return;
    sx+=.1;
    sy+=.1;
    this->repaint();

}

void PlotDialog::on_toolZoomNr_clicked()
{
    sx=1.0;
    sy=1.0;
    this->repaint();
}
