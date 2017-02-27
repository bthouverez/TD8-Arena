#version 430

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoords;

out vec2 frag_texcoords;


void main()
{
	frag_texcoords = texcoords;
	gl_Position = vec4(position, 1.0);
}