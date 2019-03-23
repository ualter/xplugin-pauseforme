#pragma once
#include "Navaid.h"
#include <list>

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

	struct airwayNavaid_t {
		std::string id;
		float latitude;
		float longitude;
	};

	struct airway_t {
		std::string id;
		std::string sequence;
		std::list<airwayNavaid_t> navaids;
	};
	
	airway_t readingAirway(std::string xplaneFileAirways, std::string strAirway, std::string strBeginNavaid, std::string strEndNavaid);
};

