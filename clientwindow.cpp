#include "clientwindow.h"
#include "ui_clientwindow.h"
#include "client.h"

#define HOST_SIZE 20

ClientWindow::ClientWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ClientWindow)
{
    ui->setupUi(this);
}

ClientWindow::~ClientWindow()
{
    delete ui;
}

void ClientWindow::on_connectButton_clicked()
{
    char host[HOST_SIZE];
    int size, times;

    sprintf(host, ui->ipAddr->text().toStdString().c_str());
    size = ui->pSize->value();
    times = ui->pTimes->value();

    if (ui->tcpRadio->isChecked()) {
        runClient(SOCK_STREAM, IPPROTO_TCP, host, size, times);
    } else {
        runClient(SOCK_DGRAM, IPPROTO_UDP, host, size, times);
    }
}
