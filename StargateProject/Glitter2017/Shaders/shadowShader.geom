#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;
//take 1 triangle as input and output 6 triangles (18 vertices)

uniform mat4 shadowMatrices[6];

out vec4 FragPos;

//Generate each triangle by transforming world-space coord to light space
//multiplying FragPos with the face's light-space transformation matrix.
void main()
{
    for(int face = 0; face < 6; ++face)
    {
        gl_Layer = face; // built-in variable that specifies to which face we render (cubemap)
        for(int i = 0; i < 3; ++i) // for each triangle's vertices
        {
            FragPos = gl_in[i].gl_Position; //sent as world coordinate to calculate depth in fragment shader
            gl_Position = shadowMatrices[face] * vec4(vec3(FragPos), 1.0);
            EmitVertex();
        }    
        EndPrimitive();
    }
}  