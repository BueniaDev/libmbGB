#include "main.h"
using namespace gb;
using namespace std;
using namespace toml;
using namespace std::placeholders;

#ifdef __cpp_lib_filesystem
using namespace std::filesystem;
#else
using namespace std::experimental::filesystem;
#endif

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
        SDL2Frontend(GBCore *cb, char *argv[]) : core(cb)
        {
	    set_path(argv);
        }
        
        ~SDL2Frontend()
        {
        
        }

	int pixel_renderer = -1;
	string vert_shader = "";
	string frag_shader = "";
	path shader_path;

	bool init_config()
	{
	    auto file_path = u8path(appname).parent_path();
	    auto stream = ifstream(file_path / "libmbgb.toml");
	    ParseResult pr = parse(stream);

	    if (!pr.valid())
	    {
		cout << "Config file could not be parsed! toml::errorReason: " << pr.errorReason << endl;
		return false;
	    }

	    const auto toml = pr.value;

	    pixel_renderer = toml.get<int>("general.pixel_renderer");

	    shader_path = file_path / "shaders";

	    string shader = toml.get<string>("shaders.frag_shader");

	    vert_shader = (shader_path / "vertex.vs").string();
	    frag_shader = (shader_path / shader).string();

	    return true;
	}
        
        bool init()
        {
	    if (!init_config())
	    {
		cout << "Error parsing config file." << endl;
		return false;
	    }

	    #ifdef __WIN32
   	    putenv("SDL_AUDIODRIVER=DirectSound");
	    #endif

	    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	    {
		cout << "SDL could not be initialized! SDL_Error: " << SDL_GetError() << endl;
		return false;
	    }

	    if (pixel_renderer == 1)
	    {
		window_flags |= SDL_WINDOW_OPENGL;
	    }

	    window = SDL_CreateWindow("mbGB-SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, (core->screenwidth * scale), (core->screenheight * scale), window_flags);

	    if (window == NULL)
	    {
		cout << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
		return false;
	    }

	    switch (pixel_renderer)
	    {
		case 0: pix_render = new SoftwareRenderer(core, window); break;
		case 1: pix_render = new OpenGLRenderer(core, window); break;
		default: cout << "Unrecognized renderer option of " << dec << (int)(pixel_renderer) << endl; break;
	    }

	    if (pix_render == NULL)
	    {
		cout << "Falling back to software renderer..." << endl;
		pix_render = new SoftwareRenderer(core, window);
	    }

	    if (!pix_render->init_renderer(vert_shader, frag_shader))
	    {
		cout << "Error: failed to initalize renderer" << endl;
		return false;
	    }

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
	    pix_render->shutdown_renderer();
	    pix_render = NULL;
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

	    pix_render->take_screenshot(screenstring);

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
		    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: core->savestate(); break;
		    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: core->loadstate(); break;
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
	    pix_render->draw_pixels(core->getframebuffer(), scale);
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

	vector<uint8_t> loadfile(string filename)
	{
	    vector<uint8_t> result;

	    fstream file(filename.c_str(), ios::in | ios::binary | ios::ate);

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

	bool savefile(string filename, vector<uint8_t> data)
	{
	    fstream file(filename.c_str(), ios::out | ios::binary);

	    if (!file.is_open())
	    {
		cout << "mbGB::File could not be written." << endl;
		return false;
	    }
	    else
	    {
		file.write((char*)data.data(), data.size());
		cout << "mbGB::File succesfully written." << endl;
		file.close();
		return true;
	    }
	}

	bool camerainit()
	{
	    #ifdef LIBMBGB_CAMERA
	    if (camera_enabled == true)
	    {
		return true;
	    }

	    if (!cap.open(0))
	    {
		cout << "OpenCV error: couldn't open camera" << endl;
		return false;
	    }

	    camera_enabled = true;

	    int width = 160;
	    int height = 120;

	    while (true)
	    {
		cap.set(CAP_PROP_FRAME_WIDTH, width);
		cap.set(CAP_PROP_FRAME_HEIGHT, height);

		width = cap.get(CAP_PROP_FRAME_WIDTH);
		height = cap.get(CAP_PROP_FRAME_HEIGHT);

		if ((width >= 128) && (height >= 120))
		{
		    break;
		}
		else
		{
		    width <<= 1;
		    height <<= 1;
		}

		if (width >= 1024)
		{
		    break;
		}
	    }

	    Mat frame;
	    cap >> frame;

	    if (frame.empty())
	    {
		camerashutdown();
		cout << "OpenCV error: couldn't get frame" << endl;
		return false;
	    }

	    Size size = frame.size();
	    width = size.width;
	    height = size.height;

	    cout << "Camera resolution is " << dec << (int)(width) << "x" << dec << (int)(height) << endl;

	    if ((width < 128) || (height < 120))
	    {
		cout << "Camera resolution is too small..." << endl;
		return false;
	    }

	    int xfactor = (width / 128);
	    int yfactor = (height / 120);

	    camera_zoomfactor = (xfactor > yfactor) ? yfactor : xfactor;

	    #elif defined LIBMBGB_DEBUG
	    cout << "This build of libmbGB was compiled without webcam support." << endl;
	    #endif // LIBMBGB_CAMERA

	    return true;
	}

	void camerashutdown()
	{
	    #ifdef LIBMBGB_CAMERA

	    if (!camera_enabled)
	    {
		return;
	    }

	    cap.release();
	    camera_enabled = false;

	    #else
	    return;
	    #endif // LIBMBGB_CAMERA
	}

	void gen_noise(array<int, (128 * 120)> &arr)
	{
	    srand(time(NULL));
	    for (int i = 0; i < 128; i++)
	    {
		for (int j = 0; j < 120; j++)
		{
		    arr[(i + (j * 128))] = (rand() & 0xFF);
		}
	    }
	}

	bool cameraframe(array<int, (128 * 120)> &arr)
	{
	    #ifdef LIBMBGB_CAMERA

	    if (!camera_enabled)
	    {
		gen_noise(arr);
	    }
	    else
	    {
		Mat frame, converted_frame;
		cap >> frame;

		if (frame.empty())
		{
		    camerashutdown();
		    cout << "OpenCV error: couldn't get frame" << endl;
		    return false;
		}

		frame.convertTo(converted_frame, CV_8U);
		uint8_t *p = converted_frame.data;

		Size size = converted_frame.size();

		int width = size.width;

		int channels = converted_frame.channels();

		if (channels != 3)
		{
		    cout << "OpenCV error: Invalid camera output" << endl;
		    return false;
		}

		size_t step = converted_frame.elemSize();

		for (int i = 0; i < 128; i++)
		{
		    for (int j = 0; j < 120; j++)
		    {
			size_t index = (((j * camera_zoomfactor) * width * step) + ((i * camera_zoomfactor) * 3));

			int red = p[index];
			int green = p[(index + 1)];
			int blue = p[(index + 2)];

			arr[(i + (j * 128))] = ((2 * red + 5 * green + 1 * blue) >> 3);
		    }
		}
	    }

	    return true;
	    #else
	    gen_noise(arr);
	    return true;
	    #endif // LIBMBGB_CAMERA
	}

	void printerframe(vector<gbRGB> &temp, bool appending)
	{
	    cout << "Size of printer data: " << dec << (int)(temp.size()) << endl;
	    string isappending = (appending) ? "Yes" : "No";
	    cout << "Appending image? " << isappending << endl;

	    if (appending)
	    {
		appendbmp(temp);
	    }
	    else
	    {
		savebmp(temp);
	    }	    
	}

	void appendbmp(vector<gbRGB> &temp)
	{
	    SDL_Surface *surface = SDL_LoadBMP(printer_filename.c_str());

	    if (surface == NULL)
	    {
		cout << "BMP could not be loaded! SDL_Error: " << SDL_GetError() << endl;
		return;
	    }

	    int append_height = (temp.size() / 160);

	    int prev_width = (surface->w / scale);
	    int prev_height = (surface->h / scale);

	    int combined_height = (prev_height + append_height);

	    SDL_Surface *printout = SDL_CreateRGBSurface(0, (160 * scale), (combined_height * scale), 32, 0, 0, 0, 0);

	    SDL_Rect pixel = {0, 0, scale, scale};

	    for (int i = 0; i < prev_width; i++)
	    {
		pixel.x = (i * scale);

		for (int j = 0; j < prev_height; j++)
		{
		    pixel.y = (j * scale);

		    SDL_FillSurfaceRect(printout, &pixel, surface);
		}
	    }

	    int new_width = (printout->w / scale);
	    int new_height = append_height;

	    SDL_Rect new_pixel = {0, 0, scale, scale};
	    for (int i = 0; i < new_width; i++)
	    {
		new_pixel.x = (i * scale);

		for (int j = 0; j < new_height; j++)
		{
		    new_pixel.y = (surface->h + (j * scale));

		    gbRGB color = temp[(i + (j * new_width))];

		    SDL_FillRect(printout, &pixel, SDL_MapRGB(printout->format, color.red, color.green, color.blue));
		}
	    }

	    SDL_SaveBMP(printout, printer_filename.c_str());
	    SDL_FreeSurface(surface);
	    SDL_FreeSurface(printout);
	}

	void savebmp(vector<gbRGB> &temp)
	{
	    int buffer_height = (temp.size() / 160);
	    SDL_Surface *printout = SDL_CreateRGBSurface(0, (160 * scale), (buffer_height * scale), 32, 0, 0, 0, 0);

	    SDL_Rect pixel = {0, 0, scale, scale};

	    int width = (printout->w / scale);
	    int height = (printout->h / scale);

	    for (int i = 0; i < width; i++)
	    {
		pixel.x = (i * scale);

		for (int j = 0; j < height; j++)
		{
		    pixel.y = (j * scale);

		    gbRGB color = temp[(i + (j * width))];

		    SDL_FillRect(printout, &pixel, SDL_MapRGB(printout->format, color.red, color.green, color.blue));
		}
	    }

	    time_t currenttime = time(nullptr);
	    string filepath = "mbGBPrinter_";
	    filepath.append(to_string(currenttime));
	    filepath.append(".bmp");
	    printer_filename = filepath;

	    SDL_SaveBMP(printout, printer_filename.c_str());
	    SDL_FreeSurface(printout);
	}

	void set_path(char *argv[])
	{
	    appname = argv[0];
	}

	string appname;
	    
	GBCore *core;
        
	SDL_Window *window = NULL;
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

	PixelRenderer *pix_render = NULL;

	Uint32 window_flags = SDL_WINDOW_SHOWN;

	#ifdef LIBMBGB_CAMERA
	VideoCapture cap;
	bool camera_enabled = false;
	int camera_zoomfactor = 1;
	#endif

	string printer_filename = "";
};

int main(int argc, char* argv[])
{
    core.setsamplerate(48000);
    core.setaudioflags(MBGB_SIGNED16);

    SDL2Frontend *front = new SDL2Frontend(&core, argv);
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
