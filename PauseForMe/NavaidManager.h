#pragma once
#include "Navaid.h"

class NavaidManager
{
public:
	navaid getNearest();
	navaid getLatitudeLongitude(float currentLatitude, float currentLongitude, const char * id, int typeNavaid);
	void calculateTime(int time[3], int speed, int distance);
	void calculateTimeFormatted(char* output, int speed, int distance);
	float calculateDistanceBetweenNavaids(navaid nav1, navaid nav2);
	NavaidManager();
	virtual ~NavaidManager();
	
};

