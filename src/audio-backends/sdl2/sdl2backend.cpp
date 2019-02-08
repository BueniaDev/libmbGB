#ifdef SDL2_AUDIO

#include "../../../include/audio-backends/sdl2/sdl2backend.h"
using namespace gb;

namespace gb
{
    void initsdl2audio()
    {
    	SDL_AudioSpec audiospec;
    	audiospec.format = AUDIO_F32SYS;
    	audiospec.freq = 44100;
    	audiospec.samples = 4096;
    	audiospec.channels = 2;
    	audiospec.callback = NULL;
    
    	SDL_AudioSpec obtainedspec;
    	SDL_OpenAudio(&audiospec, &obtainedspec);
    	SDL_PauseAudio(0);
    }

    void deinitsdl2audio()
    {
	SDL_CloseAudio();
    }

    void APU::outputaudio()
    {
        if (bufferfillamount >= 4096)
        {
            bufferfillamount = 0;
            while ((SDL_GetQueuedAudioSize(1)) > 4096 * sizeof(float))
            {
                SDL_Delay(1);
            }
            SDL_QueueAudio(1, mainbuffer, 4096 * sizeof(float));
        }
    }
}

#endif // SDL2_AUDIO
