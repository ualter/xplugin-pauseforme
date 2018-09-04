#pragma once
#include "Navaid.h"

class NavaidManager
{
public:
	navaid getNearest();
	navaid getLatitudeLongitude(float currentLatitude, float currentLongitude, const char * id, int typeNavaid);
	float calculateDistanceBetweenNavaids(navaid nav1, navaid nav2);
	NavaidManager();
	virtual ~NavaidManager();
};

