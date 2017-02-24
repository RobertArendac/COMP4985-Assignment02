#include "serverwindow.h"
#include "ui_serverwindow.h"
#include "server.h"

DWORD WINAPI threadTCP(void *arg);
DWORD WINAPI threadUDP(void *arg);

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

/**
 * @brief ServerWindow::on_startServerButton_clicked Runs servers on seperate thread
 * @author Robert Arendac
 */
void ServerWindow::on_startServerButton_clicked()
{
    HANDLE thread;
    if (ui->tcpRadio->isChecked()) {
        thread = CreateThread(NULL, 0, threadTCP, this, 0, NULL);
        CloseHandle(thread);
    } else {
        thread = CreateThread(NULL, 0, threadUDP, this, 0, NULL);
        CloseHandle(thread);
    }
}

/* Threads so GUI isn't blocked */
DWORD WINAPI threadTCP(void *arg) {
    ServerWindow *sw = (ServerWindow *)arg;
    runTCPServer(sw, SOCK_STREAM, IPPROTO_TCP);

    return 0;
}

DWORD WINAPI threadUDP(void *arg) {
    ServerWindow *sw = (ServerWindow *)arg;
    runUDPServer(sw, SOCK_DGRAM, IPPROTO_UDP);

    return 0;
}

/* Bunch of functions to update GUI */
void ServerWindow::updateTime(int timeInMs) {
    ui->tTime->display(timeInMs);
}

void ServerWindow::updateSize(int size) {
    ui->dataSize->display(size);
}

void ServerWindow::updatePackets(int numPackets) {
    ui->numPackets->display(numPackets);
}
