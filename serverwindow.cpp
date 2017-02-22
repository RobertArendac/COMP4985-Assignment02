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
        runTCPServer(this, SOCK_STREAM, IPPROTO_TCP);
    } else {
        runUDPServer(this, SOCK_DGRAM, IPPROTO_UDP);
    }
}

void ServerWindow::display(const char *msg) {
    ui->textArea->setText(msg);
}
