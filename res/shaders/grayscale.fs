// Grayscale filter for libmbGB

#version 330 core
in vec2 texture_coord;

out vec4 fragcolor;

uniform sampler2D screen_texture;
uniform int screen_width;
uniform int screen_height;

void main()
{
    vec4 in_color = texture(screen_texture, texture_coord);
    vec4 out_color = vec4(0, 0, 0, 0);

    out_color.r = (in_color.r * 0.299) + (in_color.g * 0.587) + (in_color.b * 0.114);
    out_color.g = out_color.r;
    out_color.b = out_color.r;

    fragcolor = out_color;
}