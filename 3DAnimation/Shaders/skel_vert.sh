#version 430 core

layout (location = 0) in vec3 positions;
layout (location = 1) in vec2 text_coords;
layout (location = 2) in vec3 normals;

out vec2 text_coord;
out vec3 normal_vec;
out vec3 frag_position;

uniform mat4 cam;
uniform mat4 animationTransform;
uniform mat4 proj;
uniform mat4 jointTransform;

void main()
{
	vec4 outpos =  animationTransform * jointTransform * vec4(positions,1.0f);
	frag_position = vec3(outpos);
	normal_vec = mat3(transpose(inverse(animationTransform*jointTransform))) * normals;
	gl_Position =  proj * cam*outpos;
	text_coord = text_coords;
}