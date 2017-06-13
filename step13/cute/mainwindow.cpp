#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSplitter>
#include <QMenuBar>
#include <QFileSystemModel>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    createMenu();

    ui->verticalLayout->removeWidget(ui->frameTool);
    ui->verticalLayout->removeWidget(ui->frameAddress);
    ui->verticalLayout->removeWidget(ui->frameTree);
    ui->verticalLayout->removeWidget(ui->frameList);
    ui->verticalLayout->removeWidget(ui->frameStat);

    QSplitter* spl = new QSplitter(Qt::Horizontal);

    spl->addWidget(ui->frameTree);
    spl->addWidget(ui->frameList);

    QSizePolicy policy = spl->sizePolicy();
    policy.setHorizontalStretch(QSizePolicy::Expanding);
    policy.setVerticalStretch(QSizePolicy::Expanding);

    spl->setSizePolicy(policy);

    ui->verticalLayout->addWidget(ui->frameTool);
    ui->verticalLayout->addWidget(ui->frameAddress);
    ui->verticalLayout->addWidget(spl);
    ui->verticalLayout->addWidget(ui->frameStat);

    ui->comboAddress->setEditable(true);

    QFileSystemModel* model = new QFileSystemModel;
    model->setRootPath("");

    ui->treeFile->setModel(model);
    ui->treeFile->setSortingEnabled(true);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createMenu()
{

    QAction *saveAct = new QAction(trUtf8("&Save"), this);
    QAction *printAct = new QAction(trUtf8("&Print"), this);
    QAction *printPreviewAct = new QAction(trUtf8("P&review"), this);
    QAction *exitAction = new QAction(trUtf8("E&xit"), this);

    QAction *copyAct = new QAction(trUtf8("&Copy"), this);
    QAction *cutAct = new QAction(trUtf8("C&ut"), this);
    QAction *pasteAct = new QAction(trUtf8("&Paste"), this);
    QAction *settingsAct = new QAction(trUtf8("&Settings..."), this);

    QAction *aboutAct = new QAction(trUtf8("&About..."), this);
    QAction *helpAct = new QAction(trUtf8("&Contents"), this);

//    connect(saveAct, SIGNAL(triggered()), this, SLOT(on_toolSave_clicked()));
//    connect(printAct, SIGNAL(triggered()), this, SLOT(on_toolPrint_clicked()));
//    connect(printPreviewAct, SIGNAL(triggered()), this, SLOT(printPreview()));
//    connect(printHtmlAct, SIGNAL(triggered()), this, SLOT(printHtml()));
    //connect(exitAction, SIGNAL(triggered()), this, SLOT(exit()));

//    connect(copyAct, SIGNAL(triggered()), this, SLOT(doCopy()));
//    connect(pasteAct, SIGNAL(triggered()), this, SLOT(doPaste()));
//    connect(rmrowAct, SIGNAL(triggered()), this, SLOT(on_toolDelete_clicked()));
//    connect(settingsAct, SIGNAL(triggered()), this, SLOT(on_toolConfig_clicked()));

//    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));
    //connect(helpAct, SIGNAL(triggered()), this, SLOT(aboutQt()));

    QMenu* fileMenu = menuBar()->addMenu(trUtf8("&File"));
    fileMenu->addAction(saveAct);
    fileMenu->addAction(printAct);
    fileMenu->addAction(printPreviewAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    QMenu* editMenu = menuBar()->addMenu(trUtf8("&Edit"));
    editMenu->addAction(cutAct);
    editMenu->addAction(copyAct);    
    editMenu->addAction(pasteAct);
    editMenu->addSeparator();
    editMenu->addAction(settingsAct);

    QMenu* helpMenu = menuBar()->addMenu(trUtf8("&Help"));
    helpMenu->addAction(helpAct);
    helpMenu->addSeparator();
    helpMenu->addAction(aboutAct);

    settingsAct->setDisabled(true);
    helpAct->setDisabled(true);
}

void MainWindow::on_toolGo_clicked()
{
    QFileSystemModel* model = (QFileSystemModel*)ui->treeFile->model();
    model->setRootPath(ui->comboAddress->currentText());
    ui->treeFile->reset();
}
