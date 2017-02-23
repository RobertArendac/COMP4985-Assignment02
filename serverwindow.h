#ifndef SERVERWINDOW_H
#define SERVERWINDOW_H

#include <QMainWindow>
#include <WinSock2.h>
#include <Windows.h>

namespace Ui {
class ServerWindow;
}

class ServerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ServerWindow(QWidget *parent = 0);
    ~ServerWindow();
    void updateTime(int timeInMs);
    void updatePackets(int numPackets);
    void updateSize(int size);

private slots:
    void on_startServerButton_clicked();

private:
    Ui::ServerWindow *ui;
};

#endif // SERVERWINDOW_H
