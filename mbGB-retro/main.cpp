#include <libmbGB/libmbgb.h>
#include <libretro.h>
#include <stdio.h>
using namespace gb;
using namespace std;

static array<pair<size_t, gbButton>, 8> keymap = 
{
    {
	{ RETRO_DEVICE_ID_JOYPAD_UP, gbButton::Up },
	{ RETRO_DEVICE_ID_JOYPAD_DOWN, gbButton::Down },
	{ RETRO_DEVICE_ID_JOYPAD_LEFT, gbButton::Left },
	{ RETRO_DEVICE_ID_JOYPAD_RIGHT, gbButton::Right },
	{ RETRO_DEVICE_ID_JOYPAD_A, gbButton::A },
	{ RETRO_DEVICE_ID_JOYPAD_B, gbButton::B },
	{ RETRO_DEVICE_ID_JOYPAD_START, gbButton::Start },
	{ RETRO_DEVICE_ID_JOYPAD_SELECT, gbButton::Select },
    }
};

class LibretroFrontend : public mbGBFrontend
{
    public:
	LibretroFrontend(GBCore *corecb)
	{
    	    core = corecb;
    	}
    
    	~LibretroFrontend()
    	{
    
    	}
    
    	bool init()
    	{
    	    if (env(RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE, &rumble))
    	    {
    	    	printf("Rumble supported\n");
    	    	rumbleenabled = true;
    	    }
    	    else
    	    {
    	    	printf("Rumble not supported\n");
    	        rumbleenabled = false;
    	    }
    	
    	    return true;
    	}
    
    	void shutdown()
    	{
    
    	}
    
    	void runapp()
    	{
    	    core->runcore();
    	    pollinput();
    	    
    	    if (apubuffer.size() > 2)
    	    {
    	        samplebatch(apubuffer.data(), (apubuffer.size() / 2));
    	        apubuffer.clear();
    	    }
    	}
    	
    	void pollinput()
    	{
    	    inputpoll();
    	    
    	    for (const auto& pair : keymap)
    	    {
    	        auto value = inputstate(0, RETRO_DEVICE_JOYPAD, 0, pair.first);
    	        
    	        if (value != 0)
    	        {
    	            core->keypressed(pair.second);
    	        }
    	        else
    	        {
    	            core->keyreleased(pair.second);
    	        }
    	    }
    	}
    
    	void audiocallback(audiotype left, audiotype right)
    	{
    	    if (!holds_alternative<int16_t>(left) || !holds_alternative<int16_t>(right))
    	    {
    	        return;
    	    }
    	    
    	    apubuffer.push_back(get<int16_t>(left));
    	    apubuffer.push_back(get<int16_t>(right));
    	}
    
    	void rumblecallback(bool enabled)
    	{
    	    if (!rumbleenabled)
    	    {
    	        return;
    	    }
    	
    	    if (rumble.set_rumble_state)
    	    {
    	    	if (enabled && !isrumbling)
    	    	{
    	    	    // rumble.set_rumble_state(1, RETRO_RUMBLE_STRONG, 0xFFFF); 
    	    	    isrumbling = true;
    	    	}
    	    	else if (!enabled && isrumbling)
    	    	{
    	    	    // rumble.set_rumble_state(1, RETRO_RUMBLE_STRONG, 0); 
    	    	    isrumbling = false;
    	    	}
    	    }
    	}
    
    	void pixelcallback()
    	{
    	    short framebuffer[(160 * 144)];
    
    	    for (int i = 0; i < 160; i++)
    	    {
    	    	for (int j = 0; j < 144; j++)
    	    	{
    	    	    gbRGB framecolor = core->getpixel(i, j);
    	    	    
    	    	    auto red = rgb24torgb15(framecolor.red);
    	    	    auto green = rgb24torgb15(framecolor.green);
    	    	    auto blue = rgb24torgb15(framecolor.blue);
    	    	    
    	    	    short color = (0x8000 | (red << 10) | (green << 5) | blue);
    	    	
    	    	    framebuffer[(i + (j * 160))] = color;
    	    	}
    	    }
    	
    	    vidrefresh(framebuffer, 160, 144, (160 * sizeof(short)));
    	}

	vector<uint8_t> loadfile(string filename)
	{
	    vector<uint8_t> temp;
	    FILE *ret = fopen(filename.c_str(), "rb");
	    
	    if (ret == NULL)
	    {
		cout << "Error: could not load file." << endl;
	    }
	    else
	    {
		fseek(ret, 0, SEEK_END);
		size_t sz = ftell(ret);

		fseek(ret, 0, SEEK_SET);
		temp.resize(sz, 0);
		fread(temp.data(), temp.size(), 1, ret);
		cout << "File succesfully loaded." << endl;
		fclose(ret);
	    }

	    return temp;
	}

	bool savefile(string filename, vector<uint8_t> data)
	{
	    // If the data vector is empty, don't bother saving its contents
	    if (data.empty())
	    {
		return true;
	    }

	    FILE *ret = fopen(filename.c_str(), "wb");
	    
	    if (ret == NULL)
	    {
		cout << "Error: could not save file." << endl;
		return false;
	    }
	    else
	    {
		fwrite(data.data(), data.size(), 1, ret);
		cout << "File succesfully written." << endl;
		fclose(ret);
	    }

	    return true;
	}

	// Not sure if Libretro has any webcam APIs, so we're just gonna stub these functions...

	bool camerainit()
	{
	    return true;
	}

	void camerashutdown()
	{
	    return;
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
	    gen_noise(arr);
	    return true;
	}

	// Not sure if Libretro has any printer APIs, either, so we're gonna stub this function as well...
	void printerframe(vector<gbRGB> &temp, bool appending)
	{
	    return;
	}
    
    	void setenvironment(retro_environment_t cb)
    	{
            env = cb;
    	}
    
    	void setvideorefresh(retro_video_refresh_t cb)
    	{
    	    vidrefresh = cb;
    	}
    
        void setaudiosample(retro_audio_sample_t cb)
    	{
            sample = cb;
    	}
    
    	void setaudiosamplebatch(retro_audio_sample_batch_t cb)
    	{
    	    samplebatch = cb;
    	}
    
    	void setinputpoll(retro_input_poll_t cb)
    	{
    	    inputpoll = cb;
    	}
    
    	void setinputstate(retro_input_state_t cb)
    	{
    	    inputstate = cb;
    	}
    
    	void getsysteminfo(retro_system_info *info)
    	{
    	    info->library_name = "mbGB-Retro";
    	    info->library_version = "0.2";
    	    info->need_fullpath = false;
    	    info->valid_extensions = "gb|gbc";
    	}
    
    	void getsystemavinfo(retro_system_av_info *info)
    	{
    	    info->geometry.base_width = screenwidth;
    	    info->geometry.base_height = screenheight;
    	    info->geometry.max_width = screenwidth;
    	    info->geometry.max_height = screenheight;
    	    info->geometry.aspect_ratio = 0.0f;
    	
    	    info->timing.fps = 60.0f;
    	    info->timing.sample_rate = 48000;
    	}
    	
    	uint8_t rgb24torgb15(uint8_t color)
    	{
    	    float currentratio = ((float)color / 255.0f);
    	    
    	    return (uint8_t)((float)0x1F * currentratio);
    	}
    
    	retro_environment_t env;
    	retro_video_refresh_t vidrefresh;
    	retro_audio_sample_t sample;
    	retro_audio_sample_batch_t samplebatch;
    	retro_input_poll_t inputpoll;
    	retro_input_state_t inputstate;
    	
    	retro_rumble_interface rumble;
    	retro_log_printf_t log;
    
    	int screenwidth = 160;
    	int screenheight = 144;
    	
    	bool rumbleenabled = false;
    	bool isrumbling = false;
    	
    	GBCore *core = NULL;
    	
    	vector<int16_t> apubuffer;
};



GBCore core;
LibretroFrontend *front = new LibretroFrontend(&core);

unsigned retro_api_version(void)
{
    return RETRO_API_VERSION;
}

void retro_init(void)
{
    core.setfrontend(front);
}

void retro_deinit(void)
{
    core.shutdown();
}

void retro_run(void)
{
    core.runapp();
}

void retro_set_environment(retro_environment_t callback)
{
    front->setenvironment(callback);
}

void retro_set_video_refresh(retro_video_refresh_t callback)
{
    front->setvideorefresh(callback);
}

void retro_set_audio_sample(retro_audio_sample_t callback)
{
    front->setaudiosample(callback);
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t callback)
{
    front->setaudiosamplebatch(callback);
}

void retro_set_input_poll(retro_input_poll_t callback)
{
    front->setinputpoll(callback);
}

void retro_set_input_state(retro_input_state_t callback)
{
    front->setinputstate(callback);
}

bool retro_load_game(const struct retro_game_info *game)
{
    if (game && game->data)
    {
    	core.setsamplerate(48000);
    	core.setaudioflags(MBGB_SIGNED16);
    	core.connectserialdevice(new Disconnected());

	core.romname = game->path;
        core.initcore();
        
        return true;
    }
    
    return false;
}

bool retro_load_game_special(unsigned game_type, const struct retro_game_info *game, size_t numinfo)
{
    return false;
}

void retro_unload_game(void)
{
    return;
}

void retro_reset(void)
{
    core.resetcore();
}

void retro_get_system_info(struct retro_system_info *info)
{
    front->getsysteminfo(info);
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
    front->getsystemavinfo(info);
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
    return;
}

size_t retro_serialize_size(void)
{
    cout << "Fetching size of libretro savestate..." << endl;
    return core.getstatesize();
}

bool retro_serialize(void *data, size_t size)
{
    void *temp_data = data;
    VecFile file = vfopen(temp_data, size);
    mbGBSavestate savestate(file, true);
    core.dosavestate(savestate);
    memcpy(data, savestate.get_savestate_file().data.data(), size);
    vfclose(file);
    return true;
}

bool retro_unserialize(const void *data, size_t size)
{
    VecFile file = vfopen(const_cast<void*>(data), size);
    mbGBSavestate savestate(file, false);
    core.dosavestate(savestate);
    vfclose(file);
    return true;
}

void retro_cheat_reset(void)
{
    return;
}

void retro_cheat_set(unsigned index, bool enabled, const char *code)
{
    return;
}

void *retro_get_memory_data(unsigned id)
{
    return NULL;
}

size_t retro_get_memory_size(unsigned id)
{
    return 0;
}

unsigned retro_get_region(void)
{
    return RETRO_REGION_NTSC;
}
