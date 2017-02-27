#version 430


in vec2 frag_texcoords;
out vec4 finalColor;

uniform sampler2D image;

void main()
{
	vec4 data = texture(image, frag_texcoords);
	if (data.w == 0.0)
		discard;
    finalColor = vec4(data.rgb, 1.0);
}