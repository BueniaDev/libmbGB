#include <SDL2/SDL.h>
#include <imgui.h>
#include <imgui_sdl.h>
#include <tinyfiledialogs.h>
#include <libmbGB/libmbgb.h>
#include <iostream>
using namespace std;
#undef main

int screenwidth = 160;
int screenheight = 144;
int scale = 4;

RGB tilebuffer[128 * 192];
RGB tilebuffer1[128 * 192];

uint16_t breakpoints[8];
int breakpointamount = 0;

int fpscount = 0;
Uint32 fpstime = 0;

int width = 1200;
int height = 800;

int tempwidth = (screenwidth * 2);
int tempheight = (screenheight * 2);

SDL_Window *window;
SDL_Renderer *render;
SDL_Surface *surface;
SDL_Texture *texture;

SDL_Surface *tilesurface;
SDL_Texture *tiletex;

GBCore core;

bool playing = false;
bool disabled = false;
bool regenabled = false;
bool screenenabled = false;
bool tilesenabled = false;
bool paused = false;

bool init()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
	cout << "SDL could not be initialized! SDL_Error: " << SDL_GetError() << endl;
	return false;
    }

    window = SDL_CreateWindow("mbGB-imgui", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);

    if (window == NULL)
    {
	cout << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
	return false;
    }

    render = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

    if (render == NULL)
    {
	cout << "Renderer could not be created! SDL_Error: " << SDL_GetError() << endl;
	return false;
    }

    surface = SDL_CreateRGBSurface(0, tempwidth, tempheight, 32, 0, 0, 0, 0);

    tilesurface = SDL_CreateRGBSurface(0, (256 * 2), (192 * 2), 32, 0, 0, 0, 0);

    texture = SDL_CreateTextureFromSurface(render, surface);

    tiletex = SDL_CreateTextureFromSurface(render, tilesurface);

    ImGui::CreateContext();
    ImGuiSDL::Initialize(render, width, height);

    SDL_SetRenderDrawColor(render, 114, 144, 154, 255);
    SDL_RenderClear(render);
    SDL_RenderPresent(render);

    return true;
}

bool selectrom()
{
    const char *validextensions[4] = {"*.gb", "*.GB", "*.gbc", "*.GBC"};
    const char *filepath = tinyfd_openFileDialog("Select ROM...", "", 4, validextensions, NULL, 0);
    core.romname = filepath;

    if (filepath != NULL)
    {
	return core.loadROM(core.romname);
    }

    return false;
}

uint8_t getcolor(uint8_t hibyte, uint8_t lobyte, uint8_t pos)
{
    uint8_t color = 0;
    if (TestBit(hibyte, pos))
    {
	color = BitSet(color, 0);
    }

    if (TestBit(lobyte, pos))
    {
	color = BitSet(color, 1);
    }

    return color;
}

RGB getcurrentpalette(uint8_t color)
{
    int gbcolor = 0;

    RGB temp;

    switch (color)
    {
	case 0: gbcolor = 0xFF; break;
	case 1: gbcolor = 0xCC; break;
	case 2: gbcolor = 0x77; break;
	case 3: gbcolor = 0x00; break;
    }

    temp.red = gbcolor;
    temp.green = gbcolor;
    temp.blue = gbcolor;

    return temp;
}

void updatetiles()
{
    uint16_t tiledata = 0x8000;
    for (int row = 0; row < 24; row++)
    {
	for (int column = 0; column < 16; column++)
	{
	    uint16_t start = (tiledata + (16 * column));

	    for (int line = (row * 8); line < (8 + (row * 8)); line++, start += 2)
	    {
		uint8_t upperbyte = core.coremmu->vram[start - 0x8000];
		uint8_t lowerbyte = core.coremmu->vram[(start + 1) - 0x8000];

		uint8_t upperbyte2 = core.coremmu->vram[start - 0x6000];
		uint8_t lowerbyte2 = core.coremmu->vram[(start + 1) - 0x6000];

		for (int tilecolumn = (column * 8), position = 7; tilecolumn < (8 + (column * 8)); tilecolumn++, position--)
		{
		    int index = (tilecolumn + (line * 128));
		    tilebuffer[index] = getcurrentpalette(getcolor(upperbyte, lowerbyte, position));
		    tilebuffer1[index] = getcurrentpalette(getcolor(upperbyte2, lowerbyte2, position));
		}
	    }
	}

	tiledata += 0x100;
    }
}

void blankscreen()
{
    SDL_SetRenderDrawColor(render, 255, 255, 255, 255);
    SDL_RenderClear(render);
}

void shutdown()
{
    core.shutdown();
    ImGuiSDL::Deinitialize();
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(render);
    SDL_DestroyWindow(window);
    ImGui::DestroyContext();
    SDL_Quit();
}

void initcore()
{
    core.preinit();

    if (core.biosload())
    {
	if (!core.loadBIOS(core.biosname))
	{
	    return;
	}
    }
    else if (!selectrom())
    {
	return;
    }

    core.init();
}

void resetcore()
{
    core.preinit();

    if (core.biosload())
    {
	if (!core.loadBIOS(core.biosname))
	{
	    return;
	}
    }
    else if (!core.loadROM(core.romname))
    {
	return;
    }

    core.init();
}

void stopcore()
{
    core.shutdown();
}

void tiles()
{
    ImGui::Begin("Tiles");
    ImGui::Image(tiletex, ImVec2((256 * 2), (192 * 2)));
    ImGui::End();
}

void screen()
{
    ImGui::Begin("Screen");
    ImGui::Image(texture, ImVec2(tempwidth, tempheight));
    ImGui::End();
}

void vramviewer()
{
    ImGui::Begin("VRAM Viewer");
    ImGui::End();
}

void regview()
{
    bool flagz = core.corecpu->iszero();
    bool flagn = core.corecpu->issubtract();
    bool flagh = core.corecpu->ishalf();
    bool flagc = core.corecpu->iscarry();

    ImGui::Begin("Registers");
    ImGui::SetWindowSize("Registers", ImVec2(400, 300));
    ImGui::Text("AF: %04x", core.corecpu->af.getreg());
    ImGui::SameLine();
    ImGui::Indent(80.f);
    ImGui::Text("BC: %04x", core.corecpu->bc.getreg());
    ImGui::SameLine();
    ImGui::Indent(80.f);
    ImGui::NewLine();
    ImGui::Unindent(160.f);
    ImGui::Text("DE: %04x", core.corecpu->de.getreg());
    ImGui::SameLine();
    ImGui::Indent(80.f);
    ImGui::Text("HL: %04x", core.corecpu->hl.getreg());
    ImGui::SameLine();
    ImGui::Indent(80.f);
    ImGui::NewLine();
    ImGui::Unindent(160.f);
    ImGui::Text("SP: %04x", core.corecpu->sp);
    ImGui::SameLine();
    ImGui::Indent(80.f);
    ImGui::Text("PC: %04x", core.corecpu->pc);
    ImGui::SameLine();
    ImGui::Indent(80.f);
    ImGui::NewLine();
    ImGui::Unindent(160.f);
    ImGui::Text("TIMA: %04x", core.coremmu->readByte(0xFF05));
    ImGui::SameLine();
    ImGui::Indent(80.f);
    ImGui::Text("TMA: %04x", core.coremmu->readByte(0xFF06));
    ImGui::SameLine();
    ImGui::Indent(80.f);
    ImGui::Text("TAC: %04x", core.coremmu->readByte(0xFF07));
    ImGui::SameLine();
    ImGui::Indent(80.f);
    ImGui::NewLine();
    ImGui::Unindent(240.f);
    ImGui::Checkbox("Z", &flagz);
    ImGui::SameLine();
    ImGui::Checkbox("N", &flagn);
    ImGui::SameLine();
    ImGui::Checkbox("H", &flagh);
    ImGui::SameLine();
    ImGui::Checkbox("C", &flagc);
    ImGui::SameLine();
    ImGui::End();
}

void step()
{
    paused = false;
    core.corecpu->executenextopcode(core.coremmu->readByte(core.corecpu->pc++));
    paused = true;
}

void menubar()
{
    if (ImGui::BeginMainMenuBar())
    {
	if (ImGui::BeginMenu("File"))
	{
	    if (ImGui::MenuItem("Load ROM..."))
	    {
		if (!disabled)
		{		
		    initcore();		
		    playing = true;
		    disabled = true;
		}
	    }

	    if (ImGui::MenuItem("Close ROM..."))
	    {
		if (disabled)
		{		
		    stopcore();		
		    playing = false;
		    disabled = false;
		    screenenabled = false;
		}
	    }

	    if (ImGui::MenuItem("Quit..."))
	    {
		exit(0);
	    }

	    ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Emulation"))
	{
	    if (ImGui::MenuItem("Pause"))
	    {
		paused = !paused;
	    }

	    if (ImGui::MenuItem("Reset"))
	    {
		if (disabled)
		{		
		    stopcore();		
		    playing = false;		
		    resetcore();
		    playing = true;
		    cout << "True" << endl;
		}
	    }

	    if (ImGui::BeginMenu("Set Mode..."))
	    {
		if (ImGui::MenuItem("Auto"))
		{
		    if (!disabled)
		    {
			core.coremmu->gameboy = Console::Default;
			cout << "Mode set to Default" << endl;
		    }
		}

		if (ImGui::MenuItem("DMG"))
		{
		    if (!disabled)
		    {
			core.coremmu->ismanual = true;
			core.coremmu->gameboy = Console::DMG;
			cout << "Mode set to DMG" << endl;
		    }
		}

		if (ImGui::MenuItem("CGB"))
		{
		    if (!disabled)
		    {
			core.coremmu->ismanual = true;
			core.coremmu->gameboy = Console::CGB;
			cout << "Mode set to CGB" << endl;
		    }
		}

		ImGui::EndMenu();
	    }

	    ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Debug"))
	{
	    if (ImGui::MenuItem("Step"))
	    {
		step();
	    }

	    if (ImGui::MenuItem("Registers..."))
	    {
		regenabled = !regenabled;
	    }

	    if (ImGui::MenuItem("Screen"))
	    {
		screenenabled = !screenenabled;
	    }

	    if (ImGui::MenuItem("Tiles"))
	    {
		tilesenabled = !tilesenabled;
	    }

	    if (ImGui::MenuItem("Dump memory..."))
	    {
		core.dumpmemory("memory.bin");
	    }

	    if (ImGui::MenuItem("Dump VRAM..."))
	    {
		core.dumpvram("vram.bin");
	    }

	    ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Help"))
	{
	    if (ImGui::MenuItem("About..."))
	    {

	    }

	    ImGui::EndMenu();
	}

	ImGui::EndMainMenuBar();
    } 
}

void rendertiles()
{
    SDL_LockSurface(tilesurface);    

    SDL_Rect pixel = {0, 0, 2, 2};
    SDL_Rect pixel2 = {0, 0, 2, 2};

    for (int i = 0; i < 128; i++)
    {
	pixel.x = (i * 2);
	for (int j = 0; j < 192; j++)
	{
	    pixel.y = (j * 2);
	    uint8_t red = tilebuffer[i + (j * 128)].red;
	    uint8_t green = tilebuffer[i + (j * 128)].green;
	    uint8_t blue = tilebuffer[i + (j * 128)].blue;

	    SDL_FillRect(tilesurface, &pixel, SDL_MapRGBA(tilesurface->format, red, green, blue, 255));
	}
    }

    for (int i = 128; i < 256; i++)
    {
	pixel2.x = (i * 2);
	for (int j = 0; j < 192; j++)
	{
	    pixel2.y = (j * 2);
	    uint8_t red = tilebuffer1[(i - 128) + (j * 128)].red;
	    uint8_t green = tilebuffer1[(i - 128) + (j * 128)].green;
	    uint8_t blue = tilebuffer1[(i - 128) + (j * 128)].blue;

	    SDL_FillRect(tilesurface, &pixel2, SDL_MapRGBA(tilesurface->format, red, green, blue, 255));
	}
    }

    SDL_UnlockSurface(tilesurface);

    SDL_UpdateTexture(tiletex, NULL, tilesurface->pixels, tilesurface->pitch);
}

void renderpixels()
{
    SDL_LockSurface(surface);

    SDL_Rect pixel = {0, 0, 2, 2};

    for (int i = 0; i < screenwidth; i++)
    {
	pixel.x = (i * 2);
	for (int j = 0; j < screenheight; j++)
	{
	    pixel.y = (j * 2);
	    uint8_t red = core.getpixel(i, j).red;
	    uint8_t green = core.getpixel(i, j).green;
	    uint8_t blue = core.getpixel(i, j).blue;

	    SDL_FillRect(surface, &pixel, SDL_MapRGBA(surface->format, red, green, blue, 255));
	}
    }

    SDL_UnlockSurface(surface);

    SDL_UpdateTexture(texture, NULL, surface->pixels, surface->pitch);

    updatetiles();
    rendertiles();
}

void handleinput(SDL_Event *event)
{
    if (event->type == SDL_KEYDOWN)
    {
	switch (event->key.keysym.sym)
	{
	    case SDLK_UP: core.keypressed(Button::Up); break;
	    case SDLK_DOWN: core.keypressed(Button::Down); break;
	    case SDLK_LEFT: core.keypressed(Button::Left); break;
	    case SDLK_RIGHT: core.keypressed(Button::Right); break;
	    case SDLK_a: core.keypressed(Button::A); break;
	    case SDLK_b: core.keypressed(Button::B); break;
	    case SDLK_RETURN: core.keypressed(Button::Start); break;
	    case SDLK_SPACE: core.keypressed(Button::Select); break;
	    case SDLK_n: step(); break;
	}
    }
    else if (event->type == SDL_KEYUP)
    {
	switch (event->key.keysym.sym)
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

void runcore()
{
    if (playing)
    {
	if (!paused)
	{
	    core.runcore();
	}
	renderpixels();
    }
}

void processevent(const SDL_Event* event)
{
    ImGuiIO& io = ImGui::GetIO();

    int wheel = 0;

    if (event->type == SDL_MOUSEWHEEL)
    {
	wheel = event->wheel.y;
    }
    else if (event->type == SDL_TEXTINPUT)
    {
	io.AddInputCharactersUTF8(event->text.text);
    }

    int mousex = 0;
    int mousey = 0;

    const int buttons = SDL_GetMouseState(&mousex, &mousey);

    io.DeltaTime = (1.0f / 60.0f);
    io.MousePos = ImVec2((float)(mousex), (float)(mousey));
    io.MouseDown[0] = buttons & SDL_BUTTON(SDL_BUTTON_LEFT);
    io.MouseDown[1] = buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);
    io.MouseWheel = (float)(wheel);
}

void guistuff()
{
    ImGui::NewFrame();

    menubar();

    runcore();

    if (screenenabled && disabled)
    {
	screen();
    }

    if (tilesenabled && disabled)
    {
	tiles();
    }

    if (regenabled && disabled)
    {
	regview();
    }

    SDL_SetRenderDrawColor(render, 114, 144, 154, 255);
    SDL_RenderClear(render);

    ImGui::Render();
    ImGuiSDL::Render(ImGui::GetDrawData());

    SDL_RenderPresent(render);
}

int main()
{
    if (!init())
    {
	return 1;
    }

    bool quit = false;
    SDL_Event event;

    Uint32 framecurrenttime;
    Uint32 framestarttime;

    while (!quit)
    {
	ImGuiIO& io = ImGui::GetIO();

	while (SDL_PollEvent(&event))
	{
	    processevent(&event);
	    handleinput(&event);    

	    if (event.type == SDL_QUIT)
	    {
		quit = true;
	    }
	}

	guistuff();

	framecurrenttime = SDL_GetTicks();

	if ((framecurrenttime - framestarttime) < (1000 / 60))
	{
	    SDL_Delay((1000 / 60) - (framecurrenttime - framestarttime));
	}

	framestarttime = SDL_GetTicks();

	fpscount++;

	if (((SDL_GetTicks() - fpstime) >= 1000))
	{
	    fpstime = SDL_GetTicks();
	    stringstream title;
	    title << "mbGB-imgui-" << fpscount << " FPS";
	    SDL_SetWindowTitle(window, title.str().c_str());
	    fpscount = 0;
	}
    }

    shutdown();

    return 0;
}
