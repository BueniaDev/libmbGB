#include <libmbGB/cpu.h>
#include <libmbGB/mmu.h>
#include <libmbGB/gpu.h>
#include <SDL2/SDL.h>
#include <iostream>
using namespace gb;
using namespace std;

MMU coremmu;
GPU coregpu(coremmu);
CPU corecpu(coremmu, coregpu);

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

void drawpixels()
{
    SDL_Rect pixel = {0, 0, scale, scale};

    for (int i = 0; i < screenwidth; i++)
    {
	pixel.x = (i * scale);
	for (int j = 0; j < screenheight; j++)
	{
	    pixel.y = (j * scale);
	    uint8_t red = coregpu.framebuffer[i + (j * screenwidth)].red;
	    uint8_t green = coregpu.framebuffer[i + (j * screenwidth)].green;
	    uint8_t blue = coregpu.framebuffer[i + (j * screenwidth)].blue;

	    SDL_FillRect(surface, &pixel, SDL_MapRGBA(surface->format, red, green, blue, 255));
	}
    }

    SDL_UpdateWindowSurface(window);
}

int runcore(int spentcycles)
{
    return corecpu.runfor(70224 + spentcycles);
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
	cout << "Usage: " << argv[0] << " ROM [BIOS]" << endl;
	return 1;
    }

    coremmu.init();
    coremmu.biosload = false;

    coremmu.loadROM(argv[1]);
    corecpu.init();

    if (!init())
    {
	exit(1);
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
	drawpixels();
    }

    corecpu.shutdown();
    coremmu.shutdown();
    stop();
    exit(0);
}