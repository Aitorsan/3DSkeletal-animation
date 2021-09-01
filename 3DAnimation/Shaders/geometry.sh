#version 430 core
layout (points) in;
layout (line_strip, max_vertices = 2) out;


void main()
{  

	gl_Position = gl_in[0].gl_Position;
	texture_coordinates = gs_in[0].texture_coordinates.xy+cursor_inside;
	EmitVertex();
		gl_Position = gl_in[1].gl_Position;
		texture_coordinates =texture_coordinates = gs_in[1].texture_coordinates.xy+cursor_inside;
	EmitVertex();
	gl_Position = gl_in[2].gl_Position;
	texture_coordinates =texture_coordinates = gs_in[2].texture_coordinates.xy +cursor_inside;
	EmitVertex();
	

	EndPrimitive();
}