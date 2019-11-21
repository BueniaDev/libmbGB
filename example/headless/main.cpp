#include <libmbGB/libmbgb.h>
#include <iostream>
#include <functional>
using namespace gb;
using namespace std;
using namespace std::placeholders;

GBPrinter print;

void printcb(vector<RGB> &buffer)
{
    return;
}

void recieve(uint8_t byte)
{
    cout << "Byte recieved: " << hex << (int)(byte) << endl;
}

void sendbyte(uint8_t byte)
{
    print.printerready(byte, true);
}

int main(int argc, char* argv[])
{
    print.setprintcallback(bind(&printcb, _1));
    print.setprintreccallback(bind(&recieve, _1));

    sendbyte(0x88);
    sendbyte(0x33);
    sendbyte(0x0F);
    sendbyte(0x00);
    sendbyte(0x00);
    sendbyte(0x00);
    sendbyte(0x0F);
    sendbyte(0x00);
    sendbyte(0x00);

    cout << "Program execution finished." << endl;
	
    return 0;
}
