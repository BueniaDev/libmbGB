// libmbGB's GLSL implementation of the Scale2x pixel scaling filter

#version 330 core
in vec2 texture_coord;

out vec4 fragcolor;

uniform sampler2D screen_texture;
uniform int screen_width;
uniform int screen_height;

bool get_texel(in float shiftx, in float shifty, out vec4 tex_color, in bool c_pass)
{
    bool pass = c_pass;

    float ratio_w = (1.0 / screen_width);
    float ratio_h = (1.0 / screen_height);

    vec2 tex_coord = texture_coord;

    tex_coord.x += (shiftx * ratio_w);
    tex_coord.y += (-shifty * ratio_h);

    if ((tex_coord.x < 0.0) || (tex_coord.x > 1.0))
    {
	pass = false;
    }

    if ((tex_coord.y < 0.0) || (tex_coord.y > 1.0))
    {
	pass = false;
    }

    if (pass == true)
    {
	tex_color = texture(screen_texture, tex_coord);
    }

    return pass;
}

void main()
{
    vec4 in_color = texture(screen_texture, texture_coord);

    bool color_pass = true;

    vec4 current_color = in_color; // E

    vec4 top_color = current_color; // B
    vec4 bottom_color = current_color; // H
    vec4 left_color = current_color; // D
    vec4 right_color = current_color; // F

    color_pass = get_texel(0.0, 1.0, top_color, color_pass);
    color_pass = get_texel(0.0, -1.0, bottom_color, color_pass);
    color_pass = get_texel(-1.0, 0.0, left_color, color_pass);
    color_pass = get_texel(1.0, 0.0, right_color, color_pass);

    if ((color_pass == true) && (top_color != bottom_color) && (left_color != right_color))
    {
	float quad_x = ((1.0 / screen_width) / 2.0);
	float quad_y = ((1.0 / screen_height) / 2.0);

	float texel_x = (texture_coord.x / quad_x);
	texel_x = mod(texel_x, 2.0);

	float texel_y = (texture_coord.y / quad_y);
	texel_y = mod(texel_y, 2.0);

	// E0
	if ((texel_x <= 1.0) && (texel_y <= 1.0))
	{
	    if (left_color == top_color)
	    {
		current_color = left_color;
	    }
	}
	// E1
	else if ((texel_x > 1.0) && (texel_y <= 1.0))
	{
	    if (right_color == top_color)
	    {
		current_color = right_color;
	    }
	}
	// E2
	else if ((texel_x <= 1.0) && (texel_y > 1.0))
	{
	    if (left_color == bottom_color)
	    {
		current_color = left_color;
	    }
	}
	// E3
	else if ((texel_x > 1.0) && (texel_y > 1.0))
	{
	    if (right_color == bottom_color)
	    {
		current_color = right_color;
	    }
	}
    }

    fragcolor = current_color;
}