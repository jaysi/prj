#include <QApplication>
#include <QStyleFactory>
#include "mainwindow.h"

int main(int argc, char *argv[])
{    
    QApplication a(argc, argv);    
    MainWindow w;
//    qDebug()<<QStyleFactory::keys();
//    a.setStyle("Cleanlooks");
#ifndef ANDROID
    a.setStyle("Plastique");
#else
    a.setStyle("Android");
#endif

    w.show();

    return a.exec();
}
