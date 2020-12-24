#ifndef MBGB_SDL2_H
#define MBGB_SDL2_H

#include <libmbGB/libmbgb.h>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <iostream>
#include <sstream>
#include <functional>
#include <ctime>
#include <filesystem>
#include "toml.h"
#ifdef LIBMBGB_CAMERA
#include <opencv2/opencv.hpp>
using namespace cv;
#endif // LIBMBGB_CAMERA
using namespace gb;
using namespace std;
using namespace toml;
using namespace std::placeholders;

#define GYRO_LEFT 1
#define GYRO_RIGHT 2
#define GYRO_UP 4
#define GYRO_DOWN 8

Uint32 SDL_GetPixel(SDL_Surface *surface, int x, int y)
{
    if (!surface)
    {
	SDL_SetError("Passed NULL source surface");
	return 0;
    }

    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp)
    {
	case 1:
	    return *p;
	    break;
	case 2:
	    return *(Uint16 *)p;
	    break;
	case 3:
	    if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
		return p[0] << 16 | p[1] << 8 | p[2];
	    else
		return p[0] | p[1] << 8 | p[2] << 16;
		break;
	case 4:
	    return *(Uint32 *)p;
	    break;
	default:
	    return 0;
    }
}

int SDL_FillSurfaceRect(SDL_Surface *dst, const SDL_Rect* rect, SDL_Surface *src)
{
    if (!src)
    {
	return SDL_SetError("Passed NULL source surface");
    }

    Uint8 red = 0;
    Uint8 green = 0;
    Uint8 blue = 0;
    Uint8 alpha = 0;

    Uint32 data = SDL_GetPixel(src, rect->x, rect->y);
    SDL_GetRGBA(data, src->format, &red, &green, &blue, &alpha);

    return SDL_FillRect(dst, rect, SDL_MapRGBA(dst->format, red, green, blue, alpha));
}

class PixelRenderer
{
    public:
	PixelRenderer()
	{

	}

	~PixelRenderer()
	{

	}

	virtual bool init_renderer(string vert, string frag) = 0;
	virtual void shutdown_renderer() = 0;
	virtual void draw_pixels(array<gbRGB, (160 * 144)> arr, int scale) = 0;
	virtual void take_screenshot(string filename) = 0;
};

class OpenGLRenderer : public PixelRenderer
{
    public:
	OpenGLRenderer(GBCore *gbcb, SDL_Window *cb) : core(gbcb), window(cb)
	{

	}

	~OpenGLRenderer()
	{

	}

	bool init_renderer(string vert, string frag)
	{
	    cout << "Initializing renderer..." << endl;
	    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

	    context = SDL_GL_CreateContext(window);

	    GLenum glew_err = glewInit();

	    if (glew_err != GLEW_OK)
	    {
		cout << "GLEW could not be initialized! glewError: " << glewGetErrorString(glew_err) << endl;
		return false;
	    }

	    glDeleteVertexArrays(1, &vao);
	    glDeleteBuffers(1, &vbo);
	    glDeleteBuffers(1, &ebo);

	    GLfloat vertices[] = 
	    {
		1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, 1.0f, 0.0f, 0.0f, 0.0f
	    };

	    GLuint indices[] =
	    {
		0, 1, 3,
		1, 2, 3
	    };

	    glGenVertexArrays(1, &vao);
	    glGenBuffers(1, &vbo);
	    glGenBuffers(1, &ebo);

	    glBindVertexArray(vao);

	    glBindBuffer(GL_ARRAY_BUFFER, vbo);
	    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (5 * sizeof(GLfloat)), (void*)0);
	    glEnableVertexAttribArray(0);

	    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, (5 * sizeof(GLfloat)), (GLvoid*)(3 * sizeof(GLfloat)));
	    glEnableVertexAttribArray(1);

	    glBindVertexArray(0);

	    glGenTextures(1, &lcd_texture);

	    program_id = load_shader(vert, frag);

	    if (program_id == -1)
	    {
		cout << "Error: could not generate shaders" << endl;
		return false;
	    }

	    return true;
	}

	void shutdown_renderer()
	{
	    cout << "Shutting down renderer..." << endl;

	    SDL_GL_DeleteContext(context);
	}

	void draw_pixels(array<gbRGB, (160 * 144)> arr, int scale)
	{
	    if (render_scale != scale)
	    {
		render_scale = scale;
	    }

	    glBindTexture(GL_TEXTURE_2D, lcd_texture);
	    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 160, 144, 0, GL_RGB, GL_UNSIGNED_BYTE, arr.data());
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	    glBindTexture(GL_TEXTURE_2D, 0);
	    
	    glClearColor(0, 0, 0, 0);
	    glClear(GL_COLOR_BUFFER_BIT);

	    glUseProgram(program_id);

	    glActiveTexture(GL_TEXTURE0);
	    glBindTexture(GL_TEXTURE_2D, lcd_texture);
	    glUniform1i(glGetUniformLocation(program_id, "screen_texture"), 0);

	    glBindVertexArray(vao);
	    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	    glBindVertexArray(0);

	    glUseProgram(0);

	    SDL_GL_SwapWindow(window);
	}

	void take_screenshot(string filename)
	{
	    vector<uint8_t> pixels;

	    int width = 0;
	    int height = 0;
	    SDL_GetWindowSize(window, &width, &height);

	    pixels.resize((3 * width * height), 0);

	    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

	    SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormatFrom(pixels.data(), width, height, 24, (3 * width), SDL_PIXELFORMAT_RGB24);

	    SDL_InvertSurfaceVert(surf);
	    SDL_SaveBMP(surf, filename.c_str());
	    SDL_FreeSurface(surf);
	}

	// Custom functions for inverting the vertical orientation of an SDL_Surface
	// Thanks to ar2015 for the solution
	int SDL_LockIfMust(SDL_Surface *surface)
	{
	    return SDL_MUSTLOCK(surface) ? SDL_LockSurface(surface) : 0;
	}

	void SDL_UnlockIfMust(SDL_Surface *surface)
	{
	    if (SDL_MUSTLOCK(surface))
	    {
		SDL_UnlockSurface(surface);
	    }
	}

	int SDL_InvertSurfaceVert(SDL_Surface *surface)
	{
	    Uint8 *t;
	    Uint8 *a, *b;
	    Uint8 *last;
	    Uint16 pitch;

	    if (SDL_LockIfMust(surface) < 0)
	    {
		return -2;
	    }

	    if (surface->h < 2)
	    {
		SDL_UnlockIfMust(surface);
		return 0;
	    }

	    pitch = surface->pitch;
	    t = (Uint8*)malloc(pitch);

	    if (t == NULL)
	    {
		SDL_UnlockIfMust(surface);
		return -2;
	    }

	    memcpy(t, surface->pixels, pitch);

	    a = (Uint8*)surface->pixels;
	    last = a + pitch * (surface->h - 1);
	    b = last;

	    while (a < b)
	    {
		memcpy(a, b, pitch);
		a += pitch;
		memcpy(b, a, pitch);
		b -= pitch;
	    }

	    memmove(b, (b + pitch), (last - b));
	    memcpy(last, t, pitch);
	    free(t);
	    SDL_UnlockIfMust(surface);
	    return 0;
	}

	SDL_Window *window = NULL;
	GBCore *core = NULL;

	int render_scale = 0;

	SDL_GLContext context;

	GLuint vao;
	GLuint vbo;
	GLuint ebo;

	GLuint lcd_texture;
	GLuint program_id;

	GLuint load_shader(string vertex_shader_file, string fragment_shader_file)
	{
	    GLuint vert_id = glCreateShader(GL_VERTEX_SHADER);
	    GLuint frag_id = glCreateShader(GL_FRAGMENT_SHADER);

	    string vs_code = "";
	    string fs_code = "";
	    string temp = "";

	    ifstream vs_data;
	    ifstream fs_data;

	    vs_data.open(vertex_shader_file.c_str());

	    if (!vs_data.is_open())
	    {
		cout << "OpenGL error: could not open vertex shader file" << endl;
		return -1;
	    }

	    while (getline(vs_data, temp))
	    {
		vs_code += "\n" + temp;
	    }

	    temp = "";
	    vs_data.close();

	    fs_data.open(fragment_shader_file.c_str());

	    if (!fs_data.is_open())
	    {
		cout << "OpenGL error: could not open vertex shader file" << endl;
		return -1;
	    }

	    while (getline(fs_data, temp))
	    {
		fs_code += "\n" + temp;
	    }

	    temp = "";
	    fs_data.close();

	    GLint result = GL_FALSE;
	    int log_length;

	    cout << "Compiling vertex shader: " << vertex_shader_file << endl;

	    const char* vs_code_ptr = vs_code.c_str();

	    glShaderSource(vert_id, 1, &vs_code_ptr, NULL);
	    glCompileShader(vert_id);

	    glGetShaderiv(vert_id, GL_COMPILE_STATUS, &result);
	    glGetShaderiv(vert_id, GL_INFO_LOG_LENGTH, &log_length);

	    vector<char> vs_error(log_length);
	    glGetShaderInfoLog(vert_id, log_length, NULL, &vs_error[0]);

	    cout << "Vertex shader error log: " << vs_error.data() << endl;

	    cout << "Compiling fragment shader: " << fragment_shader_file << endl;

	    const char* fs_code_ptr = fs_code.c_str();

	    glShaderSource(frag_id, 1, &fs_code_ptr, NULL);
	    glCompileShader(frag_id);

	    glGetShaderiv(frag_id, GL_COMPILE_STATUS, &result);
	    glGetShaderiv(frag_id, GL_INFO_LOG_LENGTH, &log_length);

	    vector<char> fs_error(log_length);
	    glGetShaderInfoLog(frag_id, log_length, NULL, &fs_error[0]);

	    cout << "Fragment shader error log: " << fs_error.data() << endl;

	    cout << "Linking shaders..." << endl;

	    GLuint prog_id = glCreateProgram();
	    glAttachShader(prog_id, vert_id);
	    glAttachShader(prog_id, frag_id);
	    glLinkProgram(prog_id);

	    glGetProgramiv(prog_id, GL_LINK_STATUS, &result);
	    glGetProgramiv(prog_id, GL_INFO_LOG_LENGTH, &log_length);
	    vector<char> prog_error(log_length);
	    glGetProgramInfoLog(prog_id, log_length, NULL, &prog_error[0]);

	    cout << "Linking error log: " << prog_error.data() << endl;

	    glDeleteShader(vert_id);
	    glDeleteShader(frag_id);

	    return prog_id;
	}
};

class SoftwareRenderer : public PixelRenderer
{
    public:
	SoftwareRenderer(GBCore *gbcb, SDL_Window *cb) : core(gbcb), window(cb)
	{

	}

	~SoftwareRenderer()
	{

	}

	bool init_renderer(string vert, string frag)
	{
	    pix_surface = SDL_GetWindowSurface(window);
	    return true;
	}

	void shutdown_renderer()
	{

	}

	void draw_pixels(array<gbRGB, (160 * 144)> arr, int scale)
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

	    		SDL_FillRect(pix_surface, &pixel, SDL_MapRGB(pix_surface->format, red, green, blue));
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

	    		SDL_FillRect(pix_surface, &pixel, SDL_MapRGB(pix_surface->format, red, green, blue));
		    }
    		}
    
    	    }

	    SDL_UpdateWindowSurface(window);
	}

	void take_screenshot(string filename)
	{
	    SDL_SaveBMP(pix_surface, filename.c_str());
	}

	SDL_Window *window = NULL;
	SDL_Surface *pix_surface = NULL;
	GBCore *core = NULL;
};

#endif // MBGB_SDL2_H