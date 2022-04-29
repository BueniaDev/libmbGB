#include <QApplication>
#include <QMainWindow>
#include "mainwindow.h"
#include <iostream>
using namespace std;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    mbGBWindow window;
    window.show();

    return app.exec();
}