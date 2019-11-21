#include <libmbGB/libmbgb.h>
#include <libmbGB/addons.h>
#include <SDL2/SDL.h>
#include <functional>
using namespace gb;
using namespace std;
using namespace std::placeholders;

#define PLAYER1  1
#define PLAYER2  2

GBCore core1;
GBCore core2;
LinkCable link;

SDL_Window *window = NULL;
SDL_Surface *surface = NULL;

int scale = 3;
bool player1focus = false;
bool player2focus = false;

bool initsdl()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		cout << "Error" << endl;
		return false;
	}
	
	window = SDL_CreateWindow("mbGB-link", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, ((core1.screenwidth * 2) * scale), (core1.screenheight * scale), SDL_WINDOW_SHOWN);
	
	if (window == NULL)
	{
		cout << "Error" << endl;
		return false;
	}
	
	surface = SDL_GetWindowSurface(window);
	
	return true;
}

void drawpixels()
{
	SDL_Rect pixel1 = {0, 0, scale, scale};
	SDL_Rect pixel2 = {0, 0, scale, scale};
	
	for (int i = 0; i < 160; i++)
	{
		pixel1.x = (i * scale);
		pixel2.x = ((i + 160) * scale);
		
		for (int j = 0; j < 144; j++)
		{
			pixel1.y = (j * scale);
			pixel2.y = (j * scale);
			
			uint8_t red1 = core1.getpixel(i, j).red;
			uint8_t green1 = core1.getpixel(i, j).green;
			uint8_t blue1 = core1.getpixel(i, j).blue;
			uint8_t red2 = core2.getpixel(i, j).red;
			uint8_t green2 = core2.getpixel(i, j).green;
			uint8_t blue2 = core2.getpixel(i, j).blue;
			
			
			SDL_FillRect(surface, &pixel1, SDL_MapRGB(surface->format, red1, green1, blue1));
			SDL_FillRect(surface, &pixel2, SDL_MapRGB(surface->format, red2, green2, blue2));
		}
	}
	
	SDL_UpdateWindowSurface(window);
}

void releasekeys(GBCore *core)
{
    core->keyreleased(Button::Up);
    core->keyreleased(Button::Down);
    core->keyreleased(Button::Left);
    core->keyreleased(Button::Right);
    core->keyreleased(Button::A);
    core->keyreleased(Button::B);
    core->keyreleased(Button::Select);
    core->keyreleased(Button::Select);
}

void setplayerfocus(int val)
{
    switch (val)
    {
	case 1: player1focus = true; player2focus = false; releasekeys(&core2); break;
	case 2: player1focus = false; player2focus = true; releasekeys(&core1); break;
	default: cout << "Unrecognized value" << endl; break;
    }
}

void setkeycore(Button button, bool pressed)
{
    if (player1focus)
    {
	(pressed) ? core1.keypressed(button) : core1.keyreleased(button);
    }
    else if (player2focus)
    {
	(pressed) ? core2.keypressed(button) : core2.keyreleased(button);
    }
}

void handleinput(SDL_Event event)
{
    if (event.type == SDL_KEYDOWN)
    {
	switch (event.key.keysym.sym)
	{
	    case SDLK_1: setplayerfocus(PLAYER1); break;
	    case SDLK_2: setplayerfocus(PLAYER2); break;
	    case SDLK_LEFT: setkeycore(Button::Left, true); break;
	    case SDLK_RIGHT: setkeycore(Button::Right, true); break;
	    case SDLK_UP: setkeycore(Button::Up, true); break;
	    case SDLK_DOWN: setkeycore(Button::Down, true); break;
	    case SDLK_a: setkeycore(Button::A, true); break;
	    case SDLK_b: setkeycore(Button::B, true); break;
	    case SDLK_RETURN: setkeycore(Button::Start, true); break;
	    case SDLK_SPACE: setkeycore(Button::Select, true); break;
	}
    }
    else if (event.type == SDL_KEYUP)
    {
	switch (event.key.keysym.sym)
	{
	    case SDLK_LEFT: setkeycore(Button::Left, false); break;
	    case SDLK_RIGHT: setkeycore(Button::Right, false); break;
	    case SDLK_UP: setkeycore(Button::Up, false); break;
	    case SDLK_DOWN: setkeycore(Button::Down, false); break;
	    case SDLK_a: setkeycore(Button::A, false); break;
	    case SDLK_b: setkeycore(Button::B, false); break;
	    case SDLK_RETURN: setkeycore(Button::Start, false); break;
	    case SDLK_SPACE: setkeycore(Button::Select, false); break;
	}
    }
}

void stopsdl()
{
	SDL_DestroyWindow(window);
	SDL_Quit();
}

int main(int argc, char* argv[])
{
	setplayerfocus(PLAYER1);

	if (!core1.getoptions(argc, argv))
	{
		return 1;
	}
	
	core2.romname = core1.romname;
	
	if (!core1.initcore())
	{
		return 1;
	}
	
	if (!core2.initcore())
	{
		return 1;
	}
	
	auto serial1 = bind(&Serial::recieve, &*core1.coreserial, _1);
	auto serial2 = bind(&Serial::recieve, &*core2.coreserial, _1);

	link.setlinkreccallbacks(serial1, serial2);
	
	core1.coreserial->setlinkcallback(bind(&LinkCable::link1ready, &link, _1, _2));
	core2.coreserial->setlinkcallback(bind(&LinkCable::link2ready, &link, _1, _2));
	
	if (!initsdl())
	{
		return 1;
	}
	
	bool quit = false;
	SDL_Event event;
	
	uint64_t cycles = 0;
	
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
		
		while (cycles < (70224 << core1.coremmu->doublespeed))
		{
			cycles += core1.runinstruction();
			core2.runinstruction();
		}
		
		cycles = 0;
		
		drawpixels();
	}
	
	core1.shutdown();
	core2.shutdown();
	stopsdl();
	
	return 0;
}
