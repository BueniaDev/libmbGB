	#ifndef MBGB_MAINWINDOW
#define MBGB_MAINWINDOW

#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QAudioProcessor>
#include "qgbcore.h"
#include "displaywidget.h"

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

    private slots:
	void drawFrame();
	void audioSample(qint16, qint16);
	void loadROM();
	void closeROM();

    private:
	void initMenuActions();
	void closeEvent(QCloseEvent *event) override;

	void keyPressEvent(QKeyEvent *event) override;
	void keyReleaseEvent(QKeyEvent *event) override;

	void keyChanged(QKeyEvent *event, bool is_pressed);

	void enableUi(bool is_enabled);

	Ui::MainWindow *ui;

	QGBCore core;
	DisplayWidget *disp_widget = NULL;

	QVector<qint16> audio_buffer;

	QAudioProcessor audio_proc;
};

#endif // MBGB_MAINWINDOW