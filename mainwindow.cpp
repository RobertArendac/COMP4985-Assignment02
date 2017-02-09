#include "mainwindow.h"
#include "serverwindow.h"
#include "clientwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_serverButton_clicked()
{
    this->hide();
    ServerWindow *sw = new ServerWindow();
    sw->show();
}

void MainWindow::on_clientButton_clicked()
{
    this->hide();
    ClientWindow *cw = new ClientWindow();
    cw->show();
}
