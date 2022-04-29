#ifndef QT_AUDIO_PROC_H
#define QT_AUDIO_PROC_H

#include <QtCore>
#define QAUDIOPROC_BUILD_SDL

#ifdef QAUDIOPROC_BUILD_SDL
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#endif

class QAudioProcessorBackend : public QObject
{
    public:
	QAudioProcessorBackend()
	{

	}

	virtual bool init(quint32 sample_rate) = 0;
	virtual void shutdown() = 0;
	virtual void pause(bool is_paused) = 0;
	virtual int getQueuedAudioSize() = 0;
	virtual void queueAudio(QVector<qint16> buffer) = 0;
	virtual void clearAudio() = 0;
};

class QAudioProcessorNull : public QAudioProcessorBackend
{
    public:
	QAudioProcessorNull()
	{

	}

	bool init(quint32 sample_rate)
	{
	    (void)sample_rate;
	    return true;
	}

	void shutdown()
	{
	    return;
	}

	void pause(bool is_paused)
	{
	    (void)is_paused;
	    return;
	}

	int getQueuedAudioSize()
	{
	    return 0;
	}

	void queueAudio(QVector<qint16> buffer)
	{
	    (void)buffer;
	    return;
	}

	void clearAudio()
	{

	}
};

#ifdef QAUDIOPROC_BUILD_SDL

class QAudioProcessorSDL : public QAudioProcessorBackend
{
    public:
	QAudioProcessorSDL()
	{

	}

	bool init(quint32 sample_rate)
	{
	    if (SDL_Init(SDL_INIT_AUDIO) < 0)
	    {
		qDebug() << "Could not initialize audio! SDL_Error: " << SDL_GetError() << Qt::endl;
		return false;
	    }

	    SDL_AudioSpec audiospec;
	    audiospec.freq = sample_rate;
	    audiospec.format = AUDIO_S16SYS;
	    audiospec.channels = 2;
	    audiospec.samples = 4096;
	    audiospec.callback = NULL;

	    audio_dev = SDL_OpenAudioDevice(NULL, 0, &audiospec, NULL, 0);

	    if (audio_dev == 0)
	    {
		qDebug() << "Could not initialize audio! SDL_Error: " << SDL_GetError() << Qt::endl;
		SDL_Quit();
		return false;
	    }

	    return true;
	}

	void shutdown()
	{
	    SDL_CloseAudioDevice(audio_dev);
	    SDL_Quit();
	}

	void pause(bool is_paused)
	{
	    SDL_PauseAudioDevice(audio_dev, is_paused);
	}

	int getQueuedAudioSize()
	{
	    return SDL_GetQueuedAudioSize(audio_dev);
	}

	void queueAudio(QVector<qint16> buffer)
	{
	    SDL_QueueAudio(audio_dev, buffer.data(), (buffer.size() * sizeof(qint16)));
	}

	void clearAudio()
	{
	    SDL_ClearQueuedAudio(audio_dev);
	}

    private:
	SDL_AudioDeviceID audio_dev;
};

#endif

class QAudioProcessor : public QObject
{
    public:
	QAudioProcessor()
	{
	    #ifdef QAUDIOPROC_BUILD_SDL
	    backend = new QAudioProcessorSDL();
	    #else
	    backend = new QAudioProcessorNull();
	    #endif
	}

	~QAudioProcessor()
	{
	    delete backend;
	    backend = NULL;
	}

	bool init(quint32 sample_rate)
	{
	    return backend->init(sample_rate);
	}

	void shutdown()
	{
	    backend->shutdown();
	}

	void start()
	{
	    pause(false);
	}

	void stop()
	{
	    pause(true);
	    backend->clearAudio();
	}

	void pause(bool is_paused)
	{
	    backend->pause(is_paused);
	}

	int getQueuedAudioSize()
	{
	    return backend->getQueuedAudioSize();
	}

	void queueAudio(qint16 left, qint16 right)
	{
	    buffer.push_back(left);
	    buffer.push_back(right);

	    if (buffer.size() >= 4096)
	    {
		backend->queueAudio(buffer);
		buffer.clear();
	    }
	}

	void clearQueuedAudio()
	{
	    backend->clearAudio();
	}

    private:
	QAudioProcessorBackend *backend = NULL;
	QVector<qint16> buffer;
};


#endif // QT_AUDIO_PROC_H