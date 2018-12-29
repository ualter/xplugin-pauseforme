#include "CallBackHandler.h"
#include "XPLMUtilities.h"
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMMenus.h"
#include "XPLMUtilities.h"
#include "XPWidgets.h"
#include "XPStandardWidgets.h"
#include "XPLMCamera.h"
#include "XPUIGraphics.h"
#include "XPWidgetUtils.h"
#include "XPLMPlugin.h"
#include "XPLMNavigation.h"
#include "XPLMDataAccess.h"
#include <string>

using namespace std;

CallBackHandler::CallBackHandler() {
	this->name = "Not Identified yet!";
}

CallBackHandler::~CallBackHandler() {
}

void CallBackHandler::acceptMessage(string origin, string message) {
	std::string logMsg = "CallBackHandler::acceptMessage() from " + origin + " [Command Sent --> " + message + "]";
	XPLMDebugString(logMsg.c_str());

	if (message.find("{IDENTICATION}") != std::string::npos) {
		std::size_t pos = message.find(",");
		if (pos <= 0) {
			this->name = message + " wasn't found the comma!";
		}
		else {
			this->name = message.substr(pos+1);
		}
	} else {
		this->origin   = origin;
		this->command  = message;
		this->executed = 0;
	}
}

string CallBackHandler::getCommand() {
	return this->command;
}

string CallBackHandler::getOrigin() {
	return this->origin;
}

string CallBackHandler::getName() {
	return this->name;
}

void CallBackHandler::commandExecuted() {
	this->executed = 1;
	this->command  = "";
}