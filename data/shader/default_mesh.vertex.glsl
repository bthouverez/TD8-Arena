#version 430

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 texcoords;

uniform mat4 PV;//Projection * View
uniform mat4 Model;

out vec3 frag_position;
out vec3 frag_color;
out vec3 frag_normal;
out vec2 frag_texcoords;

void main()
{
	frag_position = position;
	frag_color = color;
	frag_normal = normal;
	frag_texcoords = texcoords;
	gl_Position = PV * Model * vec4(position, 1.0);
}
