// libmbGB's GLSL implementation of the Scale3x pixel scaling filter

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

    vec4 top_left_color = current_color; // A
    vec4 top_color = current_color; // B
    vec4 top_right_color = current_color; // C
    vec4 bottom_left_color = current_color; // G
    vec4 bottom_color = current_color; // H
    vec4 bottom_right_color = current_color; // I
    vec4 left_color = current_color; // D
    vec4 right_color = current_color; // F

    color_pass = get_texel(-1.0, 1.0, top_left_color, color_pass);
    color_pass = get_texel(0.0, 1.0, top_color, color_pass);
    color_pass = get_texel(1.0, 1.0, top_right_color, color_pass);
    color_pass = get_texel(-1.0, -1.0, bottom_left_color, color_pass);
    color_pass = get_texel(0.0, -1.0, bottom_color, color_pass);
    color_pass = get_texel(1.0, -1.0, bottom_right_color, color_pass);
    color_pass = get_texel(-1.0, 0.0, left_color, color_pass);
    color_pass = get_texel(1.0, 0.0, right_color, color_pass);

    if ((color_pass == true) && (top_color != bottom_color) && (left_color != right_color))
    {
	float quad_x = ((1.0 / screen_width) / 3.0);
	float quad_y = ((1.0 / screen_height) / 3.0);

	float texel_x = (texture_coord.x / quad_x);
	texel_x = mod(texel_x, 3.0);

	float texel_y = (texture_coord.y / quad_y);
	texel_y = mod(texel_y, 3.0);

	// Column 0: E0, E3, E6
	if (texel_x <= 1.0)
	{
	    // E0
	    if (texel_y <= 1.0)
	    {
		if (left_color == top_color)
		{
		    current_color = left_color;
		}
	    }
	    // E3
	    else if (texel_y <= 2.0)
	    {
		if ((left_color == top_color) && (current_color != bottom_left_color))
		{
		    current_color = left_color;
		}
		else if ((left_color == bottom_color) && (current_color != top_left_color))
		{
		    current_color = left_color;
		}
	    }
	    // E6
	    else if (texel_y > 2.0)
	    {
		if (left_color == bottom_color)
		{
		    current_color = left_color;
		}
	    }
	}
	// Column 1: E1, E4, E7
	else if (texel_x <= 2.0)
	{
	    // E1
	    if (texel_y <= 1.0)
	    {
		if ((left_color == top_color) && (current_color != top_right_color))
		{
		    current_color = top_color;
		}
		else if ((right_color == top_color) && (current_color != top_left_color))
		{
		    current_color = top_color;
		}
	    }
	    // E7
	    else if (texel_y > 2.0)
	    {
		if ((left_color == bottom_color) && (current_color != bottom_right_color))
		{
		    current_color = bottom_color;
		}
		else if ((right_color == bottom_color) && (current_color != bottom_left_color))
		{
		    current_color = bottom_color;
		}
	    }
	}
	// Column 2: E2, E5, E8
	else
	{
	    // E2
	    if (texel_y <= 1.0)
	    {
		if (right_color == top_color)
		{
		    current_color = right_color;
		}
	    }
	    // E5
	    else if (texel_y <= 2.0)
	    {
		if ((right_color == top_color) && (current_color != bottom_right_color))
		{
		    current_color = right_color;
		}
		else if ((right_color == bottom_color) && (current_color != top_right_color))
		{
		    current_color = right_color;
		}
	    }
	    // E8
	    else
	    {
		if (right_color == bottom_color)
		{
		    current_color = right_color;
		}
	    }
	}
    }

    fragcolor = current_color;
}