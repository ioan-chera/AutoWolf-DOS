#include "wl_def.h"

int CentreAngle(int whatangle, int centangle)
{
	int dangle = whatangle - centangle;
	if(dangle > 180)
		dangle -= 360;
	else if(dangle <= -180)
		dangle += 360;
	return dangle;
}