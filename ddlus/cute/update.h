#ifndef UPDATE_H
#define UPDATE_H

#include <QThread>

#include "../console/double.h"

class Update : public QThread
{
    Q_OBJECT
public:
    explicit Update(QObject *parent = 0);

    void upOut(QString str, int r, int g, int b, bool bold, bool italic);
    void upPlot(fnum_t x, fnum_t y);
    void upConf(QString text);

//    void setMain(void* p);

signals:
    void updateOut(QString str, int r, int g, int b, bool bold, bool italic);
    void updatePlot(fnum_t x, fnum_t y);
    void updateConf(QString text);
    
public slots:

private:
//    void* pa;
    
};

#endif // UPDATE_H
