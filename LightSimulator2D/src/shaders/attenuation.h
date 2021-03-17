uniform vec3 u_Attenuation;

float Attenuate(float dist)
{
	return 1.0 / (u_Attenuation.x + (u_Attenuation.y * dist) + (u_Attenuation.z * dist * dist));
}