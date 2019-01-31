#ifdef SDL2_AUDIO

#include "../../include/libmbGB/libmbgb.h"
#include "../../include/libmbGB/apu.h"
#include <SDL2/SDL.h>
using namespace gb;

namespace gb
{
    void initsdl2audio();
    void deinitsdl2audio();
}

#endif // SDL2_AUDIO
