#ifdef SDL2_AUDIO

#include "backend.h"
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

    void APU::mixaudio()
    {
        float bufferin0 = 0;
	float bufferin1 = 0;
    
	int volume = (128 * leftvol) / 7;
    
	float sound1 = squareone.getoutputvol();
	float sound2 = squaretwo.getoutputvol();
	float sound3 = wave.getoutputvol();
	float sound4 = noise.getoutputvol();
    
	if (leftenables[0])
	{
            bufferin1 = sound1;
            SDL_MixAudioFormat((Uint8*)&bufferin0, (Uint8*)&bufferin1, AUDIO_F32SYS, sizeof(float), volume);
	}
    
    	if (leftenables[1])
    	{
            bufferin1 = sound2;
            SDL_MixAudioFormat((Uint8*)&bufferin0, (Uint8*)&bufferin1, AUDIO_F32SYS, sizeof(float), volume);
    	}
    
    	if (leftenables[2])
    	{
            bufferin1 = sound3;
            SDL_MixAudioFormat((Uint8*)&bufferin0, (Uint8*)&bufferin1, AUDIO_F32SYS, sizeof(float), volume);
    	}
    
    	if (leftenables[3])
    	{
       	    bufferin1 = sound4;
            SDL_MixAudioFormat((Uint8*)&bufferin0, (Uint8*)&bufferin1, AUDIO_F32SYS, sizeof(float), volume);
    	}

	bufferin0 = 0;
	bufferin1 = 0;

	volume = (128 * rightvol) / 7;

	if (rightenables[0])
	{
            bufferin1 = sound1;
            SDL_MixAudioFormat((Uint8*)&bufferin0, (Uint8*)&bufferin1, AUDIO_F32SYS, sizeof(float), volume);
	}
    
    	if (rightenables[1])
    	{
            bufferin1 = sound2;
            SDL_MixAudioFormat((Uint8*)&bufferin0, (Uint8*)&bufferin1, AUDIO_F32SYS, sizeof(float), volume);
    	}
    
    	if (rightenables[2])
    	{
            bufferin1 = sound3;
            SDL_MixAudioFormat((Uint8*)&bufferin0, (Uint8*)&bufferin1, AUDIO_F32SYS, sizeof(float), volume);
    	}
    
    	if (rightenables[3])
    	{
       	    bufferin1 = sound4;
            SDL_MixAudioFormat((Uint8*)&bufferin0, (Uint8*)&bufferin1, AUDIO_F32SYS, sizeof(float), volume);
    	}
    
	mainbuffer[bufferfillamount + 1] = bufferin0;
    
    	bufferfillamount += 2;
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
