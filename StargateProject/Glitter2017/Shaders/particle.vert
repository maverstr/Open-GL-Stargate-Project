#version 330 core
layout (location = 0) in vec3 aPos;

out vec4 ParticleColor;

uniform mat4 projection;
uniform mat4 view;
uniform vec3 offset;
uniform vec4 color;

void main()
{
    float scale = 0.045f;
    ParticleColor = color;
	
    gl_Position = projection * view *  vec4((aPos * scale) + offset, 1.0);
}