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

	void setResolution(int res);
	int getResolution();
	void init();
	void shutdown();

	GBCore *core = NULL;

	bool is_cleared = false;

    protected:
	void paintEvent(QPaintEvent *event);

    private:
	int resolution;
};


#endif // MBGB_QT_DISPLAY