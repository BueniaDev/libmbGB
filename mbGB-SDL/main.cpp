#include <iostream>
#include <fstream>
#include <cassert>
#include <libmbGB/libmbgb.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
using namespace gb;

class SDLFrontend : public mbGBFrontend
{
    public:
	SDLFrontend(GBCore &cb, string romname) : core(cb), rom_name(romname)
	{

	}

	~SDLFrontend()
	{

	}

	bool init()
	{
	    // initMobileAdapter();
	    // initPrinter();
	    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	    {
		return sdl_error("SDL could not be initialized!");
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

	    SDL_AudioSpec audio_spec;
	    audio_spec.format = AUDIO_S16SYS;
	    audio_spec.freq = 48000;
	    audio_spec.samples = 4096;
	    audio_spec.channels = 2;
	    audio_spec.callback = NULL;

	    SDL_OpenAudio(&audio_spec, NULL);

	    SDL_PauseAudio(0);

	    return true;
	}

	void shutdown()
	{
	    shutdownAddons();

	    closeController();
	    SDL_CloseAudio();

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

	void saveFile(string filename, vector<uint8_t> data)
	{
	    if (data.empty())
	    {
		return;
	    }

	    fstream file(filename, ios::out | ios::binary | ios::trunc);

	    if (file.is_open())
	    {
		file.write((char*)data.data(), data.size());
		cout << filename << " succesfully written." << endl;
		file.close();
	    }
	}

	void audioCallback(int16_t left, int16_t right)
	{
	    audio_buffer.push_back(left);
	    audio_buffer.push_back(right);

	    if (audio_buffer.size() >= 4096)
	    {
		audio_buffer.clear();

		while (SDL_GetQueuedAudioSize(1) > (4096 * sizeof(int16_t)))
		{
		    SDL_Delay(1);
		}

		SDL_QueueAudio(1, audio_buffer.data(), (4096 * sizeof(int16_t)));
	    }
	}

	void rumbleCallback(double strength)
	{
	    if (!is_rumble_enabled)
	    {
		return;
	    }

	    if (use_modern_api)
	    {
		uint16_t frequency = (strength * 0xFFFF);
		SDL_GameControllerRumble(controller, frequency, frequency, -1);
	    }
	    else
	    {
		if (strength > 0)
		{
		    SDL_HapticRumblePlay(haptic, strength, -1);
		}
		else
		{
		    SDL_HapticRumbleStop(haptic);
		}
	    }
	}

    private:
	GBCore &core;

	bool quit = false;
	SDL_Event event;
	SDL_Window *window = NULL;
	SDL_Renderer *render = NULL;
	SDL_Texture *texture = NULL;

	vector<int16_t> audio_buffer;

	const int scale = 2;

	bool link = false;

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
			    case SDLK_u: core.contextKeyChanged(GBContextButton::ContextUp, is_key_pressed); break;
			    case SDLK_j: core.contextKeyChanged(GBContextButton::ContextDown, is_key_pressed); break;
			    case SDLK_h: core.contextKeyChanged(GBContextButton::ContextLeft, is_key_pressed); break;
			    case SDLK_k: core.contextKeyChanged(GBContextButton::ContextRight, is_key_pressed); break;
			    case SDLK_d:
			    {
				if (is_key_pressed)
				{
				    dumpScreenshot();
				}
			    }
			    break;
			    case SDLK_s:
			    {
				if (is_key_pressed)
				{
				    core.saveState(rom_name);
				}
			    }
			    break;
			    case SDLK_l:
			    {
				if (is_key_pressed)
				{
				    core.loadState(rom_name);
				}
			    }
			    break;
			}
		    }
		    break;
		    case SDL_CONTROLLERDEVICEADDED:
		    {
			if (event.cdevice.which < num_controllers)
			{
			    openController(event.cdevice.which);
			}
		    }
		    break;
		    case SDL_CONTROLLERDEVICEREMOVED:
		    {
			if (is_connected && (instance_id == event.cdevice.which))
			{
			    closeController();
			}
		    }
		    break;
		    case SDL_CONTROLLERBUTTONDOWN:
		    case SDL_CONTROLLERBUTTONUP:
		    {
			if (is_connected && (instance_id == event.cbutton.which))
			{
			    bool is_pressed = (event.type == SDL_CONTROLLERBUTTONDOWN);

			    switch (event.cbutton.button)
			    {
				case SDL_CONTROLLER_BUTTON_A: core.keyChanged(GBButton::ButtonA, is_pressed); break;
				case SDL_CONTROLLER_BUTTON_B: core.keyChanged(GBButton::ButtonB, is_pressed); break;
				case SDL_CONTROLLER_BUTTON_START: core.keyChanged(GBButton::Start, is_pressed); break;
				case SDL_CONTROLLER_BUTTON_BACK: core.keyChanged(GBButton::Select, is_pressed); break;
				case SDL_CONTROLLER_BUTTON_DPAD_UP: core.keyChanged(GBButton::Up, is_pressed); break;
				case SDL_CONTROLLER_BUTTON_DPAD_DOWN: core.keyChanged(GBButton::Down, is_pressed); break;
				case SDL_CONTROLLER_BUTTON_DPAD_LEFT: core.keyChanged(GBButton::Left, is_pressed); break;
				case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: core.keyChanged(GBButton::Right, is_pressed); break;
			    }
			}
		    }
		    break;
		    case SDL_CONTROLLERSENSORUPDATE:
		    {
			if (is_connected && (instance_id == event.csensor.which))
			{
			    switch (event.csensor.sensor)
			    {
				case SDL_SENSOR_ACCEL:
				{
				    auto motion_data = event.csensor.data;
				    processGyro(-motion_data[0], motion_data[2]);
				}
				break;
			    }
			}
		    }
		    break;
		}
	    }
	}

	void dumpScreenshot()
	{
	    time_t current_time = time(NULL);
	    stringstream ss;
	    ss << "mbGB_" << to_string(current_time) << ".png";
	    writePNG(ss.str(), 160, 144, core.getFramebuffer());
	    cout << "Screenshot saved." << endl;
	}

	template<typename T>
	void writePNG(string filename, int width, int height, vector<T> data)
	{
	    stbi_write_png(filename.c_str(), width, height, 3, data.data(), (width * 3));
	}

	void runmachine()
	{
	    SDL_PauseAudio(paused);
	    if (!paused)
	    {
		core.runCore();
	    }

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

	void initPrinter()
	{
	    printer = new GBPrinter();
	    core.connectSerialDevice(printer);
	}

	void shutdownPrinter()
	{
	    if (printer != NULL)
	    {
		printer = NULL;
	    }
	}

	void shutdownAddons()
	{
	    shutdownPrinter();
	}

	void openController(int device)
	{
	    controller = SDL_GameControllerOpen(device);
	    SDL_Joystick *joystick = SDL_GameControllerGetJoystick(controller);

	    instance_id = SDL_JoystickInstanceID(joystick);
	    is_connected = true;

	    is_rumble_enabled = false;
	    is_sensor_enabled = false;

	    if (use_modern_api)
	    {
		if (SDL_GameControllerHasRumble(controller))
		{
		    cout << "Rumble supported" << endl;
		    is_rumble_enabled = true;
		}

		if (SDL_GameControllerHasSensor(controller, SDL_SENSOR_ACCEL))
		{
		    cout << "Tilt supported" << endl;
		    SDL_GameControllerSetSensorEnabled(controller, SDL_SENSOR_ACCEL, SDL_TRUE);
		    is_sensor_enabled = true;
		}
	    }
	    else
	    {
		// TODO: Implement sensor fallback for SDL versions less than 2.0.18
		if (SDL_JoystickIsHaptic(joystick))
		{
		    haptic = SDL_HapticOpenFromJoystick(joystick);

		    if (SDL_HapticRumbleSupported(haptic))
		    {
			cout << "Rumble supported" << endl;
			is_rumble_enabled = true;
		    }
		}
	    }
	}

	void closeController()
	{
	    if (haptic != NULL)
	    {
		SDL_HapticClose(haptic);
		haptic = NULL;
	    }

	    if (controller != NULL)
	    {
		SDL_GameControllerClose(controller);
		is_connected = false;
		controller = NULL;
	    }
	}

	void processGyro(float xpos, float ypos)
	{
	    float xpos_norm = clamp<float>((-xpos / SDL_STANDARD_GRAVITY), -1.f, 1.f);
	    float ypos_norm = clamp<float>((ypos / SDL_STANDARD_GRAVITY), -1.f, 1.f);
	    float dead_zone_norm = (float(motion_dead_zone) / 1000.f);

	    float xabs = (xpos_norm < 0) ? -xpos_norm : xpos_norm;
	    float yabs = (ypos_norm < 0) ? -ypos_norm : ypos_norm;

	    float sensorx = 0;
	    float sensory = 0;

	    if (xabs >= dead_zone_norm)
	    {
		sensorx = xpos_norm;
	    }

	    if (yabs >= dead_zone_norm)
	    {
		sensory = ypos_norm;
	    }
	    
	    core.updateAccel(sensorx, sensory);
	}

	Uint32 framecurrenttime = 0;
	Uint32 framestarttime = 0;
	int fpscount = 0;
	Uint32 fpstime = 0;

	string rom_name = "";

	const int num_controllers = 1;

	SDL_GameController *controller = NULL;
	SDL_Haptic *haptic = NULL;
	SDL_JoystickID instance_id;
	bool is_connected = false;
	bool is_rumble_enabled = false;
	bool is_sensor_enabled = false;

	const int motion_dead_zone = 150;

	#if SDL_VERSION_ATLEAST(2, 0, 18)
	const bool use_modern_api = true;
	#else
	const bool use_modern_api = false;
	#endif

	bool paused = false;

	GBPrinter *printer = NULL;
};

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
	cout << "Usage: " << argv[0] << " ROM" << endl;
	return 1;
    }

    GBModel model_type = ModelAuto;

    string bios_name;
    string rom_name = argv[1];

    for (int i = 2; i < argc; i++)
    {
	if (strcmp(argv[i], "--dmg") == 0)
	{
	    model_type = ModelDmgX;
	}

	if (strcmp(argv[i], "--cgb") == 0)
	{
	    model_type = ModelCgbX;
	}

	if ((strcmp(argv[i], "-b") == 0) || (strcmp(argv[i], "--bios") == 0))
	{
	    if ((i + 1) == argc)
	    {
		cout << "Error - No BIOS file in arguments" << endl;
		return 1;
	    }
	    else
	    {
		bios_name = argv[i + 1];
	    }
	}
    }

    GBCore core;
    SDLFrontend *front = new SDLFrontend(core, rom_name);
    core.setFrontend(front);
    core.setModel(model_type);

    if (!bios_name.empty())
    {
	if (!core.loadBIOS(bios_name))
	{
	    return 1;
	}
    }

    stringstream str;
    str << rom_name << ".sav";

    if (!core.loadROM(rom_name))
    {
	return 1;
    }

    if (!core.initCore())
    {
	return 1;
    }

    core.loadBackup(str.str());
    front->run();
    core.saveBackup(str.str());
    core.shutdown();
    return 0;
}