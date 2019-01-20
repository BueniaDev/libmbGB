#include "../../include/libmbGB/libmbgb.h"
#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
using namespace gb;
using namespace std;

char kp;
DMGCore core;


void APU::outputaudio()
{

}

int main(int argc, char* argv[])
{
    if (!core.getoptions(argc, argv))
    {
        return 1;
    }

    bool initialized = true;
    
    string romname = argv[1];
    string biosname;
    
    if (!core.loadROM(romname))
    {
	initialized = false;
    }
    
    if (core.coremmu.biosload == true)
    {
        biosname = argv[3];
        core.corecpu.resetBIOS();
        if (!core.loadBIOS(biosname))
	{
	    initialized = false;
	}
    }
    
    if (!initialized)
    {
        cout << "Unable to start mbGB." << endl;
        return 1;
    }
    
    do
    {
	core.runcore();
	cin >> kp;
    } while (!kp);

    cout << "Exiting..." << endl;

    return 0;
}
