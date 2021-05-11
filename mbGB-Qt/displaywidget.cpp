#include "displaywidget.h"
using namespace std;

DisplayWidget::DisplayWidget(GBCore *cb, QObject *parent) : core(cb), is_cleared(true)
{

}

DisplayWidget::~DisplayWidget()
{

}

void DisplayWidget::init()
{
    is_cleared = false;
    repaint();
}

void DisplayWidget::shutdown()
{
    is_cleared = true;
    repaint();
}

void DisplayWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.scale(2, 2);

    QColor bg_color(Qt::black);
    painter.fillRect(0, 0, 160, 144, bg_color);

    if (is_cleared)
    {
	return;
    }

    for (int i = 0; i < 160; i++)
    {
	for (int j = 0; j < 144; j++)
	{
	    gbRGB pixel = core->getpixel(i, j);

	    QColor pixel_color(pixel.red, pixel.green, pixel.blue);

	    painter.fillRect(i, j, 1, 1, pixel_color);
	}
    }
}