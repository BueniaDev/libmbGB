#include <libmbGB/cpu.h>
#include <libmbGB/mmu.h>
#include <libmbGB/gpu.h>
#include <libmbGB/input.h>
#include <libmbGB/timers.h>
#include <SDL2/SDL.h>
#include <iostream>
#include <sstream>
using namespace gb;
using namespace std;

MMU coremmu;
GPU coregpu(coremmu);
Timers coretimers(coremmu);
CPU corecpu(coremmu, coregpu, coretimers);
Input coreinput(coremmu);

SDL_Window *window;
SDL_Surface *surface;

int screenwidth = 160;
int screenheight = 144;
int scale = 3;

int fpscount = 0;
Uint32 fpstime = 0;

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
	exit(1);
    }

    coremmu.init();
    coremmu.biosload = false;
    
    if(!coremmu.loadROM(argv[1]))
    {
	return 1;
    }

    corecpu.init();
    coregpu.init();
    coretimers.init();
    coreinput.init();

    if (!init())
    {
	exit(1);
    }

    SDL_Event event;
    bool quit = false;

    int overspentcycles = 0;

    Uint32 framecurrenttime;
    Uint32 framestarttime;

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

	framecurrenttime = SDL_GetTicks();

	if ((framecurrenttime - framestarttime) < (1000 / 60))
	{
	    SDL_Delay((1000 / 60) - (framecurrenttime - framestarttime));
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

    coreinput.shutdown();
    coretimers.shutdown();
    coregpu.shutdown();
    corecpu.shutdown();
    coremmu.shutdown();
    stop();
    exit(0);
}
