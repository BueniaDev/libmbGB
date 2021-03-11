#ifndef MBGB_QT_H
#define MBGB_QT_H

#include <QMainWindow>
#include <QThread>
#include <QFileDialog>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QKeyEvent>
#include <iostream>
#include <libmbGB/libmbgb.h>
#include <SDL2/SDL.h>
#include "displaywidget.h"
using namespace std;
using namespace gb;

namespace Ui
{
    class MainWindow;
};

class mbGBWindow : public QMainWindow, public mbGBFrontend
{
    Q_OBJECT

    public:
        mbGBWindow(QWidget *parent = NULL);
	~mbGBWindow();

	bool init();
	void shutdown();
	void runapp();

	void audiocallback(audiotype left, audiotype right);
	void rumblecallback(bool enabled);
	void sensorcallback(uint16_t& sensorx, uint16_t& sensory);
	void pixelcallback();
	vector<uint8_t> loadfile(string filename);
	bool savefile(string filename, vector<uint8_t> data);
	bool camerainit();
	void camerashutdown();
	bool cameraframe(array<int, (128 * 120)> &arr);
	void printerframe(vector<gbRGB> &temp, bool appending);

	void gen_noise(array<int, (128 * 120)> &arr);

	QString filename;
	QString lastdir;

	bool stopped = true;

	GBCore core;

	QThread *thread = NULL;

	QGraphicsView *graphicsview = NULL;

	QElapsedTimer frame_cap_timer;

	void runthread();

	array<uint32_t, (160 * 144)> framebuffer;

	vector<int16_t> apubuffer;

	DisplayWidget *disp_widget = NULL;

    public slots:
	void openROM();
	void closeROM();
	void drawframe();

    signals:
	void signalframeready();

    private:
	void keyPressEvent(QKeyEvent *event) override;
	void keyReleaseEvent(QKeyEvent *event) override;
	void closeEvent(QCloseEvent *event) override;

    private:
	Ui::MainWindow *ui;
};


#endif // MBGB_QT_H