#pragma once
#include <sstream>

using namespace std;

struct navaid
{
	string id;
	string name;
	string typeDescription;
	float latitude;
	float longitude;
	float distance;
	float dmeDistance;
	float dmeTime;
	string fmsTime;
	int statusOK;
};

