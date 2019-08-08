#include <libmbGB/libmbgb.h>
#include <SDL2/SDL.h>
#include <iostream>
#include <sstream>
#include <functional>
using namespace gb;
using namespace std;
using namespace std::placeholders;

GBCore core;

SDL_Window *window = nullptr;
SDL_Surface *surface = nullptr;

int screenwidth = 160;
int screenheight = 144;
int scale = 3;

int fpscount = 0;
Uint32 fpstime = 0;

vector<int16_t> buffer;

void sdlcallback(int16_t left, int16_t right)
{
    buffer.push_back(left);
    buffer.push_back(right);

    if (buffer.size() >= 4096)
    {
	buffer.clear();

	while ((SDL_GetQueuedAudioSize(1)) > (4096 * sizeof(int16_t)))
	{
	    SDL_Delay(1);
	}
	SDL_QueueAudio(1, &buffer[0], (4096 * sizeof(int16_t)));
    }
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

void play()
{

}

bool initsdl()
{
    #ifdef __WIN32
    putenv("SDL_AUDIODRIVER=DirectSound");
    #endif

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

    SDL_AudioSpec audiospec;
    audiospec.format = AUDIO_S16SYS;
    audiospec.freq = 48000;
    audiospec.samples = 4096;
    audiospec.channels = 2;
    audiospec.callback = NULL;

    SDL_AudioSpec obtainedspec;
    SDL_OpenAudio(&audiospec, &obtainedspec);
    SDL_PauseAudio(0);

    return true;
}

void stop()
{
    SDL_CloseAudio();
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
	    case SDLK_l: core.loadstate(); break;
	    case SDLK_s: core.savestate(); break;
	    case SDLK_p: core.paused = !core.paused; break;
	    case SDLK_r: core.resetcore(); break;
	    case SDLK_q: screenshot(); break;
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
    core.setsamplerate(48000);
    core.setaudiocallback(bind(&sdlcallback, _1, _2));

    if (!core.getoptions(argc, argv))
    {
	return 1;
    }
 
    if (!core.initcore())
    {
	return 1;
    }

    if (!initsdl())
    {
	return 1;
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

	SDL_PauseAudio(core.paused);

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

    core.shutdown();
    stop();
    return 0;
}
