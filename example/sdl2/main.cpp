#include "../../libmbGB/include/libmbGB/libmbgb.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <functional>
using namespace gb;
using namespace std;

const int screenwidth = 160;
const int screenheight = 144;
const int scale = 3;


SDL_Window *window;
SDL_Surface *surface;

DMGCore core;

int stateid = 0;
int fpscount = 0;
Uint32 fpstime = 0;

string inttostring(int val)
{
    stringstream ss;
    ss << val;
    return ss.str();
}

void screenshot()
{
    srand(SDL_GetTicks());

    stringstream temp;

    temp << (rand() % 1024) << (rand() % 1024) << (rand() % 1024);

    string screenstring = core.romname + "-" + temp.str() + ".bmp";

    SDL_SaveBMP(surface, screenstring.c_str());

    cout << "Screenshot saved." << endl;
}

void sdlcallback()
{
    while ((SDL_GetQueuedAudioSize(1)) > 4096 * sizeof(float))
    {
        SDL_Delay(1);
    }
    SDL_QueueAudio(1, core.coreapu.mainbuffer, 4096 * sizeof(float));
}

bool initSDL()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        cout << "SDL could not be initalized! SDL_Error: " << SDL_GetError() << endl;
        return false;
    }
    
    window = SDL_CreateWindow("mbGB-SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, (screenwidth * scale), (screenheight * scale), SDL_WINDOW_SHOWN);
    
    if (window == NULL)
    {
        cout << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
        return false;
    }
    
    surface = SDL_GetWindowSurface(window);
    
    if (surface == NULL)
    {
        cout << "Renderer could not be created! SDL_Error: " << SDL_GetError() << endl;
        return false;
    }
    
    SDL_AudioSpec audiospec;
    audiospec.format = AUDIO_F32SYS;
    audiospec.freq = 44100;
    audiospec.samples = 4096;
    audiospec.channels = 2;
    audiospec.callback = NULL;
    
    SDL_AudioSpec obtainedspec;
    SDL_OpenAudio(&audiospec, &obtainedspec);
    SDL_PauseAudio(0);
    
    return true;
}

void drawpixels()
{
    SDL_Rect pixel = {0, 0, scale, scale};

    for (int i = 0; i < screenwidth; i++)
    {
        pixel.x = (i * scale);        
        for (int j = 0; j < screenheight; j++)
        {
            pixel.y = (j * scale);
            uint8_t red = core.coregpu.framebuffer[i + (j * screenwidth)].red;
            uint8_t green = core.coregpu.framebuffer[i + (j * screenwidth)].green;
            uint8_t blue = core.coregpu.framebuffer[i + (j * screenwidth)].blue;
            
            SDL_FillRect(surface, &pixel, SDL_MapRGBA(surface->format, red, green, blue, 255));
        }
    }
    
    SDL_UpdateWindowSurface(window);
}

void stopSDL()
{
    SDL_CloseAudio();
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void pause()
{
    core.paused = true;
    SDL_PauseAudio(1);
}

void resume()
{
    core.paused = false;
    drawpixels();
    SDL_PauseAudio(0);
}

void changestate(int id)
{
    stateid = id;
    cout << "Save slot changed to slot " << stateid << endl;
}

void handleinput(SDL_Event& event)
{
    if (event.type == SDL_KEYDOWN)
    {
        int key = -1;
        switch (event.key.keysym.sym)
        {
            case SDLK_UP: key = 0; break;
            case SDLK_DOWN: key = 1; break;
            case SDLK_LEFT: key = 2; break;
            case SDLK_RIGHT: key = 3; break;
            case SDLK_RETURN: key = 4; break;
            case SDLK_SPACE: key = 5; break;
            case SDLK_a: key = 6; break;
            case SDLK_b: key = 7; break;
	    case SDLK_d:
	    {
		if (!core.coregpu.dumpvram())
		{
		    exit(1);
		}
	    }
	    break;
            case SDLK_q: screenshot(); break;
            case SDLK_p: pause(); break;
            case SDLK_r: resume(); break;
            case SDLK_s: pause(); core.savestate(inttostring(stateid)); resume(); break;
            case SDLK_l: pause(); core.loadstate(inttostring(stateid)); resume(); break;
            case SDLK_0: changestate(0); break;
            case SDLK_1: changestate(1); break;
            case SDLK_2: changestate(2); break;
            case SDLK_3: changestate(3); break;
        }
        if (key != -1)
        {
            core.coreinput.keypressed(key);
        }
    }
    else if (event.type == SDL_KEYUP)
    {
        int key = -1;
        switch (event.key.keysym.sym)
        {
            case SDLK_UP: key = 0; break;
            case SDLK_DOWN: key = 1; break;
            case SDLK_LEFT: key = 2; break;
            case SDLK_RIGHT: key = 3; break;
            case SDLK_RETURN: key = 4; break;
            case SDLK_SPACE: key = 5; break;
            case SDLK_a: key = 6; break;
            case SDLK_b: key = 7; break;
        }
        if (key != -1)
        {
            core.coreinput.keyreleased(key);
        }
    }
}

int main(int argc, char* argv[])
{
    core.init();
    core.coreapu.setaudiocallback(bind(sdlcallback));   

    if (!core.getoptions(argc, argv))
    {
        return 1;
    }

    bool initialized = true;

    if (!core.loadROM(core.romname))
    {
	initialized = false;
    }

    if (core.coremmu.biosload == true)
    {	
	core.corecpu.resetBIOS();	
	if (!core.loadBIOS(core.biosname))
        {
            initialized = false;
        }
    }
    else
    {
        core.resetcpu();
    }

    if (!initSDL())
    {
	initialized = false;
    }
    
    if (!initialized)
    {
        cout << "Unable to start mbGB." << endl;
        return 1;
    }
    
    bool quit = false;
    SDL_Event ev;

    Uint32 framecurrenttime;
    Uint32 framestarttime;
    
    while (!quit)
    {   
        while (SDL_PollEvent(&ev))
        {
            handleinput(ev);
            
            if (ev.type == SDL_QUIT)
            {
                quit = true;
            }
        }
        
        if (core.paused)
        {
            SDL_PauseAudio(1);
        }
        else
        {
            SDL_PauseAudio(0);
        }
        
        if (!core.paused)
        {
            core.runcore();
            drawpixels();
        }

	framecurrenttime = SDL_GetTicks();
	if ((framecurrenttime - framestarttime) < 16)
	{
	    SDL_Delay(16 - (framecurrenttime - framestarttime));
	}

	framestarttime = SDL_GetTicks();

	fpscount++;
	if (((SDL_GetTicks() - fpstime) >= 1000))
	{
	    fpstime = SDL_GetTicks();
	    stringstream title;
	    title << "mbGB-SDL2-" << fpscount << " FPS";
	    SDL_SetWindowTitle(window, title.str().c_str());
	    fpscount = 0;
	}
    }

    stopSDL();

    return 0;
}
