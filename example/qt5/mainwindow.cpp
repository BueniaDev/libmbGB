#include "mainwindow.h"
#include "ui_mainwindow.h"
using namespace std;

mbGBWindow::mbGBWindow(QWidget *parent) : ui(new Ui::MainWindow)
{
    setWindowTitle("mbGB-Qt");
    ui->setupUi(this);
}

mbGBWindow::~mbGBWindow()
{

}

void mbGBWindow::closeEvent(QCloseEvent *event)
{
    QMainWindow::closeEvent(event);
}