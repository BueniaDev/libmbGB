#include <libmbGB/libmbgb.h>
#include <SDL2/SDL.h>
#include <iostream>
#include <sstream>
#include <functional>
#include <ctime>
using namespace gb;
using namespace std;
using namespace std::placeholders;

#define GYRO_LEFT 1
#define GYRO_RIGHT 2
#define GYRO_UP 4
#define GYRO_DOWN 8

GBCore core;

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

class SDL2Frontend : public mbGBFrontend
{
    public:
        SDL2Frontend(GBCore *cb)
        {
            core = cb;
        }
        
        ~SDL2Frontend()
        {
        
        }
        
        bool init()
        {
	    #ifdef __WIN32
   	    putenv("SDL_AUDIODRIVER=DirectSound");
	    #endif

	    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	    {
		cout << "SDL could not be initialized! SDL_Error: " << SDL_GetError() << endl;
		return false;
	    }

	    window = SDL_CreateWindow("mbGB-SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, (core->screenwidth * scale), (core->screenheight * scale), SDL_WINDOW_SHOWN);

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
	
	    if (SDL_NumJoysticks() > 0)
	    {
		player1 = SDL_GameControllerOpen(0);
	
		haptic = SDL_HapticOpen(0);
	
		if (haptic == NULL)
		{
		    return false;
		}
	
		if (SDL_HapticRumbleSupported(haptic))
		{
		    cout << "Rumble supported" << endl;
		}
		else
		{
		    cout << "Rumble not supported" << endl;
		}
	
		if (SDL_HapticRumbleInit(haptic) != 0)
		{
		    return false;
		}
	
		isrumbleenabled = true;
	    }
	    else
	    {
		isrumbleenabled = false;
	    }
	
	    return true;
	}
	
	void shutdown()
	{
	    if (SDL_NumJoysticks() > 0)
    	    {
		SDL_HapticClose(haptic);
		SDL_GameControllerClose(player1);
	    }

	    SDL_CloseAudio();
	    SDL_DestroyWindow(window);
	    SDL_Quit();
	}
	
	void runapp()
	{
	    while (!quit)
	    {
	    	pollevents(quit);
	    	runmachine();
	    	limitframerate();
	    }
	}
	
	void updatesensor(int bit, bool ispressed)
	{
	    if (ispressed)
	    {
		gyroscopeval |= bit;
 	    }
	    else
	    {
		gyroscopeval &= ~bit;
	    }
	}
	
	void runmachine()
	{
	    SDL_PauseAudio(core->paused);

	    if (!core->paused)
	    {
	    	core->runcore();
	    	drawpixels();
	    }
	}
	
	void screenshot()
	{
	    time_t currenttime = time(nullptr);
	    string screenstring = "mbGB_";
	    screenstring.append(std::to_string(currenttime));
	    screenstring.append(".bmp");

	    SDL_SaveBMP(surface, screenstring.c_str());

	    cout << "Screenshot saved." << endl;
	}
	
	void releaseallkeys()
	{
	    core->keyreleased(Button::A);
	    core->keyreleased(Button::B);
	    core->keyreleased(Button::Start);
	    core->keyreleased(Button::Select);
	    core->keyreleased(Button::Up);
	    core->keyreleased(Button::Down);
	    core->keyreleased(Button::Left);
	    core->keyreleased(Button::Right);
	}
	
	void limitframerate()
	{
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
	
	void pollevents(bool &quit)
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
	}

        bool isctrlshiftpressed(SDL_Event event)
        {
            return (event.key.keysym.mod & (KMOD_CTRL | KMOD_SHIFT)) ? true : false;
        }
	
	void handleinput(SDL_Event event)
	{
	    if (event.type == SDL_KEYDOWN)
	    {
		switch (event.key.keysym.sym)
		{
		    case SDLK_UP: core->keypressed(Button::Up); break;
		    case SDLK_DOWN: core->keypressed(Button::Down); break;
		    case SDLK_LEFT: core->keypressed(Button::Left); break;
		    case SDLK_RIGHT: core->keypressed(Button::Right); break;
		    case SDLK_a: core->keypressed(Button::A); break;
		    case SDLK_b: core->keypressed(Button::B); break;
		    case SDLK_RETURN: core->keypressed(Button::Start); break;
		    case SDLK_SPACE: core->keypressed(Button::Select); break;
		    case SDLK_F1: core->loadstate(); break;
		    case SDLK_F2: core->savestate(); break;
		    case SDLK_p: core->paused = !core->paused; break;
		    case SDLK_r: core->resetcore(); break;
		    case SDLK_q: screenshot(); break;
		    case SDLK_i: updatesensor(GYRO_UP, true); break;
		    case SDLK_j: updatesensor(GYRO_LEFT, true); break;
		    case SDLK_k: updatesensor(GYRO_DOWN, true); break;
		    case SDLK_l: 
                    {
			if (isctrlshiftpressed(event))
                        {
                            core->loadstate();
                        }
                        else
                        {
                            updatesensor(GYRO_RIGHT, true);
                        }
                    }
                    break;
		    case SDLK_s: 
                    {
			if (isctrlshiftpressed(event))
                        {
                            core->savestate();
                        }
                        else
                        {
                            core->swipebarcode();
                        }
                    }
                    break;
		}
    	    }
	    else if (event.type == SDL_KEYUP)
	    {
	    	switch (event.key.keysym.sym)
	    	{
		    case SDLK_UP: core->keyreleased(Button::Up); break;
		    case SDLK_DOWN: core->keyreleased(Button::Down); break;
		    case SDLK_LEFT: core->keyreleased(Button::Left); break;
		    case SDLK_RIGHT: core->keyreleased(Button::Right); break;
		    case SDLK_a: core->keyreleased(Button::A); break;
		    case SDLK_b: core->keyreleased(Button::B); break;
		    case SDLK_RETURN: core->keyreleased(Button::Start); break;
		    case SDLK_SPACE: core->keyreleased(Button::Select); break;
		    case SDLK_i: updatesensor(GYRO_UP, false); break;
		    case SDLK_j: updatesensor(GYRO_LEFT, false); break;
		    case SDLK_k: updatesensor(GYRO_DOWN, false); break;
		    case SDLK_l: updatesensor(GYRO_RIGHT, false); break;
		}
    	    }	
	    else if (event.type == SDL_CONTROLLERBUTTONDOWN)
    	    {
		switch (event.cbutton.button)
		{
		    case SDL_CONTROLLER_BUTTON_START: core->keypressed(Button::Start); break;
		    case SDL_CONTROLLER_BUTTON_BACK: core->keypressed(Button::Select); break;
		    case SDL_CONTROLLER_BUTTON_A: core->keypressed(Button::A); break;
		    case SDL_CONTROLLER_BUTTON_B: core->keypressed(Button::B); break;
		    case SDL_CONTROLLER_BUTTON_DPAD_UP: core->keypressed(Button::Up); break;
		    case SDL_CONTROLLER_BUTTON_DPAD_DOWN: core->keypressed(Button::Down); break;
		    case SDL_CONTROLLER_BUTTON_DPAD_LEFT: core->keypressed(Button::Left); break;
		    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: core->keypressed(Button::Right); break;
		    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: 
		    {
			if (!core->savestate())
			{
			    exit(1);
			}
		    }
		    break;
		    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: 
		    {
			if (!core->loadstate())
			{
			    exit(1);
			}
		    }
		    break;
		    case SDL_CONTROLLER_BUTTON_X: core->paused = !core->paused; break;
		}
    	    }
    	    else if (event.type == SDL_CONTROLLERBUTTONUP)
    	    {
		switch (event.cbutton.button)
		{
		    case SDL_CONTROLLER_BUTTON_START: core->keyreleased(Button::Start); break;
		    case SDL_CONTROLLER_BUTTON_BACK: core->keyreleased(Button::Select); break;
		    case SDL_CONTROLLER_BUTTON_A: core->keyreleased(Button::A); break;
		    case SDL_CONTROLLER_BUTTON_B: core->keyreleased(Button::B); break;
		    case SDL_CONTROLLER_BUTTON_DPAD_UP: core->keyreleased(Button::Up); break;
		    case SDL_CONTROLLER_BUTTON_DPAD_DOWN: core->keyreleased(Button::Down); break;
		    case SDL_CONTROLLER_BUTTON_DPAD_LEFT: core->keyreleased(Button::Left); break;
		    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: core->keyreleased(Button::Right); break;
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
		    case SDL_CONTROLLER_AXIS_RIGHTY:
		    {
			if (event.caxis.value < -controllerdeadzone)
			{
			    ysensordir = -1;
			}
			else if (event.caxis.value > controllerdeadzone)
			{
			    ysensordir = 1;
			}
			else if (event.caxis.value == 0)
			{
			    ysensordir = 0;
			}
			else
			{
			    ysensordir = 0;
			}
	
			handleaxisgyro(ysensordir, true);
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
		    case SDL_CONTROLLER_AXIS_RIGHTX:
		    {
			if (event.caxis.value < -controllerdeadzone)
			{
			    xsensordir = -1;
			}
			else if (event.caxis.value > controllerdeadzone)
			{
			    xsensordir = 1;
			}
			else if (event.caxis.value == 0)
			{
			    xsensordir = 0;
			}
			else
			{
			    xsensordir = 0;
			}
	
			handleaxisgyro(xsensordir, false);
		    }
		    break;
		}
	    }	
	}
	
	void drawpixels()
	{
	    SDL_Rect pixel = {0, 0, scale, scale};

	    if (!core->isagbmode())
	    {
		for (int i = 0; i < core->screenwidth; i++)
    		{
		    pixel.x = (i * scale);
		    for (int j = 0; j < core->screenheight; j++)
		    {
	    		pixel.y = (j * scale);
	    		uint8_t red = core->getpixel(i, j).red;
	    		uint8_t green = core->getpixel(i, j).green;
	    		uint8_t blue = core->getpixel(i, j).blue;

	    		SDL_FillRect(surface, &pixel, SDL_MapRGBA(surface->format, red, green, blue, 255));
		    }
		}
	    }
	    else
    	    {
		for (int i = 40; i < 200; i++)
		{
		    pixel.x = (i * scale);
		    for (int j = 8; j < 152; j++)
		    {
	    		pixel.y = (j * scale);
	    		int xpos = (i - 40);
	    		int ypos = (j - 8);
	    		uint8_t red = core->getpixel(xpos, ypos).red;
	    		uint8_t green = core->getpixel(xpos, ypos).green;
	    		uint8_t blue = core->getpixel(xpos, ypos).blue;

	    		SDL_FillRect(surface, &pixel, SDL_MapRGBA(surface->format, red, green, blue, 255));
		    }
    		}
    
    	    }

	    SDL_UpdateWindowSurface(window);
	}
	
	void audiocallback(audiotype left, audiotype right)
	{
	    if (!holds_alternative<int16_t>(left) || !holds_alternative<int16_t>(right))
    	    {
	        return;
	    }

	    buffer.push_back(get<int16_t>(left));
	    buffer.push_back(get<int16_t>(right));

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
	
	void rumblecallback(bool isenabled)
	{
	    if (!isrumbleenabled)
	    {
		return;
	    }

	    if (isenabled && !isrumbling)
	    {
		if (SDL_HapticRumblePlay(haptic, 0.5, 2000) != 0)
		{
		    cout << "Error playing rumble! SDL_Error: " << SDL_GetError() << endl;
		    exit(1);
		}
		
		isrumbling = true;
	    }
	    else if (!isenabled && isrumbling)
    	    {
		if (SDL_HapticRumbleStop(haptic) != 0)
		{
		    cout << "Error stopping rumble! SDL_Error: " << SDL_GetError() << endl;
	    	    exit(1);
		}

		isrumbling = false;
    	    }
	}
	
	void sensorcallback(uint16_t& sensorx, uint16_t& sensory)
	{
    	    updategyro();
    	    sensorx = (0x81D0 + xsensor);
    	    sensory = (0x81D0 + ysensor);
	}
	
	void handleaxis(int direction, Button dir1, Button dir2)
	{
	    switch (direction)
	    {
		case -1: core->keypressed(dir1); core->keyreleased(dir2); break;
		case 0: core->keyreleased(dir1); core->keyreleased(dir2); break;
		case 1: core->keyreleased(dir1); core->keypressed(dir2); break;
    	    }
	}

	void handleaxisgyro(int direction, bool isvert)
	{
    	    uint8_t dir1 = 0;
	    uint8_t dir2 = 0;

	    if (isvert)
	    {
		dir1 = GYRO_UP;
		dir2 = GYRO_DOWN;
    	    }
	    else
	    {
		dir1 = GYRO_LEFT;
		dir2 = GYRO_RIGHT;
    	    }

	    switch (direction)
	    {
		case -1: updatesensor(dir1, true); updatesensor(dir2, false); break;
		case 0: updatesensor(dir1, false); updatesensor(dir2, false); break;
		case 1: updatesensor(dir1, false); updatesensor(dir2, true); break;
	    }
	}
	
	void updategyro()
	{
	    if (TestBit(gyroscopeval, 0))
    	    {
		xsensor += 3;

		if (xsensor > 464)
		{
		    xsensor = 464;
		}

		if (xsensor < 0)
		{
		    xsensor = 10;
		}
	    }
	    else if (TestBit(gyroscopeval, 1))
	    {
		xsensor -= 3;

		if (xsensor < -464)
		{
		    xsensor = -464;
		}

		if (xsensor > 0)
		{
		    xsensor = -10;
		}
	    }
	    else if (xsensor > 0)
	    {
		xsensor -= 2;

		if (xsensor < 0)
		{
		    xsensor = 0;
		}
	    }
	    else if (xsensor < 0)
	    {
		xsensor += 2;

		if (xsensor > 0)
		{
		    xsensor = 0;
		}
	    }

	    if (TestBit(gyroscopeval, 2))
	    {
		ysensor += 3;

		if (ysensor > 464)
		{
		    ysensor = 464;
		}

		if (ysensor < 0)
		{
		    ysensor = 10;
		}
	    }
	    else if (TestBit(gyroscopeval, 3))
	    {
		ysensor -= 3;

		if (ysensor < -464)
		{
		    ysensor = -464;
		}

		if (ysensor > 0)
		{
		    ysensor = -10;
		}
	    }
	    else if (ysensor > 0)
	    {
		ysensor -= 2;

		if (ysensor < 0)
		{
		    ysensor = 0;
		}
    	    }
	    else if (ysensor < 0)
	    {
		ysensor += 2;

		if (ysensor > 0)
		{
		    ysensor = 0;
		}
	    }
	}
	
	void pixelcallback()
	{
	    drawpixels();
	}

	vector<uint8_t> loadfile(string filename, const void *unused1, int unused2)
	{
	    vector<uint8_t> result;

	    ifstream file(filename.c_str(), ios::in | ios::binary | ios::ate);

	    if (file.is_open())
	    {
		streampos size = file.tellg();
		result.resize(size, 0);
		file.seekg(0, ios::beg);
		file.read((char*)result.data(), size);
		file.close();
	    }

	    return result;
	}
	    
	GBCore *core;
        
	SDL_Window *window = NULL;
	SDL_Surface *surface = NULL;
	SDL_GameController *player1 = NULL;
	SDL_Haptic *haptic = NULL;
        
	bool isrumbleenabled = false;
	bool isrumbling = false;
        
	bool quit = false;
	SDL_Event event;
        
	Uint32 framecurrenttime = 0;
	Uint32 framestarttime = 0;
	
	int fpscount = 0;
	Uint32 fpstime = 0;
	
	JoypadControllers control;
	int playerinstanceid = 0;
	
	uint8_t gyroscopeval = 0;
	int scale = 3;
	
	int xdir = 0;
	int ydir = 0;

	int xsensordir = 0;
	int ysensordir = 0;

	int16_t xsensor = 0;
	int16_t ysensor = 0;
	
	vector<int16_t> buffer;
	
	const int controllerdeadzone = 8000;
};

int main(int argc, char* argv[])
{
    core.setsamplerate(48000);
    core.setaudioflags(MBGB_SIGNED16);

    SDL2Frontend *front = new SDL2Frontend(&core);
    core.setfrontend(front);
    
    if (!core.getoptions(argc, argv))
    {
	return 1;
    }
 
    if (!core.initcore())
    {
	return 1;
    }
    
    core.runapp();
    core.shutdown();
    
    return 0;
}
