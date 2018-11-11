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
}

CallBackHandler::~CallBackHandler() {
}

void CallBackHandler::acceptMessage(string message) {
	std::string logMsg = "CallBackHandler::acceptMessage() Command Sent --> " + message;
	XPLMDebugString( logMsg.c_str() );

	this->command = message;
	this->executed = 0;
}

string CallBackHandler::getCommand() {
	return this->command;
}

void CallBackHandler::commandExecuted() {
	this->executed = 1;
	this->command  = "";
}