#version 430 core
out vec4 pixel_color;
in vec3 frag_position;

void main()
{
  pixel_color = vec4(0.6,0.6,0.6,1.0f);
}