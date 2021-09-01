#version 430 core

layout (location = 0) in vec3 positions;


out vec3 frag_position;

uniform mat4 cam;
uniform mat4 proj;
uniform mat4 model;

void main()
{
	gl_Position =  proj * cam* model *vec4(positions,1.0f);
	frag_position = vec3(positions);
}