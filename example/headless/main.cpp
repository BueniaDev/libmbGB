// Minimal example for utilizing the libmbGB API

#include <libmbGB/libmbgb.h> // This is the main header file for libmbGB
#include <iostream>
#include <functional>
using namespace gb; // The core emulation logic is tucked into the "gb" namespace
using namespace std;
using namespace std::placeholders;

int main(int argc, char* argv[])
{
    // Create an instance of the libmbGB core
    GBCore core;
    
    // Set sample rate for audio playback, in kHz
    core.setsamplerate(48000);
    
    // Set audio type (we set it to signed 16-bit integers)
    core.setaudioflags(MBGB_SIGNED16);
    
    
    // Set the audio callback (we set it below)
    core.setaudiocallback([](audiotype left, audiotype right){
    	if (holds_alternative<int16_t>(left))
    	{
    	    cout << "Left: " << dec << (int)(get<int16_t>(left)) << endl;
    	}
    	
    	if (holds_alternative<int16_t>(right))
    	{
    	    cout << "Right: " << dec << (int)(get<int16_t>(right)) << endl;
    	}
    });
    
    // Process command-line arguments
    if (!core.getoptions(argc, argv))
    {
        return 1;
    }
    
    // Initalize the emulator
    if (!core.initcore())
    {
        return 1;
    }
    
    core.runcore(); // Run the emulator for 1 frame...
    core.shutdown(); // ...and then shut it down
    return 0;

    return 0;
}
