#version 330 core

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inTex;
out vec2 chTex;

uniform mat4 MVP;

void main()
{
	gl_Position = MVP * vec4(inPos, 0.0, 1.0);
	chTex = inTex;
}