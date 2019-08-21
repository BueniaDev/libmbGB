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
SDL_GameController* player1 = nullptr;

int playerinstanceid = 0;

int screenwidth = 160;
int screenheight = 144;
int scale = 3;

int xdir = 0;
int ydir = 0;

const int controllerdeadzone = 8000;

int fpscount = 0;
Uint32 fpstime = 0;

#ifdef LIBMBGB_SIGNED16
using sampleformat = int16_t;
#elif defined LIBMBGB_FLOAT32
using sampleformat = float;
#endif

struct Controller
{
    SDL_GameController *controller;
    SDL_JoystickID prevcontrollerid;
    SDL_JoystickID controllerid;
    bool isconnected = false;

    bool open(int id)
    {
	if (SDL_IsGameController(id))
	{
	    controller = SDL_GameControllerOpen(id);
	    SDL_Joystick *j = SDL_GameControllerGetJoystick(controller);
	    controllerid = SDL_JoystickInstanceID(j);
	    if (controllerid != prevcontrollerid)
	    {
		controllerid = prevcontrollerid;
	    }
	    prevcontrollerid = controllerid;
	    isconnected = true;
	}

	return true;
    }

    void close()
    {
	if (isconnected)
	{
	    isconnected = false;
	    SDL_GameControllerClose(controller);
	    controller = nullptr;
	}
    }
};

struct JoypadControllers
{
    static const int maxcontrollers = 4;

    Controller m_controllers[maxcontrollers];

    int getcontrollerindex(SDL_JoystickID instance)
    {
	for (int i = 0; i < maxcontrollers; i++)
	{
	    if (m_controllers[i].isconnected && m_controllers[i].controllerid == instance)
	    {
		return i;
	    }
	}
	return -1;
    }
};

JoypadControllers control;


void releaseallkeys()
{
    core.keyreleased(Button::A);
    core.keyreleased(Button::B);
    core.keyreleased(Button::Start);
    core.keyreleased(Button::Select);
    core.keyreleased(Button::Up);
    core.keyreleased(Button::Down);
    core.keyreleased(Button::Left);
    core.keyreleased(Button::Right);
}

vector<sampleformat> buffer;

void sdlcallback(sampleformat left, sampleformat right)
{
    buffer.push_back(left);
    buffer.push_back(right);

    if (buffer.size() >= 4096)
    {
	buffer.clear();

	while ((SDL_GetQueuedAudioSize(1)) > (4096 * sizeof(sampleformat)))
	{
	    SDL_Delay(1);
	}
	SDL_QueueAudio(1, &buffer[0], (4096 * sizeof(sampleformat)));
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
    #ifdef LIBMBGB_SIGNED16
    audiospec.format = AUDIO_S16SYS;
    #elif defined LIBMBGB_FLOAT32
    audiospec.format = AUDIO_F32SYS;
    #endif
    audiospec.freq = 48000;
    audiospec.samples = 4096;
    audiospec.channels = 2;
    audiospec.callback = NULL;

    SDL_AudioSpec obtainedspec;
    SDL_OpenAudio(&audiospec, &obtainedspec);
    SDL_PauseAudio(0);

    if (SDL_NumJoysticks() > 0)
    {
	player1 = SDL_GameControllerOpen(0);
    }

    return true;
}

void stop()
{
    if (SDL_NumJoysticks() > 0)
    {
	SDL_GameControllerClose(player1);
    }

    SDL_CloseAudio();
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void addgamecontroller(int id)
{

}

void removegamecontroller(int id)
{

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

void handleaxis(int direction, Button dir1, Button dir2)
{
    switch (direction)
    {
	case -1: core.keypressed(dir1); core.keyreleased(dir2); break;
	case 0: core.keyreleased(dir1); core.keyreleased(dir2); break;
	case 1: core.keyreleased(dir1); core.keypressed(dir2); break;
    }
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
    else if (event.type == SDL_CONTROLLERBUTTONDOWN)
    {
	switch (event.cbutton.button)
	{
	    case SDL_CONTROLLER_BUTTON_START: core.keypressed(Button::Start); break;
	    case SDL_CONTROLLER_BUTTON_BACK: core.keypressed(Button::Select); break;
	    case SDL_CONTROLLER_BUTTON_A: core.keypressed(Button::A); break;
	    case SDL_CONTROLLER_BUTTON_B: core.keypressed(Button::B); break;
	    case SDL_CONTROLLER_BUTTON_DPAD_UP: core.keypressed(Button::Up); break;
	    case SDL_CONTROLLER_BUTTON_DPAD_DOWN: core.keypressed(Button::Down); break;
	    case SDL_CONTROLLER_BUTTON_DPAD_LEFT: core.keypressed(Button::Left); break;
	    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: core.keypressed(Button::Right); break;
	}
    }
    else if (event.type == SDL_CONTROLLERBUTTONUP)
    {
	switch (event.cbutton.button)
	{
	    case SDL_CONTROLLER_BUTTON_START: core.keyreleased(Button::Start); break;
	    case SDL_CONTROLLER_BUTTON_BACK: core.keyreleased(Button::Select); break;
	    case SDL_CONTROLLER_BUTTON_A: core.keyreleased(Button::A); break;
	    case SDL_CONTROLLER_BUTTON_B: core.keyreleased(Button::B); break;
	    case SDL_CONTROLLER_BUTTON_DPAD_UP: core.keyreleased(Button::Up); break;
	    case SDL_CONTROLLER_BUTTON_DPAD_DOWN: core.keyreleased(Button::Down); break;
	    case SDL_CONTROLLER_BUTTON_DPAD_LEFT: core.keyreleased(Button::Left); break;
	    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: core.keyreleased(Button::Right); break;
	}
    }
    else if (event.type == SDL_CONTROLLERAXISMOTION)
    {
	switch (event.caxis.axis)
	{
	    case SDL_CONTROLLER_AXIS_LEFTY:
	    {
		if (event.caxis.value < -controllerdeadzone)
		{
		    ydir = -1;
		}
		else if (event.caxis.value > controllerdeadzone)
		{
		    ydir = 1;
		}
		else if (event.caxis.value == 0)
		{
		    ydir = 0;
		}
		else
		{
		    ydir = 0;
		}

		handleaxis(ydir, Button::Up, Button::Down);
	    }
	    break;
	    case SDL_CONTROLLER_AXIS_LEFTX:
	    {
		if (event.caxis.value < -controllerdeadzone)
		{
		    xdir = -1;
		}
		else if (event.caxis.value > controllerdeadzone)
		{
		    xdir = 1;
		}
		else if (event.caxis.value == 0)
		{
		    xdir = 0;
		}
		else
		{
		    xdir = 0;
		}

		handleaxis(xdir, Button::Left, Button::Right);
	    }
	    break;
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
	    if (event.type == SDL_QUIT)
	    {
		quit = true;
	    }
	    else if (event.type == SDL_CONTROLLERDEVICEADDED)
	    {
		control.m_controllers[event.cdevice.which].open(event.cdevice.which);
		playerinstanceid = control.getcontrollerindex(event.cdevice.which);
	    }
	    else if (event.type == SDL_CONTROLLERDEVICEREMOVED)
	    {
		if (playerinstanceid > -1)
		{
		    control.m_controllers[playerinstanceid].close();
		    releaseallkeys();
		}
	    }
	    else
	    {
		handleinput(event);
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
