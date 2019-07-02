#include <libmbGB/cpu.h>
#include <libmbGB/mmu.h>
#include <libmbGB/gpu.h>
#include <libmbGB/input.h>
#include <SDL2/SDL.h>
#include <iostream>
using namespace gb;
using namespace std;

MMU coremmu;
GPU coregpu(coremmu);
CPU corecpu(coremmu, coregpu);
Input coreinput(coremmu);

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

void handleinput(SDL_Event event)
{
    if (event.type == SDL_KEYDOWN)
    {
	switch (event.key.keysym.sym)
	{
	    case SDLK_UP: coreinput.keypressed(Button::Up); break;
	    case SDLK_DOWN: coreinput.keypressed(Button::Down); break;
	    case SDLK_LEFT: coreinput.keypressed(Button::Left); break;
	    case SDLK_RIGHT: coreinput.keypressed(Button::Right); break;
	    case SDLK_a: coreinput.keypressed(Button::A); break;
	    case SDLK_b: coreinput.keypressed(Button::B); break;
	    case SDLK_RETURN: coreinput.keypressed(Button::Start); break;
	    case SDLK_SPACE: coreinput.keypressed(Button::Select); break;
	}
    }
    else if (event.type == SDL_KEYUP)
    {
	switch (event.key.keysym.sym)
	{
	    case SDLK_UP: coreinput.keyreleased(Button::Up); break;
	    case SDLK_DOWN: coreinput.keyreleased(Button::Down); break;
	    case SDLK_LEFT: coreinput.keyreleased(Button::Left); break;
	    case SDLK_RIGHT: coreinput.keyreleased(Button::Right); break;
	    case SDLK_a: coreinput.keyreleased(Button::A); break;
	    case SDLK_b: coreinput.keyreleased(Button::B); break;
	    case SDLK_RETURN: coreinput.keyreleased(Button::Start); break;
	    case SDLK_SPACE: coreinput.keyreleased(Button::Select); break;
	}
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
	cout << "Usage: " << argv[0] << " ROM" << endl;
	return 1;
    }


    /*
    if (argc < 3)
    {
	cout << "Usage: " << argv[0] << " ROM [BIOS]" << endl;
	return 1;
    }
    */

    coremmu.init();
    coremmu.biosload = false;
    /*
    coremmu.biosload = true;

    if(!coremmu.loadBIOS(argv[2]))
    {
	return 1;
    }
    */

    if(!coremmu.loadROM(argv[1]))
    {
	return 1;
    }

    corecpu.init();
    coregpu.init();
    coreinput.init();

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
    	    handleinput(event);
	    if (event.type == SDL_QUIT)
	    {
		quit = true;
	    }
	}

	overspentcycles = runcore(overspentcycles);
	drawpixels();
    }

    coreinput.shutdown();
    coregpu.shutdown();
    corecpu.shutdown();
    coremmu.shutdown();
    stop();
    exit(0);
}
