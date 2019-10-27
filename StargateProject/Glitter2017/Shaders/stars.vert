#version 410 core

layout(location=0) in vec3 position;

uniform mat4 MVP;

void main(){
gl_Position = MVP * vec4(position.x * 5, position.y * 5 , position.z * 5, 1.0f);
gl_PointSize = 3.0f;
}