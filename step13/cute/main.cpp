#include "mainwindow.h"
#include <QApplication>

#include <QStyle>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    a.setStyle(QStyleFactory::create("fusion"));

    w.show();

    return a.exec();
}
