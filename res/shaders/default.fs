// Default shader

#version 330 core
in vec2 texture_coord;

out vec4 fragcolor;

uniform sampler2D screen_texture;

void main()
{
    fragcolor = texture(screen_texture, texture_coord);
}