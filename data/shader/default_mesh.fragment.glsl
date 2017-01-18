#version 430

in vec3 frag_position;
in vec3 frag_color;
in vec3 frag_normal;
in vec2 frag_texcoords;

out vec3 finalColor;


uniform sampler2D tex;
uniform int use_texture;
uniform vec3 lightColor;
uniform vec3 lightPosition;
uniform vec3 cameraPosition;

const float m_spec = 16.0;// exposant spéculaire (par défaut)


void main()
{	
	vec3 color = frag_color;
	if (use_texture)
	{
		color = texture(tex, frag_texcoords).rgb;
	}

	vec3 N = normalize(frag_normal);
	vec3 L = normalize(lightPosition - frag_position);
	vec3 V = normalize(cameraPosition - frag_position);
	float cosTheta = max(0.0, dot(N, L));
	vec3 H = normalize(L + V);
	float cosSpec = max(0.0, dot(N, H));
	finalColor = (0.68 * color + 0.32 * pow(cosSpec, m_spec) ) * lightColor * cosTheta ;
}
