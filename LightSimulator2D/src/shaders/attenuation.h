// Light attenuation constants
const float constantAtt = 0.4;
const float linearAtt = 1.0;
const float quadraticAtt = 5.0;

float Attenuate(float dist)
{
	return 1.0 / (constantAtt + (linearAtt * dist) + (quadraticAtt * dist * dist));
}