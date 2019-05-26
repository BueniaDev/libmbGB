#ifndef GB_APU
#define GB_APU

#include "libmbgb_api.h"
#include "utils.h"
#include <functional>
using namespace std;

namespace gb
{
    using apuoutput = function<void()>;

    class LIBMBGB_API APU
    {
    public:
        APU();
        ~APU();
        
        void reset();
        void updateaudio(int cycles);
        void mixaudio();
	void outputaudio();
	void setaudiocallback(apuoutput cb);
        apuoutput audiocallback;
        
        int frametimer;
        int framesequencer;
        int samplecounter;
        bool output = false;
        
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
            int lengthcounter = 0;
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
        
        struct wavechannel
        {
            uint8_t readreg(uint16_t address);
            void writereg(uint16_t address, uint8_t value);
            void trigger();
            void lengthclock();
            void step();
            bool enabled = false;
            uint8_t volumecode = 0;
            int lengthcounter = 0;
            uint8_t lengthload = 0;
            uint16_t timerload = 0;
            int timer = 0;
            int outputvol = 0;
            bool getrunning();
            float getoutputvol();
            bool dacenabled = true;
            uint8_t waveram[16] = {0};
            int positioncounter = 0;
            bool triggerbit = false;
            bool lengthenable = false;
        };
        
        struct noisechannel
        {
            uint8_t readreg(uint16_t address);
            void writereg(uint16_t address, uint8_t value);
            void trigger();
            void lengthclock();
            void envclock();
            void step();
            int lengthcounter = 0;
            uint8_t lengthload = 0;
            bool enabled = false;
            bool enveloperunning = true;
            uint8_t envelopeperiodload = 0;
            int envelopeperiod = 0;
            bool envelopeaddmode = false;
            uint8_t volume = 0;
            uint8_t volumeload = 0;
            int outputvol = 0;
            uint8_t clockshift = 0;
            bool widthmode = false;
            uint8_t divisorcode = 0;
            bool triggerbit = false;
            bool lengthenable = false;
            uint16_t lfsr = 0;
            int divisors[8] =
            {
                8, 16, 32, 48,
                64, 80, 96, 112
            };
            
            bool getrunning();
            float getoutputvol();
            bool dacenabled = true;
            int timer = 0;
        };
        
        squarewave squareone;
        squarewave squaretwo;
        wavechannel wave;
        noisechannel noise;
    };
}

#endif // GB_APU
