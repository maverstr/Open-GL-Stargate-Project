#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;

void main()
{
	//only transforms vertices coord to world space coord and send it to geometry shader
    gl_Position = model * vec4(aPos, 1.0);
}  