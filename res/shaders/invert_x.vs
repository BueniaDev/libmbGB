// Vertex shader for a basic X coordinate flip

#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex_coord;

out vec2 texture_coord;

void main()
{
    vec4 pos = vec4(pos, 1.0f);
    pos.x = -pos.x;

    gl_Position = pos;
    texture_coord = tex_coord;
}