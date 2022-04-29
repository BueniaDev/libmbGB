#ifndef MBGB_QT_CORE
#define MBGB_QT_CORE

#include <QObject>
#include <QString>
#include <QFile>
#include <QDebug>

#include "coreupdater.h"
#include <functional>
#include <vector>
using namespace std;

class QGBCore : public QObject, public mbGBFrontend
{
    Q_OBJECT

    public:
	QGBCore(uint32_t rate = 48000) : updater(core)
	{
	    sample_rate = rate;
	    core.setsamplerate(rate);
	    core.setfrontend(this);
	}

	~QGBCore()
	{

	}

	void setROM(QString filename)
	{
	    core.romname = filename.toStdString();
	}

	bool start()
	{
	    if (!core.initcore())
	    {
		return false;
	    }

	    updater.start();
	    return true;
	}

	void pause(bool is_paused)
	{
	    core.paused = is_paused;
	}

	void reset()
	{
	    core.resetcore();
	}

	void stop()
	{
	    core.shutdown();
	    updater.stop();
	}

	bool isRunning()
	{
	    return updater.isRunning();
	}

	GBCore &getCore()
	{
	    return core;
	}

	void keyChanged(gbButton button, bool is_pressed)
	{
	    core.keychanged(button, is_pressed);
	}

    signals:
	void drawFrame();
	void audioSample(qint16, qint16);

    private:
	GBCore core;
	mbGBUpdater updater;

	uint32_t sample_rate = 0;

	void pixelcallback()
	{
	    emit drawFrame();
	    updater.setVBlank();
	}

	void audiocallback(int16_t left, int16_t right)
	{
	    emit audioSample(left, right);
	}

	vector<uint8_t> loadfile(string filename)
	{
	    QFile file(QString::fromStdString(filename));
	    vector<uint8_t> rom;

	    if (!file.open(QIODevice::ReadOnly))
	    {
		return rom;
	    }

	    rom.resize(file.size());
	    file.read((char*)rom.data(), rom.size());
	    file.close();
	    return rom;
	}

	bool savefile(string filename, vector<uint8_t> data)
	{
	    if (data.empty())
	    {
		return true;
	    }

	    QFile file(QString::fromStdString(filename));
	    file.open(QIODevice::WriteOnly);
	    file.write((char*)data.data(), data.size());
	    file.close();
	    return true;
	}
};


#endif // MBGB_QT_CORE