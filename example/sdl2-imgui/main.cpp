#include <SDL2/SDL.h>
#include <imgui.h>
#include <imgui_sdl.h>
#include <libmbGB/libmbgb.h>
#include <iostream>
using namespace std;
using namespace gb;

int screenwidth = 160;
int screenheight = 144;
int scale = 4;

int width = (screenwidth * scale);
int height = (screenheight * scale);

SDL_Window *window;
SDL_Renderer *render;

GBCore core;

bool playing = false;
bool disabled = false;

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

    render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (render == NULL)
    {
	cout << "Renderer could not be created! SDL_Error: " << SDL_GetError() << endl;
	return false;
    }

    ImGui::CreateContext();
    ImGuiSDL::Initialize(render, width, height);

    SDL_SetRenderDrawColor(render, 255, 255, 255, 255);
    SDL_RenderClear(render);
    SDL_RenderPresent(render);

    return true;
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
	    exit(1);
	}
    }

    if (!core.loadROM("tetris.gb"))
    {
	exit(1);
    }

    core.init();
}

void stopcore()
{
    core.shutdown();
}

void vramviewer()
{

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

	    }

	    if (ImGui::MenuItem("Reset"))
	    {
		if (disabled)
		{		
		    stopcore();		
		    playing = false;		
		    initcore();
		    playing = true;
		}
	    } 


	    ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Debug"))
	{
	    if (ImGui::MenuItem("VRAM Viewer..."))
	    {

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

void renderpixels()
{
    SDL_Rect pixel = {0, 0, scale, scale};
    for (int i = 0; i < 160; i++)
    {
	pixel.x = (i * scale);
	for (int j = 0; j < 144; j++)
	{
	    pixel.y = (j * scale);
	    uint8_t red = core.getpixel(i, j).red;
	    uint8_t green = core.getpixel(i, j).green;
	    uint8_t blue = core.getpixel(i, j).blue;

	    SDL_SetRenderDrawColor(render, red, green, blue, 255);
	    SDL_RenderFillRect(render, &pixel);
	}
    }
}

void runcore()
{
    if (playing)
    {
	core.runcore();	
	renderpixels();
    }
    else
    {      
	blankscreen();
    }
}

void guistuff()
{
    ImGui::NewFrame();

    menubar();

    runcore();

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

    while (!quit)
    {
	ImGuiIO& io = ImGui::GetIO();

	int wheel = 0;

	while (SDL_PollEvent(&event))
	{
	    if (event.type == SDL_QUIT)
	    {
		quit = true;
	    }
	    else if (event.type == SDL_MOUSEWHEEL)
	    {
		wheel = event.wheel.y;
	    }
	}

	int mousex, mousey;

	int buttons = SDL_GetMouseState(&mousex, &mousey);

	io.DeltaTime = 1.0f / 60.0f;
	io.MousePos = ImVec2((float)(mousex), (float)(mousey));
	io.MouseDown[0] = buttons & SDL_BUTTON(SDL_BUTTON_LEFT);
	io.MouseDown[1] = buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);
	io.MouseWheel = (float)(wheel);

	guistuff();
    }

    shutdown();

    return 0;
}
