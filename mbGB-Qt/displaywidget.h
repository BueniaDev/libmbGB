#ifndef MBGB_QT_DISPLAY
#define MBGB_QT_DISPLAY

#include <QWidget>
#include <QImage>
#include <QPainter>
#include <iostream>
#include <libmbGB/libmbgb.h>
using namespace std;
using namespace gb;

class DisplayWidget : public QWidget
{
    public:
	DisplayWidget(GBCore *cb, QObject *parent = NULL);
	~DisplayWidget();

	void init();
	void shutdown();

	GBCore *core = NULL;

	bool is_cleared = false;

    protected:
	void paintEvent(QPaintEvent *event);
};


#endif // MBGB_QT_DISPLAY