#include "libretro.h"
#include <libmbGB/libmbgb.h>

#include <array>
#include <vector>
using namespace gb;
using namespace std;

static constexpr int screenwidth = 160;
static constexpr int screenheight = 144;
static constexpr int scale = 3;

static constexpr int width = (screenwidth * scale);
static constexpr int height = (screenheight * scale);

static uint8_t rgb24torgb15(uint8_t c);
static void retrocallback(int16_t left, int16_t right);
static void processinput();
static void drawpixels();

static GBCore core;
static vector<int16_t> apubuffer;
static short framebuffer[screenwidth * screenheight];

static array<pair<size_t, Button>, 8> keymap = 
{
    {
	{ RETRO_DEVICE_ID_JOYPAD_UP, Button::Up },
	{ RETRO_DEVICE_ID_JOYPAD_DOWN, Button::Down },
	{ RETRO_DEVICE_ID_JOYPAD_LEFT, Button::Left },
	{ RETRO_DEVICE_ID_JOYPAD_RIGHT, Button::Right },
	{ RETRO_DEVICE_ID_JOYPAD_A, Button::A },
	{ RETRO_DEVICE_ID_JOYPAD_B, Button::B },
	{ RETRO_DEVICE_ID_JOYPAD_START, Button::Start },
	{ RETRO_DEVICE_ID_JOYPAD_SELECT, Button::Select },
    }
};

static retro_environment_t environment_cb;
static retro_video_refresh_t video_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;
static retro_log_printf_t log_cb;

unsigned retro_api_version(void)
{
    return RETRO_API_VERSION;
}

void retro_init()
{
    retro_log_callback log;
    int level = 4;

    if (environment_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log))
    {
	log_cb = log.log;
    }
    else
    {
	log_cb = nullptr;
    }

    environment_cb(RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL, &level);
}

void retro_deinit()
{

}

void retro_get_system_info(retro_system_info* info)
{
    memset(info, 0, sizeof(retro_system_info));
    info->library_name = "mbGB-retro";
    info->library_version = "Alpha";
    info->need_fullpath = false;
    info->valid_extensions = "gb|gbc";
}

void retro_get_system_av_info(retro_system_av_info* info)
{
    memset(info, 0, sizeof(retro_system_av_info));
    info->timing.fps = 60.0f;
    info->timing.sample_rate = 48000;
    info->geometry.base_width = screenwidth;
    info->geometry.base_height = screenheight;
    info->geometry.max_width = screenwidth;
    info->geometry.max_height = screenheight;
    info->geometry.aspect_ratio = (float)(160 / 144);
}

void retro_set_environment(retro_environment_t cb)
{
    environment_cb = cb;
}

bool retro_load_game(const retro_game_info* info)
{
    if (info && info->data)
    {
	core.coreapu->setaudiocallback(bind(&retrocallback, placeholders::_1, placeholders::_2));    	

	core.preinit();	

	if (!core.loadROM(info->path, (uint8_t*)(info->data), info->size))
	{
	    return false;
	}

	core.romname = info->path;

	core.init();

	return true;
    }
}

bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info)
{
    return false;
}

void retro_unload_game(void)
{
    core.shutdown();
}

void retro_reset(void)
{

}

void retro_run(void)
{
    core.runcore();

    processinput();

    drawpixels();

    video_cb(framebuffer, screenwidth, screenheight, (screenwidth * sizeof(short)));

    if (apubuffer.size() > 2)
    {
	audio_batch_cb(&apubuffer[0], (apubuffer.size() / 2));
	apubuffer.clear();
    }
}

void retrocallback(int16_t left, int16_t right)
{
    apubuffer.push_back(left);
    apubuffer.push_back(right);
}

void processinput()
{
    input_poll_cb();

    for (const auto& pair : keymap)
    {
	auto value = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, pair.first);

	if (value != 0)
	{
	    core.keypressed(pair.second);
	}
	else
	{
	    core.keyreleased(pair.second);
	}
    }
}

void drawpixels()
{
    for (int i = 0; i < 160; i++)
    {
	for (int j = 0; j < 144; j++)
	{
	    uint8_t red = core.getpixel(i, j).red;
	    uint8_t green = core.getpixel(i, j).green;
	    uint8_t blue = core.getpixel(i, j).blue;

	    auto r = rgb24torgb15(red);
	    auto g = rgb24torgb15(green);
	    auto b = rgb24torgb15(blue);

	    short rgb = (0x8000 | (r << 10) | (g << 5) | b);

	    framebuffer[(i + (j * 160))] = rgb;
	}
    }
}

uint8_t rgb24torgb15(uint8_t c)
{
    static constexpr int max15 = 0x1F;
    float currentratio = ((float)(c) / 255.0f);

    return ((uint8_t)((float)(max15 * currentratio)));
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
    audio_batch_cb = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
    video_cb = cb;
}

void retro_set_audio_sample(retro_audio_sample_t cb)
{
}

void retro_set_input_poll(retro_input_poll_t cb)
{
    input_poll_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb)
{
    input_state_cb = cb;
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
}

size_t retro_serialize_size(void)
{
    return 0;
}

bool retro_serialize(void *data, size_t size)
{
    return false;
}

bool retro_unserialize(const void *data, size_t size)
{
    return false;
}

void retro_cheat_reset(void)
{

}

void retro_cheat_set(unsigned index, bool enabled, const char *code)
{

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
