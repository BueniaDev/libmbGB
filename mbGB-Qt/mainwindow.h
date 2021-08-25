#ifndef MBGB_QT_H
#define MBGB_QT_H

#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <iostream>
#include <libmbGB/libmbgb.h>
#include <SDL2/SDL.h>
#include "displaywidget.h"
#include "emusettingsdialog.h"
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

	void initActions();
	void initMenus();

	void readSettings();
	void writeSettings();

	void audiocallback(int16_t left, int16_t right);
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
	mbGBSettings *settings = NULL;
	void setScale(int scale);

    public slots:
	void openROM();
	void closeROM();
	void pauseCore();
	void resetCore();
	void openEmuSettings();
	void drawframe();
	void set1x();
	void set2x();
	void set3x();
	void set4x();

    signals:
	void signalframeready();

    private:
	void keyPressEvent(QKeyEvent *event) override;
	void keyReleaseEvent(QKeyEvent *event) override;
	void closeEvent(QCloseEvent *event) override;
	Ui::MainWindow *ui;
};


#endif // MBGB_QT_H