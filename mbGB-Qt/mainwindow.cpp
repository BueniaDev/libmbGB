#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <functional>

mbGBWindow::mbGBWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    disp_widget = new DisplayWidget(core.getCore(), this);
    setCentralWidget(disp_widget);

    if (!audio_proc.init(48000))
    {
	cout << "Could not initialize audio processor." << endl;
	exit(0);
    }

    ui->statusBar->showMessage("Ready");
    initMenuActions();
}

mbGBWindow::~mbGBWindow()
{
    closeROM();
    audio_proc.shutdown();
    delete disp_widget;
    delete ui;
}

void mbGBWindow::initMenuActions()
{
    connect(ui->actionLoadROM, SIGNAL(triggered()), this, SLOT(loadROM()));
    connect(ui->actionCloseROM, SIGNAL(triggered()), this, SLOT(closeROM()));
    connect(ui->actionQuit, SIGNAL(triggered()), QApplication::instance(), SLOT(quit()));

    connect(&core, SIGNAL(drawFrame()), this, SLOT(drawFrame()));
    connect(&core, SIGNAL(audioSample(qint16, qint16)), this, SLOT(audioSample(qint16, qint16)));
}

void mbGBWindow::closeEvent(QCloseEvent *event)
{
    closeROM();
    QMainWindow::closeEvent(event);
}

void mbGBWindow::keyPressEvent(QKeyEvent *event)
{
    keyChanged(event, true);
}

void mbGBWindow::keyReleaseEvent(QKeyEvent *event)
{
    keyChanged(event, false);
}

void mbGBWindow::keyChanged(QKeyEvent *event, bool is_pressed)
{
    switch (event->key())
    {
	case Qt::Key_Up: core.keyChanged(gbButton::Up, is_pressed); break;
	case Qt::Key_Down: core.keyChanged(gbButton::Down, is_pressed); break;
	case Qt::Key_Left: core.keyChanged(gbButton::Left, is_pressed); break;
	case Qt::Key_Right: core.keyChanged(gbButton::Right, is_pressed); break;
	case Qt::Key_A: core.keyChanged(gbButton::A, is_pressed); break;
	case Qt::Key_B: core.keyChanged(gbButton::B, is_pressed); break;
	case Qt::Key_Return: core.keyChanged(gbButton::Start, is_pressed); break;
	case Qt::Key_Space: core.keyChanged(gbButton::Select, is_pressed); break;
	default: break;
    }
}

void mbGBWindow::drawFrame()
{
    disp_widget->repaint();
}

void mbGBWindow::audioSample(qint16 left, qint16 right)
{
    if (audio_proc.getQueuedAudioSize() > 48000)
    {
	return;
    }

    audio_proc.queueAudio(left, right);
}

void mbGBWindow::enableUi(bool is_enabled)
{
    ui->actionCloseROM->setEnabled(is_enabled);
    ui->menuSave->setEnabled(is_enabled);
    ui->menuLoad->setEnabled(is_enabled);
}

void mbGBWindow::loadROM()
{
    bool was_running = core.isRunning();

    closeROM();
    auto filename = QFileDialog::getOpenFileName(this, tr("Please select a ROM file..."), "", tr("Game Boy (Color) ROMs (*.gb *.gbc)"));

    auto valid_rom = false;

    if (filename.size() > 1)
    {
	core.setROM(filename);
	valid_rom = true;
    }

    if (was_running || valid_rom)
    {
	if (!core.start())
	{
	    QMessageBox::critical(this, "Error", "Could not load ROM.");
	    return;
	}

	enableUi(true);
	audio_proc.start();
	statusBar()->showMessage("Loaded " + filename);
	disp_widget->init();
    }
}

void mbGBWindow::closeROM()
{
    if (core.isRunning())
    {
	audio_proc.stop();
	core.stop();
	disp_widget->shutdown();
	enableUi(false);
    }
}