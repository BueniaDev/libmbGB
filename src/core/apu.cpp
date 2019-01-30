#include "../../include/libmbGB/apu.h"
#include <iostream>
using namespace gb;
using namespace std;

namespace gb
{
    APU::APU()
    {
        reset();
    }
    
    APU::~APU()
    {
        cout << "APU::Shutting down..." << endl;
    }
    
    void APU::reset()
    {
        frametimer = 8192;
        framesequencer = 0;
        samplecounter = 95;
        
        squareone.timer = 0;
        squareone.timerload = 0;
        squareone.volume = 0;
        squareone.volumeload = 0;
        
        squaretwo.timer = 0;
        squaretwo.timerload = 0;
        squaretwo.volume = 0;
        squaretwo.volumeload = 0;
        
        wave.timer = 0;
        wave.timerload = 0;
        wave.volumecode = 0;
        
        noise.volume = 0;
        noise.volumeload = 0;
        
        cout << "APU::Initialized" << endl;
    }
    
    uint8_t APU::readapu(uint16_t address)
    {
        uint8_t returndata = 0;
        
        if ((address >= 0xFF10) && (address <= 0xFF14))
        {
            returndata = squareone.readreg(address);
        }
        else if ((address >= 0xFF16) && (address <= 0xFF19))
        {
            returndata = squaretwo.readreg(address);
        }
        else if ((address >= 0xFF1A) && (address <= 0xFF1E))
        {
            returndata = wave.readreg(address);
        }
        else if ((address >= 0xFF20) && (address <= 0xFF23))
        {
            returndata = noise.readreg(address);
        }
        else if ((address >= 0xFF24) && (address <= 0xFF26))
        {
            switch (address)
            {
                case 0xFF24:
                {
                    returndata = (rightvol) | (vinrightenable << 3) | (leftvol << 4) | (vinleftenable << 7);
                }
                break;
                case 0xFF25:
                {
                    for (int i = 0; i < 4; i++)
                    {
                        returndata |= (rightenables[i] << i);
                    }
                    
                    for (int i = 0; i < 4; i++)
                    {
                        returndata |= (leftenables[i] << (i + 4));
                    }
                }
                break;
                case 0xFF26:
                {
                   returndata |= powercontrol << 7;
                   returndata |= squareone.getrunning() << 0;
                   returndata |= squaretwo.getrunning() << 1;
                   returndata |= wave.getrunning() << 2;
                }
                break;
            }
        }
        else if ((address >= 0xFF30) && (address <= 0xFF3F))
        {
            returndata = wave.readreg(address);
        }
        
        if (address <= 0xFF26)
        {
            returndata |= orvalues[address - 0xFF10];
        }
        
        return returndata;
    }
    
    void APU::writeapu(uint16_t address, uint8_t value)
    {
        if ((address >= 0xFF10) && (address <= 0xFF14))
        {
            squareone.writereg(address, value);
        }
        else if ((address >= 0xFF16) && (address <= 0xFF19))
        {
            squaretwo.writereg(address, value);
        }
        else if ((address >= 0xFF1A) && (address <= 0xFF1E))
        {
            wave.writereg(address, value);
        }
        else if ((address >= 0xFF20) && (address <= 0xFF23))
        {
            noise.writereg(address, value);
        }
        else if ((address >= 0xFF24) && (address <= 0xFF26))
        {
            switch (address)
            {
                case 0xFF24:
                {
                    rightvol = value & 0x7;
                    vinrightenable = TestBit(value, 3);
                    leftvol = (value >> 4) & 0x7;
                    vinleftenable = TestBit(value, 7);
                }
                break;
                case 0xFF25:
                {
                    for (int i = 0; i < 4; i++)
                    {
                        rightenables[i] = TestBit((value >> i), 0);
                    }
                    
                    for (int i = 0; i < 4; i++)
                    {
                        leftenables[i] = TestBit((value >> (i + 4)), 0);
                    }
                }
                break;
                case 0xFF26:
                {
                    if (!TestBit(value, 7))
                    {
                        for (int i = 0xFF10; i <= 0xFF25; i++)
                        {
                            writeapu(i, 0);
                        }
                        powercontrol = false;
                    }
                    else if (!powercontrol)
                    {
                        framesequencer = 0;
                        for (int i = 0; i < 16; i++)
                        {
                            wave.writereg(0xFF30 + i, 0);
                        }
                        powercontrol = true;
                    }
                }
                break;
            }
        }
        else if ((address >= 0xFF30) && (address <= 0xFF3F))
        {
            wave.writereg(address, value);
        }
        else
        {
            return;
        }
    }
    
    uint8_t APU::squarewave::readreg(uint16_t address)
    {
        uint8_t returndata = 0;
        uint8_t squareregister = (address & 0xF) % 0x5;
        
        switch (squareregister)
        {
            case 0x0:
            {
                returndata = (sweepshift) | ((sweepnegate) << 3) | (sweepperiodload << 4);
            }
            break;
            case 0x1:
            {
                returndata = (lengthload & 0x3F) | ((duty & 0x3) << 6);
            }
            break;
            case 0x2:
            {
                returndata = (envelopeperiodload & 0x7) | (envelopeaddmode << 3) | ((volumeload & 0xF) << 4);
            }
            break;
            case 0x3:
            {
                returndata = timerload & 0xFF;
            }
            break;
            case 0x4:
            {
                returndata = ((timerload >> 8) & 0x7) | (lengthenable << 6) | (triggerbit << 7);
            }
            break;
        }
        
        return returndata;
    }
    
    void APU::squarewave::writereg(uint16_t address, uint8_t value)
    {
        uint8_t squareregister = (address & 0xF) % 0x5;
        
        switch (squareregister)
        {
            case 0x0:
            {
                sweepshift = value & 0x7;
                sweepnegate = TestBit(value, 3);
                sweepperiodload = (value >> 4) & 0x7;
            }
            break;
            case 0x1:
            {
                lengthload = value & 0x3F;
                duty = (value >> 6) & 0x3;
            }
            break;
            case 0x2:
            {
                dacenabled = (value & 0xF8) != 0;
                volumeload = (value >> 4) & 0xF;
                envelopeaddmode = TestBit(value, 3);
                envelopeperiodload = (value & 0x7);
                envelopeperiod = envelopeperiodload;
                volume = volumeload;
            }
            break;
            case 0x3:
            {
                timerload = (timerload & 0x700) | value;
            }
            break;
            case 0x4:
            {
                timerload = (timerload & 0xFF) | ((value & 0x7) << 8);
                lengthenable = TestBit(value, 6);
                triggerbit = TestBit(value, 7);
                
                if (TestBit(value, 7))
                {
                    trigger();
                }
            }
            break;
        }
    }
    
    uint8_t APU::wavechannel::readreg(uint16_t address)
    {
        uint8_t returndata = 0;
        
        uint8_t registerval = address & 0xF;
        
        if ((address >= 0xFF1A) && (address <= 0xFF1E))
        {
            switch (registerval)
            {
                case 0xA:
                {
                    returndata = (dacenabled) << 7;
                }
                break;
                case 0xB:
                {
                    returndata = lengthload;
                }
                break;
                case 0xC:
                {
                    returndata = volumecode << 5;
                }
                break;
                case 0xD:
                {
                    returndata = timerload & 0xFF;
                }
                break;
                case 0xE:
                {
                    returndata = ((timerload >> 8) & 0x7) | (lengthenable << 6) | (triggerbit << 7);
                }
                break;
            }
        }
        else if (address >= 0xFF30 && (address <= 0xFF3F))
        {
            returndata = waveram[registerval];
        }
        
        return returndata;
    }
    
    void APU::wavechannel::writereg(uint16_t address, uint8_t value)
    {
        uint8_t registerval = address & 0xF;
        
        if ((address >= 0xFF1A) && (address <= 0xFF1E))
        {
            switch (registerval)
            {
                case 0xA:
                {
                    dacenabled = TestBit(value, 7);
                }
                break;
                case 0xB:
                {
                    lengthload = value;
                }
                break;
                case 0xC:
                {
                    volumecode = (value >> 5) & 0x3;
                }
                break;
                case 0xD:
                {
                    timerload = (timerload & 0x700) | value;
                }
                break;
                case 0xE:
                {
                    timerload = (timerload & 0xFF) | ((value & 0x7) << 8);
                    lengthenable = TestBit(value, 6);
                    triggerbit = TestBit(value, 7);
                    
                    if (triggerbit)
                    {
                        trigger();
                    }
                }
                break;
            }
        }
        else if (address >= 0xFF30 && (address <= 0xFF3F))
        {
            waveram[registerval] = value;
        }
    }
    
    uint8_t APU::noisechannel::readreg(uint16_t address)
    {
        uint8_t returndata = 0;
        
        switch (address)
        {
            case 0xFF20:
            {
                returndata = lengthload & 0x3F;
            }
            break;
            case 0xFF21:
            {
                returndata = (envelopeperiodload & 0x7) | (envelopeaddmode << 3) | ((volumeload & 0xF) << 4);
            }
            break;
            case 0xFF22:
            {
                returndata = (divisorcode) | (widthmode << 3) | (clockshift << 4);
            }
            break;
            case 0xFF23:
            {
                returndata = (lengthenable << 6) | (triggerbit << 7);
            }
            break;
        }
        
        return returndata;
    }
    
    void APU::noisechannel::writereg(uint16_t address, uint8_t value)
    {
        switch (address)
        {
            case 0xFF20:
            {
                lengthload = value & 0x3F;
            }
            break;
            case 0xFF21:
            {
                dacenabled = (value & 0xF8) != 0;
                volumeload = (value >> 4) & 0xF;
                envelopeaddmode = TestBit(value, 3);
                envelopeperiodload = (value & 0x7);
                envelopeperiod = envelopeperiodload;
                volume = volumeload;
            }
            break;
            case 0xFF22:
            {
                divisorcode = value & 0x7;
                widthmode = TestBit(value, 3);
                clockshift = (value >> 4) & 0xF;
            }
            break;
            case 0xFF23:
            {
                lengthenable = TestBit(value, 6);
                triggerbit = TestBit(value, 7);
                if (triggerbit)
                {
                    trigger();
                }
            }
            break;
        }
    }
    
    uint16_t APU::squarewave::sweepcalculation()
    {
        uint16_t newfreq = 0;
        newfreq = sweepshadow >> sweepshift;
        if (sweepnegate)
        {
            newfreq = sweepshadow - newfreq;
        }
        else
        {
            newfreq = sweepshadow + newfreq;
        }
        
        if (newfreq > 2047)
        {
            enabled = false;
        }
        
        return newfreq;
    }
    
    void APU::squarewave::sweepclock()
    {
        if (--sweepperiod <= 0)
        {
            sweepperiod = sweepperiodload;
            if (sweepperiod == 0)
            {
                sweepperiod = 8;
            }
            
            if (sweepenable && sweepperiodload > 0)
            {
                uint16_t newfreq = sweepcalculation();
                if (newfreq <= 2047 && sweepshift > 0)
                {
                    sweepshadow = newfreq;
                    timerload = newfreq;
                    sweepcalculation();
                }
                sweepcalculation();
            }
        }
    }
    
    void APU::squarewave::lengthclock()
    {
        if (lengthcounter > 0 && lengthenable)
        {
            lengthcounter--;
            if (lengthcounter == 0)
            {
                enabled = false;
            }
        }
    }
    
    void APU::wavechannel::lengthclock()
    {
        if (lengthcounter > 0 && lengthenable)
        {
            lengthcounter--;
            if (lengthcounter == 0)
            {
                enabled = false;
            }
        }
    }
    
    void APU::noisechannel::lengthclock()
    {
        if (lengthcounter > 0 && lengthenable)
        {
            lengthcounter--;
            if (lengthcounter == 0)
            {
                enabled = false;
            }
        }
    }
    
    void APU::squarewave::envclock()
    {
        if (--envelopeperiod <= 0)
        {
            envelopeperiod = envelopeperiodload;
            
            if (enveloperunning && envelopeperiodload > 0)
            {
                if (envelopeaddmode && volume < 15)
                {
                    volume++;
                }
                else if (!envelopeaddmode && volume > 0)
                {
                    volume--;
                }
            }
            
            if (volume == 0 || volume == 15)
            {
                enveloperunning = false;
            }
        }
    }
    
    void APU::noisechannel::envclock()
    {
        if (--envelopeperiod <= 0)
        {
            envelopeperiod = envelopeperiodload;
            if (envelopeperiod == 0)
            {
                envelopeperiod = 8;
            }
            
            if (enveloperunning && envelopeperiodload > 0)
            {
                if (envelopeaddmode && volume < 15)
                {
                    volume++;
                }
                else if (!envelopeaddmode && volume > 0)
                {
                    volume--;
                }
            }
            
            if (volume == 0 || volume == 15)
            {
                enveloperunning = false;
            }
        }
    }
    
    bool APU::squarewave::getrunning()
    {
        return lengthcounter > 0;
    }
    
    bool APU::wavechannel::getrunning()
    {
        return lengthcounter > 0;
    }
    
    bool APU::noisechannel::getrunning()
    {
        return lengthcounter > 0;
    }
    
    float APU::squarewave::getoutputvol()
    {
        return ((float)outputvol) / 100;
    }
    
    float APU::wavechannel::getoutputvol()
    {
        return ((float)outputvol) / 100;
    }
    
    float APU::noisechannel::getoutputvol()
    {
        return ((float)outputvol) / 100;
    }
    
    void APU::squarewave::trigger()
    {
        enabled = true;
        if (lengthcounter == 0)
        {
            lengthcounter = 64;
        }
        
        timer = (2048 - timerload) * 4;
        
        enveloperunning = true;
        envelopeperiod = envelopeperiodload;
        volume = volumeload;
        
        sweepshadow = timerload;
        sweepperiod = sweepperiodload;
        if (sweepperiod == 0)
        {
            sweepperiod = 8;
        }
        
        sweepenable = sweepperiod > 0 || sweepshift > 0;
        
        if (sweepshift > 0)
        {
            sweepcalculation();
        }
    }
    
    void APU::wavechannel::trigger()
    {
        enabled = true;
        if (lengthcounter == 0)
        {
            lengthcounter = 256;
        }
        timer = (2048 - timerload) * 2;
        positioncounter = 0;
    }
    
    void APU::noisechannel::trigger()
    {
        enabled = true;
        
        if (lengthcounter == 0)
        {
            lengthcounter = 64;
        }
        
        timer = divisors[divisorcode];
        envelopeperiod = envelopeperiodload;
        enveloperunning = true;
        volume = volumeload;
        lfsr = 0x7FFF;
    }
    
    void APU::squarewave::step()
    {
        if (--timer <= 0)
        {
            timer = (2048 - timerload) * 4;
            sequencepointer = (sequencepointer + 1) & 0x7;
        }
        
        if (enabled && dacenabled)
        {
            outputvol = volume;
        }
        else
        {
            outputvol = 0;
        }
        
        if (dutytable[duty][sequencepointer] == 0)
        {
            outputvol = 0;
        }
    }
    
    void APU::wavechannel::step()
    {
        if (--timer <= 0)
        {
            timer = (2048 - timerload) * 2;
            positioncounter = (positioncounter + 1) & 0x1F;
            if (enabled && dacenabled)
            {
                int position = positioncounter / 2;
                uint8_t outputbyte = waveram[position];
                bool highbit = (positioncounter & 0x1) == 0;
                if (highbit)
                {
                    outputbyte >>= 4;
                }
                
                outputbyte &= 0xF;
                
                if (volumecode > 0)
                {
                    outputbyte >>= volumecode - 1;
                }
                else
                {
                    outputbyte = 0;
                }
                outputvol = outputbyte;
            }
            else
            {
                outputvol = 0;
            }
        }
    }
    
    void APU::noisechannel::step()
    {
        if (--timer <= 0)
        {
            timer = divisors[divisorcode] << clockshift;
            
            uint8_t result = (lfsr & 0x1) ^ ((lfsr >> 1) & 0x1);
            lfsr >>= 1;
            lfsr |= result << 14;
            if (widthmode)
            {
                lfsr &= ~0x40;
                lfsr |= result << 6;
            }
            
            if (enabled && dacenabled && (lfsr & 0x1) == 0)
            {
                outputvol = volume;
            }
            else
            {
                outputvol = 0;
            }
        }
    }
    
    void APU::updateaudio(int cycles)
    {   
        int apucycles = cycles * 1.22;
        
        while (apucycles-- != 0)
        {
           if (--frametimer <= 0)
           {
               frametimer = 8192;
               switch (framesequencer)
               {
                   case 0:
                   {
                       squareone.lengthclock();
                       squaretwo.lengthclock();
                       wave.lengthclock();
                       noise.lengthclock();
                   }
                   break;
                   case 1: break;
                   case 2:
                   {
                       squareone.sweepclock();
                       squareone.lengthclock();
                       squaretwo.lengthclock();
                       wave.lengthclock();
                       noise.lengthclock();
                   }
                   break;
                   case 3: break;
                   case 4:
                   {
                       squareone.lengthclock();
                       squaretwo.lengthclock();
                       wave.lengthclock();
                       noise.lengthclock();
                   }
                   break;
                   case 5: break;
                   case 6:
                   {
                       squareone.sweepclock();
                       squareone.lengthclock();
                       squaretwo.lengthclock();
                       wave.lengthclock();
                       noise.lengthclock();
                   }
                   break;
                   case 7:
                   {
                       squareone.envclock();
                       squaretwo.envclock();
                       noise.envclock();
                   }
                   break;
               }
               framesequencer++;
               if (framesequencer >= 8)
               {
                   framesequencer = 0;
               }
           }
           
           squareone.step();
           squaretwo.step();
           wave.step();
           noise.step();
           
           #ifdef UNIX_AUDIO
           if (--samplecounter <= 0)
           {
               samplecounter = 95;
               mixaudio();
           }
           outputaudio();
           #endif // UNIX_AUDIO
        }
    }
}