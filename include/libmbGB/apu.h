#ifndef GB_APU
#define GB_APU

#include "libmbgb_api.h"
#include "utils.h"

namespace gb
{
    
    class LIBMBGB_API APU
    {
    public:
        APU();
        ~APU();
        
        void reset();
        void updateaudio(int cycles);
        void mixaudio(); // Must be defined with your audio API of choice
        void outputaudio(); // Must be defined with your audio API of choice
        
        int frametimer;
        int framesequencer;
        int samplecounter;
        
        bool vinleftenable = false;
        uint8_t leftvol = 0;
        bool vinrightenable = false;
        uint8_t rightvol = 0;
        bool leftenables[4] = {false};
        bool rightenables[4] = {false};
        bool powercontrol = false;

        
        int bufferfillamount = 0;
        float mainbuffer[4096] = {0};
        
        uint8_t orvalues[23] = 
        {  
            0x80,0x3f,0x00,0xff,0xbf,
            0xff,0x3f,0x00,0xff,0xbf,
            0x7f,0xff,0x9f,0xff,0xbf,
            0xff,0xff,0x00,0x00,0xbf,
            0x00,0x00,0x70 
        };
        
        uint8_t readapu(uint16_t address);
        void writeapu(uint16_t address, uint8_t value);
        
        struct squarewave
        {
            uint8_t readreg(uint16_t address);
            void writereg(uint16_t address, uint8_t value);
            void trigger();
            void lengthclock();
            void sweepclock();
            uint16_t sweepcalculation();
            void envclock();
            void step();
            bool lengthenable = false;
            bool triggerbit = false;
            uint8_t sweepshift = 0;
            bool sweepnegate = false;
            uint8_t sweepperiodload = 0;
            int sweepperiod = 0;
            bool sweepenable = false;
            uint16_t sweepshadow = 0;
            bool enabled = false;
            uint16_t timerload = 0;
            int timer = 0;
            uint8_t lengthcounter = 0;
            uint8_t lengthload = 0;
            bool enveloperunning = true;
            uint8_t envelopeperiodload = 0;
            int envelopeperiod = 0;
            bool envelopeaddmode = false;
            uint8_t volume = 0;
            uint8_t volumeload = 0;
            int duty = 0;
            int sequencepointer = 0;
            int outputvol = 0;
            int dutytable[4][8] =
            {
                {0, 0, 0, 0, 0, 0, 0, 1},
                {1, 0, 0, 0, 0, 0, 0, 1},
                {1, 0, 0, 0, 0, 1, 1, 1},
                {0, 1, 1, 1, 1, 1, 1, 0}
            };
            
            bool getrunning();
            float getoutputvol();
            bool dacenabled = true;
        };
        
        squarewave squareone;
        squarewave squaretwo;
    };
}

#endif // GB_APU