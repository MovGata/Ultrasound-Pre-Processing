R"(#version 120

attribute vec2 position;
attribute vec2 texcoord;

uniform mat4 projection;
uniform mat4 modelview;

varying vec2 texcoordout;

void main()
{
    gl_Position = projection * modelview * vec4(position, 0.0, 1.0);
    texcoordout = texcoord;
})"