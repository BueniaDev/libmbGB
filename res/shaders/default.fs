// Default fragment shader

#version 330 core
in vec2 texture_coord;

out vec4 fragcolor;

uniform sampler2D screen_texture;
uniform int screen_width;
uniform int screen_height;

void main()
{
    fragcolor = texture(screen_texture, texture_coord);
}