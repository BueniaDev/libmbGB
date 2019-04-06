#include "../../include/libmbGB/libmbgb.h"
#include <SDL2/SDL.h>
#define SDL2_AUDIO
#ifdef NULL_AUDIO
	#include "../../include/audio-backends/null/nullbackend.h"
#endif // NULL_AUDIO
#ifdef SDL2_AUDIO
	#include "../../include/audio-backends/sdl2/sdl2backend.h"
#endif // SDL2_AUDIO
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <string>
using namespace gb;
using namespace std;

const int screenwidth = 160;
const int screenheight = 144;
const int scale = 3;


SDL_Window *window;
SDL_Renderer *render;

DMGCore core;

int stateid = 0;

bool initSDL()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
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
    
    render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    if (render == NULL)
    {
        cout << "Renderer could not be created! SDL_Error: " << SDL_GetError() << endl;
        return false;
    }
    
    #ifdef SDL2_AUDIO
	initsdl2audio();
    #endif // SDL2_AUDIO
    
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
            
            SDL_SetRenderDrawColor(render, red, green, blue, 0xFF);
            SDL_RenderFillRect(render, &pixel);
        }
    }
    
    SDL_RenderPresent(render);
}

void stopSDL()
{
    #ifdef SDL2_AUDIO
	deinitsdl2audio();
    #endif // SDL2_AUDIO
    SDL_DestroyRenderer(render);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void pause()
{
    core.paused = true;
    #ifdef SDL2_AUDIO
	SDL_PauseAudio(1);
    #endif // SDL2_AUDIO
}

void resume()
{
    core.paused = false;
    drawpixels();
    #ifdef SDL2_AUDIO
	SDL_PauseAudio(0);
    #endif
}

void changestate(int id)
{
    stateid = id;
    cout << "Save slot changed to slot " << stateid << endl;
}

string inttostring(int val)
{
    stringstream ss;
    ss << val;
    return ss.str();
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

    if (!initSDL())
    {
	initialized = false;
    }
    
    if (!initialized)
    {
        cout << "Unable to start mbGB." << endl;
        return 1;
    }

    const Uint32 fps = 60;
    const Uint32 minframetime = 1000 / fps;
    
    bool quit = false;
    SDL_Event ev;
    Uint32 starttime;
    Uint32 lasttime = SDL_GetTicks();
    
    while (!quit)
    {
        starttime = SDL_GetTicks();
        
        while (SDL_PollEvent(&ev))
        {
            handleinput(ev);
            
            if (ev.type == SDL_QUIT)
            {
                quit = true;
            }
        }
        
        #ifdef SDL2_AUDIO
        if (core.paused)
        {
            SDL_PauseAudio(1);
        }
        else
        {
            SDL_PauseAudio(0);
        }
        #endif // SDL2_AUDIO
        
        if (!core.paused)
        {
            while (!core.coregpu.newvblank)
            {
                core.runcore();
            }
            core.coregpu.newvblank = false;
            drawpixels();
        }
        
        if (SDL_GetTicks() - starttime < minframetime)
        {
            SDL_Delay(minframetime - (SDL_GetTicks() - starttime));
        }
        
        lasttime = starttime;
    }

    stopSDL();

    return 0;
}
