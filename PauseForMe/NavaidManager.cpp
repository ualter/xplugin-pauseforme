/* First define the platform */
#define APL 0
#define IBM 1

#pragma warning(disable: 4996)
#pragma warning(disable: 4244)
#pragma warning(disable: 4267)


#include "NavaidManager.h"
#include "XPLMNavigation.h"
#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"
#include <sstream>
#include "Navaid.h"
#include <math.h>
#include <vector>
#include <fstream>

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

void NavaidManager::calculateTime(int time[3], int speed, int distance) {
	if (speed > 9) {
		// convert nautical miles to km
		speed = speed * 1.852;

		int   hourI = distance / speed;
		float hourF = ((float)distance / speed) - hourI;
		int   minuI = hourF * 60;
		float minuF = (hourF * 60) - minuI;
		int   secoI = minuF * 60;
		float secoF = (minuF * 60) - secoI;
		int   milim = secoF * 1000;

		time[0] = hourI;
		time[1] = minuI;
		time[2] = secoI;
	}
	else {
		time[0] = 99;
		time[1] = 99;
		time[2] = 99;
	}
}
void NavaidManager::calculateTimeFormatted(char* output, int speed, int distance) {

	int arrayTime[3];
	calculateTime(arrayTime, speed, distance);
	char buffer[10];
	sprintf(buffer, "%02d:%02d:%02d", arrayTime[0], arrayTime[1], arrayTime[2]);
	strncpy(output, buffer, 10);
}

std::vector<string> splitStringBy(std::string line) {
	std::stringstream lineToSplit(line);
	std::string segment;
	std::vector<std::string> seglist;
	while (std::getline(lineToSplit, segment, ','))
	{
		seglist.push_back(segment);
	}
	return seglist;
}


NavaidManager::airway_t NavaidManager::readingAirway(std::string xplaneFileAirways, ::string strAirway, std::string strBeginNavaid, std::string strEndNavaid)
{
	std::string line;
	std::string seekAirway    = "A," + strAirway;
	std::string seekBegNavaid = "S," + strBeginNavaid;
	std::string seekEndNavaid = "S," + strEndNavaid;
	char inputFilename[]      = "c:/X-Plane 11/Custom Data/GNS430/navdata/ATS.txt";

	ifstream inFile;
	inFile.open(inputFilename);

	bool readingAirway = false;
	bool foundBegNavaid = false;
	bool foundEndNavaid = false;

	airway_t airway;

	if (inFile) {
		size_t pos;
		while (inFile.good() && !foundEndNavaid) {
			getline(inFile, line);

			if (line.empty() && readingAirway) {
				// End of an Airway (if were reading a found one before)
				readingAirway = false;
				foundBegNavaid = false;
				foundEndNavaid = false;
				if (!foundEndNavaid) {
					airway.navaids.clear();
				}
			}
			else if (readingAirway) {
				// Already found the Airway and reading its Navaid's lines
				if (foundBegNavaid) {
					airwayNavaid_t navaid;
					std::vector<string> splittedLine = splitStringBy(line);
					navaid.id = splittedLine.at(1);
					navaid.latitude = atof(splittedLine.at(2).c_str());
					navaid.longitude = atof(splittedLine.at(3).c_str());
					airway.navaids.push_back(navaid);

					// Check if this is the End Navaid Searched
					pos = line.find(seekEndNavaid);
					if (pos != std::string::npos)
					{
						foundEndNavaid = true;
					}
				}
				else {
					pos = line.find(seekBegNavaid);
					if (pos != std::string::npos)
					{
						foundBegNavaid = true;
						airwayNavaid_t navaid;
						std::vector<string> splittedLine = splitStringBy(line);
						navaid.id = splittedLine.at(1);
						navaid.latitude = atof(splittedLine.at(2).c_str());
						navaid.longitude = atof(splittedLine.at(3).c_str());
						airway.navaids.push_back(navaid);
					}
				}
			}
			else {
				// Check if found the searched airway
				pos = line.find(seekAirway);
				if (pos != std::string::npos) {
					// Searched Airway found
					readingAirway = true;
					std::vector<string> splittedLine = splitStringBy(line);
					airway.id = splittedLine.at(1);
					airway.sequence = splittedLine.at(2);
				}
			}
		}
	}
	else {
		//"FILE NOT FOUND!!!
	}
	return airway;
}

