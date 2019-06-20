#include <libmbGB/cpu.h>
#include <libmbGB/mmu.h>
#include <SDL2/SDL.h>
#include <iostream>
using namespace gb;
using namespace std;

MMU coremmu;
CPU corecpu(coremmu);

SDL_Window *window;
SDL_Surface *surface;

int screenwidth = 160;
int screenheight = 144;
int scale = 3;

bool init()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
	cout << "SDL could not be initialized! SDL_Error: " << SDL_GetError() << endl;
	return false;
    }

    window = SDL_CreateWindow("mbGB-SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, (screenwidth * scale), (screenheight * scale), SDL_WINDOW_SHOWN);

    if (window == NULL)
    {
	cout << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
	return false;
    }

    surface = SDL_GetWindowSurface(window);

    return true;
}

void stop()
{
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int runcore(int spentcycles)
{
    return corecpu.runfor(70224 + spentcycles);
}

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

    if (!init())
    {
	return 1;
    }

    SDL_Event event;
    bool quit = false;

    int overspentcycles = 0;

    while (!quit)
    {
	while (SDL_PollEvent(&event))
	{
	    if (event.type == SDL_QUIT)
	    {
		quit = true;
	    }
	}

	overspentcycles = runcore(overspentcycles);
    }

    stop();

    corecpu.shutdown();
    coremmu.shutdown();
    return 0;
}