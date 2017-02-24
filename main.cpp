#include "mainwindow.h"
#include <QApplication>

/**
 * Qt main, I didn't change anything here.
 * @author Qt
 */
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
