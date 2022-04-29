#include "displaywidget.h"
using namespace std;

DisplayWidget::DisplayWidget(GBCore &cb, QObject *parent) : core(cb), is_cleared(true)
{
    is_cleared = true;
    setResolution(2);
    setSize(160, 144);
}

DisplayWidget::~DisplayWidget()
{

}

void DisplayWidget::setSize(int w, int h)
{
    width = w;
    height = h;
    resize((width * resolution), (height * resolution));
}

void DisplayWidget::setResolution(int r)
{
    resolution = r;
    resize((width * resolution), (height * resolution));
}

int DisplayWidget::getResolution()
{
    return resolution;
}

void DisplayWidget::init()
{
    repaint();
    is_cleared = false;
}

void DisplayWidget::shutdown()
{
    is_cleared = true;
    repaint();
}

void DisplayWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.scale(resolution, resolution);

    QColor bg_color(Qt::black);
    painter.fillRect(0, 0, width, height, bg_color);

    if (is_cleared)
    {
	return;
    }

    for (int i = 0; i < width; i++)
    {
	for (int j = 0; j < height; j++)
	{
	    gbRGB pixel = core.getpixel(i, j);

	    QColor pixel_color(pixel.red, pixel.green, pixel.blue);

	    painter.fillRect(i, j, 1, 1, pixel_color);
	}
    }
}