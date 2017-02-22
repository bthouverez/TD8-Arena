#version 430

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

uniform mat4 PV;//Projection * View
uniform vec3 laserCenterPosition; // position du centre du segment
uniform vec3 laserDirection; // vecteur unitaire
uniform float laserLength;// longueur totale du laser

out vec3 frag_color;

void main()
{
  vec3 pos;
  if (position.x < 0.0)  
    pos = laserCenterPosition - (laserLength * 0.5) * laserDirection;
  else 
    pos = laserCenterPosition + (laserLength * 0.5) * laserDirection; 

	frag_color = color;
	gl_Position = PV * vec4(pos, 1.0);
}
