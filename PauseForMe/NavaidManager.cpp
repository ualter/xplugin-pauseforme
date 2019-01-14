/* First define the platform */
#define APL 0
#define IBM 1

#include "NavaidManager.h"
#include "XPLMNavigation.h"
#include "XPLMDataAccess.h"
#include <sstream>
#include "Navaid.h"
#include <math.h>

#define PI 3.14159265f
#define EARTH_RADIO_KM 6371 // radius of earth in kilometres

using namespace std;

NavaidManager::NavaidManager()
{
}


NavaidManager::~NavaidManager()
{
}

navaid NavaidManager::getNearest()
{
	navaid retorno;
	retorno.name = "Not Found";
	float lat = XPLMGetDataf(XPLMFindDataRef("sim/flightmodel/position/latitude"));
	float lon = XPLMGetDataf(XPLMFindDataRef("sim/flightmodel/position/longitude"));
	/* Find the nearest airport to us. */
	XPLMNavRef	ref = XPLMFindNavAid(NULL, NULL, &lat, &lon, NULL, xplm_Nav_Airport);
	if (ref != XPLM_NAV_NOT_FOUND)
	{
		char	id[10];
		char	name[256];
		//char	buf[256];
		XPLMGetNavAidInfo(ref, NULL, &lat, &lon, NULL, NULL, NULL, id, name, NULL);
		retorno.id = id;
		retorno.name = name;
	}
	return retorno;
}

navaid NavaidManager::getLatitudeLongitude(float currentLatitude, float currentLongitude, const char * id, int typeNavaid)
{
	navaid navRetorno;
	//XPLMNavRef	refTeste = XPLMFindNavAid(NULL, "SBBE", NULL, NULL, NULL, xplm_Nav_Airport);


	/*XPLMNavRef	ref = XPLMFindNavAid(NULL, id, NULL, NULL, NULL, 
		xplm_Nav_Airport | xplm_Nav_NDB | xplm_Nav_VOR | xplm_Nav_Fix | xplm_Nav_DME );*/

	//XPLMNavRef	ref = XPLMFindNavAid(NULL, id, &currentLatitude, &currentLongitude, NULL, xplm_Nav_NDB);

	XPLMNavRef	ref = XPLMFindNavAid(NULL, id, &currentLatitude, &currentLongitude, NULL, typeNavaid);
	if (ref != XPLM_NAV_NOT_FOUND)
	{
		float latitude, longitude;
		char	id[10];
		char	name[256];
		XPLMGetNavAidInfo(ref, NULL, &latitude, &longitude, NULL, NULL, NULL, id, name, NULL);
		navRetorno.id = id;
		navRetorno.name = name;
		navRetorno.latitude = latitude;
		navRetorno.longitude = longitude;
		navRetorno.distance = 9999;
		navRetorno.statusOK = 1;
	} else {
		navRetorno.name = "NotFound";
		navRetorno.statusOK = 0;
	}
	return navRetorno;
}

float NavaidManager::calculateDistanceBetweenNavaids(navaid navCurrent, navaid navOther) {
	float sin1 = sinf(navCurrent.latitude*PI / 180);
	float sin2 = sinf(navOther.latitude*PI / 180);
	float cos1 = cosf(navCurrent.latitude*PI / 180);
	float cos2 = cosf(navOther.latitude*PI / 180);
	float cos3 = cosf((navOther.longitude*PI / 180) - (navCurrent.longitude*PI / 180));

	float distanceKm = acos((sin1 * sin2) + (cos1 * cos2  * cos3)) * EARTH_RADIO_KM;
	float distanceNm = ceilf(distanceKm / 1.852f);

	return distanceNm;
}

