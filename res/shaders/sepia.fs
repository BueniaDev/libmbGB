// Sepia filter for libmbGB

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

    out_color.r = (in_color.r * 0.393) + (in_color.g * 0.769) + (in_color.b * 0.189);
    out_color.g = (in_color.r * 0.349) + (in_color.g * 0.686) + (in_color.b * 0.168);
    out_color.b = (in_color.r * 0.272) + (in_color.g * 0.534) + (in_color.b * 0.131);

    fragcolor = out_color;
}