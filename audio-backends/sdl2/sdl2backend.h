#ifdef SDL2_AUDIO

#include "../../include/libmbGB/apu.h"
#include "../../include/libmbGB/libmbgb_api.h"
#include <SDL2/SDL.h>
using namespace gb;

namespace gb
{
    void LIBMBGB_API initsdl2audio();
    void LIBMBGB_API deinitsdl2audio();
}

#endif // SDL2_AUDIO
