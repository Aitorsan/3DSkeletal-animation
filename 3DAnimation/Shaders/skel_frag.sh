#version 430 core

in vec2 text_coord;
in vec3 normal_vec;
in vec3 frag_position;

out vec4 pixel_color;

//light uniforms
uniform	vec3 light_color;
uniform	vec3 light_position;
uniform float light_attenuation;

// camera position
uniform vec3 camera_pos;

// texture
uniform sampler2D texture_image;

vec3 ComputeDiffuse()
{
     vec3 norm = normalize(normal_vec);
     vec3 light_dir = normalize(light_position - frag_position );
     float angle = max ( 0, dot(norm,light_dir));
	 vec3 diffuse = light_color * angle;
	 return diffuse;
}

void main()
{
        
	vec3 diffuse = ComputeDiffuse();

	pixel_color = vec4(diffuse,1.0f) * vec4(1.0,0.2f,0.2f,1.0f);
}