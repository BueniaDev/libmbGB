#ifndef MBGBQT_UPDATER_H
#define MBGBQT_UPDATER_H

#include <QThread>
#include <memory>
#include <libmbGB/libmbGB.h>
using namespace gb;

class mbGBUpdater
{
    public:
	class Worker : public QThread
	{
	    public:
		Worker(GBCore &core) : gbcore(core), running(false), is_vblank(false)
		{

		}

		void run()
		{
		    QElapsedTimer timer;
		    timer.start();

		    while (running)
		    {
			if (!gbcore.paused)
			{
			    gbcore.runinstruction();
			}
			else
			{
			    msleep(1);
			}

			if (is_vblank)
			{
			    is_vblank = false;
			    int64_t elapsed_time = timer.elapsed();

			    int64_t expected_time = (1000 / 60);

			    if (elapsed_time < expected_time)
			    {
				msleep((expected_time - elapsed_time));
			    }

			    timer.restart();
			}
		    }
		}

		void start()
		{
		    running = true;
		    QThread::start();
		}

		void wait()
		{
		    running = false;
		    QThread::wait();
		}

		void setVBlank()
		{
		    is_vblank = true;
		}

	    private:
		GBCore &gbcore;
		volatile bool running;
		volatile bool is_vblank;
	};

	mbGBUpdater(GBCore &core) : gbcore(core)
	{

	}

	~mbGBUpdater()
	{

	}

	void start()
	{
	    worker.reset(new Worker(gbcore));
	    worker->start();
	}

	void stop()
	{
	    if (worker)
	    {
		worker->wait();
	    }
	}

	bool isRunning()
	{
	    return worker && (worker->isRunning());
	}

	void setVBlank()
	{
	    if (worker)
	    {
		worker->setVBlank();
	    }
	}

    private:
	GBCore &gbcore;
	unique_ptr<Worker> worker;
};

#endif // MBGBQT_UPDATER_H
