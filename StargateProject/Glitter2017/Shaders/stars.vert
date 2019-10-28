#version 410 core

layout(location= 0 ) in vec3 position;
layout(location = 1) in float size;

uniform mat4 MVP;

void main(){
gl_Position = MVP * vec4(position.x , position.y , position.z, 1.0f);
gl_PointSize = float(mod((position.x * position.y),3) +1);
}