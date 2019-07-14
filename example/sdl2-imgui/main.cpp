#include <SDL2/SDL.h>

#include <imgui.h>
#include <imgui_sdl.h>
#include <iostream>
using namespace std;

int screenwidth = 160;
int screenheight = 144;
int scale = 4;

int width = (screenwidth * scale);
int height = (screenheight * scale);

SDL_Window *window;
SDL_Renderer *render;

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

    SDL_SetRenderDrawColor(render, 139, 172, 15, 255);
    SDL_RenderClear(render);
    SDL_RenderPresent(render);

    return true;
}

void shutdown()
{
    ImGuiSDL::Deinitialize();
    SDL_DestroyRenderer(render);
    SDL_DestroyWindow(window);
    ImGui::DestroyContext();
    SDL_Quit();
}

void menubar()
{
    if (ImGui::BeginMainMenuBar())
    {
	if (ImGui::BeginMenu("File"))
	{
	    if (ImGui::MenuItem("Load ROM..."))
	    {

	    }

	    if (ImGui::MenuItem("Close ROM..."))
	    {

	    }

	    if (ImGui::MenuItem("Quit..."))
	    {
		exit(0);
	    }

	    ImGui::EndMenu();
	}

	ImGui::EndMainMenuBar();
    } 
}

void guistuff()
{
    ImGui::NewFrame();
    menubar();

    SDL_SetRenderDrawColor(render, 139, 172, 15, 255);
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
