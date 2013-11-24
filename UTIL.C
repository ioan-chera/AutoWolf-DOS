#include "wl_def.h"
#pragma hdrstop

int CentreAngle(int whatangle, int centangle)
{
	int dangle = whatangle - centangle;
	if(dangle > 180)
		dangle -= 360;
	else if(dangle <= -180)
		dangle += 360;
	return dangle;
}

int DirAngle(fixed mx, fixed my, fixed nx, fixed ny)
{
	float dx = (nx - mx);
	float dy = -(ny - my);
	float ang = atan2(dy, dx);
	int rang = (int)(180.0f/PI_F*ang);
	while(rang < 0)
		rang += 360;
	while(rang >= 360)
		rang -= 360;
	return rang/5*5;
}