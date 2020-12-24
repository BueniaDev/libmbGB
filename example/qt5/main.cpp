#include <QApplication>
#include "mainwindow.h"

#include <iostream>
using namespace std;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    mbGBWindow win;
    win.show();

    return app.exec();
}