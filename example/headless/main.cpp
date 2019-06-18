#include <libmbGB/cpu.h>
#include <libmbGB/mmu.h>
#include <iostream>
using namespace gb;
using namespace std;

MMU coremmu;
CPU corecpu(coremmu);

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
	cout << "Usage: " << argv[0] << " ROM [BIOS]" << endl;
	return 1;
    }

    coremmu.init();
    coremmu.biosload = true;
    corecpu.init();

    coremmu.loadBIOS(argv[2]);
    coremmu.loadROM(argv[1]);

    corecpu.runfor(32);

    coremmu.shutdown();
    return 0;
}