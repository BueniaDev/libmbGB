#ifndef MBGB_QT_H
#define MBGB_QT_H

#include <QMainWindow>
#include <iostream>
using namespace std;

namespace Ui
{
    class MainWindow;
};

class mbGBWindow : public QMainWindow
{
    Q_OBJECT

    public:
        mbGBWindow(QWidget *parent = NULL);
	~mbGBWindow();

    private:
	void closeEvent(QCloseEvent *event) override;

    private:
	Ui::MainWindow *ui;
};


#endif // MBGB_QT_H