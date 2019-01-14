#pragma once
#include <sstream>

using namespace std;

struct navaid
{
	string id;
	string name;
	float latitude;
	float longitude;
	float distance;
	float dmeDistance;
	float dmeTime;
	int statusOK;
};

