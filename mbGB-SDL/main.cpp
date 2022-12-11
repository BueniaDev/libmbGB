#include <iostream>
#include <fstream>
#include <cassert>
#include <libmbGB/libmbgb.h>
#include <SDL2/SDL.h>
using namespace gb;
using namespace std;

class SDL2Frontend : public mbGBFrontend
{
    public:
	SDL2Frontend(GBCore &cb) : core(cb)
	{

	}

	~SDL2Frontend()
	{

	}

	bool init()
	{
	    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	    {
		return sdl_error("SDL2 could not be initialized!");
	    }

	    window = SDL_CreateWindow("mbGB-SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, (160 * scale), (144 * scale), SDL_WINDOW_SHOWN);

	    if (window == NULL)
	    {
		return sdl_error("Window could not be created!");
	    }

	    render = SDL_CreateRenderer(window, -1, 0);

	    if (render == NULL)
	    {
		return sdl_error("Renderer could not be created!");
	    }

	    texture = SDL_CreateTexture(render, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, 160, 144);

	    if (texture == NULL)
	    {
		return sdl_error("Texture could not be created!");
	    }

	    SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
	    return true;
	}

	void shutdown()
	{
	    if (texture != NULL)
	    {
		SDL_DestroyTexture(texture);
		texture = NULL;
	    }

	    if (render != NULL)
	    {
		SDL_DestroyRenderer(render);
		render = NULL;
	    }

	    if (window != NULL)
	    {
		SDL_DestroyWindow(window);
		window = NULL;
	    }

	    SDL_Quit();
	}

	void run()
	{
	    while (!quit)
	    {
		pollevents();
		runmachine();
		limitframerate();
	    }
	}

	void drawFrame()
	{
	    assert(render && texture);
	    SDL_UpdateTexture(texture, NULL, core.getFramebuffer().data(), core.getDepth());
	    SDL_RenderClear(render);
	    SDL_RenderCopy(render, texture, NULL, NULL);
	    SDL_RenderPresent(render);
	}

	vector<uint8_t> loadFile(string filename)
	{
	    cout << "Loading file of " << filename << "..." << endl;
	    vector<uint8_t> data;
	    fstream file(filename, ios::in | ios::binary | ios::ate);

	    if (file.is_open())
	    {
		streampos size = file.tellg();
		data.resize(size, 0);
		file.seekg(0, ios::beg);
		file.read((char*)data.data(), data.size());
		cout << filename << " succesfully loaded." << endl;
		file.close();
	    }

	    return data;
	}

    private:
	GBCore &core;

	bool quit = false;
	SDL_Event event;
	SDL_Window *window = NULL;
	SDL_Renderer *render = NULL;
	SDL_Texture *texture = NULL;

	const int scale = 2;

	bool sdl_error(string message)
	{
	    cout << message << " SDL_Error: " << SDL_GetError() << endl;
	    return false;
	}

	void pollevents()
	{
	    while (SDL_PollEvent(&event))
	    {
		switch (event.type)
		{
		    case SDL_QUIT: quit = true; break;
		    case SDL_KEYDOWN:
		    case SDL_KEYUP:
		    {
			bool is_key_pressed = (event.type == SDL_KEYDOWN);

			switch (event.key.keysym.sym)
			{
			    case SDLK_a: core.keyChanged(GBButton::ButtonA, is_key_pressed); break;
			    case SDLK_b: core.keyChanged(GBButton::ButtonB, is_key_pressed); break;
			    case SDLK_RETURN: core.keyChanged(GBButton::Start, is_key_pressed); break;
			    case SDLK_SPACE: core.keyChanged(GBButton::Select, is_key_pressed); break;
			    case SDLK_UP: core.keyChanged(GBButton::Up, is_key_pressed); break;
			    case SDLK_DOWN: core.keyChanged(GBButton::Down, is_key_pressed); break;
			    case SDLK_LEFT: core.keyChanged(GBButton::Left, is_key_pressed); break;
			    case SDLK_RIGHT: core.keyChanged(GBButton::Right, is_key_pressed); break;
			}
		    }
		    break;
		}
	    }
	}

	void runmachine()
	{
	    core.runCore();
	    drawFrame();
	}

	void limitframerate()
	{
	    framecurrenttime = SDL_GetTicks();

	    int64_t frame_period = (1000 / 60);

	    if ((framecurrenttime - framestarttime) < frame_period)
	    {
		SDL_Delay(frame_period - (framecurrenttime - framestarttime));
	    }

	    framestarttime = SDL_GetTicks();
	    fpscount += 1;

	    if (((SDL_GetTicks() - fpstime) >= 1000))
	    {
		fpstime = SDL_GetTicks();
		stringstream title;
		title << "mbGB-SDL-" << fpscount << " FPS";
		SDL_SetWindowTitle(window, title.str().c_str());
		fpscount = 0;
	    }
	}

	Uint32 framecurrenttime = 0;
	Uint32 framestarttime = 0;
	int fpscount = 0;
	Uint32 fpstime = 0;
};

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
	cout << "Usage: " << argv[0] << " ROM" << endl;
	return 1;
    }

    GBCore core;
    SDL2Frontend *front = new SDL2Frontend(core);
    core.setFrontend(front);

    if (!core.loadBIOS("dmg_bios.bin"))
    {
	return 1;
    }

    if (!core.loadROM(argv[1]))
    {
	return 1;
    }

    if (!core.initCore())
    {
	return 1;
    }

    front->run();
    core.shutdown();
    return 0;
}