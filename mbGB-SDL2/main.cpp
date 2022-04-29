#include "main.h"
using namespace gb;
using namespace std;
using namespace toml;
using namespace std::placeholders;

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
        SDL2Frontend(GBCore &cb, char *argv[]) : core(cb)
        {
	    set_path(argv);
        }
        
        ~SDL2Frontend()
        {
        
        }

	int pixel_renderer = -1;
	string vert_shader = "";
	string frag_shader = "";

	bool init_config()
	{
	    #ifdef __WIN32__
	    const char seperator = '\\';
	    #else
	    const char seperator = '/';
	    #endif // __WIN32__
	    base_path = SDL_GetBasePath();
	    stringstream configpath;
	    configpath << base_path << "libmbgb.toml";
	    auto stream = ifstream(configpath.str().c_str());
	    ParseResult pr = parse(stream);

	    if (!pr.valid())
	    {
		cout << "Config file could not be parsed! toml::errorReason: " << pr.errorReason << endl;
		return false;
	    }

	    const auto toml = pr.value;

	    pixel_renderer = toml.get<int>("general.pixel_renderer");

	    bool is_cheats_enable = toml.get<bool>("general.enable_cheats");

	    if (is_cheats_enable)
	    {
		string cheat_filename = toml.get<string>("general.cheats_path");

		if (!parse_cheat_codes(cheat_filename))
		{
		    return false;
		}
	    }

	    // Filepath of vertex shader
	    string shader_vert = toml.get<string>("shaders.vert_shader");
	    stringstream vertshader_str;
	    vertshader_str << base_path << "shaders" << seperator << shader_vert << ".vs";

	    // Filepath of fragment shader
	    string shader_frag = toml.get<string>("shaders.frag_shader");
	    stringstream fragshader_str;
	    fragshader_str << base_path << "shaders" << seperator << shader_frag << ".fs";

	    vert_shader = vertshader_str.str();
	    frag_shader = fragshader_str.str();

	    return true;
	}

	bool init_core()
	{
	    if (!init_config())
	    {
		cout << "Error parsing config file." << endl;
		return false;
	    }

	    if (!core.initcore())
	    {
		return false;
	    }

	    return true;
	}
        
        bool init()
        {
	    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	    {
		cout << "SDL2 could not be initialized! SDL_Error: " << SDL_GetError() << endl;
		return false;
	    }

	    #ifdef USE_OPENGL
	    if (pixel_renderer == 1)
	    {
		window_flags |= SDL_WINDOW_OPENGL;
	    }
	    #endif // USE_OPENGL

	    window = SDL_CreateWindow("mbGB-SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, (core.screenwidth * scale), (core.screenheight * scale), window_flags);

	    if (window == NULL)
	    {
		cout << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
		return false;
	    }

	    switch (pixel_renderer)
	    {
		case 0: pix_render = new SoftwareRenderer(core, window); break;
		#ifdef USE_OPENGL
		case 1: pix_render = new OpenGLRenderer(core, window); break;
		#endif // USE_OPENGL
		default: cout << "Unrecognized backend option of " << dec << (int)(pixel_renderer) << endl; break;
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

	    if (SDL_OpenAudio(&audiospec, NULL) < 0)
	    {
		cout << "Could not open audio! SDL_Error: " << SDL_GetError() << endl;
		return false;
	    }

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

	void stop_core()
	{
	    core.shutdown();
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
	    SDL_PauseAudio(core.paused);

	    if (!core.paused)
	    {
	    	core.runcore();
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
	    core.keyreleased(gbButton::A);
	    core.keyreleased(gbButton::B);
	    core.keyreleased(gbButton::Start);
	    core.keyreleased(gbButton::Select);
	    core.keyreleased(gbButton::Up);
	    core.keyreleased(gbButton::Down);
	    core.keyreleased(gbButton::Left);
	    core.keyreleased(gbButton::Right);
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

	void reset()
	{
	    return;
	}
	
	void handleinput(SDL_Event event)
	{
	    if ((event.type == SDL_KEYDOWN) || (event.type == SDL_KEYUP))
	    {
		bool is_pressed = (event.type == SDL_KEYDOWN);

		switch (event.key.keysym.sym)
		{
		    case SDLK_UP: core.keychanged(gbButton::Up, is_pressed); break;
		    case SDLK_DOWN: core.keychanged(gbButton::Down, is_pressed); break;
		    case SDLK_LEFT: core.keychanged(gbButton::Left, is_pressed); break;
		    case SDLK_RIGHT: core.keychanged(gbButton::Right, is_pressed); break;
		    case SDLK_a: core.keychanged(gbButton::A, is_pressed); break;
		    case SDLK_b: core.keychanged(gbButton::B, is_pressed); break;
		    case SDLK_RETURN: core.keychanged(gbButton::Start, is_pressed); break;
		    case SDLK_SPACE: core.keychanged(gbButton::Select, is_pressed); break;
		    case SDLK_p:
		    {
			if (is_pressed)
			{
			    core.paused = !core.paused;
			}
		    }
		    break;
		    case SDLK_r:
		    {
			if (is_pressed)
			{
			    core.resetcore();
			}
		    }
		    break;
		    case SDLK_q:
		    {
			if (is_pressed)
			{
			    screenshot();
			}
		    }
		    break;
		    case SDLK_l:
		    {
			if (is_pressed && isctrlshiftpressed(event))
			{
			    core.loadstate();
			}
		    }
		    break;
		    case SDLK_s: 
		    {
			if (is_pressed)
			{
			    if (isctrlshiftpressed(event))
			    {
				core.savestate();
			    }
			    else
			    {
				core.swipebarcode();
			    }
			}
		    }
		    break;
		}
	    }
	    else if ((event.type == SDL_CONTROLLERBUTTONDOWN) || (event.type == SDL_CONTROLLERBUTTONUP))
	    {
		bool is_pressed = (event.type == SDL_CONTROLLERBUTTONDOWN);

		switch (event.cbutton.button)
		{
		    case SDL_CONTROLLER_BUTTON_START: core.keychanged(gbButton::Start, is_pressed); break;
		    case SDL_CONTROLLER_BUTTON_BACK: core.keychanged(gbButton::Select, is_pressed); break;
		    case SDL_CONTROLLER_BUTTON_A: core.keychanged(gbButton::A, is_pressed); break;
		    case SDL_CONTROLLER_BUTTON_B: core.keychanged(gbButton::B, is_pressed); break;
		    case SDL_CONTROLLER_BUTTON_DPAD_UP: core.keychanged(gbButton::Up, is_pressed); break;
		    case SDL_CONTROLLER_BUTTON_DPAD_DOWN: core.keychanged(gbButton::Down, is_pressed); break;
		    case SDL_CONTROLLER_BUTTON_DPAD_LEFT: core.keychanged(gbButton::Left, is_pressed); break;
		    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: core.keychanged(gbButton::Right, is_pressed); break;
		    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
		    {
			if (is_pressed)
			{
			    core.savestate();
			}
		    }
		    break;
		    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
		    {
			if (is_pressed)
			{
			    core.loadstate();
			}
		    }
		    break;
		    case SDL_CONTROLLER_BUTTON_LEFTSTICK: core.keychanged(gbButton::A, is_pressed); break;
		    case SDL_CONTROLLER_BUTTON_X:
		    {
			if (is_pressed)
			{
			    core.paused = !core.paused;
			}
		    }
		    break;
		    case SDL_CONTROLLER_BUTTON_Y:
		    {
			if (is_pressed)
			{
			    core.swipebarcode();
			}
		    }
		    break;
		}
	    }
    	    else if (event.type == SDL_CONTROLLERAXISMOTION)
	    {
		switch (event.caxis.axis)
		{
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
	
			handleaxis(xdir, false);
		    }
		    break;
		    case SDL_CONTROLLER_AXIS_RIGHTX:
		    {
			accel_x = -(float(event.caxis.value) / 32767.0);
			core.set_accel_values(accel_x, accel_y);
		    }
		    break;
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
	
			handleaxis(ydir, true);
		    }
		    break;
		    case SDL_CONTROLLER_AXIS_RIGHTY:
		    {
			accel_y = -(float(event.caxis.value) / 32767.0);
			core.set_accel_values(accel_x, accel_y);
		    }
		    break;
		}
	    }	
	}
	
	void drawpixels()
	{
	    pix_render->draw_pixels(core.getframebuffer(), scale);
	}
	
	void audiocallback(int16_t left, int16_t right)
	{
	    if (SDL_GetQueuedAudioSize(1) > 48000)
	    {
		return;
	    }

	    buffer.push_back(left);
	    buffer.push_back(right);

	    SDL_QueueAudio(1, buffer.data(), (2 * sizeof(int16_t)));
	    buffer.clear();
	}
	
	void rumblecallback(double strength)
	{
	    if (!isrumbleenabled)
	    {
		return;
	    }

	    if (strength == 0)
	    {
		if (SDL_HapticRumbleStop(haptic) != 0)
		{
		    cout << "Error stopping rumble! SDL_Error: " << SDL_GetError() << endl;
	    	    exit(1);
		}
	    }
	    else
	    {
		if (SDL_HapticRumblePlay(haptic, strength, 250) != 0)
		{
		    cout << "Error playing rumble! SDL_Error: " << SDL_GetError() << endl;
		    exit(1);
		}
	    }
	}

	void handleaxis(int direction, bool isvert)
	{
	    gbButton dir1 = (isvert) ? gbButton::Up : gbButton::Left;
	    gbButton dir2 = (isvert) ? gbButton::Down : gbButton::Right;

	    switch (direction)
	    {
		case -1: core.keypressed(dir1); core.keyreleased(dir2); break;
		case 0: core.keyreleased(dir1); core.keyreleased(dir2); break;
		case 1: core.keyreleased(dir1); core.keypressed(dir2); break;
    	    }
	}
	
	void pixelcallback()
	{
	    // drawpixels();
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
	    // If the data vector is empty, don't bother saving its contents
	    if (data.empty())
	    {
		return true;
	    }

	    fstream file(filename.c_str(), ios::out | ios::binary);

	    if (!file.is_open())
	    {
		cout << "mbGB::Savefile could not be written." << endl;
		return false;
	    }
	    
	    file.write((char*)data.data(), data.size());
	    file.close();
	    return true;
	}

	bool camerainit()
	{
	    #ifdef USE_WEBCAM
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

	    #else
	    cout << "Notice: This build of libmbGB was compiled without webcam support." << endl;
	    #endif // USE_WEBCAM

	    return true;
	}

	void camerashutdown()
	{
	    #ifdef USE_WEBCAM

	    if (!camera_enabled)
	    {
		return;
	    }

	    cap.release();
	    camera_enabled = false;

	    #else
	    return;
	    #endif // USE_WEBCAM
	}

	bool cameraframe(array<int, (128 * 120)> &arr)
	{
	    #ifdef USE_WEBCAM
	    if (!camera_enabled)
	    {
		return mbGBFrontend::cameraframe(arr);
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
	    return mbGBFrontend::camerframe(arr);
	    #endif // USE_WEBCAM
	}

	void printerframe(vector<gbRGB> &temp, bool appending)
	{
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

		    SDL_FillRect(printout, &new_pixel, SDL_MapRGB(printout->format, color.red, color.green, color.blue));
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

	bool parse_cheat_codes(string filename)
	{
	    stringstream cheatpath;
	    cheatpath << base_path << filename << ".toml";
	    auto stream = ifstream(cheatpath.str().c_str());
	    ParseResult pr = parse(stream);

	    if (!pr.valid())
	    {
		cout << "Cheat file could not be parsed! toml::errorReason: " << pr.errorReason << endl;
		return false;
	    }

	    const auto toml = pr.value;

	    const Value *base = toml.find("cheats");

	    if (base == NULL)
	    {
		cout << "Cheats array does not exist" << endl;
		return false;
	    }

	    if (!base->is<vector<string>>())
	    {
		cout << "Cheats array is invalid" << endl;
		return false;
	    }

	    vector<string> cheat_codes = base->as<vector<string>>();
	    core.addcheats(cheat_codes);
	    return true;
	}

	string appname;
	    
	GBCore &core;
        
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

	#ifdef USE_WEBCAM
	VideoCapture cap;
	bool camera_enabled = false;
	int camera_zoomfactor = 1;
	#endif

	string printer_filename = "";
	string base_path = "";

	float accel_x = 0;
	float accel_y = 0;
};

int main(int argc, char* argv[])
{
    GBCore core;
    core.setsamplerate(48000);

    SDL2Frontend *front = new SDL2Frontend(core, argv);
    core.setfrontend(front);

    if (!core.getoptions(argc, argv))
    {
	return 1;
    }

    if (!front->init_core())
    {
	return 1;
    }
    
    front->runapp();
    front->stop_core();
    return 0;
}
