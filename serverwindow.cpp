#include "serverwindow.h"
#include "ui_serverwindow.h"
#include "server.h"

ServerWindow::ServerWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ServerWindow)
{
    ui->setupUi(this);
}

ServerWindow::~ServerWindow()
{
    delete ui;
}

void ServerWindow::on_startServerButton_clicked()
{
    if (ui->tcpRadio->isChecked()) {
        runServer(SOCK_STREAM, IPPROTO_TCP);
    } else {
        runServer(SOCK_DGRAM, IPPROTO_UDP);
    }
}
