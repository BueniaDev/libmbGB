#ifdef NULL_AUDIO

#include "nullbackend.h"
using namespace gb;

namespace gb
{
    void initaudio()
    {

    }

    void deinitaudio()
    {

    }

    void APU::outputaudio()
    {
        if (bufferfillamount >= 4096)
        {
	bufferfillamount = 0;
        }
    }
}

#endif // NULL_AUDIO
