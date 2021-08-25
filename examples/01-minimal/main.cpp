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
    
    // Set the audio callback (we set it below)
    core.setaudiocallback([](int16_t left, int16_t right){
    	cout << "Left: " << dec << left << endl;
    	cout << "Right: " << dec << right << endl;
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
