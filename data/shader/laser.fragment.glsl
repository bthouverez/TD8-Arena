#version 430

in vec3 frag_color;

out vec3 finalColor;


void main()
{	
	finalColor = clamp(frag_color, 0.0, 1.0);

	// gamma corr:
	const float g = 1.0/2.2;
	finalColor.r = pow(finalColor.r, g);
	finalColor.g = pow(finalColor.g, g);
	finalColor.b = pow(finalColor.b, g);
}
