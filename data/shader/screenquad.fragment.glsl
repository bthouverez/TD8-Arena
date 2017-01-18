#version 430


in vec2 frag_texcoords;
out vec4 finalColor;

uniform sampler2D image;

void main()
{
	finalColor = vec4(texture(image, frag_texcoords).rgb, 1.0);
}