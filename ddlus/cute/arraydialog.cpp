#include "arraydialog.h"
#include "ui_arraydialog.h"

#include "mainwindow.h"

void uparrlist(struct session* sess){

    arrayDialog* a = (arrayDialog*)sess->aed;
    struct variable* var;

    a->clearArrayList();

    for(var = sess->cursess->vfirst; var; var = var->next){
        if(var->type == VAR_ARRAY) a->addArray(var->name);
    }

}

void setarr(struct session* sess, char* name, int maxi, int maxj){
    int i, j;
    struct variable* var;
    arrayDialog* a = (arrayDialog*)sess->aed;

    a->setArray(sess, name, maxi, maxj);

    for(i = 0; i < maxi + 1; i++){
        if(maxj > -1){
            for(j = 0; j < maxj + 1; j++){
                var = array_ptr(sess, name, i, j, NULL);
                if(var){
                    a->setCell(i, j, var->exp, var->val, var->desc, var->flags);
                }
            }
        } else {
            var = array_ptr(sess, name, i, -1, NULL);
            if(var){
                a->setCell(i, 0, var->exp, var->val, var->desc, var->flags);
            }
        }
    }

}

int editarray(struct session* sess){

    char* name;
    //struct variable* var;
    int maxj, maxi;

    arrayDialog* a = (arrayDialog*)sess->aed;
    if(!a){
        printe(MSG_IMPLEMENT, "plot");
        return false_;
    }

    struct tok_s* ar;
    ar = poptok(sess);
    if(!ar){
        maxi = -1; maxj = -1; name = NULL;
        goto startit;//!
//        printe(MSG_MOREARGS);
//        return false_;
    }

    if(ar->id != TOK_ARRAY){
        printe(MSG_BADARG, ar->str);
        return false_;
    }

    if(!array_limits(sess, ar->str, &maxi, &maxj)){
        printe(MSG_ERROR, "array_limits");
        return false_;
    }
    name = ar->str;

startit:

    setarr(sess, name, maxi, maxj);
    uparrlist(sess);

//    a->setArray(sess, ar->str, maxi+1, maxj<0?1:maxj+1);

//    for(i = 0; i < maxi + 1; i++){
//        if(maxj > -1){
//            for(j = 0; j < maxj + 1; j++){
//                var = array_ptr(sess, ar->str, i, j, NULL);
//                if(var){
//                    a->setCell(i, j, var->exp, var->val, var->desc, var->flags);
//                }
//            }
//        } else {
//            var = array_ptr(sess, ar->str, i, -1, NULL);
//            if(var){
//                a->setCell(i, 0, var->exp, var->val, var->desc, var->flags);
//            }
//        }
//    }

    a->show();

    return true_;

}

arrayDialog::arrayDialog(QWidget *parent, void *qMain) :
    QDialog(parent),
    ui(new Ui::arrayDialog)
{
    ui->setupUi(this);
    main = qMain;
    copy_item = NULL;
    ui->tableArray->installEventFilter(this);

    QStringList list = QColor::colorNames();
    ui->comboCellColor->addItem(tr("transparent"));
    ui->comboCellColor->addItems(list);
}

arrayDialog::~arrayDialog()
{
    delete ui;
}

void arrayDialog::clearArrayList(){
    ui->comboArrays->clear();
}

void arrayDialog::addArray(char* name){
    if(ui->comboArrays->findText(trUtf8(name)) == -1) ui->comboArrays->insertItem(0, trUtf8(name));
}

void arrayDialog::setArray(struct session* sess, char *name, int i, int j){
    ui->labelArray->setText(trUtf8("%1[%2][%3]").arg(trUtf8(name)).arg(i+1).arg(j<0?-1:j+1));
    ui->tableArray->setRowCount(i+1);
    ui->tableArray->setColumnCount(j<0?1:j+1);

//    ui->spinRow->setMinimum(i+1);
//    ui->spinRow->setValue(i+1);
//    if(j < 0){ui->spinCol->setValue(-1); ui->spinCol->setEnabled(false);}
//    else {ui->spinCol->setEnabled(true); ui->spinCol->setMinimum(j+1); ui->spinCol->setValue(j+1);}

    strName = trUtf8(name);
    maxi = i;
    maxj = j;
    sessptr = sess;

}

void arrayDialog::setCell(int i, int j, char *exp, fnum_t val, char *desc, char flags){
    QTableWidgetItem* item;
    item = new QTableWidgetItem;
    if(!item) return;

    item->setText(tr("%1").arg((qreal)val));
    item->setToolTip(trUtf8(desc));
    item->setStatusTip(trUtf8(exp));

    if(flags & VARF_LOCK){
//        Qt::ItemFlags flags = item->flags();
//        flags ^= ~(Qt::ItemIsEditable);
//        item->setFlags(flags);
        item->setBackgroundColor(QColor("gray"));
    }

    ui->tableArray->setItem(i, j<0?0:j, item);

}

void arrayDialog::on_tableArray_cellChanged(int row, int column)
{
    if(!ui->toolAutoSync->isChecked()){

        set_arrayval(sessptr, strName.toUtf8().data(), row, maxj<0?-1:column, ui->tableArray->item(row, column)?ui->tableArray->item(row, column)->text().toDouble():0);

    } else {

        //set_aexp(sessptr, strName.toUtf8().data(), row, maxj<0?-1:column, ui->tableArray->item(row, column)?ui->tableArray->item(row, column)->text().toUtf8().data():"");
        runExp(tr("re(%1);").arg(strName));

        QList <QTableWidgetItem*> list;// = ui->tableArray->items();

        for(int i = 0; i < list.count(); i++){
            list.at(i)->setText(trUtf8("%1").arg((qreal)array_val(sessptr, strName.toUtf8().data(), list.at(i)->row(), maxj<0?-1:list.at(i)->column())));
        }

    }
}

void arrayDialog::on_lineExp_returnPressed()
{
    if(!ui->tableArray->item(ui->tableArray->currentRow(), ui->tableArray->currentColumn())) return;
    set_aexp(sessptr, strName.toUtf8().data(), ui->tableArray->currentRow(), maxj<0?-1:ui->tableArray->currentColumn(), ui->lineExp->text().toUtf8().data());
    ui->tableArray->item(ui->tableArray->currentRow(), ui->tableArray->currentColumn())->setStatusTip(ui->lineExp->text());
    ui->tableArray->setFocus();
}

void arrayDialog::on_tableArray_cellActivated(int row, int column)
{
    struct variable* var;

    var = array_ptr(sessptr, strName.toUtf8().data(), row, column, NULL);

    if(!var) return;

    ui->lineExp->setText(trUtf8(var->exp));
}

void arrayDialog::on_tableArray_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    struct variable* var;

    var = array_ptr(sessptr, strName.toUtf8().data(), currentRow, maxj<0?-1:currentColumn, NULL);

    if(!var){
        //ui->labelArray->setText(trUtf8("%1[%2][%3]").arg(strName).arg(maxi).arg(maxj<0?-1:maxj));
        return;
    }

    ui->lineExp->setText(trUtf8(var->exp));

    ui->labelArray->setText(trUtf8("%1[%2][%3]").arg(strName).arg(currentRow).arg(maxj<0?-1:currentColumn));
}

int arrayDialog::runExp(QString exp){
    switch(run_expstr(sessptr, exp.toUtf8().data())){
    case true_:
        delete_exp(sessptr);
        run_exp(sessptr);
        finish_exp(sessptr);
        return true_;
        //list.at(i)->setText(trUtf8("%1").arg((qreal)array_val(sessptr, strName.toUtf8().data(), list.at(i)->row(), maxj==-1?-1:list.at(i)->column())));
        break;
    case continue_:
        return continue_;
        break;
    case false_:
        run_exp_ci(sessptr);
        finish_exp(sessptr);
        //list.at(i)->setText(trUtf8("%1").arg((qreal)array_val(sessptr, strName.toUtf8().data(), list.at(i)->row(), maxj<0?-1:list.at(i)->column())));
        break;
    }
    return false_;

}

void arrayDialog::on_toolRecalc_clicked()
{
    QList <QTableWidgetItem*> list = ui->tableArray->selectedItems();
    QString strExp;

    for(int i = 0; i < list.count(); i++){
        strExp = trUtf8("re(%1[%2][%3]);").arg(strName).arg(list.at(i)->row()).arg(maxj<0?-1:list.at(i)->column());
        runExp(strExp);
        list.at(i)->setText(trUtf8("%1").arg((qreal)array_val(sessptr, strName.toUtf8().data(), list.at(i)->row(), maxj<0?-1:list.at(i)->column())));
    }
}

void arrayDialog::on_comboArrays_currentIndexChanged(const QString &arg1)
{
    int i, j;
    if(!array_limits(sessptr, arg1.toUtf8().data(), &i, &j)){
        return;
    }
    setarr(sessptr, arg1.toUtf8().data(), i, j);
}

void arrayDialog::clearCell(QTableWidget* table){
    QList <QTableWidgetItem* > itemList = table->selectedItems();

    while(!itemList.isEmpty()){
        delete itemList.takeFirst();
    }
}

void arrayDialog::setCellItem(QTableWidget* table, QTableWidgetItem* copy_item){
    int i, row, col;
    QTableWidgetItem* item;
    int tR, bR, rC, lC, swap;

    QList<QTableWidgetSelectionRange> selList = table->selectedRanges();

    for(i = 0; i < selList.count(); i++){

        tR = selList.at(i).topRow();
        bR = selList.at(i).bottomRow();

        if(tR > bR){
            swap = bR;
            bR = tR;
            tR = swap;
        }

        rC = selList.at(i).rightColumn();
        lC = selList.at(i).leftColumn();

        if(rC > lC){
            swap = lC;
            lC = rC;
            rC = swap;
        }

        for(row = tR; row <= bR; row++){

            for(col = rC; col <= lC; col++){

                item = table->item(row, col);
                if(item){
                    table->removeCellWidget(row, col);
                    delete item;
                }
                item = cloneItem(copy_item);
                table->setItem(row, col, item);

                set_aexp(sessptr, strName.toUtf8().data(), row, maxj<0?-1:col, item->statusTip().toUtf8().data());
                set_adesc(sessptr, strName.toUtf8().data(), row, maxj<0?-1:col, item->toolTip().toUtf8().data());

            }

        }

    }
}

QTableWidgetItem* arrayDialog::cloneItem(QTableWidgetItem* src){
    QTableWidgetItem* item = new QTableWidgetItem;
    if(!item) return NULL;

    qDebug()<<"clone: "<<src->text()<<" "<<src->toolTip()<<" "<<src->statusTip();

    item->setText(src->text());
    item->setToolTip(src->toolTip());
    item->setStatusTip(src->statusTip());

    return item;
}

bool arrayDialog::eventFilter(QObject *obj, QEvent *event){

    QTableWidget* table;
    QKeyEvent* key;
    //QDropEvent* drop;
    //QTableWidgetItem* item;
    //QStringList list;
    //QString str;
//    const QClipboard *clipboard = QApplication::clipboard();

    if( obj->objectName() == tr("tableArray")){

        table = dynamic_cast<QTableWidget*>(obj);
        if(event->type() == QEvent::KeyPress){

            key = dynamic_cast<QKeyEvent*>(event);

            switch(key->key()){

            case Qt::Key_Delete:
                clearCell(table);
                break;

            case Qt::Key_D:
                if(key->modifiers() == Qt::ControlModifier){
                    setComment(table);
                }
                break;

            case Qt::Key_C:
                if(key->modifiers() == Qt::ControlModifier){

                    if(table->currentItem()){

                        if(copy_item) delete copy_item;

                        copy_item = cloneItem(table->currentItem());

                    } else {
                        if(copy_item) delete copy_item;
                        copy_item = NULL;
                    }

                }

                break;

            case Qt::Key_V:

                if(key->modifiers() == Qt::ControlModifier){

                    //str = QApplication::clipboard()->text();
                    if(copy_item) setCellItem(table, copy_item);

                }

                break;

            case Qt::Key_Up:
            case Qt::Key_Down:
            case Qt::Key_Left:
            case Qt::Key_Right:
                break;
            case Qt::Key_Enter:
            case Qt::Key_Return:
                if(key->modifiers() == Qt::AltModifier){
                    ui->lineExp->setFocus();
                }
                break;

            default:

                break;
            }

        }

    }
    return false;
}

void arrayDialog::setComment(QTableWidget* table){
    findDialog* d;
    if(!table->currentItem()) return;
    d = new findDialog(0, tr("Comment..."), tr(":/res/topic-26.png"));
    d->strFind = table->currentItem()->toolTip();
    d->exec();
    if(set_adesc(sessptr, strName.toUtf8().data(), table->currentRow(), maxj<0?-1:table->currentColumn(), d->strFind.toUtf8().data())){
        table->currentItem()->setToolTip(d->strFind);
    }
    delete d;
}


void arrayDialog::on_toolComment_clicked()
{
    setComment(ui->tableArray);
}

void arrayDialog::on_toolExp_clicked()
{
    ui->lineExp->setFocus();
}

void arrayDialog::on_toolSetDim_clicked()
{
    int i, j;
    if(strName.isEmpty()) return;
    QString exp = tr("array(%1[%2][%3])").arg(strName).arg(ui->spinRow->value()).arg(maxj<0?-1:ui->spinCol->value());
    if(runExp(exp) == true_){
        if(array_limits(sessptr, strName.toUtf8().data(), &i, &j))
            setarr(sessptr, strName.toUtf8().data(), i, j);
    }
}

void arrayDialog::on_toolNew_clicked()
{
    if(ui->spinRow->value() <= 0) return;
    findDialog* d = new findDialog(0, tr("New Array..."), tr(":/res/add_list-26.png"));
    d->exec();
    if(d->strFind.isEmpty()) return;
    strName = d->strFind;
    delete d;
    if(ui->spinCol->value() > 0) maxj = ui->spinCol->value();
    else maxj = -1;
    on_toolSetDim_clicked();
    uparrlist(sessptr);
}

void arrayDialog::on_comboCellColor_currentIndexChanged(const QString &arg1)
{
    int i, row, col;
    QTableWidgetItem* item;
    int tR, bR, rC, lC, swap;

    QList<QTableWidgetSelectionRange> selList = ui->tableArray->selectedRanges();

    for(i = 0; i < selList.count(); i++){

        tR = selList.at(i).topRow();
        bR = selList.at(i).bottomRow();

        if(tR > bR){
            swap = bR;
            bR = tR;
            tR = swap;
        }

        rC = selList.at(i).rightColumn();
        lC = selList.at(i).leftColumn();

        if(rC > lC){
            swap = lC;
            lC = rC;
            rC = swap;
        }

        for(row = tR; row <= bR; row++){

            for(col = rC; col <= lC; col++){

                item = ui->tableArray->item(row, col);
                if(!item){
                    item = new QTableWidgetItem;
                    ui->tableArray->setItem(row, col, item);
                }
                item->setBackgroundColor(QColor(arg1));
            }

        }

    }
}

void arrayDialog::on_tableArray_itemSelectionChanged()
{
    int i, row, col;
    QTableWidgetItem* item;
    int tR, bR, rC, lC, swap;
    long count = 0;
    qreal sum = 0.0, ave = 0.0, min = 0.0, max = 0.0, prod = 1.0, val;
    bool ok, init = false;


    QList<QTableWidgetSelectionRange> selList = ui->tableArray->selectedRanges();

    for(i = 0; i < selList.count(); i++){

        tR = selList.at(i).topRow();
        bR = selList.at(i).bottomRow();

        if(tR > bR){
            swap = bR;
            bR = tR;
            tR = swap;
        }

        rC = selList.at(i).rightColumn();
        lC = selList.at(i).leftColumn();

        if(rC > lC){
            swap = lC;
            lC = rC;
            rC = swap;
        }

        for(row = tR; row <= bR; row++){

            for(col = rC; col <= lC; col++){                

                item = ui->tableArray->item(row, col);
                if(item){
                    count++;
                    val = item->text().toDouble(&ok);
                    if(ok){
                        if(!init){
                            min = val;
                            max = val;
                            init = true;
                        }
                        if(min > val) min = val;
                        if(max < val) max = val;
                        sum += val;
                        if(val) prod *= val;
                    }
                }

            }

        }

    }

    ave = sum / count;

    ui->lineStats->setText(tr("sum: %1, product: %2, min: %3, max: %4, average: %5, count: %6").arg(sum).arg(prod).arg(min).arg(max).arg(ave).arg(count));
}

void arrayDialog::on_toolPaint_clicked()
{
    on_comboCellColor_currentIndexChanged(ui->comboCellColor->currentText());
}
