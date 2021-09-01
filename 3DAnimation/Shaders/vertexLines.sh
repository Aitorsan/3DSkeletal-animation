#version 430 core

layout (location = 0) in vec4 positions;

uniform mat4 cam;
uniform mat4 proj;

void main()
{
	 
  gl_Position = proj * cam * positions;
}