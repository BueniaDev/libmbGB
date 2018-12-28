#include "../include/libmbGB/libmbgb.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <string>
using namespace gb;
using namespace std;

SDL_Window *window;
SDL_Renderer *render;

DMGCore core;

int total = 0;
int timer = 0;
int current = 0;
int counter = 0;
bool first = true;


void vramdump(string filename, uint16_t start, uint16_t end)
{
    FILE *fh = fopen(filename.c_str(), "wb");
    fwrite(core.coremmu.memorymap + start, sizeof(uint8_t), end - start + 1, fh);
    fclose(fh);
}

bool initSDL()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        cout << "SDL could not be initalized! SDL_Error: " << SDL_GetError() << endl;
        return false;
    }
    
    window = SDL_CreateWindow("mbGB", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 320, 288, SDL_WINDOW_SHOWN);
    
    if (window == NULL)
    {
        cout << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
        return false;
    }
    
    render = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    
    if (render == NULL)
    {
        cout << "Renderer could not be created! SDL_Error: " << SDL_GetError() << endl;
        return false;
    }
    
    return true;
}

void drawpixels()
{
    SDL_Rect pixel = {0, 0, 2, 2};

    for (int i = 0; i < 160; i++)
    {
	pixel.x = (i * 2);        
	for (int j = 0; j < 144; j++)
        {
            pixel.y = (j * 2);
	    uint8_t red = core.coregpu.framebuffer[i + (j * 160)][0];
            uint8_t green = core.coregpu.framebuffer[i + (j * 160)][1];
            uint8_t blue = core.coregpu.framebuffer[i + (j * 160)][2];
            
	    SDL_SetRenderDrawColor(render, red, green, blue, 0xFF);
            SDL_RenderFillRect(render, &pixel);
        }
    }
    
    SDL_RenderPresent(render);
}

void stopSDL()
{
    SDL_DestroyRenderer(render);
    SDL_DestroyWindow(window);
    SDL_Quit();
}


void checkfps()
{
    if (first)
    {
        first = false;
        timer = SDL_GetTicks();
    }
    
    counter++;
    current = SDL_GetTicks();
    
    if ((timer + 1000) < current)
    {
        timer = current;
        total = counter;
        counter = 0;
    }
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
    
    float fps = 60;
    float interval = 1000 / fps;
    
    unsigned int time2 = SDL_GetTicks();
    
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

        unsigned int current = SDL_GetTicks();
        
        if ((time2 + interval) < current)
        {
            checkfps();
            core.runcore();
            drawpixels();
            time2 = current;
        }
    }

    stopSDL();
    
    return 0;
}
