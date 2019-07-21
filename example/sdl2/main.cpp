#include <libmbGB/libmbgb.h>
#include <SDL2/SDL.h>
#include <iostream>
#include <sstream>
using namespace gb;
using namespace std;


GBCore core;

SDL_Window *window;
SDL_Surface *surface;

int screenwidth = 160;
int screenheight = 144;
int scale = 3;

int fpscount = 0;
Uint32 fpstime = 0;

bool init()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
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
	    uint8_t red = core.getpixel(i, j).red;
	    uint8_t green = core.getpixel(i, j).green;
	    uint8_t blue = core.getpixel(i, j).blue;

	    SDL_FillRect(surface, &pixel, SDL_MapRGBA(surface->format, red, green, blue, 255));
	}
    }

    SDL_UpdateWindowSurface(window);
}

void handleinput(SDL_Event event)
{
    if (event.type == SDL_KEYDOWN)
    {
	switch (event.key.keysym.sym)
	{
	    case SDLK_UP: core.keypressed(Button::Up); break;
	    case SDLK_DOWN: core.keypressed(Button::Down); break;
	    case SDLK_LEFT: core.keypressed(Button::Left); break;
	    case SDLK_RIGHT: core.keypressed(Button::Right); break;
	    case SDLK_a: core.keypressed(Button::A); break;
	    case SDLK_b: core.keypressed(Button::B); break;
	    case SDLK_RETURN: core.keypressed(Button::Start); break;
	    case SDLK_SPACE: core.keypressed(Button::Select); break;
	    // case SDLK_s: core.savestate("cpu.mbsave"); break;
	    // case SDLK_d: core.dumpvram("vram.bin"); break;
	}
    }
    else if (event.type == SDL_KEYUP)
    {
	switch (event.key.keysym.sym)
	{
	    case SDLK_UP: core.keyreleased(Button::Up); break;
	    case SDLK_DOWN: core.keyreleased(Button::Down); break;
	    case SDLK_LEFT: core.keyreleased(Button::Left); break;
	    case SDLK_RIGHT: core.keyreleased(Button::Right); break;
	    case SDLK_a: core.keyreleased(Button::A); break;
	    case SDLK_b: core.keyreleased(Button::B); break;
	    case SDLK_RETURN: core.keyreleased(Button::Start); break;
	    case SDLK_SPACE: core.keyreleased(Button::Select); break;
	}
    }
}

int main(int argc, char* argv[])
{
    if (!core.getoptions(argc, argv))
    {
	exit(1);
    }

    core.preinit();

    if (core.biosload())
    {
	if (!core.loadBIOS(core.biosname))
	{
	    exit(1);
	}
    }
    
    if(!core.loadROM(core.romname))
    {
	exit(1);
    }

    core.init();

    if (!init())
    {
	exit(1);
    }

    SDL_Event event;
    bool quit = false;

    Uint32 framecurrenttime = 0;
    Uint32 framestarttime = 0;

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

	core.runcore();
	drawpixels();

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
	    fpscount = 0;
	}
    }

    core.shutdown();
    stop();
    exit(0);
}
