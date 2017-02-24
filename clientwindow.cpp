#include "clientwindow.h"
#include "ui_clientwindow.h"
#include "client.h"

// IP address should not be greater than 20 characters
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

/**
 * @brief ClientWindow::on_connectButton_clicked When connect button is clicked, IP, packet size, and times
 * are extracted. Protocol radio button is checked and appropriate function is run
 * @author Robert Arendac
 */
void ClientWindow::on_connectButton_clicked()
{
    char host[HOST_SIZE];
    int size, times;

    sprintf(host, ui->ipAddr->text().toStdString().c_str());
    size = ui->pSize->value();
    times = ui->pTimes->value();

    if (ui->tcpRadio->isChecked()) {
        runClient(this, SOCK_STREAM, IPPROTO_TCP, host, size, times);
    } else {
        runClient(this, SOCK_DGRAM, IPPROTO_UDP, host, size, times);
    }
}

/**
 * @brief ClientWindow::updateTime Updates the time value in the GUI
 * @param time Represents data transfer time
 * @author Robert Arendac
 */
void ClientWindow::updateTime(int time) {
    ui->tTime->display(time);
}
