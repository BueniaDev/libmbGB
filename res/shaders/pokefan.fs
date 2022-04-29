// Implementation of Pokefan531's "gbc-color" shader (ported from libretro)

#version 330 core
in vec2 texture_coord;

out vec4 fragcolor;

#define target_gamma 2.2
#define display_gamma 2.2
#define sat 1.0
#define lum 0.94
#define contrast 1.0
#define blr 0.0
#define blg 0.0
#define blb 0.0
#define r 0.82
#define g 0.665
#define b 0.73
#define rg 0.125
#define rb 0.195
#define gr 0.24
#define gb 0.075
#define br -0.06
#define bg 0.21

uniform sampler2D screen_texture;
uniform int screen_width;
uniform int screen_height;

void main()
{
    vec4 screen = pow(texture(screen_texture, texture_coord), vec4(2.2 + (1.0 * -1.0))).rgba;
    vec4 avglum = vec4(0.5);
    screen = mix(screen, avglum, (1.0 - contrast));

    mat4 color = mat4(r, rg, rb, 0.0,
			gr, g, gb, 0.0,
			br, bg, b, 0.0,
			blr, blg, blb, 0.0);

    mat4 adjust = mat4((1.0 - sat) * 0.3086 + sat, (1.0 - sat) * 0.3086, (1.0 - sat) * 0.3086, 1.0,
    (1.0 - sat) * 0.6094, (1.0 - sat) * 0.6094 + sat, (1.0 - sat) * 0.6094, 1.0,
    (1.0 - sat) * 0.0820, (1.0 - sat) * 0.0820, (1.0 - sat) * 0.0820 + sat, 1.0,
    0.0, 0.0, 0.0, 1.0);

    color *= adjust;
    screen = clamp(screen * lum, 0.0, 1.0);
    screen = color * screen;

    fragcolor = pow(screen, vec4(1.0 / display_gamma));
}