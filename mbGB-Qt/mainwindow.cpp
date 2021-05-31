#include "mainwindow.h"
#include "ui_mainwindow.h"
using namespace std;

mbGBWindow::mbGBWindow(QWidget *parent) : ui(new Ui::MainWindow)
{
    frame_cap_timer.start();

    SDL_Init(SDL_INIT_AUDIO);

    SDL_AudioSpec audiospec;
    audiospec.format = AUDIO_S16SYS;
    audiospec.freq = 48000;
    audiospec.samples = 4096;
    audiospec.channels = 2;
    audiospec.callback = NULL;

    SDL_OpenAudio(&audiospec, NULL);

    core.setsamplerate(48000);
    core.setaudioflags(MBGB_SIGNED16);
    core.setfrontend(this);

    ui->setupUi(this);
    resize((160 * 2), ((144 * 2) + menuBar()->height()));
    setWindowTitle("mbGB-Qt");
    disp_widget = new DisplayWidget(&core, this);
    setCentralWidget(disp_widget);

    settings = new mbGBSettings(&core, this);

    ui->actionCloseROM->setEnabled(false);

    connect(ui->actionOpenROM, SIGNAL(triggered()), this, SLOT(openROM()));
    connect(ui->actionCloseROM, SIGNAL(triggered()), this, SLOT(closeROM()));
    connect(ui->actionQuit, SIGNAL(triggered()), QApplication::instance(), SLOT(quit()));

    connect(ui->actionPause, SIGNAL(triggered()), this, SLOT(pauseCore()));
    connect(ui->actionReset, SIGNAL(triggered()), this, SLOT(resetCore()));

    connect(ui->actionGeneral, SIGNAL(triggered()), this, SLOT(openEmuSettings()));

    connect(ui->actionAboutQt, SIGNAL(triggered()), QApplication::instance(), SLOT(aboutQt()));

    connect(this, SIGNAL(signalframeready()), this, SLOT(drawframe()));

    stopped = true;

    readSettings();
}

mbGBWindow::~mbGBWindow()
{
    closeROM();
    writeSettings();
    SDL_CloseAudio();
    SDL_Quit();
    exit(0);
}

void mbGBWindow::readSettings()
{
    QSettings emusettings("mbGB-Qt", "mbGB-Qt");
    settings->readSettings(emusettings);
    lastdir = emusettings.value("lastDir", ".").toString();
}

void mbGBWindow::writeSettings()
{
    QSettings emusettings("mbGB-Qt", "mbGB-Qt");
    settings->writeSettings(emusettings);
    emusettings.setValue("lastDir", lastdir);
}

void mbGBWindow::runthread()
{
    while (!stopped)
    {
	if (!core.paused)
	{
	    core.runinstruction();
	}
	else
	{
	    thread->msleep(1);
	}
    }
}

void mbGBWindow::openROM()
{
    filename = QFileDialog::getOpenFileName(this, tr("Please select a ROM..."), lastdir, tr("Game Boy (Color) ROMS (*.gb *.gbc)"));
    lastdir = QFileInfo(filename).absolutePath();

    if (filename != "")
    {
	if (thread == NULL)
	{
	    thread = QThread::create(bind(&mbGBWindow::runthread, this));
	    core.romname = filename.toStdString();

	    if (!core.initcore())
	    {
		QMessageBox::critical(this, "Error", "Could not load ROM.");
		thread = NULL;
		return;
	    }

	    ui->actionOpenROM->setEnabled(false);
	    ui->actionCloseROM->setEnabled(true);

	    stopped = false;
	    disp_widget->init();
	    SDL_PauseAudio(0);
	    thread->start();
	}
    }
}

void mbGBWindow::closeROM()
{
    if (thread != NULL)
    {
	SDL_PauseAudio(1);
	stopped = true;
	core.shutdown();
	disp_widget->shutdown();
	thread->quit();
	thread->wait();
	thread = NULL;
	ui->actionOpenROM->setEnabled(true);
	ui->actionCloseROM->setEnabled(false);
    }
}

void mbGBWindow::pauseCore()
{
    core.paused = ui->actionPause->isChecked();
    SDL_PauseAudio(core.paused);
}

void mbGBWindow::resetCore()
{
    core.resetcore();
}

void mbGBWindow::openEmuSettings()
{
    core.paused = true;
    settings->show();
}

void mbGBWindow::drawframe()
{
    disp_widget->repaint();
}

void mbGBWindow::closeEvent(QCloseEvent *event)
{
    closeROM();
    writeSettings();
    QMainWindow::closeEvent(event);
}

bool mbGBWindow::init()
{
    return true;
}

void mbGBWindow::shutdown()
{
    return;
}

void mbGBWindow::runapp()
{
    return;
}

void mbGBWindow::audiocallback(audiotype left, audiotype right)
{
    if (!holds_alternative<int16_t>(left) || !holds_alternative<int16_t>(right))
    {
        return;
    }

    apubuffer.push_back(get<int16_t>(left));
    apubuffer.push_back(get<int16_t>(right));

    if (apubuffer.size() >= 4096)
    {
	apubuffer.clear();

	while ((SDL_GetQueuedAudioSize(1)) > (4096 * sizeof(int16_t)))
	{
	    SDL_Delay(1);
	}
	SDL_QueueAudio(1, &apubuffer[0], (4096 * sizeof(int16_t)));
    }
}

void mbGBWindow::rumblecallback(bool enabled)
{
    return;
}

void mbGBWindow::sensorcallback(uint16_t& sensorx, uint16_t& sensory)
{
    return;
}

void mbGBWindow::pixelcallback()
{
    emit signalframeready();

    int64_t elapsed_time = frame_cap_timer.elapsed();

    if (elapsed_time < 16)
    {
	thread->msleep((16 - elapsed_time));
    }

    frame_cap_timer.restart();
}

vector<uint8_t> mbGBWindow::loadfile(string filename)
{
    vector<uint8_t> result;

    fstream file(filename.c_str(), ios::in | ios::binary | ios::ate);

    if (file.is_open())
    {
	streampos size = file.tellg();
	result.resize(size, 0);
	file.seekg(0, ios::beg);
	file.read((char*)result.data(), size);
	file.close();
    }

    return result;
}

bool mbGBWindow::savefile(string filename, vector<uint8_t> data)
{
    // If the data vector is empty, don't bother saving its contents
    if (data.empty())
    {
	return true;
    }

    fstream file(filename.c_str(), ios::out | ios::binary);

    if (!file.is_open())
    {
	cout << "mbGB::Savefile could not be written." << endl;
	return false;
    }

    file.write((char*)data.data(), data.size());
    file.close();
    return true;
}

void mbGBWindow::keyPressEvent(QKeyEvent *event)
{
    if (stopped || core.paused)
    {
	return;
    }

    switch (event->key())
    {
	case Qt::Key_A: core.keypressed(gbButton::A); break;
	case Qt::Key_B: core.keypressed(gbButton::B); break;
	case Qt::Key_Return: core.keypressed(gbButton::Start); break;
	case Qt::Key_Space: core.keypressed(gbButton::Select); break;
	case Qt::Key_Up: core.keypressed(gbButton::Up); break;
	case Qt::Key_Down: core.keypressed(gbButton::Down); break;
	case Qt::Key_Left: core.keypressed(gbButton::Left); break;
	case Qt::Key_Right: core.keypressed(gbButton::Right); break;
	case Qt::Key_I: core.sensorpressed(gbGyro::gyUp); break;
	case Qt::Key_J: core.sensorpressed(gbGyro::gyLeft); break;
	case Qt::Key_K: core.sensorpressed(gbGyro::gyDown); break;
	case Qt::Key_L: core.sensorpressed(gbGyro::gyRight); break;
    }
}

void mbGBWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (stopped || core.paused)
    {
	return;
    }

    switch (event->key())
    {
	case Qt::Key_A: core.keyreleased(gbButton::A); break;
	case Qt::Key_B: core.keyreleased(gbButton::B); break;
	case Qt::Key_Return: core.keyreleased(gbButton::Start); break;
	case Qt::Key_Space: core.keyreleased(gbButton::Select); break;
	case Qt::Key_Up: core.keyreleased(gbButton::Up); break;
	case Qt::Key_Down: core.keyreleased(gbButton::Down); break;
	case Qt::Key_Left: core.keyreleased(gbButton::Left); break;
	case Qt::Key_Right: core.keyreleased(gbButton::Right); break;
	case Qt::Key_I: core.sensorreleased(gbGyro::gyUp); break;
	case Qt::Key_J: core.sensorreleased(gbGyro::gyLeft); break;
	case Qt::Key_K: core.sensorreleased(gbGyro::gyDown); break;
	case Qt::Key_L: core.sensorreleased(gbGyro::gyRight); break;
    }
}

bool mbGBWindow::camerainit()
{
    return true;
}

void mbGBWindow::camerashutdown()
{
    return;
}

void mbGBWindow::gen_noise(array<int, (128 * 120)> &arr)
{
    srand(time(NULL));
    for (int i = 0; i < 128; i++)
    {
	for (int j = 0; j < 120; j++)
	{
	    arr[(i + (j * 128))] = (rand() & 0xFF);
	}
    }
}

bool mbGBWindow::cameraframe(array<int, (128 * 120)> &arr)
{
    gen_noise(arr);
    return true;
}

void mbGBWindow::printerframe(vector<gbRGB> &temp, bool appending)
{
    cout << "Size of printer data: " << dec << (int)(temp.size()) << endl;
    string isappending = (appending) ? "Yes" : "No";
    cout << "Appending image? " << isappending << endl;
}