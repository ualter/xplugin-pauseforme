/**
* PauseForMe
*
* This is a plugin just for help those who likes to put your plane on the correct flying heading and waiting (respecting the time, or not!)
* until certain event happens to PAUSE the game. Some options of events being tracking are:
* - Max/Min Distance to DME (Nav1/Nav2/GPS);
* - Max/Min Time to DME (Nav1/Nav2/GPS);
* - Max/Min Altitude;
* - Max/Min Airspeed; or
* - Alignment with a radial signal (Nav1/Nav2)
*
* What comes first it will trigger the PAUSE. Then, you have the chance to get back the command from the autopilot from that
* point on.
*
* @author: Ualter Otoni Azambuja Junior (ualter.junior@gmail.com)
*
* Aug-09-2014 Beta-v1.0
* Aug-14-2014 v1.5
* Aug-17-2014 v1.5 Win64
* Aug-28-2014 v2.0 Win64
* Aug-31-2014 v2.1 Win64
* Set-04-2014 v2.5 Win64
* Set-21-2022 v2.9 Win64
*
*/

#define APL 0
#define IBM 1

#pragma warning(disable: 4996)
#pragma warning(disable: 4244)
#pragma warning(disable: 4267)
#pragma warning(disable: 4312)

#include "SocketServer.h"
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <algorithm>
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
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <thread>
#include "Coordenada.h"
#include "NavaidManager.h"
#include "Navaid.h"
#include "CallBackHandler.h"
#include <boost/algorithm/string.hpp>


#if IBM
    #include <windows.h>
#endif
#if LIN
    #include <GL/gl.h>
#elif __GNUC__
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif

#ifndef XPLM301
    #error This is made to be compiled against the XPLM301 SDK
#endif


#undef NULL
#ifdef __cplusplus
#  if !defined(__MINGW32__) && !defined(_MSC_VER)
#    define NULL __null
#  else
#    define NULL 0
#  endif
#else
#  define NULL ((void*)0)
#endif

#define CHECKINTERVAL 1 //seconds between checks

static XPWidgetID wCaptionNav1, wCaptionNav2;
static XPWidgetID wMainWindow, wSubWindow, wBtnSave, wBtnReload, wBtnCancel, wBtnStopStartWebSocket;
static XPWidgetID wCaptionFreqNav1, wCaptionFreqNav2, wCaptionDistDmeNav1, wCaptionDistDmeNav2;
static XPWidgetID wTextDistDmeMaxNav1, wTextDistDmeMaxNav2, wTextDistDmeMinNav1, wTextDistDmeMinNav2;
static XPWidgetID wCaptionTimeDmeNav1, wCaptionTimeDmeNav2;
static XPWidgetID wTextTimeDmeMaxNav1, wTextTimeDmeMaxNav2, wTextTimeDmeMinNav1, wTextTimeDmeMinNav2;
static XPWidgetID wChkPauseNav1, wChkPauseNav2;
static XPWidgetID wChkHSIAlignNav1, wChkHSIAlignNav2;
static XPWidgetID wHSIAlignmentNav1, wHSIAlignmentNav2;
static XPWidgetID wCaptionAirspeed, wCaptionAltitude;
static XPWidgetID wTextAirspeedMax, wTextAirspeedMin;
static XPWidgetID wTextAltitudeMax, wTextAltitudeMin;
static XPWidgetID wChkAltitude, wChkAirspeed, wChkGPS;
static XPWidgetID wAlertWindow, wBtnAlertWindowClose;
static XPWidgetID wCaptionGPS, wTextGPSDmeDistanceMin, wTextGPSDmeDistanceMax, wTextGPSDmeTimeMin, wTextGPSDmeTimeMax, wCaptionGPSDmeDistance, wCaptionGPSDmeTime;
static XPWidgetID wChkLatLon, wCaptionLatLon, wTextLatitude, wTextLongitude;


static XPWidgetID wChkNavaidAirport;
static XPWidgetID wTextNavaidAirportID;
static XPWidgetID wCaptionNavaidAirportDistance;
static XPWidgetID wCaptionNavaidAirportDesc;
static XPWidgetID wTextNavaidAirportDistanceMin;

static XPWidgetID wChkNavaidVOR;
static XPWidgetID wTextNavaidVORID;
static XPWidgetID wCaptionNavaidVORDistance;
static XPWidgetID wCaptionNavaidVORDesc;
static XPWidgetID wTextNavaidVORDistanceMin;

static XPWidgetID wChkNavaidNDB;
static XPWidgetID wTextNavaidNDBID;
static XPWidgetID wCaptionNavaidNDBDistance;
static XPWidgetID wCaptionNavaidNDBDesc;
static XPWidgetID wTextNavaidNDBDistanceMin;

static XPWidgetID wChkNavaidFix;
static XPWidgetID wTextNavaidFixID;
static XPWidgetID wCaptionNavaidFixDistance;
static XPWidgetID wCaptionNavaidFixDesc;
static XPWidgetID wTextNavaidFixDistanceMin;

static XPWidgetID wChkNavaidDME;
static XPWidgetID wTextNavaidDMEID;
static XPWidgetID wCaptionNavaidDMEDistance;
static XPWidgetID wCaptionNavaidDMEDesc;
static XPWidgetID wTextNavaidDMEDistanceMin;
static XPWidgetID wDataRef1, wDataRef2, wDataRef3, wDataRefValue1, wDataRefValue2, wDataRefValue3;
static XPWidgetID wChkDataRef1, wChkDataRef2, wChkDataRef3;
static XPWidgetID wCaptionDataRef1, wCaptionDataRef2, wCaptionDataRef3, wTextFlightPlan, wTextTime, wChkTime;
static XPWidgetID wBtnFpTranslate, wBtnFpBringBackOriginal, wBtnPasteFlightPlan, wBtnCopyFlightPlan, wBtnCleanFlightPlan, wBtnSendFlightPlan, wCaptionTime;
static XPWidgetID wTextLoadFlightPlan, wBtnLoadFlightPlan, wLoadFlightPlanResult;

int isDataRef1Selected, isDataRef2Selected, isDataRef3Selected;

static XPWidgetID wChkToUnSelect;

std::string dataRef1;
std::string dataRefValue1;
std::string dataRef2;
std::string dataRefValue2;
std::string dataRef3;
std::string dataRefValue3;

NavaidManager navManager;
navaid navaidGpsDestination;
navaid navaidFmsDestination;
navaid navaidCurrentDestination;

struct xpln_fms_entry {
	float						type;
	char						id[5];
	float						altitude;
	float						lat;
	float						lon;
};
struct xpln_fms_entries {
	char						packet_id[4];
	int						    nb_of_entries;
	int						    active_entry_index;
	int						    destination_entry_index;
	struct xpln_fms_entry		entries[99];
};
struct xpln_fms_entry		fms_entry;
struct xpln_fms_entries		fms_entries;

float userNavaidAirportDistance;
std::string userNavaidAirportID;
int isNavaidAirportSelected;
navaid navaidAirport;

float userNavaidVORDistance;
std::string userNavaidVORID;
int isNavaidVORSelected;
navaid navaidVOR;

float userNavaidNDBDistance;
std::string userNavaidNDBID;
int isNavaidNDBSelected;
navaid navaidNDB;

float userNavaidFixDistance;
std::string userNavaidFixID;
int isNavaidFixSelected;
navaid navaidFix;

float userNavaidDMEDistance;
std::string userNavaidDMEID;
int isNavaidDMESelected;
navaid navaidDME;

static XPWidgetID widgetDebug1, widgetDebug2, widgetDebug3;
static XPLMMenuID id;

int userNav1DistMaxDme = 9999, userNav1DistMinDme = -1;
int userNav1TimeDmeMax = 9999, userNav1TimeDmeMin = -1;
int userNav1IsHSIAlignEnable;
int userNav1IsCheckEnable;
int userNav1HasDme;
int userNav2DistMaxDme = 9999, userNav2DistMinDme = -1;
int userNav2TimeDmeMax = 9999, userNav2TimeDmeMin = -1;
int userNav2IsHSIAlignEnable;
int userNav2IsCheckEnable;
int userNav2HasDme;
int userGPSDmeDistanceMin = -1, userGPSDmeDistanceMax = 999999;
int userGPSDmeTimeMin = -1, userGPSDmeTimeMax = 999999;
int userChkLatLon = 0;

float currentFreqNav1 = 0;
float currentDistDmeNav1 = 9999;
float currentTimeDmeNav1 = 9999;
float currentHsiAlignmentNav1;
float currentFreqNav2 = 0;
float currentDistDmeNav2 = 999;
float currentTimeDmeNav2 = 999;
float currentHsiAlignmentNav2;
int isBatteryOn;
int isGamePaused;
int currentAltitude;
int currentAirspeed;
int currentGPSDistDme;
int currentGPSTimeDme;

int isAltitudeSelected;
int isAirspeedSelected;
int isGPSSelected, isLatLongSelected;
int userAltitudeMin = -1, userAltitudeMax = 99999;
int userAirspeedMin = -1, userAirspeedMax = 99999;
static int MenuItem1;
static int AlertWindowShown;
static int serverSocketStarted = 0;
char debug_message[128];
char msgPause[128];
char msgPause2[128];
std::string fileName = "PauseForMe.ini";
std::string timePause = "";
int isTimePauseSelected = 0;
std::string flightPlan = "";
std::string flightPlanBringBack = "";
std::string xplane11LoadFlightPlan = "";
int hourTimeZ;
int minuTimeZ;
int secoTimeZ;
std::string pathATSFile;
static std::string supposedPathATSFile = "Custom Data/GNS430/navdata/ATS.txt";
std::list<NavaidManager::airwayNavaid_t> listTranslatedFlightPlan;

Coordenada objCurrentLongitude(2), objCurrentLatitude(2), objUserLongitude(2), objUserLatitude(2);
int acceptableDifference = 2;

std::string *xpMovilCommand = nullptr;

//float currentLongitude, currentLatitude;
//std::string  currentLongitudeStr, currentLatitudeStr;

float CallBackXPlane(float  inElapsedSinceLastCall,
	float  inElapsedTimeSinceLastFlightLoop,
	int    inCounter,
	void * inRefcon);
float CallBackXPlaneSocketServer(float  inElapsedSinceLastCall,
	float  inElapsedTimeSinceLastFlightLoop,
	int    inCounter,
	void * inRefcon);
static void PauseForMeMenuHandler(void *, void *);

struct CommandConfigPauseNavaid {
	std::string command;
	std::string navaid;
	std::string type;
	std::string distance;
};

// For Altitude or Airspeed
struct CommandConfigPauseLowerUpper {
	std::string command;
	std::string parameter;
	std::string lower;
	std::string upper;
};

struct CommandConfigPauseTime {
	std::string command;
	std::string time;
};

std::vector<string> splitStringBy(std::string line, std::string delimiter);
CommandConfigPauseNavaid interpretCommandConfigPauseNavaId(std::string line);
CommandConfigPauseLowerUpper interpretCommandConfigPauseLowerUpper(std::string line);
CommandConfigPauseTime        interpretCommandConfigPauseTime(std::string line);

void hideWindowPaused();
void checkPreferenceFile();
void log(std::string s);
void replaceAll(std::string &str, std::string replaceThis, std::string byThis, int compensate);
void encodeForJson(std::string &str);
void resetDataRefsValues();
void switchSocketServer();
void stopSocketServer();
void startSocketServer();
void sendStoppedMessageSocketClients();
void sendStartedMessageSocketClients();
void sendPausedMessageSocketClients();
void sendUnpausedMessageSocketClients();
void sendMessageSocketClients(std::string msg);

void translateFlightPlan();
void bringBackMyFlightPlan();
std::string writeDownFlightPlan();
void getPathATSFile();
bool XSBGetTextFromClipboard(std::string& outText);
bool XSBSetTextToClipboard(const std::string& inText);
void pasteFlightPlan();
void copyFlightPlan();
void cleanFlightPlan();
void readXPlane11FMS();

XPLMCommandRef SetupOnCommand = NULL;
XPLMCommandRef SetupOffCommand = NULL;
XPLMCommandRef SetupStartStopTrasmitter = NULL;

std::thread threadTaskSocketServer;
SocketServer* socketServer;

int SetupOnCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void * inRefcon);
int SetupOffCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void * inRefcon);
int SetupStartStopTrasmitterHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void * inRefcon);

using namespace std::placeholders;

void taskSocketServer(SocketServer* socketServer) {
	log("Starting Socket Server");
	socketServer->start();
}


CallBackHandler* callBackHandler;

PLUGIN_API int XPluginStart(
	char *		outName,
	char *		outSig,
	char *		outDesc)
{
	try {
		checkPreferenceFile();

		//startSocketServer();

		strcpy(outName, "PauseForMe");
		strcpy(outSig, "ualter.otoni.PauseForMe");
		strcpy(outDesc, "A plug-in to pause the X-Plane Flight Simulator at a specific time or event, that you want");

		// Create our commands; these will increment and decrement our custom dataref.
		SetupOnCommand = XPLMCreateCommand("Ualter/PauseForMe/SetupOn", "Open Setup window");
		SetupOffCommand = XPLMCreateCommand("Ualter/PauseForMe/SetupOff", "Close Setup window");
		SetupStartStopTrasmitter = XPLMCreateCommand("Ualter/PauseForMe/StartStopTrasmitter", "START/STOP Transmitter");
		// Register our custom commands
		XPLMRegisterCommandHandler(SetupOnCommand, SetupOnCommandHandler, 1, (void *)0);
		XPLMRegisterCommandHandler(SetupOffCommand, SetupOffCommandHandler, 1, (void *)0);
		//XPLMRegisterCommandHandler(SetupStartStopTrasmitter, SetupStartStopTrasmitterHandler, 1, (void *)0);

		// Build menu
		int item;
		item = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "Pause For Me", NULL, 1);
		id = XPLMCreateMenu("Menu Pause For Me", XPLMFindPluginsMenu(), item, PauseForMeMenuHandler, NULL);
		XPLMAppendMenuItem(id, "Setup", (void *)"Setup", 1);

		// Used by widget to make sure only one widgets instance created
		MenuItem1 = 0;
	}
	catch (const std::exception& e) {
		std::string error = e.what();
		std::string msg = "XPluginStart --> ERROR --> " + error;
		XPLMDebugString(msg.c_str());
	}
	catch (const std::string& e) {
		std::string msg = "XPluginStart --> ERROR --> " + e;
		XPLMDebugString(msg.c_str());
	}
	return 1;
}

static int widgetWidgetHandler(
	XPWidgetMessage			inMessage,
	XPWidgetID				inWidget,
	intptr_t				inParam1,
	intptr_t				inParam2);

std::string formatFreqHz(float number)
{
	std::ostringstream ostr;
	ostr << std::fixed << std::setprecision(0) << number;
	std::string str = ostr.str();
	if (str.length() > 3) {
		str.insert(3, ".");
	}
	str.append("hz");
	return str;
}



std::string convertToString(long number)
{
	/*char * buf = 0;
	buf = (char*)malloc(_CVTBUFSIZE);
	int err;
	err = _gcvt_s(buf, _CVTBUFSIZE, number, units);
	return buf;*/
	std::ostringstream ostr;
	ostr << std::fixed << std::setprecision(0) << number;
	std::string str = ostr.str();
	return str;
}

std::string convertFloatWithDecimalsToString(long number)
{
	return std::to_string(number);
}

std::string formatLatLon(float number)
{
	std::ostringstream ostr;
	ostr << std::fixed << std::setprecision(10) << number;
	std::string str = ostr.str();
	str = str.substr(0, str.find(".") + 3);
	return str;
}

long convertToNumber(std::string str)
{
	long result;
	std::stringstream convert(str);
	convert >> result;
	return result;
}

std::string formatNumber(float number, int decimal)
{
	std::ostringstream ss;
	ss.imbue(std::locale());
	ss << std::fixed << std::setprecision(decimal) << number;
	std::string ret = ss.str();
	return ret;
}

// An opaque handle to the window we will create
static XPLMWindowID	g_window;


void CreateWidgetWindow()
{

	// Set the window's initial bounds
	// Note that we're not guaranteed that the main monitor's lower left is at (0, 0)...
	// We'll need to query for the global desktop bounds!
	int left, bottom, right, top;
	XPLMGetScreenBoundsGlobal(&left, &top, &right, &bottom);

	//int x = 150;
	//int y = 900;
	int x = left + 100;
	int y = top  - 100;
	int w = 940;
	int h = 509;

	int x2 = x + w;
	int y2 = y - h;

	wMainWindow = XPCreateWidget(x, y, x2, y2, 1, "Pause For Me / v2.9", 1, NULL, xpWidgetClass_MainWindow);
	XPSetWidgetProperty(wMainWindow, xpProperty_MainWindowHasCloseBoxes, 1);

	// Window
	wSubWindow = XPCreateWidget(x + 30, y - 40, x2 - 30, y2 + 30, 1, "", 0, wMainWindow, xpWidgetClass_SubWindow);
	XPSetWidgetProperty(wSubWindow, xpProperty_SubWindowType, xpSubWindowStyle_SubWindow);

	int leftX = x;
	int topY = y;
	int rightX = x;
	int bottomY = y;

	int topMargin = 70;
	int leftMargin = 40;
	int widthCaption = 30;
	int gapCaptionField = 6;
	int heightFields = 22;

	int tmpX, tmpY;

	tmpX = x + 88; // Deslocamento eixo lateral
	// Headers
	XPCreateWidget(tmpX, y - 45, tmpX + 90, y - 55, 1, "Freq.", 0, wMainWindow, xpWidgetClass_Caption);
	XPCreateWidget(tmpX, y - 58, tmpX + 90, y - 63, 1, " Hz  ", 0, wMainWindow, xpWidgetClass_Caption);
	tmpX += 139;
	XPCreateWidget(tmpX, y - 45, tmpX + 15, y - 55, 1, "   DME  ", 0, wMainWindow, xpWidgetClass_Caption);
	XPCreateWidget(tmpX, y - 58, tmpX + 15, y - 63, 1, "Distance", 0, wMainWindow, xpWidgetClass_Caption);
	tmpX += 220;
	XPCreateWidget(tmpX, y - 45, tmpX + 15, y - 55, 1, "   DME   ", 0, wMainWindow, xpWidgetClass_Caption);
	XPCreateWidget(tmpX, y - 58, tmpX + 15, y - 63, 1, "   Time   ", 0, wMainWindow, xpWidgetClass_Caption);
	tmpX += 152;
	XPCreateWidget(tmpX, y - 45, tmpX + 15, y - 55, 1, "    HSI    ", 0, wMainWindow, xpWidgetClass_Caption);
	XPCreateWidget(tmpX, y - 58, tmpX + 15, y - 63, 1, "Alignment ", 0, wMainWindow, xpWidgetClass_Caption);
	tmpX += 75;
	XPCreateWidget(tmpX, y - 45, tmpX + 15, y - 55, 1, "  Enable  ", 0, wMainWindow, xpWidgetClass_Caption);
	XPCreateWidget(tmpX, y - 58, tmpX + 15, y - 63, 1, "  Pause   ", 0, wMainWindow, xpWidgetClass_Caption);

	// Nav1 Caption
	leftX = x + leftMargin;
	topY = topY - topMargin - 3;
	rightX = leftX + widthCaption;
	bottomY = topY - heightFields;
	wCaptionNav1 = XPCreateWidget(leftX, topY, rightX, bottomY, 1, "Nav1:", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wCaptionNav1, xpProperty_CaptionLit, 0);
	// Nav1 FreqHz Caption
	leftX += (widthCaption + gapCaptionField);
	int widthField = 50;
	wCaptionFreqNav1 = XPCreateWidget(leftX, topY, leftX + widthField, bottomY, 1, "Nav1", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wCaptionFreqNav1, xpProperty_CaptionLit, 0);
	// Nav1 DME Distance Text Min.
	leftX += 80;
	wTextDistDmeMinNav1 = XPCreateWidget(leftX, topY, leftX + widthField, bottomY, 1, convertToString(userNav1DistMinDme).c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextDistDmeMinNav1, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(wTextDistDmeMinNav1, xpProperty_MaxCharacters, 4);
	XPCreateWidget(leftX + widthField, topY, leftX + widthField + 10, bottomY, 1, ">=", 0, wMainWindow, xpWidgetClass_Caption);
	XPCreateWidget(leftX + widthField - 39, topY + 15, leftX + widthField, bottomY + 15, 1, "Min", 0, wMainWindow, xpWidgetClass_Caption);
	XPCreateWidget(leftX + widthField + 105, topY + 15, leftX + widthField + 115, bottomY + 15, 1, "Max", 0, wMainWindow, xpWidgetClass_Caption);
	// Nav1 DME Distance Caption
	leftX += 72;
	widthField = 50;
	wCaptionDistDmeNav1 = XPCreateWidget(leftX, topY, leftX + widthField, bottomY, 1, "0", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wCaptionDistDmeNav1, xpProperty_CaptionLit, 0);
	XPCreateWidget(leftX + widthField - 1, topY, leftX + widthField + 10 - 1, bottomY, 1, ">=", 0, wMainWindow, xpWidgetClass_Caption);
	// Nav1 DME Distance Text Max.
	leftX += 72;
	widthField = 50;
	wTextDistDmeMaxNav1 = XPCreateWidget(leftX, topY, leftX + widthField, bottomY, 1, convertToString(userNav1DistMaxDme).c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextDistDmeMaxNav1, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(wTextDistDmeMaxNav1, xpProperty_MaxCharacters, 4);
	// Nav1 DME Time Text Min.
	leftX += 78;
	widthField = 50;
	wTextTimeDmeMinNav1 = XPCreateWidget(leftX, topY, leftX + widthField, bottomY, 1, convertToString(userNav1TimeDmeMin).c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextTimeDmeMinNav1, xpProperty_TextFieldType, xpTextEntryField);
	XPCreateWidget(leftX + widthField, topY, leftX + widthField + 10, bottomY, 1, ">=", 0, wMainWindow, xpWidgetClass_Caption);
	// Nav1 DME Time Caption
	leftX += 70;
	widthField = 50;
	wCaptionTimeDmeNav1 = XPCreateWidget(leftX, topY, leftX + widthField, bottomY, 1, "0", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wCaptionTimeDmeNav1, xpProperty_CaptionLit, 0);
	XPCreateWidget(leftX + 43, topY, leftX + widthField + 43 + 10, bottomY, 1, ">=", 0, wMainWindow, xpWidgetClass_Caption);
	// Nav1 DME Time Text Max.
	leftX += 68;
	widthField = 50;
	wTextTimeDmeMaxNav1 = XPCreateWidget(leftX, topY, leftX + widthField, bottomY, 1, convertToString(userNav1TimeDmeMax).c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextTimeDmeMaxNav1, xpProperty_TextFieldType, xpTextEntryField);
	// Nav1 HSI Alignment
	leftX += 82;
	widthField = 50;
	wHSIAlignmentNav1 = XPCreateWidget(leftX, topY, leftX + widthField, bottomY, 1, "0", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wHSIAlignmentNav1, xpProperty_CaptionLit, 0);
	// Nav1 HSI Alignment Checkbox
	leftX += 20;
	widthField = 50;
	wChkHSIAlignNav1 = XPCreateWidget(leftX, topY, leftX + widthField, bottomY, 1, "", 0, wMainWindow, xpWidgetClass_Button);
	XPSetWidgetProperty(wChkHSIAlignNav1, xpProperty_ButtonType, xpRadioButton);
	XPSetWidgetProperty(wChkHSIAlignNav1, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
	XPSetWidgetProperty(wChkHSIAlignNav1, xpProperty_ButtonState, userNav1IsHSIAlignEnable);
	// Nav1 Checkbox
	leftX += 53;
	widthField = 50;
	wChkPauseNav1 = XPCreateWidget(leftX, topY, leftX + widthField, bottomY, 1, "", 0, wMainWindow, xpWidgetClass_Button);
	XPSetWidgetProperty(wChkPauseNav1, xpProperty_ButtonType, xpRadioButton);
	XPSetWidgetProperty(wChkPauseNav1, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
	XPSetWidgetProperty(wChkPauseNav1, xpProperty_ButtonState, userNav1IsCheckEnable);

	topY -= 20;

	// Nav2 Caption
	leftX = x + leftMargin;
	rightX = leftX + widthCaption;
	bottomY = topY - heightFields;
	wCaptionNav2 = XPCreateWidget(leftX, topY, rightX, bottomY, 1, "Nav2:", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wCaptionNav2, xpProperty_CaptionLit, 0);
	// Nav2 Caption FreqHz
	leftX += (widthCaption + gapCaptionField);
	widthField = 50;
	wCaptionFreqNav2 = XPCreateWidget(leftX, topY, leftX + widthField, bottomY, 1, "Nav2", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wCaptionFreqNav2, xpProperty_CaptionLit, 0);
	// Nav2 DME Distance Text Min.
	leftX += 80;
	wTextDistDmeMinNav2 = XPCreateWidget(leftX, topY, leftX + widthField, bottomY, 1, convertToString(userNav2DistMinDme).c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextDistDmeMinNav2, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(wTextDistDmeMinNav2, xpProperty_MaxCharacters, 4);
	XPCreateWidget(leftX + widthField, topY, leftX + widthField + 10, bottomY, 1, ">=", 0, wMainWindow, xpWidgetClass_Caption);
	XPCreateWidget(leftX + widthField + 184, topY + 35, leftX + widthField + 194, bottomY + 35, 1, "Min", 0, wMainWindow, xpWidgetClass_Caption);
	XPCreateWidget(leftX + widthField + 320, topY + 35, leftX + widthField + 330, bottomY + 35, 1, "Max", 0, wMainWindow, xpWidgetClass_Caption);
	// Nav2 DME Distance Caption Max.
	leftX += 72;
	widthField = 50;
	wCaptionDistDmeNav2 = XPCreateWidget(leftX, topY, leftX + widthField, bottomY, 1, "0", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wCaptionDistDmeNav2, xpProperty_CaptionLit, 0);
	XPCreateWidget(leftX + widthField - 1, topY, leftX + widthField + 10 - 1, bottomY, 1, ">=", 0, wMainWindow, xpWidgetClass_Caption);
	// Nav2 DME Distance Text
	leftX += 72;
	widthField = 50;
	wTextDistDmeMaxNav2 = XPCreateWidget(leftX, topY, leftX + widthField, bottomY, 1, convertToString(userNav2DistMaxDme).c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextDistDmeMaxNav2, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(wTextDistDmeMaxNav2, xpProperty_MaxCharacters, 4);
	// Nav2 DME Time Text Min.
	leftX += 78;
	widthField = 50;
	wTextTimeDmeMinNav2 = XPCreateWidget(leftX, topY, leftX + widthField, bottomY, 1, convertToString(userNav2TimeDmeMin).c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextTimeDmeMinNav2, xpProperty_TextFieldType, xpTextEntryField);
	XPCreateWidget(leftX + widthField, topY, leftX + widthField + 10, bottomY, 1, ">=", 0, wMainWindow, xpWidgetClass_Caption);
	// Nav2 DME Time Caption
	leftX += 70;
	widthField = 50;
	wCaptionTimeDmeNav2 = XPCreateWidget(leftX, topY, leftX + widthField, bottomY, 1, "0", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wCaptionTimeDmeNav2, xpProperty_CaptionLit, 0);
	XPCreateWidget(leftX + 43, topY, leftX + widthField + 43 + 10, bottomY, 1, ">=", 0, wMainWindow, xpWidgetClass_Caption);
	// Nav2 DME Time Text Max.
	leftX += 68;
	widthField = 50;
	wTextTimeDmeMaxNav2 = XPCreateWidget(leftX, topY, leftX + widthField, bottomY, 1, convertToString(userNav2TimeDmeMax).c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextTimeDmeMaxNav2, xpProperty_TextFieldType, xpTextEntryField);
	// Nav1 HSI Alignment
	leftX += 82;
	widthField = 50;
	wHSIAlignmentNav2 = XPCreateWidget(leftX, topY, leftX + widthField, bottomY, 1, "0", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wHSIAlignmentNav2, xpProperty_CaptionLit, 0);
	// Nav1 HSI Alignment Checkbox
	leftX += 20;
	widthField = 50;
	wChkHSIAlignNav2 = XPCreateWidget(leftX, topY, leftX + widthField, bottomY, 1, "", 0, wMainWindow, xpWidgetClass_Button);
	XPSetWidgetProperty(wChkHSIAlignNav2, xpProperty_ButtonType, xpRadioButton);
	XPSetWidgetProperty(wChkHSIAlignNav2, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
	XPSetWidgetProperty(wChkHSIAlignNav2, xpProperty_ButtonState, userNav2IsHSIAlignEnable);
	// Nav2 Checkbox
	leftX += 53;
	widthField = 50;
	wChkPauseNav2 = XPCreateWidget(leftX, topY, leftX + widthField, bottomY, 1, "", 0, wMainWindow, xpWidgetClass_Button);
	XPSetWidgetProperty(wChkPauseNav2, xpProperty_ButtonType, xpRadioButton);
	XPSetWidgetProperty(wChkPauseNav2, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
	XPSetWidgetProperty(wChkPauseNav2, xpProperty_ButtonState, userNav2IsCheckEnable);

	// Altitude
	topY -= 55;
	tmpY = topY;
	int altitudeY = topY + 10;
	int padX = 15;
	leftX = x + leftMargin + 43 + padX;
	rightX = leftX + widthCaption;
	bottomY = altitudeY - heightFields;
	XPCreateWidget(leftX, altitudeY, rightX, bottomY, 1, "Altitude", 0, wMainWindow, xpWidgetClass_Caption);
	// Altitude Checkbox
	tmpX = leftMargin - 88;
	wChkAltitude = XPCreateWidget(leftX + tmpX, altitudeY - 3, leftX + tmpX + widthField, bottomY - 3, 1, "", 0, wMainWindow, xpWidgetClass_Button);
	XPSetWidgetProperty(wChkAltitude, xpProperty_ButtonType, xpRadioButton);
	XPSetWidgetProperty(wChkAltitude, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
	XPSetWidgetProperty(wChkAltitude, xpProperty_ButtonState, isAltitudeSelected);
	// Altitude Caption
	leftX = x + leftMargin + padX;
	altitudeY -= 30;
	widthField = 50;
	bottomY = altitudeY - heightFields;
	wCaptionAltitude = XPCreateWidget(leftX, altitudeY, leftX + widthField, bottomY, 1, "0", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wCaptionAltitude, xpProperty_CaptionLit, 1);
	// Altitude Text Min
	leftX += 90;
	altitudeY += 10;
	bottomY = altitudeY - heightFields;
	widthField = 50;
	wTextAltitudeMin = XPCreateWidget(leftX, altitudeY, leftX + widthField, bottomY, 1, convertToString(userAltitudeMin).c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextAltitudeMin, xpProperty_TextFieldType, xpTextEntryField);
	XPCreateWidget(x + leftMargin + 60 + padX, altitudeY, x + leftMargin + 65 + padX, bottomY, 1, "Min:", 0, wMainWindow, xpWidgetClass_Caption);
	// Altitude Text Max
	leftX = x + leftMargin + 90 + padX;
	altitudeY -= 20;
	bottomY = altitudeY - heightFields;
	widthField = 50;
	wTextAltitudeMax = XPCreateWidget(leftX, altitudeY, leftX + widthField, bottomY, 1, convertToString(userAltitudeMax).c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextAltitudeMax, xpProperty_TextFieldType, xpTextEntryField);
	XPCreateWidget(x + leftMargin + 60 + padX, altitudeY, x + leftMargin + 65 + padX, bottomY, 1, "Max:", 0, wMainWindow, xpWidgetClass_Caption);

	topY -= 40;

	// Airspeed
	topY = tmpY;
	tmpX = 170;
	int airspdY = topY + 10;
	leftX = x + leftMargin + tmpX + padX + padX;
	rightX = leftX + widthCaption;
	bottomY = airspdY - heightFields;
	XPCreateWidget(leftX, airspdY, rightX, bottomY, 1, "Airspeed", 0, wMainWindow, xpWidgetClass_Caption);
	// Airspeed Checkbox
	tmpX = tmpX + leftMargin - 88 + padX;
	wChkAirspeed = XPCreateWidget(leftX - 80, airspdY - 2, leftX + widthField, bottomY - 2, 1, "", 0, wMainWindow, xpWidgetClass_Button);
	XPSetWidgetProperty(wChkAirspeed, xpProperty_ButtonType, xpRadioButton);
	XPSetWidgetProperty(wChkAirspeed, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
	XPSetWidgetProperty(wChkAirspeed, xpProperty_ButtonState, isAirspeedSelected);
	// Airspeed Caption
	leftX = x + leftMargin + tmpX + padX;
	airspdY -= 30;
	widthField = 50;
	bottomY = airspdY - heightFields;
	wCaptionAirspeed = XPCreateWidget(leftX + 10, airspdY, leftX + widthField, bottomY, 1, "0", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wCaptionAirspeed, xpProperty_CaptionLit, 1);
	// Airspeed Text Min
	leftX += 90;
	airspdY += 10;
	bottomY = airspdY - heightFields;
	widthField = 50;
	wTextAirspeedMin = XPCreateWidget(leftX, airspdY, leftX + widthField, bottomY, 1, convertToString(userAirspeedMin).c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextAirspeedMin, xpProperty_TextFieldType, xpTextEntryField);
	XPCreateWidget(x + leftMargin + 60 + tmpX + padX, airspdY, x + leftMargin + 65 + tmpX + padX, bottomY, 1, "Min:", 0, wMainWindow, xpWidgetClass_Caption);
	// Airspeed Text Max
	leftX = x + leftMargin + 90 + tmpX + padX;
	airspdY -= 20;
	bottomY = airspdY - heightFields;
	widthField = 50;
	wTextAirspeedMax = XPCreateWidget(leftX, airspdY, leftX + widthField, bottomY, 1, convertToString(userAirspeedMax).c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextAirspeedMax, xpProperty_TextFieldType, xpTextEntryField);

	topY -= 40;

	// GPS
	topY     = tmpY + 18;
	int gpsY = topY + 5;
	tmpX = 660;
	leftX = x + leftMargin + tmpX + padX + padX;
	rightX = leftX + widthCaption;
	bottomY = gpsY - heightFields;
	XPCreateWidget(leftX, gpsY - 3, rightX, bottomY - 3, 1, "GPS", 0, wMainWindow, xpWidgetClass_Caption);
	// GPS Checkbox
	tmpX = tmpX + leftMargin - 88 + padX;
	wChkGPS = XPCreateWidget(leftX - 80, gpsY - 5, leftX + widthField, bottomY - 5, 1, "", 0, wMainWindow, xpWidgetClass_Button);
	XPSetWidgetProperty(wChkGPS, xpProperty_ButtonType, xpRadioButton);
	XPSetWidgetProperty(wChkGPS, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
	XPSetWidgetProperty(wChkGPS, xpProperty_ButtonState, isGPSSelected);
	// GPS Caption
	leftX = x + leftMargin + tmpX + padX;
	gpsY -= 22;
	widthField = 50;
	bottomY = gpsY - heightFields;
	wCaptionGPS = XPCreateWidget(leftX - 80, gpsY, (leftX - 80) + 150, bottomY, 1, "0", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wCaptionGPS, xpProperty_CaptionLit, 1);
	// GPS DME Distance Caption
	gpsY -= 34; bottomY = gpsY - heightFields;
	XPCreateWidget(leftX - 8, gpsY + 35, rightX + 10, bottomY, 1, "Min", 0, wMainWindow, xpWidgetClass_Caption);
	XPCreateWidget(leftX + 155, gpsY + 35, rightX + 165, bottomY, 1, "Max", 0, wMainWindow, xpWidgetClass_Caption);
	XPCreateWidget(leftX - 100, gpsY, rightX, bottomY, 1, "DME Distance:", 0, wMainWindow, xpWidgetClass_Caption);
	// GPS DME Distance Text Min.
	tmpX = leftX - 18;
	wTextGPSDmeDistanceMin = XPCreateWidget(tmpX, gpsY, tmpX + widthField, bottomY, 1, convertToString(userGPSDmeDistanceMin).c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextGPSDmeDistanceMin, xpProperty_TextFieldType, xpTextEntryField);
	XPCreateWidget(tmpX + widthField, gpsY, tmpX + widthField + 10, bottomY, 1, ">=", 0, wMainWindow, xpWidgetClass_Caption);
	tmpX = leftX + widthField + 3;
	wCaptionGPSDmeDistance = XPCreateWidget(tmpX, gpsY, tmpX + widthField, bottomY, 1, "0", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wCaptionGPSDmeDistance, xpProperty_CaptionLit, 1);
	// GPS DME Distance Text Max.
	tmpX = leftX + widthField + widthField + 45;
	wTextGPSDmeDistanceMax = XPCreateWidget(tmpX, gpsY, tmpX + widthField, bottomY, 1, convertToString(userGPSDmeDistanceMax).c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextGPSDmeDistanceMax, xpProperty_TextFieldType, xpTextEntryField);
	tmpX = leftX + widthField + 70;
	XPCreateWidget(tmpX, gpsY, tmpX + 10, bottomY, 1, ">=", 0, wMainWindow, xpWidgetClass_Caption);
	// GPS DME Time Min.
	gpsY -= 18; bottomY = gpsY - heightFields;
	XPCreateWidget(leftX - 100, gpsY, rightX, bottomY, 1, "DME Time:", 0, wMainWindow, xpWidgetClass_Caption);
	// GPS DME Time Text Min.
	tmpX = leftX - 18;
	wTextGPSDmeTimeMin = XPCreateWidget(tmpX, gpsY, tmpX + widthField, bottomY, 1, convertToString(userGPSDmeTimeMin).c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextGPSDmeTimeMin, xpProperty_TextFieldType, xpTextEntryField);
	XPCreateWidget(tmpX + widthField, gpsY, tmpX + widthField + 10, bottomY, 1, ">=", 0, wMainWindow, xpWidgetClass_Caption);
	tmpX = leftX + widthField + 3;
	wCaptionGPSDmeTime = XPCreateWidget(tmpX, gpsY, tmpX + widthField, bottomY, 1, "0", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wCaptionGPSDmeTime, xpProperty_CaptionLit, 1);
	// GPS DME Time Text Max.
	tmpX = leftX + widthField + widthField + 45;
	wTextGPSDmeTimeMax = XPCreateWidget(tmpX, gpsY, tmpX + widthField, bottomY, 1, convertToString(userGPSDmeTimeMax).c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextGPSDmeTimeMax, xpProperty_TextFieldType, xpTextEntryField);
	tmpX = leftX + widthField + 70;
	XPCreateWidget(tmpX, gpsY, tmpX + 10, bottomY, 1, ">=", 0, wMainWindow, xpWidgetClass_Caption);

	topY -= 104;

	// Latitude e Longitude
	leftX = x + leftMargin + 35;
	int leftXLatLong = leftX;
	rightX = leftX;
	bottomY = topY - 10;
	tmpY = topY;
	tmpX = leftX;
	int latLngY = topY;
	int topYlatLong = topY + 30;
	XPCreateWidget(leftX + 5, topYlatLong - 5, rightX + 100, bottomY - 5, 1, "Latitude / Longitude (Decimal degrees)", 0, wMainWindow, xpWidgetClass_Caption);
	leftX -= 30;
	topYlatLong -= 20;
	wChkLatLon = XPCreateWidget(leftX, topYlatLong, leftX + widthField, bottomY - 40, 1, "", 0, wMainWindow, xpWidgetClass_Button);
	XPSetWidgetProperty(wChkLatLon, xpProperty_ButtonType, xpRadioButton);
	XPSetWidgetProperty(wChkLatLon, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
	XPSetWidgetProperty(wChkLatLon, xpProperty_ButtonState, isLatLongSelected);
	leftX += 45;
	topYlatLong += 5;
	wCaptionLatLon = XPCreateWidget(leftX, topYlatLong, leftX + 100, bottomY - 40, 1, "00.000 / 00.000", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wCaptionGPS, xpProperty_CaptionLit, 1);
	leftX += 100;
	bottomY -= 40;
	XPCreateWidget(leftX, topYlatLong, leftX + 100, bottomY, 1, "=", 0, wMainWindow, xpWidgetClass_Caption);
	leftX += 15;
	topYlatLong -= 3;
	bottomY -= 3;
	wTextLatitude = XPCreateWidget(leftX, topYlatLong - 20, leftX + 55, bottomY + 22, 1, objUserLatitude.getValorStr().c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextLatitude, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(wTextLatitude, xpProperty_MaxCharacters, 7);
	leftX += 58;
	topYlatLong += 3;
	bottomY += 3;
	XPCreateWidget(leftX, topYlatLong, leftX + 100, bottomY, 1, "/", 0, wMainWindow, xpWidgetClass_Caption);
	leftX += 12;
	topYlatLong -= 3;
	bottomY -= 3;
	wTextLongitude = XPCreateWidget(leftX, topYlatLong - 20, leftX + 55, bottomY + 22, 1, objUserLongitude.getValorStr().c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextLongitude, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(wTextLongitude, xpProperty_MaxCharacters, 7);

	topY -= 12;

	// Navaids
	int hTextField = 22;
	leftX = x + leftMargin + 570;
	rightX = leftX;
	topY = tmpY - 10;
	bottomY = topY - 10;
	XPCreateWidget(leftX, topY, rightX + 100, bottomY, 1, "Navaids", 0, wMainWindow, xpWidgetClass_Caption);
	// Airport Navaid Line
	leftX -= 150;
	topY -= 20;
	tmpX = leftX;
	wChkNavaidAirport = XPCreateWidget(leftX, topY, leftX + widthField, bottomY - 38, 1, "", 0, wMainWindow, xpWidgetClass_Button);
	XPSetWidgetProperty(wChkNavaidAirport, xpProperty_ButtonType, xpRadioButton);
	XPSetWidgetProperty(wChkNavaidAirport, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
	XPSetWidgetProperty(wChkNavaidAirport, xpProperty_ButtonState, isNavaidAirportSelected);
	leftX += 35;
	topY -= 7;
	bottomY = topY - 10;
	XPCreateWidget(leftX, topY, leftX + 100, bottomY, 1, "Airport:", 0, wMainWindow, xpWidgetClass_Caption);
	leftX += 44;
	topY += 5;
	bottomY = topY - heightFields;
	wTextNavaidAirportID = XPCreateWidget(leftX + 5, topY, leftX + 55, bottomY, 1, userNavaidAirportID.c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextNavaidAirportID, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(wTextNavaidAirportID, xpProperty_MaxCharacters, 7);
	leftX += 60;
	topY += 3;
	wCaptionNavaidAirportDistance = XPCreateWidget(leftX, topY, leftX + 30, bottomY, 1, "9999nm", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wCaptionNavaidAirportDistance, xpProperty_CaptionLit, 1);
	leftX += 55;
	XPCreateWidget(leftX, topY, leftX + 100, bottomY, 1, "<=", 0, wMainWindow, xpWidgetClass_Caption);
	leftX += 23;
	topY -= 3;
	bottomY = topY - heightFields;
	wTextNavaidAirportDistanceMin = XPCreateWidget(leftX + 5, topY, leftX + 55, bottomY, 1, convertToString(userNavaidAirportDistance).c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextNavaidAirportDistanceMin, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(wTextNavaidAirportDistanceMin, xpProperty_MaxCharacters, 7);
	leftX += 55;
	wCaptionNavaidAirportDesc = XPCreateWidget(leftX, topY, leftX + 30, bottomY, 1, "----", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wCaptionNavaidAirportDesc, xpProperty_CaptionLit, 0);
	topY += 3;
	// VOR Navaid Line
	leftX = tmpX;
	topY -= 20;
	wChkNavaidVOR = XPCreateWidget(leftX, topY, leftX + widthField, bottomY - 39, 1, "", 0, wMainWindow, xpWidgetClass_Button);
	XPSetWidgetProperty(wChkNavaidVOR, xpProperty_ButtonType, xpRadioButton);
	XPSetWidgetProperty(wChkNavaidVOR, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
	XPSetWidgetProperty(wChkNavaidVOR, xpProperty_ButtonState, isNavaidVORSelected);
	leftX += 35;
	topY -= 7;
	bottomY = topY - 8;
	XPCreateWidget(leftX, topY-10, leftX + 100, bottomY-8, 1, "VOR:", 0, wMainWindow, xpWidgetClass_Caption);
	leftX += 44;
	topY -= 3;
	bottomY = topY - heightFields;
	wTextNavaidVORID = XPCreateWidget(leftX + 5, topY, leftX + 55, bottomY, 1, userNavaidVORID.c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextNavaidVORID, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(wTextNavaidVORID, xpProperty_MaxCharacters, 7);
	leftX += 60;
	topY += 3;
	wCaptionNavaidVORDistance = XPCreateWidget(leftX, topY, leftX + 30, bottomY, 1, "9999nm", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wCaptionNavaidVORDistance, xpProperty_CaptionLit, 1);
	leftX += 55;
	XPCreateWidget(leftX, topY, leftX + 100, bottomY, 1, "<=", 0, wMainWindow, xpWidgetClass_Caption);
	leftX += 23;
	topY -= 3;
	bottomY = topY - heightFields;
	wTextNavaidVORDistanceMin = XPCreateWidget(leftX + 5, topY, leftX + 55, bottomY, 1, convertToString(userNavaidVORDistance).c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextNavaidVORDistanceMin, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(wTextNavaidVORDistanceMin, xpProperty_MaxCharacters, 7);
	leftX += 55;
	wCaptionNavaidVORDesc = XPCreateWidget(leftX, topY, leftX + 30, bottomY, 1, "----", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wCaptionNavaidVORDesc, xpProperty_CaptionLit, 0);
	topY += 3;
	// NDB Navaid Line
	leftX = tmpX;
	topY -= 20;
	wChkNavaidNDB = XPCreateWidget(leftX, topY, leftX + widthField, bottomY - 39, 1, "", 0, wMainWindow, xpWidgetClass_Button);
	XPSetWidgetProperty(wChkNavaidNDB, xpProperty_ButtonType, xpRadioButton);
	XPSetWidgetProperty(wChkNavaidNDB, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
	XPSetWidgetProperty(wChkNavaidNDB, xpProperty_ButtonState, isNavaidNDBSelected);
	leftX += 35;
	topY -= 7;
	bottomY = topY - 8;
	XPCreateWidget(leftX, topY-10, leftX + 100, bottomY-8, 1, "NDB:", 0, wMainWindow, xpWidgetClass_Caption);
	leftX += 44;
	topY -= 3;
	bottomY = topY - heightFields;
	wTextNavaidNDBID = XPCreateWidget(leftX + 5, topY, leftX + 55, bottomY, 1, userNavaidNDBID.c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextNavaidNDBID, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(wTextNavaidNDBID, xpProperty_MaxCharacters, 7);
	leftX += 60;
	topY += 3;
	wCaptionNavaidNDBDistance = XPCreateWidget(leftX, topY, leftX + 30, bottomY, 1, "9999nm", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wCaptionNavaidNDBDistance, xpProperty_CaptionLit, 1);
	leftX += 55;
	XPCreateWidget(leftX, topY, leftX + 100, bottomY, 1, "<=", 0, wMainWindow, xpWidgetClass_Caption);
	leftX += 23;
	topY -= 3;
	bottomY = topY - heightFields;
	wTextNavaidNDBDistanceMin = XPCreateWidget(leftX + 5, topY, leftX + 55, bottomY, 1, convertToString(userNavaidNDBDistance).c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextNavaidNDBDistanceMin, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(wTextNavaidNDBDistanceMin, xpProperty_MaxCharacters, 7);
	leftX += 55;
	wCaptionNavaidNDBDesc = XPCreateWidget(leftX, topY, leftX + 30, bottomY, 1, "----", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wCaptionNavaidNDBDesc, xpProperty_CaptionLit, 0);
	topY += 3;
	// Fix Navaid Line
	leftX = tmpX;
	topY -= 20;
	wChkNavaidFix = XPCreateWidget(leftX, topY, leftX + widthField, bottomY - 39, 1, "", 0, wMainWindow, xpWidgetClass_Button);
	XPSetWidgetProperty(wChkNavaidFix, xpProperty_ButtonType, xpRadioButton);
	XPSetWidgetProperty(wChkNavaidFix, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
	XPSetWidgetProperty(wChkNavaidFix, xpProperty_ButtonState, isNavaidFixSelected);
	leftX += 35;
	topY -= 7;
	bottomY = topY - 8;
	XPCreateWidget(leftX, topY-10, leftX + 100, bottomY-8, 1, "Fix:", 0, wMainWindow, xpWidgetClass_Caption);
	leftX += 44;
	topY -= 3;
	bottomY = topY - heightFields;
	wTextNavaidFixID = XPCreateWidget(leftX + 5, topY, leftX + 55, bottomY, 1, userNavaidFixID.c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextNavaidFixID, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(wTextNavaidFixID, xpProperty_MaxCharacters, 7);
	leftX += 60;
	topY += 3;
	wCaptionNavaidFixDistance = XPCreateWidget(leftX, topY, leftX + 30, bottomY, 1, "9999nm", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wCaptionNavaidFixDistance, xpProperty_CaptionLit, 1);
	leftX += 55;
	XPCreateWidget(leftX, topY, leftX + 100, bottomY, 1, "<=", 0, wMainWindow, xpWidgetClass_Caption);
	leftX += 23;
	topY -= 3;
	bottomY = topY - heightFields;
	wTextNavaidFixDistanceMin = XPCreateWidget(leftX + 5, topY, leftX + 55, bottomY, 1, convertToString(userNavaidFixDistance).c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextNavaidFixDistanceMin, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(wTextNavaidFixDistanceMin, xpProperty_MaxCharacters, 7);
	leftX += 55;
	wCaptionNavaidFixDesc = XPCreateWidget(leftX, topY, leftX + 30, bottomY, 1, "----", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wCaptionNavaidFixDesc, xpProperty_CaptionLit, 0);
	topY += 3;
	//// DME Navaid Line
	leftX = tmpX;
	//topY -= 10;
	topY -= 20;
	wChkNavaidDME = XPCreateWidget(leftX, topY - 9, leftX + widthField, bottomY - 29, 1, "", 0, wMainWindow, xpWidgetClass_Button);
	XPSetWidgetProperty(wChkNavaidDME, xpProperty_ButtonType, xpRadioButton);
	XPSetWidgetProperty(wChkNavaidDME, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
	XPSetWidgetProperty(wChkNavaidDME, xpProperty_ButtonState, isNavaidDMESelected);

	leftX += 35;
	topY -= 7;
	bottomY = topY - 8;
	XPCreateWidget(leftX, topY-10, leftX + 100, bottomY-8, 1, "DME:", 0, wMainWindow, xpWidgetClass_Caption);
	leftX += 44;
	topY -= 3;
	bottomY = topY - heightFields;
	wTextNavaidDMEID = XPCreateWidget(leftX + 5, topY, leftX + 55, bottomY, 1, userNavaidDMEID.c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextNavaidDMEID, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(wTextNavaidDMEID, xpProperty_MaxCharacters, 7);
	leftX += 60;
	topY += 3;
	wCaptionNavaidDMEDistance = XPCreateWidget(leftX, topY, leftX + 30, bottomY, 1, "9999nm", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wCaptionNavaidDMEDistance, xpProperty_CaptionLit, 1);
	leftX += 55;
	XPCreateWidget(leftX, topY, leftX + 100, bottomY, 1, "<=", 0, wMainWindow, xpWidgetClass_Caption);
	leftX += 23;
	topY -= 3;
	bottomY = topY - heightFields;
	wTextNavaidDMEDistanceMin = XPCreateWidget(leftX + 5, topY, leftX + 55, bottomY, 1, convertToString(userNavaidDMEDistance).c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextNavaidDMEDistanceMin, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(wTextNavaidDMEDistanceMin, xpProperty_MaxCharacters, 7);
	leftX += 55;
	wCaptionNavaidDMEDesc = XPCreateWidget(leftX, topY, leftX + 10, bottomY, 1, "-----", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wCaptionNavaidDMEDesc, xpProperty_CaptionLit, 0);
	topY += 3;

	// Datarefs
	int dataRefleftX   = leftXLatLong;
	int dataReftopY    = topYlatLong - 65;
	int dataRefRightX  = dataRefleftX;
	int dataRefBottomY = topYlatLong - 75;
	XPCreateWidget(dataRefleftX + 35, dataReftopY - 5, dataRefRightX + 130, dataRefBottomY - 5, 1, "        Pause with DataRefs       ", 0, wMainWindow, xpWidgetClass_Caption);
	XPCreateWidget(dataRefleftX - 28, dataReftopY - 15, dataRefRightX + 50, dataRefBottomY - 25, 1, "DataRef...", 0, wMainWindow, xpWidgetClass_Caption);
	XPCreateWidget(dataRefleftX + 255, dataReftopY - 15, dataRefRightX + 265, dataRefBottomY - 25, 1, "Pause when...", 0, wMainWindow, xpWidgetClass_Caption);

	int sizeDataRefField      = 260;
	int sizeDataRefValueField = 60;
	int marginDataRefFields   = 30;
	int gapBetweenFields      = 2;
	int gapBetweenChks        = 5;

	// Dataref 1
	dataRefleftX += 5;
	dataReftopY  -= 40;
	dataRefBottomY = dataReftopY - heightFields;
	wDataRef1 = XPCreateWidget(dataRefleftX - marginDataRefFields, dataReftopY, dataRefleftX + sizeDataRefField, dataRefBottomY, 1, dataRef1.c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wDataRef1, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(wDataRef1, xpProperty_MaxCharacters, 80);
	int xDataRef = dataRefleftX + sizeDataRefField;
	wDataRefValue1 = XPCreateWidget(xDataRef + gapBetweenFields, dataReftopY, xDataRef + gapBetweenFields + sizeDataRefValueField, dataRefBottomY, 1, dataRefValue1.c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wDataRefValue1, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(wDataRefValue1, xpProperty_MaxCharacters, 10);
	xDataRef = dataRefleftX + sizeDataRefField + sizeDataRefValueField + gapBetweenChks;
	wChkDataRef1 = XPCreateWidget(xDataRef + gapBetweenFields, dataReftopY - 1, xDataRef + gapBetweenChks + 5, dataRefBottomY - 1, 1, "", 0, wMainWindow, xpWidgetClass_Button);
	XPSetWidgetProperty(wChkDataRef1, xpProperty_ButtonType, xpRadioButton);
	XPSetWidgetProperty(wChkDataRef1, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
	XPSetWidgetProperty(wChkDataRef1, xpProperty_ButtonState, isDataRef1Selected);
	xDataRef = dataRefleftX + sizeDataRefField + sizeDataRefValueField + gapBetweenChks + 13;
	wCaptionDataRef1 = XPCreateWidget(xDataRef, dataReftopY + 1, xDataRef + 10, dataRefBottomY + 1, 1, "-----", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wCaptionDataRef1, xpProperty_CaptionLit, 0);

	// Dataref 2
	dataReftopY   -= 20;
	dataRefBottomY = dataReftopY - heightFields;
	wDataRef2 = XPCreateWidget(dataRefleftX - marginDataRefFields, dataReftopY, dataRefleftX + sizeDataRefField, dataRefBottomY, 1, dataRef2.c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wDataRef2, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(wDataRef2, xpProperty_MaxCharacters, 80);
	xDataRef = dataRefleftX + sizeDataRefField;
	wDataRefValue2 = XPCreateWidget(xDataRef + gapBetweenFields, dataReftopY, xDataRef + gapBetweenFields + sizeDataRefValueField, dataRefBottomY, 1, dataRefValue2.c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wDataRefValue2, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(wDataRefValue2, xpProperty_MaxCharacters, 10);
	xDataRef = dataRefleftX + sizeDataRefField + sizeDataRefValueField + gapBetweenChks;
	wChkDataRef2 = XPCreateWidget(xDataRef + gapBetweenFields, dataReftopY - 1, xDataRef + gapBetweenChks + 5, dataRefBottomY - 1, 1, "", 0, wMainWindow, xpWidgetClass_Button);
	XPSetWidgetProperty(wChkDataRef2, xpProperty_ButtonType, xpRadioButton);
	XPSetWidgetProperty(wChkDataRef2, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
	XPSetWidgetProperty(wChkDataRef2, xpProperty_ButtonState, isDataRef2Selected);
	xDataRef = dataRefleftX + sizeDataRefField + sizeDataRefValueField + gapBetweenChks + 13;
	wCaptionDataRef2 = XPCreateWidget(xDataRef, dataReftopY + 1, xDataRef + 10, dataRefBottomY + 1, 1, "-----", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wCaptionDataRef2, xpProperty_CaptionLit, 0);

	// Dataref 3
	dataReftopY -= 20;
	dataRefBottomY = dataReftopY - heightFields;
	wDataRef3 = XPCreateWidget(dataRefleftX - marginDataRefFields, dataReftopY, dataRefleftX + sizeDataRefField, dataRefBottomY, 1, dataRef3.c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wDataRef3, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(wDataRef3, xpProperty_MaxCharacters, 80);
	xDataRef = dataRefleftX + sizeDataRefField;
	wDataRefValue3 = XPCreateWidget(xDataRef + gapBetweenFields, dataReftopY, xDataRef + gapBetweenFields + sizeDataRefValueField, dataRefBottomY, 1, dataRefValue3.c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wDataRefValue3, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(wDataRefValue3, xpProperty_MaxCharacters, 10);
	xDataRef = dataRefleftX + sizeDataRefField + sizeDataRefValueField + gapBetweenChks;
	wChkDataRef3 = XPCreateWidget(xDataRef + gapBetweenFields, dataReftopY - 1, xDataRef + gapBetweenChks + 5, dataRefBottomY - 1, 1, "", 0, wMainWindow, xpWidgetClass_Button);
	XPSetWidgetProperty(wChkDataRef3, xpProperty_ButtonType, xpRadioButton);
	XPSetWidgetProperty(wChkDataRef3, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
	XPSetWidgetProperty(wChkDataRef3, xpProperty_ButtonState, isDataRef3Selected);
	xDataRef = dataRefleftX + sizeDataRefField + sizeDataRefValueField + gapBetweenChks + 13;
	wCaptionDataRef3 = XPCreateWidget(xDataRef, dataReftopY + 1, xDataRef + 10, dataRefBottomY + 1, 1, "-----", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wCaptionDataRef3, xpProperty_CaptionLit, 0);

	//*********************************************************************************************************************************

	// Time
	int timeX = x    + 410;
	int timeY = topY + 190;
	           XPCreateWidget(timeX + 12, timeY + 21, timeX + 18, timeY + 8, 1, "Time", 0, wMainWindow, xpWidgetClass_Caption);
	wChkTime = XPCreateWidget(timeX - 3, timeY + 24, timeX + 5 , timeY, 1, "", 0, wMainWindow, xpWidgetClass_Button);
	XPSetWidgetProperty(wChkTime, xpProperty_ButtonType, xpRadioButton);
	XPSetWidgetProperty(wChkTime, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
	XPSetWidgetProperty(wChkTime, xpProperty_ButtonState, isTimePauseSelected);

	wTextTime = XPCreateWidget(timeX, timeY, timeX + 60, timeY - 23, 1, timePause.c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextTime, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(wTextTime, xpProperty_MaxCharacters, 8);
	timeX += 61;
	wCaptionTime = XPCreateWidget(timeX, timeY, timeX + 62, timeY - 20, 1, "10:20:20", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wCaptionTime, xpProperty_CaptionLit, 1);
	

	//*********************************************************************************************************************************

	//Flight Plan
	int fpX = x + 43;
	int fpY = topY + 17;
	XPCreateWidget(fpX-3, fpY - 51, fpX + 50, fpY - 61, 1, "Flight Plan:", 0, wMainWindow, xpWidgetClass_Caption);
	wTextFlightPlan = XPCreateWidget(fpX, fpY - 65, fpX + 500, fpY - 85, 1, flightPlan.c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextFlightPlan, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(wTextFlightPlan, xpProperty_MaxCharacters, 8000);
	wBtnSendFlightPlan = XPCreateWidget(fpX + 502, fpY - 65, fpX + 552, fpY - 85, 1, " Send ", 0, wMainWindow, xpWidgetClass_Button);
	XPSetWidgetProperty(wBtnSendFlightPlan, xpProperty_ButtonType, xpPushButton);
	wBtnFpTranslate = XPCreateWidget(fpX, fpY - 87, fpX + 124, fpY - 97, 1, "Translate Flight Plan", 0, wMainWindow, xpWidgetClass_Button);
	XPSetWidgetProperty(wBtnFpTranslate, xpProperty_ButtonType, xpPushButton);
	wBtnFpBringBackOriginal = XPCreateWidget(fpX + 126, fpY - 87, fpX + 246, fpY - 97, 1, "Bring Back My Plan", 0, wMainWindow, xpWidgetClass_Button);
	XPSetWidgetProperty(wBtnFpBringBackOriginal, xpProperty_ButtonType, xpPushButton);
	wBtnCopyFlightPlan = XPCreateWidget(fpX + 248, fpY - 87, fpX + 348, fpY - 97, 1, "Copy", 0, wMainWindow, xpWidgetClass_Button);
	XPSetWidgetProperty(wBtnCopyFlightPlan, xpProperty_ButtonType, xpPushButton);
	wBtnPasteFlightPlan = XPCreateWidget(fpX + 350, fpY - 87, fpX + 450, fpY - 97, 1, "Paste", 0, wMainWindow, xpWidgetClass_Button);
	XPSetWidgetProperty(wBtnPasteFlightPlan, xpProperty_ButtonType, xpPushButton);
	wBtnCleanFlightPlan = XPCreateWidget(fpX + 452, fpY - 87, fpX + 552, fpY - 97, 1, "Clean", 0, wMainWindow, xpWidgetClass_Button);
	XPSetWidgetProperty(wBtnCleanFlightPlan, xpProperty_ButtonType, xpPushButton);
	// X-Plane 11 Load Flight Plan	
	XPCreateWidget(fpX - 3, fpY - 102, fpX + 50, fpY - 112, 1, "X-Plane 11 Flight Plan:", 0, wMainWindow, xpWidgetClass_Caption);
	wTextLoadFlightPlan = XPCreateWidget(fpX + 130, fpY - 99, fpX + 230, fpY - 119, 1, xplane11LoadFlightPlan.c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	wBtnLoadFlightPlan = XPCreateWidget(fpX + 233, fpY - 105, fpX + 325, fpY - 115, 1, " Load ", 0, wMainWindow, xpWidgetClass_Button);
	XPSetWidgetProperty(wBtnLoadFlightPlan, xpProperty_ButtonType, xpPushButton);
	wLoadFlightPlanResult = XPCreateWidget(fpX + 325, fpY - 101, fpX + 420, fpY - 113, 1, "", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wLoadFlightPlanResult, xpProperty_CaptionLit, 1);

	//*********************************************************************************************************************************

	topY -= 45;

	// Button Save
	tmpX = 630;
	leftX = x + tmpX;
	bottomY = topY - heightFields - 5;
	wBtnSave = XPCreateWidget(leftX, topY, leftX + 60, bottomY, 1, "Save", 0, wMainWindow, xpWidgetClass_Button);
	XPSetWidgetProperty(wBtnSave, xpProperty_ButtonType, xpPushButton);
	// Button Exit
	leftX = x + (tmpX + 70);
	bottomY = topY - heightFields - 5;
	wBtnCancel = XPCreateWidget(leftX, topY, leftX + 60, bottomY, 1, "Exit", 0, wMainWindow, xpWidgetClass_Button);
	XPSetWidgetProperty(wBtnCancel, xpProperty_ButtonType, xpPushButton);
	// Button Transmitter Switch
	std::string btnLabelTransmitter = "START Transmitter";
	if ( serverSocketStarted ) {
		btnLabelTransmitter = "STOP Transmitter";
	}
	leftX = x + (tmpX + 140);
	bottomY = topY - heightFields - 5;
	wBtnStopStartWebSocket = XPCreateWidget(leftX, topY, leftX + 125, bottomY, 1, btnLabelTransmitter.c_str(), 0, wMainWindow, xpWidgetClass_Button);
	XPSetWidgetProperty(wBtnStopStartWebSocket, xpProperty_ButtonType, xpPushButton);

	topY -= 28;
	leftX = 620;
	XPWidgetID email = XPCreateWidget(leftX + 233, topY, leftX + 283, topY - 5, 1, "ualter.junior@gmail.com", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(email, xpProperty_CaptionLit, 1);
	
	//*********************************************************************************************************************************
	//*********************************************************************************************************************************
	//*********************************************************************************************************************************
	//*********************************************************************************************************************************
	//*********************************************************************************************************************************
	// Just For Debug Purposes

	// Button Reload (For Development Process)
	leftX += 100;
	bottomY = topY-heightFields;
	wBtnReload = XPCreateWidget(leftX, topY-5, leftX+80, bottomY,1,"Reload",0,wMainWindow,xpWidgetClass_Button);
	XPSetWidgetProperty(wBtnReload,xpProperty_ButtonType,xpPushButton);
	
	//topY -= 15;
	//bottomY = topY-heightFields;
	//leftX   = x+leftMargin;
	//widgetDebug1 = XPCreateWidget(leftX, topY, leftX+80, bottomY,1,"Debug1!",0,wMainWindow,xpWidgetClass_Caption);
	//leftX += 160;
	//widgetDebug2 = XPCreateWidget(leftX, topY, leftX+80, bottomY,1,"Debug2!",0,wMainWindow,xpWidgetClass_Caption);
	//leftX += 160;
	//widgetDebug3 = XPCreateWidget(leftX, topY, leftX+80, bottomY,1,"Debug3!",0,wMainWindow,xpWidgetClass_Caption);

	XPAddWidgetCallback(wMainWindow, widgetWidgetHandler);
}

void PauseForMeMenuHandler(void * mRef, void * iRef)
{
	if (!strcmp((char *)iRef, "Setup"))
	{
		if (MenuItem1 == 0)
		{
			CreateWidgetWindow();
			MenuItem1 = 1;
			XPLMRegisterFlightLoopCallback(CallBackXPlane, 1.0, NULL);
		}
	}
}

void saveFileValues()
{
	std::ifstream fin(fileName);
	if ( fin ) {
		fin.close();
		std::remove(fileName.c_str());
	}

	std::ofstream fileIniWriter;
	fileIniWriter.open(fileName);
	fileIniWriter << "nav1_dme_min_dist="+ convertToString(userNav1DistMinDme) + "\n";
	fileIniWriter << "nav1_dme_max_dist=" + convertToString(userNav1DistMaxDme) + "\n";
	fileIniWriter << "nav1_dme_min_time=" + convertToString(userNav1TimeDmeMin) + "\n";
	fileIniWriter << "nav1_dme_max_time=" + convertToString(userNav1TimeDmeMax) + "\n";
	fileIniWriter << "nav1_hsi_alignment=" + convertToString(userNav1IsHSIAlignEnable) + "\n";

	fileIniWriter << "nav2_dme_min_dist=" + convertToString(userNav2DistMinDme) + "\n";
	fileIniWriter << "nav2_dme_max_dist=" + convertToString(userNav2DistMaxDme) + "\n";
	fileIniWriter << "nav2_dme_min_time=" + convertToString(userNav2TimeDmeMin) + "\n";
	fileIniWriter << "nav2_dme_max_time=" + convertToString(userNav2TimeDmeMax) + "\n";
	fileIniWriter << "nav2_hsi_alignment=" + convertToString(userNav2IsHSIAlignEnable) + "\n";

	fileIniWriter << "altitude_min=" + convertToString(userAltitudeMin) + "\n";
	fileIniWriter << "altitude_max=" + convertToString(userAltitudeMax) + "\n";
	fileIniWriter << "airspeed_min=" + convertToString(userAirspeedMin) + "\n";
	fileIniWriter << "airspeed_max=" + convertToString(userAirspeedMax) + "\n";

	fileIniWriter << "gps_dme_min_dist=" + convertToString(userGPSDmeDistanceMin) + "\n";
	fileIniWriter << "gps_dme_max_dist=" + convertToString(userGPSDmeDistanceMax) + "\n";
	fileIniWriter << "gps_dme_min_time=" + convertToString(userGPSDmeTimeMin) + "\n";
	fileIniWriter << "gps_dme_max_time=" + convertToString(userGPSDmeTimeMax) + "\n";

	fileIniWriter << "latitude=" + objUserLatitude.getValorStr() + "\n";
	fileIniWriter << "longitude=" + objUserLongitude.getValorStr() + "\n";

	fileIniWriter << "navaidAirportId=" + userNavaidAirportID + "\n";
	fileIniWriter << "navaidAirportDistance=" + convertToString(userNavaidAirportDistance) + "\n";
	fileIniWriter << "navaidVORId=" + userNavaidVORID + "\n";
	fileIniWriter << "navaidVORDistance=" + convertToString(userNavaidVORDistance) + "\n";
	fileIniWriter << "navaidNDBId=" + userNavaidNDBID + "\n";
	fileIniWriter << "navaidNDBDistance=" + convertToString(userNavaidNDBDistance) + "\n";
	fileIniWriter << "navaidFixId=" + userNavaidFixID + "\n";
	fileIniWriter << "navaidFixDistance=" + convertToString(userNavaidFixDistance) + "\n";
	fileIniWriter << "navaidDMEId=" + userNavaidDMEID + "\n";
	fileIniWriter << "navaidDMEDistance=" + convertToString(userNavaidDMEDistance) + "\n";

	fileIniWriter << "dataRef1=" + dataRef1 + "\n";
	fileIniWriter << "dataRefValue1=" + dataRefValue1 + "\n";
	fileIniWriter << "dataRef2=" + dataRef2 + "\n";
	fileIniWriter << "dataRefValue2=" + dataRefValue2 + "\n";
	fileIniWriter << "dataRef3=" + dataRef3 + "\n";
	fileIniWriter << "dataRefValue3=" + dataRefValue3 + "\n";

	fileIniWriter << "timePause=" + timePause + "\n";
	fileIniWriter << "flightPlan=" + flightPlan + "\n";
	fileIniWriter << "xplane11LoadFlightPlan=" + xplane11LoadFlightPlan + "\n";

	fileIniWriter << "ats_txt=" + pathATSFile + "\n";

	fileIniWriter.close();
}

void toupperChar(char *c) {
	for (int i = 0; i <= 3; i++)
		c[i] = toupper(c[i]);
}

int widgetWidgetHandler(XPWidgetMessage			inMessage,
	XPWidgetID				inWidget,
	intptr_t				inParam1,
	intptr_t				inParam2)
{
	char buffer[256];
	if (inMessage == xpMessage_CloseButtonPushed)
	{
		if (MenuItem1 == 1)
		{
			XPHideWidget(wMainWindow);
			MenuItem1 = 0;
		}
		return 1;
	}

	if (inMessage == xpMsg_PushButtonPressed)
	{
		if (inParam1 == (intptr_t)wBtnSave)
		{
			XPGetWidgetDescriptor(wTextDistDmeMinNav1, buffer, sizeof(buffer));
			userNav1DistMinDme = convertToNumber(buffer);
			XPGetWidgetDescriptor(wTextDistDmeMaxNav1, buffer, sizeof(buffer));
			userNav1DistMaxDme = convertToNumber(buffer);

			XPGetWidgetDescriptor(wTextTimeDmeMinNav1, buffer, sizeof(buffer));
			userNav1TimeDmeMin = convertToNumber(buffer);
			XPGetWidgetDescriptor(wTextTimeDmeMaxNav1, buffer, sizeof(buffer));
			userNav1TimeDmeMax = convertToNumber(buffer);

			XPGetWidgetDescriptor(wTextDistDmeMinNav2, buffer, sizeof(buffer));
			userNav2DistMinDme = convertToNumber(buffer);
			XPGetWidgetDescriptor(wTextDistDmeMaxNav2, buffer, sizeof(buffer));
			userNav2DistMaxDme = convertToNumber(buffer);

			XPGetWidgetDescriptor(wTextTimeDmeMinNav2, buffer, sizeof(buffer));
			userNav2TimeDmeMin = convertToNumber(buffer);
			XPGetWidgetDescriptor(wTextTimeDmeMaxNav2, buffer, sizeof(buffer));
			userNav2TimeDmeMax = convertToNumber(buffer);

			XPGetWidgetDescriptor(wTextAltitudeMin, buffer, sizeof(buffer));
			userAltitudeMin = convertToNumber(buffer);
			XPGetWidgetDescriptor(wTextAltitudeMax, buffer, sizeof(buffer));
			userAltitudeMax = convertToNumber(buffer);
			XPGetWidgetDescriptor(wTextAirspeedMin, buffer, sizeof(buffer));
			userAirspeedMin = convertToNumber(buffer);
			XPGetWidgetDescriptor(wTextAirspeedMax, buffer, sizeof(buffer));
			userAirspeedMax = convertToNumber(buffer);

			XPGetWidgetDescriptor(wTextGPSDmeDistanceMin, buffer, sizeof(buffer));
			userGPSDmeDistanceMin = convertToNumber(buffer);
			XPGetWidgetDescriptor(wTextGPSDmeDistanceMax, buffer, sizeof(buffer));
			userGPSDmeDistanceMax = convertToNumber(buffer);
			XPGetWidgetDescriptor(wTextGPSDmeTimeMin, buffer, sizeof(buffer));
			userGPSDmeTimeMin = convertToNumber(buffer);
			XPGetWidgetDescriptor(wTextGPSDmeTimeMax, buffer, sizeof(buffer));
			userGPSDmeTimeMax = convertToNumber(buffer);

			XPGetWidgetDescriptor(wTextLatitude, buffer, sizeof(buffer));
			objUserLatitude.setValor(buffer);
			XPGetWidgetDescriptor(wTextLongitude, buffer, sizeof(buffer));
			objUserLongitude.setValor(buffer);

			XPGetWidgetDescriptor(wTextNavaidAirportID, buffer, sizeof(buffer));
			toupperChar(buffer);
			userNavaidAirportID = buffer;
			XPGetWidgetDescriptor(wTextNavaidVORID, buffer, sizeof(buffer));
			toupperChar(buffer);
			userNavaidVORID = buffer;
			XPGetWidgetDescriptor(wTextNavaidNDBID, buffer, sizeof(buffer));
			toupperChar(buffer);
			userNavaidNDBID = buffer;
			XPGetWidgetDescriptor(wTextNavaidFixID, buffer, sizeof(buffer));
			toupperChar(buffer);
			userNavaidFixID = buffer;
			XPGetWidgetDescriptor(wTextNavaidDMEID, buffer, sizeof(buffer));
			toupperChar(buffer);
			userNavaidDMEID = buffer;

			XPGetWidgetDescriptor(wTextNavaidAirportDistanceMin, buffer, sizeof(buffer));
			userNavaidAirportDistance = convertToNumber(buffer);
			XPGetWidgetDescriptor(wTextNavaidVORDistanceMin, buffer, sizeof(buffer));
			userNavaidVORDistance = convertToNumber(buffer);
			XPGetWidgetDescriptor(wTextNavaidNDBDistanceMin, buffer, sizeof(buffer));
			userNavaidNDBDistance = convertToNumber(buffer);
			XPGetWidgetDescriptor(wTextNavaidFixDistanceMin, buffer, sizeof(buffer));
			userNavaidFixDistance = convertToNumber(buffer);
			XPGetWidgetDescriptor(wTextNavaidDMEDistanceMin, buffer, sizeof(buffer));
			userNavaidDMEDistance = convertToNumber(buffer);

			// DataRefs
			XPGetWidgetDescriptor(wDataRef1, buffer, sizeof(buffer));
			dataRef1 = buffer;
			XPGetWidgetDescriptor(wDataRefValue1, buffer, sizeof(buffer));
			dataRefValue1 = buffer;

			XPGetWidgetDescriptor(wDataRef2, buffer, sizeof(buffer));
			dataRef2 = buffer;
			XPGetWidgetDescriptor(wDataRefValue2, buffer, sizeof(buffer));
			dataRefValue2 = buffer;

			XPGetWidgetDescriptor(wDataRef3, buffer, sizeof(buffer));
			dataRef3 = buffer;
			XPGetWidgetDescriptor(wDataRefValue3, buffer, sizeof(buffer));
			dataRefValue3 = buffer;

			// Time
			XPGetWidgetDescriptor(wTextTime, buffer, sizeof(buffer));
			timePause = buffer;

			// Flight Plan
			char bufFp[3070];
			XPGetWidgetDescriptor(wTextFlightPlan, bufFp, sizeof(bufFp));
			flightPlan = bufFp;

			saveFileValues();
			return 1;
		}
		if (inParam1 == (intptr_t)wBtnAlertWindowClose)
		{
			hideWindowPaused();
			XPLMCommandKeyStroke(xplm_key_pause);
			resetDataRefsValues();
		}
		if (inParam1 == (intptr_t)wBtnCancel)
		{
			XPHideWidget(wMainWindow);
			MenuItem1 = 0;
			return 1;
		}
		if (inParam1 == (intptr_t)wBtnReload)
		{
			XPLMReloadPlugins();
			return 1;
		}
		if (inParam1 == (intptr_t)wBtnStopStartWebSocket)
		{
			switchSocketServer();
			return 1;
		}
		if (inParam1 == (intptr_t)wBtnFpTranslate)
		{
			translateFlightPlan();
			return 1;
		}
		if (inParam1 == (intptr_t)wBtnFpBringBackOriginal)
		{
			bringBackMyFlightPlan();
			return 1;
		}
		if (inParam1 == (intptr_t)wBtnPasteFlightPlan)
		{
			pasteFlightPlan();
			return 1;
		}
		if (inParam1 == (intptr_t)wBtnCopyFlightPlan)
		{
			copyFlightPlan();
			return 1;
		}
		if (inParam1 == (intptr_t)wBtnCleanFlightPlan)
		{
			cleanFlightPlan();
			return 1;
		}
		if (inParam1 == (intptr_t)wBtnSendFlightPlan) {
			// Send Flight Plan if the Transmitter is OPEN
			translateFlightPlan();
			std::string jsonFP = writeDownFlightPlan();
			if (serverSocketStarted) {
				socketServer->broadcast(jsonFP);
			}
		}
		if (inParam1 == (intptr_t)wBtnLoadFlightPlan)
		{
			readXPlane11FMS();
			return 1;
		}
	}

	if (inMessage == xpMsg_ButtonStateChanged)
	{
		if (inParam1 == (intptr_t)wChkPauseNav1)
		{
			long isStateTrue = XPGetWidgetProperty(wChkPauseNav1, xpProperty_ButtonState, 0);
			isStateTrue ? userNav1IsCheckEnable = 1 : userNav1IsCheckEnable = 0;
			return 1;
		}
		if (inParam1 == (intptr_t)wChkPauseNav2)
		{
			long isStateTrue = XPGetWidgetProperty(wChkPauseNav2, xpProperty_ButtonState, 0);
			isStateTrue ? userNav2IsCheckEnable = 1 : userNav2IsCheckEnable = 0;
			return 1;
		}
		if (inParam1 == (intptr_t)wChkHSIAlignNav1)
		{
			long isStateTrue = XPGetWidgetProperty(wChkHSIAlignNav1, xpProperty_ButtonState, 0);
			isStateTrue ? userNav1IsHSIAlignEnable = 1 : userNav1IsHSIAlignEnable = 0;
			return 1;
		}
		if (inParam1 == (intptr_t)wChkHSIAlignNav2)
		{
			long isStateTrue = XPGetWidgetProperty(wChkHSIAlignNav2, xpProperty_ButtonState, 0);
			isStateTrue ? userNav2IsHSIAlignEnable = 1 : userNav2IsHSIAlignEnable = 0;
			return 1;
		}
		if (inParam1 == (intptr_t)wChkAltitude)
		{
			long isStateTrue = XPGetWidgetProperty(wChkAltitude, xpProperty_ButtonState, 0);
			isStateTrue ? isAltitudeSelected = 1 : isAltitudeSelected = 0;
			return 1;
		}
		if (inParam1 == (intptr_t)wChkAirspeed)
		{
			long isStateTrue = XPGetWidgetProperty(wChkAirspeed, xpProperty_ButtonState, 0);
			isStateTrue ? isAirspeedSelected = 1 : isAirspeedSelected = 0;
			return 1;
		}
		if (inParam1 == (intptr_t)wChkGPS)
		{
			long isStateTrue = XPGetWidgetProperty(wChkGPS, xpProperty_ButtonState, 0);
			isStateTrue ? isGPSSelected = 1 : isGPSSelected = 0;
			return 1;
		}
		if (inParam1 == (intptr_t)wChkLatLon)
		{
			long isStateTrue = XPGetWidgetProperty(wChkLatLon, xpProperty_ButtonState, 0);
			isStateTrue ? isLatLongSelected = 1 : isLatLongSelected = 0;
			return 1;
		}
		if (inParam1 == (intptr_t)wChkNavaidAirport)
		{
			long isStateTrue = XPGetWidgetProperty(wChkNavaidAirport, xpProperty_ButtonState, 0);
			isStateTrue ? isNavaidAirportSelected = 1 : isNavaidAirportSelected = 0;
			return 1;
		}
		if (inParam1 == (intptr_t)wChkNavaidVOR)
		{
			long isStateTrue = XPGetWidgetProperty(wChkNavaidVOR, xpProperty_ButtonState, 0);
			isStateTrue ? isNavaidVORSelected = 1 : isNavaidVORSelected = 0;
			return 1;
		}
		if (inParam1 == (intptr_t)wChkNavaidNDB)
		{
			long isStateTrue = XPGetWidgetProperty(wChkNavaidNDB, xpProperty_ButtonState, 0);
			isStateTrue ? isNavaidNDBSelected = 1 : isNavaidNDBSelected = 0;
			return 1;
		}
		if (inParam1 == (intptr_t)wChkNavaidFix)
		{
			long isStateTrue = XPGetWidgetProperty(wChkNavaidFix, xpProperty_ButtonState, 0);
			isStateTrue ? isNavaidFixSelected = 1 : isNavaidFixSelected = 0;
			return 1;
		}
		if (inParam1 == (intptr_t)wChkNavaidDME)
		{
			long isStateTrue = XPGetWidgetProperty(wChkNavaidDME, xpProperty_ButtonState, 0);
			isStateTrue ? isNavaidDMESelected = 1 : isNavaidDMESelected = 0;
			return 1;
		}
		if (inParam1 == (intptr_t)wChkDataRef1)
		{
			long isStateTrue = XPGetWidgetProperty(wChkDataRef1, xpProperty_ButtonState, 0);
			isStateTrue ? isDataRef1Selected = 1 : isDataRef1Selected = 0;
			resetDataRefsValues();
			return 1;
		}
		if (inParam1 == (intptr_t)wChkDataRef2)
		{
			long isStateTrue = XPGetWidgetProperty(wChkDataRef2, xpProperty_ButtonState, 0);
			isStateTrue ? isDataRef2Selected = 1 : isDataRef2Selected = 0;
			resetDataRefsValues();
			return 1;
		}
		if (inParam1 == (intptr_t)wChkDataRef3)
		{
			long isStateTrue = XPGetWidgetProperty(wChkDataRef3, xpProperty_ButtonState, 0);
			isStateTrue ? isDataRef3Selected = 1 : isDataRef3Selected = 0;
			resetDataRefsValues();
			return 1;
		}
		if (inParam1 == (intptr_t)wChkTime)
		{
			long isStateTrue = XPGetWidgetProperty(wChkTime, xpProperty_ButtonState, 0);
			isStateTrue ? isTimePauseSelected = 1 : isTimePauseSelected = 0;
			return 1;
		}
	}

	return 0;
}

std::string getDescriptionGPSDestinationType(int destinationType)
{
	std::string ret;
		switch (destinationType) {
		case xplm_Nav_Airport:
			ret = "Airport";
			break;
		case xplm_Nav_NDB:
			ret = "NBD";
			break;
		case xplm_Nav_VOR:
			ret = "VOR";
			break;
		case xplm_Nav_ILS:
			ret = "ILS";
			break;
		case xplm_Nav_Localizer:
			ret = "Localizer";
			break;
		case xplm_Nav_GlideSlope:
			ret = "Glideslope";
			break;
		case xplm_Nav_OuterMarker:
			ret = "OuterMarker";
			break;
		case xplm_Nav_MiddleMarker:
			ret = "MiddleMarker";
			break;
		case xplm_Nav_InnerMarker:
			ret = "InnerMarker";
			break;
		case xplm_Nav_Fix:
			ret = "FIX";
			break;
		case xplm_Nav_DME:
			ret = "DME";
			break;
		case xplm_Nav_LatLon:
			ret = "Lat./Long.";
			break;
		case 28:
			ret = "FMS";
			break;
		default:
			ret = "Not Found";
			break;
	}
	return ret;
}

float roundLatLon(float d)
{
	return floor(d + 0.5);
}

navaid setInfoNavaid(int sizeCheck, XPWidgetID textId, XPWidgetID captionDistance, XPWidgetID captionDesc, 
	float currentLatitude, float currentLongitude, int typeNavaid) {

	navaid navaidTarget;
	navaid navCurrent;
	navCurrent.latitude = currentLatitude;
	navCurrent.longitude = currentLongitude;
	char buffer[128];
	char label[64];
	// Airport
	buffer[0] = 0;
	XPGetWidgetDescriptor(textId, buffer, sizeof(buffer));
	if (buffer[0] != 0 && strlen(buffer) >= sizeCheck) {
		toupperChar(buffer);
		navaidTarget = navManager.getLatitudeLongitude(currentLatitude, currentLongitude, buffer, typeNavaid);
		if (navaidTarget.statusOK == 1) {
			float distanceNavs = navManager.calculateDistanceBetweenNavaids(navCurrent, navaidTarget);
			navaidTarget.distance = distanceNavs;
			sprintf(label, "%snm", convertToString(distanceNavs).c_str());
			XPSetWidgetDescriptor(captionDistance, label);
			XPSetWidgetDescriptor(captionDesc, navaidTarget.name.c_str());
		}
		else {
			XPSetWidgetDescriptor(captionDistance, "---");
			XPSetWidgetDescriptor(captionDesc, "Not Found");
		}
	}
	else {
		XPSetWidgetDescriptor(captionDistance, "");
		XPSetWidgetDescriptor(captionDesc, "");
	}
	return navaidTarget;

}

/*
* Search for All Informations necessary to check the Pausing feature
*/
void getXPlaneDataInfos()
{
	char label[256];

	// Current Latitude e Longitude
	double currentLatitude = XPLMGetDatad(XPLMFindDataRef("sim/flightmodel/position/latitude"));
	double currentLongitude = XPLMGetDatad(XPLMFindDataRef("sim/flightmodel/position/longitude"));
	objCurrentLatitude.setValor(currentLatitude);
	objCurrentLongitude.setValor(currentLongitude);
	sprintf(label, "%s / %s", objCurrentLatitude.getValorStr().c_str(), objCurrentLongitude.getValorStr().c_str());
	XPSetWidgetDescriptor(wCaptionLatLon, label);
	navaidCurrentDestination.latitude  = currentLatitude;
	navaidCurrentDestination.longitude = currentLongitude;

	// General Parameters
	isBatteryOn       = XPLMGetDatai(XPLMFindDataRef("sim/cockpit/electrical/battery_on"));
	isGamePaused      = XPLMGetDatai(XPLMFindDataRef("sim/time/paused"));

	// Altitude and Airspeed
	currentAltitude = (int)XPLMGetDataf(XPLMFindDataRef("sim/cockpit2/gauges/indicators/altitude_ft_pilot"));
	currentAirspeed = (int)XPLMGetDataf(XPLMFindDataRef("sim/cockpit2/gauges/indicators/airspeed_kts_pilot"));
	sprintf(label, "%dft", (int)currentAltitude);
	XPSetWidgetDescriptor(wCaptionAltitude, label);
	sprintf(label, "%dkts", (int)currentAirspeed);
	XPSetWidgetDescriptor(wCaptionAirspeed, label);

	// GPS Destination
	currentGPSDistDme              = (int)XPLMGetDataf(XPLMFindDataRef("sim/cockpit/radios/gps_dme_dist_m"));
	currentGPSTimeDme              = (int)XPLMGetDataf(XPLMFindDataRef("sim/cockpit/radios/gps_dme_time_secs"));
	if (currentGPSTimeDme < 0) {
		currentGPSTimeDme = 0;
	}
	XPLMNavRef  gpsDestination     = XPLMGetGPSDestination();
	XPLMNavType gpsDestinationType = XPLMGetGPSDestinationType();
	int   outFrequency;
	char  outID[10];
	char  outName[256];
	float outLatitude;
	float outLongitude;
	float outHeading;
	XPLMGetNavAidInfo(gpsDestination, &gpsDestinationType, &outLatitude, &outLongitude, NULL, &outFrequency, &outHeading, outID, outName, NULL);
	if (strcmp(outID, "----") != 0)  {
		std::string descripDestTypeGPS = getDescriptionGPSDestinationType(gpsDestinationType);
		sprintf(label, "%s: %s  (%s)", descripDestTypeGPS.c_str(), outName, outID);
		navaidGpsDestination.id              = std::string(outID);
		navaidGpsDestination.name            = std::string(outName);
		navaidGpsDestination.latitude        = outLatitude;
		navaidGpsDestination.longitude       = outLongitude;
		navaidGpsDestination.dmeDistance     = currentGPSDistDme;
		navaidGpsDestination.dmeTime         = currentGPSTimeDme;
		float distanceNavs = navManager.calculateDistanceBetweenNavaids(navaidCurrentDestination, navaidGpsDestination);
		navaidGpsDestination.distance        = distanceNavs;
		navaidGpsDestination.typeDescription = descripDestTypeGPS;
		navaidGpsDestination.statusOK        = 1;
	} else {
		navaidGpsDestination.id        = "----";
		navaidGpsDestination.name      = "None";
		navaidGpsDestination.statusOK  = 0;
		std::string descripDestTypeGPS = "";
		sprintf(label, "%s", "");
	}

	// FMS Destination
	int entryFmsNextDestination;
	if ( XPLMCountFMSEntries() > 2 ) {
		entryFmsNextDestination = XPLMGetDestinationFMSEntry();
		XPLMNavType fmsNavType;
		XPLMNavRef fmsNavRef;
		int fmsAltitude;
		float fmsLatitude;
		float fmsLongitude;
		char fmsOutID[10];
		char fmsOutName[256];
		XPLMGetFMSEntryInfo(entryFmsNextDestination, &fmsNavType, fmsOutID, &fmsNavRef, &fmsAltitude, &fmsLatitude, &fmsLongitude);
		XPLMGetNavAidInfo(fmsNavRef, NULL, NULL, NULL, NULL, NULL, NULL, fmsOutID, fmsOutName, NULL);
		navaidFmsDestination.id              = std::string(fmsOutID);
		navaidFmsDestination.name            = std::string(fmsOutName);
		navaidFmsDestination.latitude        = fmsLatitude;
		navaidFmsDestination.longitude       = fmsLongitude;
		navaidFmsDestination.dmeDistance     = -1;
		navaidFmsDestination.dmeTime         = -1;
		float distanceNavs                   = navManager.calculateDistanceBetweenNavaids(navaidCurrentDestination, navaidFmsDestination);
		navaidFmsDestination.distance        = distanceNavs;
		char timeFms[10];
		navManager.calculateTimeFormatted(timeFms, currentAirspeed, (int)distanceNavs);
		navaidFmsDestination.fmsTime         = std::string(timeFms);
		navaidFmsDestination.statusOK        = 1;
		navaidFmsDestination.typeDescription = getDescriptionGPSDestinationType(fmsNavType);
	} else {
		navaidFmsDestination.id          = "---";
		navaidFmsDestination.name        = "None";
		navaidFmsDestination.statusOK    = 0;
	}

	/*
	log("GPS --> descripDestTypeGPS:" + navaidGpsDestination.typeDescription
		+ ", latitude=" + std::to_string(navaidGpsDestination.latitude)
		+ ", longitude=" + std::to_string(navaidGpsDestination.longitude)
		+ ", of " + std::string(navaidGpsDestination.id)
		+ "-" + std::string(navaidGpsDestination.name)
		+ ", distance=" + std::to_string(navaidGpsDestination.distance)
		+ ", time=" + std::to_string(navaidGpsDestination.dmeTime)
	);

	log("FMS --> entryFmsNextDestination:" + std::to_string(entryFmsNextDestination)
		+ ", latitude=" + std::to_string(navaidFmsDestination.latitude)
		+ ", longitude=" + std::to_string(navaidFmsDestination.longitude)
		+ ", of " + std::string(navaidFmsDestination.id)
		+ "-" + std::string(navaidFmsDestination.name)
		+ ", distance=" + std::to_string(navaidFmsDestination.distance)
		+ ", time=" + navaidFmsDestination.fmsTime
	);
	*/

	XPSetWidgetDescriptor(wCaptionGPS, label);
	sprintf(label, "%dnm", currentGPSDistDme);
	XPSetWidgetDescriptor(wCaptionGPSDmeDistance, label);
	sprintf(label, "%dmin", currentGPSTimeDme);
	XPSetWidgetDescriptor(wCaptionGPSDmeTime, label);

	currentFreqNav1 = XPLMGetDatai(XPLMFindDataRef("sim/cockpit/radios/nav1_freq_hz"));
	currentDistDmeNav1 = XPLMGetDataf(XPLMFindDataRef("sim/cockpit2/radios/indicators/nav1_dme_distance_nm"));
	currentTimeDmeNav1 = XPLMGetDataf(XPLMFindDataRef("sim/cockpit2/radios/indicators/nav1_dme_time_min"));
	currentHsiAlignmentNav1 = XPLMGetDataf(XPLMFindDataRef("sim/cockpit2/radios/indicators/nav1_hdef_dots_pilot"));
	if (currentHsiAlignmentNav1 < 0) {
		currentHsiAlignmentNav1 *= -1;
	}
	userNav1HasDme = XPLMGetDatai(XPLMFindDataRef("sim/cockpit/radios/nav1_has_dme"));

	currentFreqNav2 = XPLMGetDatai(XPLMFindDataRef("sim/cockpit/radios/nav2_freq_hz"));
	currentTimeDmeNav2 = XPLMGetDataf(XPLMFindDataRef("sim/cockpit2/radios/indicators/nav2_dme_time_min"));
	currentDistDmeNav2 = XPLMGetDataf(XPLMFindDataRef("sim/cockpit2/radios/indicators/nav2_dme_distance_nm"));
	currentHsiAlignmentNav2 = XPLMGetDataf(XPLMFindDataRef("sim/cockpit2/radios/indicators/nav2_hdef_dots_pilot"));
	if (currentHsiAlignmentNav2 < 0) {
		currentHsiAlignmentNav2 *= -1;
	}
	userNav2HasDme = XPLMGetDatai(XPLMFindDataRef("sim/cockpit/radios/nav2_has_dme"));

	int lit;
	userNav1HasDme ? lit = 1 : lit = 0;
	XPSetWidgetProperty(wCaptionFreqNav1, xpProperty_CaptionLit, lit);
	userNav2HasDme ? lit = 1 : lit = 0;
	XPSetWidgetProperty(wCaptionFreqNav2, xpProperty_CaptionLit, lit);

	/**
	* Just to layout the screen
	*/
	//    currentDistDmeNav1 = 9999;
	//    currentDistDmeNav2 = 9999;
	//    currentTimeDmeNav1 = 999;
	//    currentTimeDmeNav2 = 999;
	//    currentHsiAlignmentNav1 = 2.50;
	//    currentHsiAlignmentNav2 = 2.50;


	XPSetWidgetDescriptor(wCaptionFreqNav1, formatFreqHz(currentFreqNav1).c_str());
	sprintf(label, "%dnm", (int)currentDistDmeNav1);
	XPSetWidgetDescriptor(wCaptionDistDmeNav1, label);
	sprintf(label, "%dmin", (int)currentTimeDmeNav1);
	XPSetWidgetDescriptor(wCaptionTimeDmeNav1, label);
	XPSetWidgetDescriptor(wHSIAlignmentNav1, formatNumber(currentHsiAlignmentNav1, 2).c_str());

	XPSetWidgetDescriptor(wCaptionFreqNav2, formatFreqHz(currentFreqNav2).c_str());
	sprintf(label, "%dnm", (int)currentDistDmeNav2);
	XPSetWidgetDescriptor(wCaptionDistDmeNav2, label);
	sprintf(label, "%dmin", (int)currentTimeDmeNav2);
	XPSetWidgetDescriptor(wCaptionTimeDmeNav2, label);
	XPSetWidgetDescriptor(wHSIAlignmentNav2, formatNumber(currentHsiAlignmentNav2, 2).c_str());

	// Feed Info About Navaids
	navaidAirport = setInfoNavaid(4,wTextNavaidAirportID, wCaptionNavaidAirportDistance, wCaptionNavaidAirportDesc,
		currentLatitude, currentLongitude, xplm_Nav_Airport);
	navaidVOR = setInfoNavaid(3, wTextNavaidVORID, wCaptionNavaidVORDistance, wCaptionNavaidVORDesc,
		currentLatitude, currentLongitude, xplm_Nav_VOR);
	navaidNDB = setInfoNavaid(2, wTextNavaidNDBID, wCaptionNavaidNDBDistance, wCaptionNavaidNDBDesc,
		currentLatitude, currentLongitude, xplm_Nav_NDB);
	navaidFix = setInfoNavaid(2, wTextNavaidFixID, wCaptionNavaidFixDistance, wCaptionNavaidFixDesc,
		currentLatitude, currentLongitude, xplm_Nav_Fix);
	navaidDME = setInfoNavaid(2, wTextNavaidDMEID, wCaptionNavaidDMEDistance, wCaptionNavaidDMEDesc,
		currentLatitude, currentLongitude, xplm_Nav_DME);

	// Time
	hourTimeZ = XPLMGetDatai(XPLMFindDataRef("sim/cockpit2/clock_timer/zulu_time_hours"));
	minuTimeZ = XPLMGetDatai(XPLMFindDataRef("sim/cockpit2/clock_timer/zulu_time_minutes"));
	secoTimeZ = XPLMGetDatai(XPLMFindDataRef("sim/cockpit2/clock_timer/zulu_time_seconds"));

	sprintf(label, "%02d:%02d:%02d", hourTimeZ, minuTimeZ, secoTimeZ);
	XPSetWidgetDescriptor(wCaptionTime, label);
}

std::string checkSignal(int vlrDir, int vlrEsq) {
	if (vlrDir > vlrEsq) {
		return ">";
	}
	else
		if (vlrDir < vlrEsq) {
		return "<";
		}
	return "=";
}

int checkDataRefs(int number);
/*
* Check if something hit the state necessary to pause the x-plane and...  pause!
*/
float pauseXPlane() {
	float result = 0;

	if (userNav1IsCheckEnable && userNav1HasDme) {
		if (((int)currentDistDmeNav1) <= userNav1DistMinDme) {
			std::string signal = checkSignal((int)currentDistDmeNav1, userNav1DistMinDme);
			sprintf(msgPause, "Distance DME Nav1 Min. (%d %s %d).", ((int)currentDistDmeNav1), signal.c_str(), userNav1DistMinDme);
			result = 1;
		}
		else
		if (((int)currentDistDmeNav1) >= userNav1DistMaxDme) {
			std::string signal = checkSignal((int)currentDistDmeNav1, userNav1DistMaxDme);
			sprintf(msgPause, "Distance DME Nav1 Max. (%d %s %d).", ((int)currentDistDmeNav1), signal.c_str(), userNav1DistMaxDme);
			result = 1;
		}
		else
		if (((int)currentTimeDmeNav1) <= userNav1TimeDmeMin) {
			std::string signal = checkSignal((int)currentTimeDmeNav1, userNav1TimeDmeMin);
			sprintf(msgPause, "Time Distance DME Min. Nav1 (%d %s %d).", ((int)currentTimeDmeNav1), signal.c_str(), userNav1TimeDmeMin);
			result = 1;
		}
		else
		if (((int)currentTimeDmeNav1) >= userNav1TimeDmeMax) {
			std::string signal = checkSignal((int)currentTimeDmeNav1, userNav1TimeDmeMax);
			sprintf(msgPause, "Time Distance DME Max. Nav1 (%d %s %d).", ((int)currentTimeDmeNav1), signal.c_str(), userNav1TimeDmeMax);
			result = 1;
		}
		else
		if (userNav1IsHSIAlignEnable && currentHsiAlignmentNav1 <= 0.50f) {
			sprintf(msgPause, "HSI Nav1 Aligned (%d <= 0.50)", ((int)currentHsiAlignmentNav1));
			result = 1;
		}
		if (result == 1) {
			wChkToUnSelect = wChkPauseNav1;
			userNav1IsCheckEnable = 0;
		}
	}

	if (userNav2IsCheckEnable && userNav2HasDme) {
		if (((int)currentDistDmeNav2) <= userNav2DistMinDme) {
			std::string signal = checkSignal((int)currentDistDmeNav2, userNav2DistMinDme);
			sprintf(msgPause, "Distance DME Nav2 Min. (%d %s %d).", ((int)currentDistDmeNav2), signal.c_str(), userNav2DistMinDme);
			result = 1;
		}
		else
		if (((int)currentDistDmeNav2) >= userNav2DistMaxDme) {
			std::string signal = checkSignal((int)currentDistDmeNav2, userNav2DistMaxDme);
			sprintf(msgPause, "Distance DME Nav2 Max. (%d %s %d).", ((int)currentDistDmeNav2), signal.c_str(), userNav2DistMaxDme);
			result = 1;
		}
		else
		if (((int)currentTimeDmeNav2) <= userNav2TimeDmeMin) {
			std::string signal = checkSignal((int)currentTimeDmeNav2, userNav2TimeDmeMin);
			sprintf(msgPause, "Time Distance DME Min. Nav2 (%d %s %d).", ((int)currentTimeDmeNav2), signal.c_str(), userNav2TimeDmeMin);
			result = 1;
		}
		else
		if (((int)currentTimeDmeNav2) >= userNav2TimeDmeMax) {
			std::string signal = checkSignal((int)currentTimeDmeNav2, userNav2TimeDmeMax);
			sprintf(msgPause, "Time Distance DME Max. Nav2 (%d %s %d).", ((int)currentTimeDmeNav2), signal.c_str(), userNav2TimeDmeMax);
			result = 1;
		}
		else
		if (userNav2IsHSIAlignEnable && currentHsiAlignmentNav2 <= 0.50f) {
			sprintf(msgPause, "HSI Nav2 Aligned (%d <= 0.50)", ((int)currentHsiAlignmentNav2));
			result = 1;
		}
		if (result == 1) {
			wChkToUnSelect = wChkPauseNav2;
			userNav2IsCheckEnable = 0;
		}
	}

	if (isAltitudeSelected) {
		if (currentAltitude <= userAltitudeMin) {
			std::string signal = checkSignal(currentAltitude, userAltitudeMin);
			sprintf(msgPause, "Altitude Min. (%d %s %d)", currentAltitude, signal.c_str(), userAltitudeMin);
			result = 1;
		}
		else
		if (result == 0 && (currentAltitude >= userAltitudeMax))  {
			std::string signal = checkSignal(currentAltitude, userAltitudeMax);
			sprintf(msgPause, "Altitude Max. (%d %s %d)", currentAltitude, signal.c_str(), userAltitudeMax);
			result = 1;
		}
		if (result == 1) {
			wChkToUnSelect = wChkAltitude;
			isAltitudeSelected = 0;
		}
	}

	if (isAirspeedSelected) {
		if (currentAirspeed <= userAirspeedMin) {
			std::string signal = checkSignal(currentAirspeed, userAirspeedMin);
			sprintf(msgPause, "Airspeed Min. (%d %s %d)", currentAirspeed, signal.c_str(), userAirspeedMin);
			result = 1;
		}
		else
		if (result == 0 && (currentAirspeed >= userAirspeedMax))  {
			std::string signal = checkSignal(currentAirspeed, userAirspeedMax);
			sprintf(msgPause, "Airspeed Max. (%d %s %d)", currentAirspeed, signal.c_str(), userAirspeedMax);
			result = 1;
		}
		if (result == 1) {
			wChkToUnSelect = wChkAirspeed;
			isAirspeedSelected = 0;
		}
	}

	if (isGPSSelected) {
		if (((int)currentGPSDistDme) <= userGPSDmeDistanceMin) {
			std::string signal = checkSignal((int)currentGPSDistDme, userGPSDmeDistanceMin);
			sprintf(msgPause, "Distance DME GPS Min. (%d %s %d).", ((int)currentGPSDistDme), signal.c_str(), userGPSDmeDistanceMin);
			result = 1;
		}
		else
		if (((int)currentGPSDistDme) >= userGPSDmeDistanceMax) {
			std::string signal = checkSignal((int)currentGPSDistDme, userGPSDmeDistanceMax);
			sprintf(msgPause, "Distance DME GPS Max. (%d %s %d).", ((int)currentGPSDistDme), signal.c_str(), userGPSDmeDistanceMax);
			result = 1;
		}
		else
		if (((int)currentGPSTimeDme) <= userGPSDmeTimeMin) {
			std::string signal = checkSignal((int)currentGPSTimeDme, userGPSDmeTimeMin);
			sprintf(msgPause, "Time Distance GPS Min. (%d %s %d).", ((int)currentGPSTimeDme), signal.c_str(), userGPSDmeTimeMin);
			result = 1;
		}
		else
		if (((int)currentGPSTimeDme) >= userGPSDmeTimeMax) {
			std::string signal = checkSignal((int)currentGPSTimeDme, userGPSDmeTimeMax);
			sprintf(msgPause, "Time Distance GPS Max. (%d %s %d).", ((int)currentGPSTimeDme), signal.c_str(), userGPSDmeTimeMax);
			result = 1;
		}
		if (result == 1) {
			wChkToUnSelect = wChkGPS;
			isGPSSelected = 0;
		}
	}
	if (isLatLongSelected) {
		int resultLatitude  = objCurrentLatitude.compare(objUserLatitude, acceptableDifference);
		int resultLongitude = objCurrentLongitude.compare(objUserLongitude, acceptableDifference);

		if (resultLatitude == 1 && resultLongitude == 1) {
			sprintf(msgPause, "Latitude/Longitude near of %s/%s", objCurrentLatitude.getValorStr().c_str(), objCurrentLongitude.getValorStr().c_str());
			result = 1;
		}
		if (result == 1) {
			wChkToUnSelect = wChkLatLon;
			isLatLongSelected = 0;
		}
	}
	if (isNavaidAirportSelected) {
		if (userNavaidAirportDistance >= navaidAirport.distance) {
			sprintf(msgPause, "Distance Airport %s-%s (%s <= %s)", navaidAirport.id.c_str(), navaidAirport.name.c_str(),
				convertToString(navaidAirport.distance).c_str(),convertToString(userNavaidAirportDistance).c_str());
			result = 1;
			wChkToUnSelect = wChkNavaidAirport;
			isNavaidAirportSelected = 0;
		}
	}
	if (isNavaidVORSelected) {
		if (userNavaidVORDistance >= navaidVOR.distance) {
			sprintf(msgPause, "Distance VOR %s-%s (%s <= %s)", navaidVOR.id.c_str(), navaidVOR.name.c_str(),
				convertToString(navaidVOR.distance).c_str(), convertToString(userNavaidVORDistance).c_str());
			result = 1;
			wChkToUnSelect = wChkNavaidVOR;
			isNavaidVORSelected = 0;
		}
	}
	if (isNavaidNDBSelected) {
		if (userNavaidNDBDistance >= navaidNDB.distance) {
			sprintf(msgPause, "Distance NDB %s-%s (%s <= %s)", navaidNDB.id.c_str(), navaidNDB.name.c_str(),
				convertToString(navaidNDB.distance).c_str(), convertToString(userNavaidNDBDistance).c_str());
			result = 1;
			wChkToUnSelect = wChkNavaidNDB;
			isNavaidNDBSelected = 0;
		}
	}
	if (isNavaidFixSelected) {
		if (userNavaidFixDistance >= navaidFix.distance) {
			sprintf(msgPause, "Distance Fix %s-%s (%s <= %s)", navaidFix.id.c_str(), navaidFix.name.c_str(),
				convertToString(navaidFix.distance).c_str(), convertToString(userNavaidFixDistance).c_str());
			result = 1;
			wChkToUnSelect = wChkNavaidFix;
			isNavaidFixSelected = 0;
		}
	}
	if (isNavaidDMESelected) {
		if (userNavaidDMEDistance >= navaidDME.distance) {
			sprintf(msgPause, "Distance DME %s-%s (%s <= %s)", navaidDME.id.c_str(), navaidDME.name.c_str(),
				convertToString(navaidDME.distance).c_str(), convertToString(userNavaidDMEDistance).c_str());
			result = 1;
			wChkToUnSelect = wChkNavaidDME;
			isNavaidDMESelected = 0;
		}
	}

	if (isDataRef1Selected) {
		result = checkDataRefs(1);
		if (result == 1) {
			wChkToUnSelect     = wChkDataRef1;
			isDataRef1Selected = 0;
		}
	} else
	if (isDataRef2Selected) {
		result = checkDataRefs(2);
		if (result == 1) {
			wChkToUnSelect     = wChkDataRef2;
			isDataRef2Selected = 0;
		}
	} else
	if (isDataRef3Selected) {
		result = checkDataRefs(3);
		if (result == 1) {
			wChkToUnSelect     = wChkDataRef3;
			isDataRef3Selected = 0;
		}
	}

	if (isTimePauseSelected) {
		if (timePause.length() == 8) {
			int hourPause = atoi(timePause.substr(0, 2).c_str());
			int minPause  = atoi(timePause.substr(3, 5).c_str());
			int secPause  = atoi(timePause.substr(6, 8).c_str());

			if ( hourPause == hourTimeZ && minPause == minuTimeZ && secPause == secoTimeZ ) {
				sprintf(msgPause, "Time reached:  == %s hrs", timePause.c_str());
				result              = 1;
				isTimePauseSelected = 0;
				wChkToUnSelect = wChkTime;
			}
		}
	}

	return result;
}

int checkDataRefs(int number) {
	XPWidgetID wDataRef;
	XPWidgetID wDataRefValue;
	XPWidgetID debugId;
	XPWidgetID wCheckbox;
	XPWidgetID wCaption;
	int        result = 0;

	if (number == 1) {
		wDataRef      = wDataRef1;
		wDataRefValue = wDataRefValue1;
		debugId       = widgetDebug1;
		wCheckbox     = wChkDataRef1;
		wCaption = wCaptionDataRef1;
	} else
	if (number == 2) {
		wDataRef      = wDataRef2;
		wDataRefValue = wDataRefValue2;
		debugId       = widgetDebug2;
		wCheckbox     = wChkDataRef2;
		wCaption      = wCaptionDataRef2;
	} else
	if (number == 3) {
		wDataRef      = wDataRef3;
		wDataRefValue = wDataRefValue3;
		debugId       = widgetDebug3;
		wCheckbox     = wChkDataRef3;
		wCaption      = wCaptionDataRef3;
	}

	char b1[256], b2[256];
	XPGetWidgetDescriptor(wDataRef, b1, sizeof(b1));
	XPGetWidgetDescriptor(wDataRefValue, b2, sizeof(b2));
	
	std::string s                      = b2;
	std::string signal                 = "";
	std::string realDataRefValueStr    = "";
	std::string wishedDataRefValueStr  = "";
	// Float or Int (if there's a decimal point, so...  it is a Float value, otherwise it will be treated as a Integer)
	if (s.find('.') != std::string::npos) {
		float realDataRefValue   = XPLMGetDataf(XPLMFindDataRef(b1));
		float wishedDataRefValue = 0;

		XPSetWidgetDescriptor(wCaption, std::to_string(realDataRefValue).c_str());
		XPSetWidgetProperty(wCaption, xpProperty_CaptionLit, 1);

		if (s.substr(0, 2) == ">=") {
			signal = ">=";
			wishedDataRefValue = convertToNumber(s.substr(2));

			log(s.substr(2));
			log(std::to_string(wishedDataRefValue));

			if (realDataRefValue >= wishedDataRefValue) {
				result = 1;
			}
		}
		else
		if (s.substr(0, 2) == "<=") {
			signal = "<=";
			wishedDataRefValue = convertToNumber(s.substr(2));
			if (realDataRefValue <= wishedDataRefValue) {
				result = 1;
			}
		}
		else
		if (s.substr(0, 2) == "<>") {
			signal = "<>";
			wishedDataRefValue = convertToNumber(s.substr(2));
			if (realDataRefValue != wishedDataRefValue) {
				result = 1;
			}
		} else
		if (s.substr(0, 1) == "=") {
			signal = "=";
			wishedDataRefValue = convertToNumber(s.substr(1));
			if (realDataRefValue == wishedDataRefValue) {
				result = 1;
			}
		}
		else
		if (s.substr(0, 1) == ">" ) {
			signal = ">";
			wishedDataRefValue = convertToNumber(s.substr(1));
			if (realDataRefValue > wishedDataRefValue) {
				result = 1;
			}
		} else
		if (s.substr(0, 1) == "<") {
			signal = "<";
			wishedDataRefValue = convertToNumber(s.substr(1));
			if (realDataRefValue < wishedDataRefValue) {
				result = 1;
			}
		} else {
			signal = "=";
			wishedDataRefValue = convertToNumber(s);
			if (realDataRefValue == wishedDataRefValue) {
				result = 1;
			}
		}

		if (result == 1) {
			realDataRefValueStr   = convertToString(realDataRefValue).c_str();
			wishedDataRefValueStr = convertToString(wishedDataRefValue).c_str();
		}
	} else {
		int   realDataRefValue   = (int)XPLMGetDataf(XPLMFindDataRef(b1));
		int   wishedDataRefValue = 0;

		XPSetWidgetDescriptor(wCaption, std::to_string(realDataRefValue).c_str());
		XPSetWidgetProperty(wCaption, xpProperty_CaptionLit, 1);

		if (s.substr(0, 2) == ">=") {
			signal = ">=";
			wishedDataRefValue = convertToNumber(s.substr(2));
			if (realDataRefValue >= wishedDataRefValue) {
				result = 1;
			}
		} else
		if (s.substr(0, 2) == "<=") {
			signal = "<=";
			wishedDataRefValue = convertToNumber(s.substr(2));
			if (realDataRefValue <= wishedDataRefValue) {
				result = 1;
			}
		} else
		if (s.substr(0, 2) == "<>") {
			signal = "<>";
			wishedDataRefValue = convertToNumber(s.substr(2));
			if (realDataRefValue != wishedDataRefValue) {
				result = 1;
			}
		} else
		if (s.substr(0, 1) == "=") {
			signal = "=";
			wishedDataRefValue = convertToNumber(s.substr(1));
			if (realDataRefValue == wishedDataRefValue) {
				result = 1;
			}
		} else
		if (s.substr(0, 1) == ">") {
			signal = ">";
			wishedDataRefValue = convertToNumber(s.substr(1));
			if (realDataRefValue > wishedDataRefValue) {
				result = 1;
			}
		} else
		if (s.substr(0, 1) == "<") {
			signal = "<";
			wishedDataRefValue = convertToNumber(s.substr(1));
			if (realDataRefValue < wishedDataRefValue) {
				result = 1;
			}
		} else {
			signal = "=";
			wishedDataRefValue = convertToNumber(s);
			if (realDataRefValue == wishedDataRefValue) {
				result = 1;
			}
		}

		if (result == 1) {
			realDataRefValueStr = convertToString(realDataRefValue).c_str();
			wishedDataRefValueStr = convertToString(wishedDataRefValue).c_str();
		}
	}

	sprintf(msgPause, "DataRef %s (%s %s %s)", b1, realDataRefValueStr.c_str(), signal.c_str(), wishedDataRefValueStr.c_str());
	return result;
}

void hideWindowPaused() {
	XPHideWidget(wAlertWindow);
	AlertWindowShown = 0;
	//sprintf(msgPause, "");
	//sprintf(msgPause2, "");
	msgPause[0] = '\0';
	msgPause2[0] = '\0';
}

void showWindowPaused() {
	int left, bottom, right, top;
	XPLMGetScreenBoundsGlobal(&left, &top, &right, &bottom);

	//int aX = 730, aY = 780;
	//int aX = left + 300;

	
	int aW = 600, aH = 280;
	int aX = ((right - left) - 600) / 2;
	//int aY = top  - 280;
	int aY = ((bottom + top) + 280) / 2;

	log(msgPause);
	log(msgPause2);

	wAlertWindow = XPCreateWidget(aX, aY, aX + aW, aY - aH, 1, "Pause For Me!!!", 1, NULL, xpWidgetClass_SubWindow);
	//XPSetWidgetProperty(wAlertWindow, xpProperty_MainWindowHasCloseBoxes, 1) (Erro quando ligado!!!);
	XPWidgetID c0 = XPCreateWidget(aX + 20, aY, aX + aW + 20, aY - aH + 220, 1, "════════════════════════════ ►►► PAUSE FOR ME!!! ◄◄◄ ════════════════════════════", 0, wAlertWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(c0, xpProperty_CaptionLit, 1);

	XPWidgetID c1 = XPCreateWidget(aX + 20, aY, aX + aW + 20, aY - aH + 52, 1, "REASON ►►►", 0, wAlertWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(c1, xpProperty_CaptionLit, 1);

	XPWidgetID c2 = XPCreateWidget(aX + 20, aY, aX + aW + 20, aY - aH + 10, 1, msgPause, 0, wAlertWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(c2, xpProperty_CaptionLit, 0);

	XPWidgetID c3 = XPCreateWidget(aX + 20, aY, aX + aW + 20, aY - aH - 15, 1, msgPause2, 0, wAlertWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(c3, xpProperty_CaptionLit, 0);

	wBtnAlertWindowClose = XPCreateWidget(aX + 200, aY, aX + 400, aY - aH - 190, 1, "  ◄◄◄ UNPAUSE ►►►  ", 0, wAlertWindow, xpWidgetClass_Button);
	XPSetWidgetProperty(wBtnAlertWindowClose, xpProperty_ButtonType, xpPushButton);
	XPAddWidgetCallback(wAlertWindow, widgetWidgetHandler);
	AlertWindowShown = 1;
}

float CallBackXPlane(float  inElapsedSinceLastCall,
	float  inElapsedTimeSinceLastFlightLoop,
	int    inCounter,
	void * inRefcon)
{
	//PAUSE CHECKING via SCREEN USER PARAMETERS
	getXPlaneDataInfos();
	float pause = pauseXPlane();
	if (pause) {
		if (!isGamePaused)
		{
			if (!AlertWindowShown) {
				showWindowPaused();
			}
			XPLMCommandKeyStroke(xplm_key_pause);
			if (wChkToUnSelect != NULL)
				XPSetWidgetProperty(wChkToUnSelect, xpProperty_ButtonState, 0);
		}
	}
	// NOW CHECK IF THE MOBILE APP HAS ASKED SOMETHING
	if (serverSocketStarted == 1) {

		// Any Command to be executed?
		if (!callBackHandler->getCommand().empty()) {

			// Check by Who?  (Identification)
			std::string identification = "Not Identified!";
			std::size_t pos = callBackHandler->getCommand().find(",");
			if (pos > 0) {
				identification = callBackHandler->getCommand().substr(pos + 1);
			}

			// Check PAUSE/UNPAUSE REQUESTED BY MOBILE APP
			if (callBackHandler->getCommand().rfind("{PAUSE}", 0) == 0) {

				// (Un)Pausing X-Plane
				XPLMCommandKeyStroke(xplm_key_pause);
				callBackHandler->commandExecuted();

				// Check if the command Pause or Unpause the X-Plane
				int isGamePaused = XPLMGetDatai(XPLMFindDataRef("sim/time/paused"));
				if (isGamePaused) {
					if (!AlertWindowShown) {
						// Compose the Pause Message
						std::string msg  = "Remote connection \"" + identification + "\" said to Pause!";
						std::string msg2 = "[Origin: " + callBackHandler->getOrigin() + "]";
						sprintf(msgPause, "%s", msg.c_str());
						sprintf(msgPause2, "%s", msg2.c_str());
						showWindowPaused();
					}
				}
				else {
					// In case of an unpause activity, hidden the window
					hideWindowPaused();
					AlertWindowShown = 0;
				}
			} else 
			if (callBackHandler->getCommand().rfind("{CONFIG_PAUSE_NAVAID}", 0) == 0) {
				log("Received CHANGE config pause value " + identification + ", Command: " + callBackHandler->getCommand());
				CommandConfigPauseNavaid command = interpretCommandConfigPauseNavaId(callBackHandler->getCommand());

				int found = 0;
				XPWidgetID textId;
				XPWidgetID textDistanceMin;
				XPWidgetID wChk;

				if (command.type.compare("Airport") == 0) {
					textId                    = wTextNavaidAirportID;
					textDistanceMin           = wTextNavaidAirportDistanceMin;
					wChk                      = wChkNavaidAirport;
					userNavaidAirportDistance = atoi(command.distance.c_str());
					isNavaidAirportSelected   = 1;
					found                     = 1;
				} else
				if (command.type.compare("VOR") == 0) {
					textId                = wTextNavaidVORID;
					textDistanceMin       = wTextNavaidVORDistanceMin;
					wChk                  = wChkNavaidVOR;
					userNavaidVORDistance = atoi(command.distance.c_str());
					isNavaidVORSelected   = 1;
					found                 = 1;
				} else
				if (command.type.compare("NDB") == 0) {
					textId                = wTextNavaidNDBID;
					textDistanceMin       = wTextNavaidNDBDistanceMin;
					wChk                  = wChkNavaidNDB;
					userNavaidNDBDistance = atoi(command.distance.c_str());
					isNavaidNDBSelected   = 1;
					found                 = 1;
				} else
				if (command.type.compare("FIX") == 0) {
					textId                = wTextNavaidFixID;
					textDistanceMin       = wTextNavaidFixDistanceMin;
					wChk                  = wChkNavaidFix;
					userNavaidFixDistance = atoi(command.distance.c_str());
					isNavaidFixSelected   = 1;
					found                 = 1;
				} else
				if (command.type.compare("DME") == 0) {
					textId                = wTextNavaidDMEID;
					textDistanceMin       = wTextNavaidDMEDistanceMin;
					wChk                  = wChkNavaidDME;
					userNavaidDMEDistance = atoi(command.distance.c_str());
					isNavaidDMESelected   = 1;
					found = 1;
				}
				
				if (found == 1) {
					XPSetWidgetDescriptor(textId, command.navaid.c_str());
					XPSetWidgetDescriptor(textDistanceMin, command.distance.c_str());
					XPSetWidgetProperty(wChk, xpProperty_ButtonState, 1);
					saveFileValues();
				}
				else {
					log("Ops... Not found the type " + command.type + " for Config Pause Navaid App");
				}
				callBackHandler->commandExecuted();
			}
			else
			if (callBackHandler->getCommand().rfind("{CLOSE}", 0) == 0) {
				log("Received CLOSED request by " + identification);
				callBackHandler->commandExecuted();
			} else
			if (callBackHandler->getCommand().rfind("{CONFIG_PAUSE_ALTITUDE}", 0) == 0 ||
				callBackHandler->getCommand().rfind("{CONFIG_PAUSE_AIRSPEED}", 0) == 0) {
				log("Received CHANGE config pause value " + identification + ", Command: " + callBackHandler->getCommand());
				CommandConfigPauseLowerUpper command = interpretCommandConfigPauseLowerUpper(callBackHandler->getCommand());

				XPWidgetID textLower;
				XPWidgetID textUpper;
				XPWidgetID wChk;
				int found = 0;
				if (command.command.compare("{CONFIG_PAUSE_ALTITUDE}") == 0) {
					textLower          = wTextAltitudeMin;
					textUpper          = wTextAltitudeMax;
					isAltitudeSelected = 1;
					userAltitudeMin    = atoi(command.lower.c_str());
					userAltitudeMax    = atoi(command.upper.c_str());
					wChk               = wChkAltitude;
					found = 1;
				}
				else 
				if (command.command.compare("{CONFIG_PAUSE_AIRSPEED}") == 0) {
					textLower          = wTextAirspeedMin;
					textUpper          = wTextAirspeedMax;
					isAirspeedSelected = 1;
					userAirspeedMin    = atoi(command.lower.c_str());
					userAirspeedMax    = atoi(command.upper.c_str());
					wChk               = wChkAirspeed;
					found = 1;
				}

				if (found) {
					XPSetWidgetDescriptor(textLower, command.lower.c_str());
					XPSetWidgetDescriptor(textUpper, command.upper.c_str());
					XPSetWidgetProperty(wChk, xpProperty_ButtonState, 1);
					saveFileValues();
				}
				else {
					log("Ops... Not found the command " + command.command + " for Config Pause Altitude/Airspeed App");
				}
				callBackHandler->commandExecuted();
			} else 
			if ( callBackHandler->getCommand().rfind("{CONFIG_PAUSE_TIME}", 0) == 0 ) {
				log("Received CHANGE config pause value " + identification + ", Command: " + callBackHandler->getCommand());
				CommandConfigPauseTime command = interpretCommandConfigPauseTime(callBackHandler->getCommand());

				XPSetWidgetDescriptor(wTextTime, command.time.c_str());
				XPSetWidgetProperty(wChkTime, xpProperty_ButtonState, 1);
				isTimePauseSelected = 1;
				timePause           = command.time.c_str();
				saveFileValues();
				callBackHandler->commandExecuted();
			}
			else {
				log("Command not recognized: " + callBackHandler->getCommand());
			}
		}
	}
	return CHECKINTERVAL;
}

int pausedMessageSentToClients = 0;

float CallBackXPlaneSocketServer(float  inElapsedSinceLastCall,
	float  inElapsedTimeSinceLastFlightLoop,
	int    inCounter,
	void * inRefcon)
{
	if (serverSocketStarted == 1) {
		int isGamePaused = XPLMGetDatai(XPLMFindDataRef("sim/time/paused"));
		if (isGamePaused) {
			if (!pausedMessageSentToClients) {
				pausedMessageSentToClients = 1;
				sendPausedMessageSocketClients();
			}
		}
		else {
			if (pausedMessageSentToClients) {
				pausedMessageSentToClients = 0;
				sendUnpausedMessageSocketClients();
			}
			double currentLatitude   = XPLMGetDatad(XPLMFindDataRef("sim/flightmodel/position/latitude"));
			double currentLongitude  = XPLMGetDatad(XPLMFindDataRef("sim/flightmodel/position/longitude"));
			int    currentAltitude   = (int)XPLMGetDataf(XPLMFindDataRef("sim/cockpit/pressure/cabin_altitude_actual_m_msl"));
			int    currentAltitude2  = (int)XPLMGetDataf(XPLMFindDataRef("sim/cockpit2/gauges/indicators/altitude_ft_pilot"));
			int    airspeed          = (int)XPLMGetDataf(XPLMFindDataRef("sim/cockpit2/gauges/indicators/airspeed_kts_pilot"));
			int    trueAirspeed      = (int)XPLMGetDataf(XPLMFindDataRef("sim/cockpit2/gauges/indicators/true_airspeed_kts_pilot"));
			int    vsFpm             = (int)XPLMGetDataf(XPLMFindDataRef("sim/cockpit2/gauges/indicators/vvi_fpm_pilot"));
			int    groundspeed       = (int)XPLMGetDataf(XPLMFindDataRef("sim/flightmodel/position/groundspeed"));   // meters per second
			int    headingDegMag     = (int)XPLMGetDataf(XPLMFindDataRef("sim/cockpit2/gauges/indicators/compass_heading_deg_mag"));
			int    heading           = (int)XPLMGetDataf(XPLMFindDataRef("sim/cockpit2/gauges/indicators/heading_electric_deg_mag_pilot"));
			int    headingAHARS      = (int)XPLMGetDataf(XPLMFindDataRef("sim/cockpit2/gauges/indicators/heading_AHARS_deg_mag_pilot"));
			
			

			int    autoPilotHeading  = (int)XPLMGetDataf(XPLMFindDataRef("sim/cockpit/autopilot/heading"));
			int    autoPilotAltitude = (int)XPLMGetDataf(XPLMFindDataRef("sim/cockpit/autopilot/altitude"));
			int    autoPilotVsFpm    = (int)XPLMGetDataf(XPLMFindDataRef("sim/cockpit2/autopilot/vvi_dial_fpm"));
			int    autoPilotVsStatus = XPLMGetDatai(XPLMFindDataRef("sim/cockpit2/autopilot/vvi_status"));
			int    autoPilotAirspeed = (int)XPLMGetDataf(XPLMFindDataRef("sim/cockpit/autopilot/airspeed"));
			int    autoPilotOnOff    = XPLMGetDatai(XPLMFindDataRef("sim/cockpit2/annunciators/autopilot"));

			// Convert groundSpeed: meters per seconds --> to --> knots per second
			groundspeed = groundspeed * 1.94384;

			// GPS Destination
			XPLMNavRef gpsDestination      = XPLMGetGPSDestination();
			XPLMNavType gpsDestinationType = XPLMGetGPSDestinationType();
			char label[256];
			int  outFrequency;
			char outID[10];
			char outName[256];
			XPLMGetNavAidInfo(gpsDestination, &gpsDestinationType, NULL, NULL, NULL, &outFrequency, NULL, outID, outName, NULL);
			if (strcmp(outID, "----") != 0) {
				std::string descripDestTypeGPS = getDescriptionGPSDestinationType(gpsDestinationType);
				sprintf(label, "%s: %s  (%s)", descripDestTypeGPS.c_str(), outName, outID);
			}
			else {
				std::string descripDestTypeGPS = "FMS";
				sprintf(label, "%s", "FMS");
			}

			//REMOTE MOBILE APPLICATION FEEDING
			std::ostringstream oss;
			oss << "{";
			oss << "   \"airplane\":";
			oss << "   {";
			oss << "     \"lat\":" << currentLatitude;
			oss << "    ,\"lng\":" << currentLongitude;
			oss << "    ,\"airspeed\":" << airspeed;
			oss << "    ,\"trueAirspeed\":" << trueAirspeed;
			oss << "    ,\"currentAltitude\":" << currentAltitude;
			oss << "    ,\"currentAltitude2\":" << currentAltitude2;
			oss << "    ,\"vsFpm\":" << vsFpm;
			oss << "    ,\"groundspeed\":" << groundspeed;
			oss << "    ,\"currentFreqNav1\":" << currentFreqNav1;
			oss << "    ,\"currentDistDmeNav1\":" << currentDistDmeNav1;
			oss << "    ,\"currentTimeDmeNav1\":" << currentTimeDmeNav1;
			oss << "    ,\"currentFreqNav2\":" << currentTimeDmeNav2;
			oss << "    ,\"currentTimeDmeNav2\":" << currentTimeDmeNav2;
			oss << "    ,\"currentDistDmeNav2\":" << currentTimeDmeNav2;
			oss << "    ,\"currentGPSDistDme\":" << currentGPSDistDme;
			oss << "    ,\"currentGPSTimeDme\":" << currentGPSTimeDme;
			oss << "    ,\"destination\":\"" << label << "\"";
			oss << "    ,\"headingDegMag\":" << headingDegMag;
			oss << "    ,\"headingAHARS\":" << headingAHARS;
			oss << "    ,\"heading\":" << heading;
			oss << "    ,\"autopilot\":";
			oss << "       {";
			oss << "          \"on\":" << autoPilotOnOff;
			oss << "         ,\"heading\":" << autoPilotHeading;
			oss << "         ,\"altitude\":" << autoPilotAltitude;
			oss << "         ,\"vsFpm\":" << autoPilotVsFpm;
			oss << "         ,\"vsStatus\":" << autoPilotVsStatus;
			oss << "         ,\"airspeed\":" << autoPilotAirspeed;
			oss << "       }";
			oss << "    ,\"pauseforme\":";
			oss << "       {";
			oss << "         \"navaid\":";
			oss << "           {";
			oss << "              \"userAirportDistance\":" << userNavaidAirportDistance;
			oss << "             ,\"userVORDistance\":" << userNavaidVORDistance;
			oss << "             ,\"userNDBDistance\":" << userNavaidNDBDistance;
			oss << "             ,\"userFixDistance\":" << userNavaidFixDistance;
			oss << "             ,\"userDMEDistance\":" << userNavaidDMEDistance;
			oss << "             ,\"config\":";
			oss << "                {";
			oss << "                  \"id\":";
			oss << "                    {";
			oss << "                       \"airport\":\"" << navaidAirport.id << "\"";
			oss << "                      ,\"vor\":\"" << navaidVOR.id << "\"";
			oss << "                      ,\"ndb\":\"" << navaidNDB.id << "\"";
			oss << "                      ,\"fix\":\"" << navaidFix.id << "\"";
			oss << "                      ,\"dme\":\"" << navaidDME.id << "\"";
			oss << "                    }";
			oss << "                 ,\"selected\":";
			oss << "                    {";
			oss << "                       \"airport\":" << isNavaidAirportSelected;
			oss << "                      ,\"vor\":" << isNavaidVORSelected;
			oss << "                      ,\"ndb\":" << isNavaidNDBSelected;
			oss << "                      ,\"fix\":" << isNavaidFixSelected;
			oss << "                      ,\"dme\":" << isNavaidDMESelected;
			oss << "                    }";
			oss << "                 ,\"distance\":";
			oss << "                    {";
			oss << "                       \"airport\":" << navaidAirport.distance;
			oss << "                      ,\"vor\":"     << navaidVOR.distance;
			oss << "                      ,\"ndb\":"     << navaidNDB.distance;
			oss << "                      ,\"fix\":"     << navaidFix.distance;
			oss << "                      ,\"dme\":"     << navaidDME.distance;
			oss << "                    }";
			oss << "                }";
			oss << "           }";
			oss << "        ,\"timePause\":\"" << timePause << "\"";
			oss << "        ,\"timePauseSelected\":" << isTimePauseSelected;
			oss << "        ,\"altitude\":";
			oss << "           {";
			oss << "              \"min\":" << userAltitudeMin;
			oss << "             ,\"max\":" << userAltitudeMax;
			oss << "             ,\"selected\":" << isAltitudeSelected;
			oss << "           }";
			oss << "        ,\"speed\":";
			oss << "           {";
			oss << "              \"min\":" << userAirspeedMin;
			oss << "             ,\"max\":" << userAirspeedMax;
			oss << "             ,\"selected\":" << isAirspeedSelected;
			oss << "           }";
			oss << "       }";
			oss << "    ,\"nextDestination\":";
			oss << "       {";
			oss << "         \"gps\":";
			oss << "           {";
			oss << "              \"id\":\"" << navaidGpsDestination.id << "\"";
			oss << "             ,\"name\":\"" << navaidGpsDestination.name << "\"";
			oss << "             ,\"latitude\":" << navaidGpsDestination.latitude;
			oss << "             ,\"longitude\":" << navaidGpsDestination.longitude;
			oss << "             ,\"dmeDistance\":" << navaidGpsDestination.dmeDistance;
			oss << "             ,\"dmeTime\":" << navaidGpsDestination.dmeTime;
			oss << "             ,\"distance\":" << navaidGpsDestination.distance;
			oss << "             ,\"type\":\"" << navaidGpsDestination.typeDescription << "\"";
			oss << "             ,\"status\":" << navaidGpsDestination.statusOK;
			oss << "           }";
			oss << "         ,\"fms\":";
			oss << "           {";
			oss << "              \"id\":\"" << navaidFmsDestination.id << "\"";
			oss << "             ,\"name\":\"" << navaidFmsDestination.name << "\"";
			oss << "             ,\"latitude\":" << navaidFmsDestination.latitude;
			oss << "             ,\"longitude\":" << navaidFmsDestination.longitude;
			oss << "             ,\"fmsTime\":\"" << navaidFmsDestination.fmsTime << "\"";
			oss << "             ,\"distance\":" << navaidFmsDestination.distance;
			oss << "             ,\"type\":\"" << navaidFmsDestination.typeDescription << "\"";
			oss << "             ,\"status\":" << navaidFmsDestination.statusOK;
			oss << "           }";
			oss << "       }";
			oss << "    ,\"time\":\"" << hourTimeZ << ":" << minuTimeZ << ":" << secoTimeZ << "\"";
			oss << "   }";
			oss << "  ,\"isPaused\":" << std::to_string(isGamePaused);
			oss << "  ,\"isBatteryOn\":" << std::to_string(isBatteryOn);
			oss << "}";

			//log(oss.str());
			socketServer->broadcast(oss.str());
		}
	}

	return CHECKINTERVAL;
}



PLUGIN_API void	XPluginStop(void)
{
	stopSocketServer();
	XPLMUnregisterFlightLoopCallback(CallBackXPlane, NULL);
	XPLMUnregisterCommandHandler(SetupOnCommand, SetupOnCommandHandler, 0, 0);
	XPLMUnregisterCommandHandler(SetupOffCommand, SetupOffCommandHandler, 0, 0);
}

PLUGIN_API void XPluginDisable(void)
{
}

PLUGIN_API int XPluginEnable(void)
{
	return 1;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void * inParam)
{
}


// Read X-Plane 11 Flightplan file (to send to the clients)
void readXPlane11FMS() {

	//AQUI
	/*int totalFMSEntries = XPLMCountFMSEntries();
	log(convertToString(totalFMSEntries));

	int Index = XPLMGetDisplayedFMSEntry();
	XPLMNavType outType;
	char        outID[80];
	XPLMNavRef  outRef;
	int         outAltitude;
	float       outLat, outLon;
	XPLMGetFMSEntryInfo(Index, &outType, outID, &outRef, &outAltitude, &outLat, &outLon);
	log(outID);*/

	char bufFp[3070];
	XPGetWidgetDescriptor(wTextLoadFlightPlan, bufFp, sizeof(bufFp));
	xplane11LoadFlightPlan = bufFp;

	std::ifstream planFile("c:\\X-Plane 11\\Output\\FMS plans\\" + xplane11LoadFlightPlan + ".fmx");
	if (planFile.good()) {
		log("File Plan Found!");
		std::string waypoints = "";
		std::string deprwy = "";
		std::string sid = "";
		std::string star = "";
		std::string app = "";
		std::string line;
		while (std::getline(planFile, line)) {
			std::string waypoint  = line.substr(0, line.find(","));
			vector<string> result = splitStringBy(line, ",");

			if (strcmp(waypoint.c_str(), "DEPRWY") == 0) {
				if (result.size() > 1) {
					deprwy = "RWY" + result[1];
				}
			} else if (strcmp(waypoint.c_str(), "SID") == 0) {
				if (result.size() > 1) {
					sid = "" + result[1];
				}
			} else if (strcmp(waypoint.c_str(), "STAR") == 0) {
				if (result.size() > 1) {
					star = "" + result[1];
				}
			} else if (strcmp(waypoint.c_str(), "APP") == 0) {
				if (result.size() > 1) {
					app = "" + result[1];
				}
			} else if ( 
				(waypoint.find("FLIGHT_NUM") != std::string::npos) ||
				(waypoint.find("TRANS")      != std::string::npos) ) {
				break;
			} else {
				if (strcmp(waypoints.c_str(), "") == 0) {
					waypoints = waypoint;
				} else {
					waypoints = waypoints + " " + waypoint;
				}
			}
		}
		log(waypoints);
		XPSetWidgetDescriptor(wTextFlightPlan, "");
		XPSetWidgetDescriptor(wTextFlightPlan, waypoints.c_str());
		flightPlan = waypoints;
		std::string msg = "OK! " + deprwy + " " + sid + " " + star + " " + app;
		XPSetWidgetDescriptor(wLoadFlightPlanResult, msg.c_str());
	}
	else {
		log("File Plan " + xplane11LoadFlightPlan + ".fmx Not found!");
		XPSetWidgetDescriptor(wLoadFlightPlanResult, "Not Found!");
	}
}

void checkPreferenceFile() {
	std::ifstream fileIniReader(fileName.c_str());
	if (fileIniReader.good()) {
		std::string line;
		while (std::getline(fileIniReader, line)) {
			std::string param = "";
			std::string value = "";

			param = line.substr(0, line.find("="));
			value = line.substr(line.find("=") + 1);

			// Nav1
			if (strcmp(param.c_str(), "nav1_dme_min_dist") == 0)  {
				userNav1DistMinDme = atoi(value.c_str());
			}
			else
			if (strcmp(param.c_str(), "nav1_dme_max_dist") == 0)  {
			userNav1DistMaxDme = atoi(value.c_str());
			}
			else
			if (strcmp(param.c_str(), "nav1_dme_min_time") == 0)  {
				userNav1TimeDmeMin = atoi(value.c_str());
			}
			else
			if (strcmp(param.c_str(), "nav1_dme_max_time") == 0)  {
				userNav1TimeDmeMax = atoi(value.c_str());
			}
			else
			if (strcmp(param.c_str(), "nav1_hsi_alignment") == 0)  {
				userNav1IsHSIAlignEnable = atoi(value.c_str());
			}
			else
			// Nav2
			if (strcmp(param.c_str(), "nav2_dme_min_dist") == 0)  {
				userNav2DistMinDme = atoi(value.c_str());
			}
			else
			if (strcmp(param.c_str(), "nav2_dme_max_dist") == 0)  {
				userNav2DistMaxDme = atoi(value.c_str());
			}
			else
			if (strcmp(param.c_str(), "nav2_dme_min_time") == 0)  {
				userNav2TimeDmeMin = atoi(value.c_str());
			}
			else
			if (strcmp(param.c_str(), "nav2_dme_max_time") == 0)  {
				userNav2TimeDmeMax = atoi(value.c_str());
			}
			else
			if (strcmp(param.c_str(), "nav2_hsi_alignment") == 0)  {
				userNav2IsHSIAlignEnable = atoi(value.c_str());
			}
			// Altitude
			else
			if (strcmp(param.c_str(), "altitude_min") == 0)  {
				userAltitudeMin = atoi(value.c_str());
			}
			else
			if (strcmp(param.c_str(), "altitude_max") == 0)  {
				userAltitudeMax = atoi(value.c_str());
			}
			// Airspeed
			else
			if (strcmp(param.c_str(), "airspeed_min") == 0)  {
				userAirspeedMin = atoi(value.c_str());
			}
			else
			if (strcmp(param.c_str(), "airspeed_max") == 0)  {
				userAirspeedMax = atoi(value.c_str());
			} else
			// GPS
			if (strcmp(param.c_str(), "gps_dme_min_dist") == 0)  {
				userGPSDmeDistanceMin = atoi(value.c_str());
			}
			else
			if (strcmp(param.c_str(), "gps_dme_max_dist") == 0)  {
			userGPSDmeDistanceMax = atoi(value.c_str());
			}
			else
			if (strcmp(param.c_str(), "gps_dme_min_time") == 0)  {
				userGPSDmeTimeMin = atoi(value.c_str());
			}
			else
			if (strcmp(param.c_str(), "gps_dme_max_time") == 0)  {
				userGPSDmeTimeMax = atoi(value.c_str());
			}
			else
			if (strcmp(param.c_str(), "latitude") == 0)  {
				objUserLatitude.setValor(value.c_str());
			}
			else
			if (strcmp(param.c_str(), "longitude") == 0)  {
				objUserLongitude.setValor(value.c_str());
			}
			else
			if (strcmp(param.c_str(), "navaidAirportId") == 0)  {
				userNavaidAirportID = value.c_str();
			}
			else
			if (strcmp(param.c_str(), "navaidAirportDistance") == 0)  {
				userNavaidAirportDistance = atoi(value.c_str());
			}
			else
			if (strcmp(param.c_str(), "navaidVORId") == 0)  {
				userNavaidVORID = value.c_str();
			}
			else
			if (strcmp(param.c_str(), "navaidVORDistance") == 0)  {
				userNavaidVORDistance = atoi(value.c_str());
			}
			else
			if (strcmp(param.c_str(), "navaidNDBId") == 0)  {
				userNavaidNDBID = value.c_str();
			}
			else
			if (strcmp(param.c_str(), "navaidNDBDistance") == 0)  {
				userNavaidNDBDistance = atoi(value.c_str());
			}
			else
			if (strcmp(param.c_str(), "navaidFixId") == 0)  {
				userNavaidFixID = value.c_str();
			}
			else
			if (strcmp(param.c_str(), "navaidFixDistance") == 0)  {
				userNavaidFixDistance = atoi(value.c_str());
			}
			else
			if (strcmp(param.c_str(), "navaidDMEId") == 0)  {
				userNavaidDMEID = value.c_str();
			}
			else
			if (strcmp(param.c_str(), "navaidDMEDistance") == 0)  {
				userNavaidDMEDistance = atoi(value.c_str());
			}
			else
			if (strcmp(param.c_str(), "dataRef1") == 0) {
				dataRef1 = value.c_str();
			}
			else
			if (strcmp(param.c_str(), "dataRefValue1") == 0) {
				dataRefValue1 = value.c_str();
			}
			if (strcmp(param.c_str(), "dataRef2") == 0) {
				dataRef2 = value.c_str();
			}
			else
			if (strcmp(param.c_str(), "dataRefValue2") == 0) {
				dataRefValue2 = value.c_str();
			}
			if (strcmp(param.c_str(), "dataRef3") == 0) {
				dataRef3 = value.c_str();
			}
			else
			if (strcmp(param.c_str(), "dataRefValue3") == 0) {
				dataRefValue3 = value.c_str();
			}
			else
			if (strcmp(param.c_str(), "timePause") == 0) {
				timePause = value.c_str();
			}
			else
			if (strcmp(param.c_str(), "flightPlan") == 0) {
				flightPlan = value.c_str();
				flightPlanBringBack = flightPlan;
			}
			else
			if (strcmp(param.c_str(), "xplane11LoadFlightPlan") == 0) {
				xplane11LoadFlightPlan = value.c_str();
			}
			else
			if (strcmp(param.c_str(), "ats_txt") == 0) {
				pathATSFile = value.c_str();
			}
		}

		// Check if the CFG file already has the ATS Path File, otherwise lets create it here to be saved later
		if (pathATSFile.empty()) {
			getPathATSFile();
		}

		fileIniReader.close();
	} else {
		fileIniReader.close();
		std::ofstream fileIniWriter;
		fileIniWriter.open("PauseForMe.ini");
		fileIniWriter << "nav1_dme_min_dist=-1\n";
		fileIniWriter << "nav1_dme_max_dist=9999\n";
		fileIniWriter << "nav1_dme_min_time=-1\n";
		fileIniWriter << "nav1_dme_max_time=9999\n";
		fileIniWriter << "nav1_hsi_alignment=0\n";

		fileIniWriter << "nav2_dme_min_dist=-1\n";
		fileIniWriter << "nav2_dme_max_dist=9999\n";
		fileIniWriter << "nav2_dme_min_time=-1\n";
		fileIniWriter << "nav2_dme_max_time=9999\n";
		fileIniWriter << "nav2_hsi_alignment=0\n";

		fileIniWriter << "altitude_min=-1\n";
		fileIniWriter << "altitude_max=99999\n";
		fileIniWriter << "airspeed_min=-1\n";
		fileIniWriter << "airspeed_max=99999\n";

		fileIniWriter << "gps_dme_min_dist=-1\n";
		fileIniWriter << "gps_dme_max_dist=9999\n";
		fileIniWriter << "gps_dme_min_time=-1\n";
		fileIniWriter << "gps_dme_max_time=9999\n";

		fileIniWriter << "latitude=0.000\n";
		fileIniWriter << "longitude=0.000\n";

		fileIniWriter << "navaidAirportId=\n";
		fileIniWriter << "navaidAirportDistance=0\n";
		fileIniWriter << "navaidVORId=\n";
		fileIniWriter << "navaidVORDistance=0\n";
		fileIniWriter << "navaidNDBId=\n";
		fileIniWriter << "navaidNDBDistance=0\n";
		fileIniWriter << "navaidFixId=\n";
		fileIniWriter << "navaidFixDistance=0\n";
		fileIniWriter << "navaidDMEId=\n";
		fileIniWriter << "navaidDMEDistance=0\n";

		fileIniWriter << "dataRef1=sim/cockpit/electrical/battery_on\n";
		fileIniWriter << "dataRefValue1=0\n";

		fileIniWriter << "flightPlan=LEBL SLL GIR PPG LFPG\n";
		fileIniWriter << "xplane11LoadFlightPlan=\n";
		fileIniWriter << "timePause=22:10\n";

		getPathATSFile();
		fileIniWriter << "ats_txt=" << pathATSFile << "\n";

		flightPlanBringBack = "LEBL SLL GIR PPG LFPG";

		fileIniWriter.close();
	}
}

int SetupOnCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void * inRefcon)
{
	if (MenuItem1 == 0)
	{
		CreateWidgetWindow();
		MenuItem1 = 1;
		XPLMRegisterFlightLoopCallback(CallBackXPlane, 1.0, NULL);
		
	}
	return 0;
}

int SetupOffCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void * inRefcon)
{
	if (MenuItem1 == 1)
	{
		XPHideWidget(wMainWindow);
		MenuItem1 = 0;
	}
	return 0;
}

int SetupStartStopTrasmitterHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void * inRefcon)
{
	switchSocketServer();
	return 0;
}



void resetDataRefsValues() {
	if (isDataRef1Selected == 0) {
		XPSetWidgetDescriptor(wCaptionDataRef1, "----");
		XPSetWidgetProperty(wCaptionDataRef1, xpProperty_CaptionLit, 0);
	}
	if (isDataRef2Selected == 0) {
		XPSetWidgetDescriptor(wCaptionDataRef2, "----");
		XPSetWidgetProperty(wCaptionDataRef2, xpProperty_CaptionLit, 0);
	}
	if (isDataRef3Selected == 0) {
		XPSetWidgetDescriptor(wCaptionDataRef3, "----");
		XPSetWidgetProperty(wCaptionDataRef3, xpProperty_CaptionLit, 0);
	}
}

void log(std::string s) {
	XPLMDebugString(" ---> LOG:");
	XPLMDebugString(s.c_str());
	XPLMDebugString("\n");
}

void replaceAll(std::string &str, std::string replaceThis, std::string byThis, int compensate) {
	size_t index = 0;
	while (true) {
		/* Locate the substring to replace. */
		index = str.find(replaceThis, index);
		if (index == std::string::npos) break;
		/* Make the replacement. */
		str.replace(index, byThis.length() + compensate, byThis);
		/* Advance index forward so the next iteration doesn't pick it up as well. */
		index += byThis.length() + compensate;
	}
}

void encodeForJson(std::string &str) {
	replaceAll(str, "\"", "|", 0);
	replaceAll(str, "|", "\\\"", -1);
}

void switchSocketServer() {
	if (serverSocketStarted == 1) {
		stopSocketServer();
	}
	else {
		startSocketServer();
	}
}

void sendStoppedMessageSocketClients()
{
	sendMessageSocketClients("STOPPED");
}
void sendStartedMessageSocketClients() {
	sendMessageSocketClients("STARTED");
}
void sendPausedMessageSocketClients() {
	std::string pauseMsg = "PAUSED";
	if ( (msgPause != NULL) && (msgPause[0] != '\0') ) {
		pauseMsg += ",";
		std::string cleanedMsg = std::string(msgPause);
		encodeForJson(cleanedMsg);
		pauseMsg += cleanedMsg;
		if ( (msgPause2 != NULL) && (msgPause2[0] != '\0') ) {
			pauseMsg += ",";
			std::string cleanedMsg = std::string(msgPause2);
			encodeForJson(cleanedMsg);
			pauseMsg += cleanedMsg;
		}
	}
	sendMessageSocketClients(pauseMsg);
}
void sendUnpausedMessageSocketClients() {
	sendMessageSocketClients("PLAY");
}
void sendMessageSocketClients(std::string msg) {
	std::ostringstream oss;
	oss << "{";
	oss << "   \"message\":\"" << msg.c_str() << "\"";
	oss << "}";
	socketServer->broadcast(oss.str());
}

void stopSocketServer() {
	if (serverSocketStarted == 1) {
		XPLMUnregisterFlightLoopCallback(CallBackXPlaneSocketServer, NULL);
		// Send Signal to Remote Clients, The WebServer Socket has stopped!
		sendStoppedMessageSocketClients();
		socketServer->stop();
		threadTaskSocketServer.join();
		serverSocketStarted = 0;
		XPSetWidgetDescriptor(wBtnStopStartWebSocket, "START Transmitter");
	}
}

void startSocketServer() {
	if (serverSocketStarted == 0){
		socketServer = new SocketServer(9002);
		callBackHandler = new CallBackHandler();
		socketServer->setCallBack(callBackHandler);
		serverSocketStarted = 1;
		threadTaskSocketServer = std::thread(taskSocketServer, socketServer);
		XPLMRegisterFlightLoopCallback(CallBackXPlaneSocketServer, 1.0, NULL);
		XPSetWidgetDescriptor(wBtnStopStartWebSocket, "STOP Transmitter");
		sendStartedMessageSocketClients();
	}
}

/**
 Building Flight Plan JSON
*/
std::string writeDownFlightPlan() {
	std::ostringstream oss;
	oss << "{ \"flightPlan\": {";
	oss << "  \"waypoints\":[";

	// Start looking the Navaid most close to coordinates that my Plane is localized
	float currentLatitude = XPLMGetDataf(XPLMFindDataRef("sim/flightmodel/position/latitude"));
	float currentLongitude = XPLMGetDataf(XPLMFindDataRef("sim/flightmodel/position/longitude"));

	int index = 0;
	std::list<NavaidManager::airwayNavaid_t>::iterator itAirwaysTranslated = listTranslatedFlightPlan.begin();
	while (itAirwaysTranslated != listTranslatedFlightPlan.end()) {
		struct NavaidManager::airwayNavaid_t structNavaid = *itAirwaysTranslated;
		std::string navId = structNavaid.id;


		XPLMNavRef navRef;
		if (navId.length() == 3) {
			navRef = XPLMFindNavAid(NULL, navId.c_str(), &currentLatitude, &currentLongitude, NULL, xplm_Nav_NDB | xplm_Nav_VOR );
		} else
		if (navId.length() == 4) {
			navRef = XPLMFindNavAid(NULL, navId.c_str(), &currentLatitude, &currentLongitude, NULL, xplm_Nav_Airport);
		} else
		if (navId.length() == 5) {
			navRef = XPLMFindNavAid(NULL, navId.c_str(), &currentLatitude, &currentLongitude, NULL, xplm_Nav_Fix);
		}
		else {
			navRef = XPLMFindNavAid(NULL, navId.c_str(), &currentLatitude, &currentLongitude, NULL, xplm_Nav_Unknown | xplm_Nav_Airport | xplm_Nav_NDB | xplm_Nav_VOR | xplm_Nav_ILS
				| xplm_Nav_Localizer | xplm_Nav_GlideSlope | xplm_Nav_OuterMarker | xplm_Nav_MiddleMarker | xplm_Nav_InnerMarker | xplm_Nav_Fix | xplm_Nav_DME | xplm_Nav_LatLon );
		}

		int   outFrequency;
		char  outID[10];
		char  outName[256];
		float outLatitude;
		float outLongitude;
		float outHeading;
		XPLMNavType outType;

		XPLMGetNavAidInfo(navRef, &outType, &outLatitude, &outLongitude, NULL, &outFrequency, &outHeading, outID, outName, NULL);
		if (strcmp(outID, "----") != 0) {
			index++;
			if (index > 1) {
				oss << "       ,";
			}
			oss << "      {";
			oss << "        \"index\":" << index << ",";
			oss << "        \"id\":\"" << outID << "\",";
			oss << "        \"name\":\"" << outName << "\",";
			//oss << "        \"latitude\":\"" << structNavaid.latitude << "\",";
			//oss << "        \"longitude\":\"" << structNavaid.longitude << "\",";
			oss << "        \"latitude\":\"" << outLatitude << "\",";
			oss << "        \"longitude\":\"" << outLongitude << "\",";
			oss << "        \"type\":\"" << getDescriptionGPSDestinationType(outType) << "\"";
			oss << "      }";

			// Looking the next Navaid according with the last navaid localization (proximity)
			currentLatitude = outLatitude;
			currentLongitude = outLongitude;
		}
		else {
			log(navId + " Not Found at Flight Plan Search");
		}

		itAirwaysTranslated++;
		index++;
	}
	oss << "    ]";
	oss << "}}";
	return oss.str();
}

void translateFlightPlan() {
	listTranslatedFlightPlan.clear();

	char buffer[3072];
	XPGetWidgetDescriptor(wTextFlightPlan, buffer, sizeof(buffer));
	std::string text = buffer;
	std::istringstream iss(text);

	struct NavaidManager::airwayNavaid_t structNavaid;
	int index = 0;
	std::string navIdBefore;
	std::string navIdAfter;
	std::vector<std::string> results((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());
	for (std::vector<string>::iterator it = results.begin(); it != results.end(); it++) {
		std::string navId = *it;
		boost::to_upper(navId);

		// It must have the Start Navaid and the End Navaid as references to look for a specific Airway, otherwise it is impossible to have an unique logic to identify the searched Airway 
		// For example:  
		//    NTM UN857 VELIN (VALID) --> That's ok, as with this referentes a unique Airway should be found     
		//    LEBL SLL NTM UN857 VELIN (VALID) --> That's ok, as with this referentes a unique Airway should be found
		//    UN857 VELIN (INVALID) --> There will be found more than one UN857
		//    NTM UN857 (INVALID) --> There will be found more than one UN857
		if (index > 0 && index < (results.size()-1) ) { // Only from the Second to the one before the last element

			// Get the Previous and the Posterior Navaid informed together with the Airway (airway is always in the middle of the Start-End of its navaids, must be!)
			navIdBefore = results.at(index - 1);
			navIdAfter  = results.at(index + 1);

			// Localize (check if this is a Airway) the Airway with the parameters
			// pathATSFile (supposedly = "c:/X-Plane 11/Custom Data/GNS430/navdata/ATS.txt")
			NavaidManager::airway_t airway = navManager.readingAirway(pathATSFile, navId, navIdBefore, navIdAfter);
			if (!airway.id.empty() && !airway.navaids.empty() && airway.navaids.size() > 0) {
				// Airway identified (found)
				// Remove the last one inserted , because it will be present on the first occurence of the Airway
				listTranslatedFlightPlan.pop_back();

				// Iterate over all the its returned Navaids 
				std::list<NavaidManager::airwayNavaid_t>::iterator itAirways = airway.navaids.begin();
				int indexAirwayNavaids       = 0;
				std::string lastNavaidAirway = "";
				while (itAirways != airway.navaids.end()) {
					NavaidManager::airwayNavaid_t navaid = *itAirways;
					indexAirwayNavaids++;

					// Debug purposes
					//char bf[64];
					//sprintf(bf, "     (%d) %s (%f/%f)", indexAirwayNavaids, navaid.id.c_str(), navaid.latitude, navaid.longitude);
					//log(bf);

					itAirways++;
					// Add all the new Navaids in between (Start-End) from the Airway
					listTranslatedFlightPlan.push_back(navaid);
					// save the last navaid.id of the Airways list
					lastNavaidAirway = navaid.id;
				}
				if (indexAirwayNavaids > 0) {
					// Remove the last one inserted from the Airway section, because it will be present on the next occurence of this FlightPlan as the End Marker of the Airway we just read
					// Check (just in case) if the index+1 does not overflow the size of the results
					if ((index + 1) < results.size()) {
						std::string nextNavaidOnFlightPlan = results.at(index + 1);
						// Check if the next Navaid on the FlightPlan is equal of the last one of the Airway list, if this is TRUE, we remove one in order to not duplicate them
						if (strcmp(nextNavaidOnFlightPlan.c_str(), lastNavaidAirway.c_str()) == 0) {
							listTranslatedFlightPlan.pop_back();
						}
					}
				}
			}
			else {
				//log(navId + " Is Not an Airway (ignoring..)");
				structNavaid.id = navId;
				structNavaid.longitude = 0;
				structNavaid.latitude = 0;
				listTranslatedFlightPlan.push_back(structNavaid);
			}
		}
		else {
			structNavaid.id = navId;
			structNavaid.longitude = 0;
			structNavaid.latitude = 0;
			listTranslatedFlightPlan.push_back(structNavaid);
		}
		index++;
	}

	index = 0;
	std::ostringstream newFlightPlan;
	std::list<NavaidManager::airwayNavaid_t>::iterator itAirwaysTranslated = listTranslatedFlightPlan.begin();
	while (itAirwaysTranslated != listTranslatedFlightPlan.end()) {
		struct NavaidManager::airwayNavaid_t navaid = *itAirwaysTranslated;
		if (index > 0) {
			// Remove SID and STAR, if exists
			if ((strcmp(navaid.id.c_str(), "SID") != 0) && (strcmp(navaid.id.c_str(), "STAR") != 0)) {
				newFlightPlan << " " << navaid.id;
			}
		}
		else {
			newFlightPlan << navaid.id;
		}
		itAirwaysTranslated++;
		index++;
	}
	XPSetWidgetDescriptor(wTextFlightPlan, newFlightPlan.str().c_str());
}

void bringBackMyFlightPlan() {
	XPSetWidgetDescriptor(wTextFlightPlan, flightPlanBringBack.c_str());
}

void pasteFlightPlan() {
	std::string fromClipboard;
	XSBGetTextFromClipboard(fromClipboard);
	XPSetWidgetDescriptor(wTextFlightPlan, fromClipboard.c_str());
}

void copyFlightPlan() {
	char buffer[3072];
	XPGetWidgetDescriptor(wTextFlightPlan, buffer, sizeof(buffer));
	std::string toClipboard = buffer;
	XSBSetTextToClipboard(toClipboard);
}

void cleanFlightPlan() {
	XPSetWidgetDescriptor(wTextFlightPlan, "");
	flightPlan = "";
}

void getPathATSFile() {
	// Writing the supposed path of the ATS.txt  file - For Airways Translation
	char buffer[256];
	XPLMGetSystemPath(buffer);
	pathATSFile = buffer;
	pathATSFile += supposedPathATSFile;
	replaceAll(pathATSFile, "\\", "/", 0);
}

bool XSBGetTextFromClipboard(std::string& outText) {
	#if IBM
		HGLOBAL   	hglb;
		LPTSTR    	lptstr;
		bool		retVal = false;
		static XPLMDataRef hwndDataRef = XPLMFindDataRef("sim/operation/windows/system_window");
		HWND hwndMain = (HWND)XPLMGetDatai(hwndDataRef);

		if (!IsClipboardFormatAvailable(CF_TEXT))
			return false;

		if (!OpenClipboard(hwndMain))
			return false;

		hglb = GetClipboardData(CF_TEXT);
		if (hglb != NULL)
		{
			lptstr = (LPSTR)GlobalLock(hglb);
			if (lptstr != NULL)
			{
				outText = lptstr;
				GlobalUnlock(hglb);
				retVal = true;
			}
		}
		CloseClipboard();
		return retVal;
	#endif

	#if APL
		ScrapRef	scrap;
		if (::GetCurrentScrap(&scrap) != noErr)
			return false;

		SInt32		byteCount = 0;
		OSStatus	status = ::GetScrapFlavorSize(scrap, kScrapFlavorTypeText, &byteCount);
		if (status != noErr)
			return false;

		outText.resize(byteCount);

		return (::GetScrapFlavorData(scrap, kScrapFlavorTypeText, &byteCount, &*outText.begin()) == noErr);
	#endif
}

bool XSBSetTextToClipboard(const std::string& inText) {
	#if IBM
		LPTSTR  lptstrCopy;
		HGLOBAL hglbCopy;
		static XPLMDataRef hwndDataRef = XPLMFindDataRef("sim/operation/windows/system_window");
		HWND hwndMain = (HWND)XPLMGetDatai(hwndDataRef);

		if (!OpenClipboard(hwndMain))
			return false;
		EmptyClipboard();

		hglbCopy = GlobalAlloc(GMEM_MOVEABLE, sizeof(TCHAR) * (inText.length() + 1));
		if (hglbCopy == NULL)
		{
			CloseClipboard();
			return false;
		}

		lptstrCopy = (LPSTR)GlobalLock(hglbCopy);
		strcpy(lptstrCopy, inText.c_str());
		GlobalUnlock(hglbCopy);

		SetClipboardData(CF_TEXT, hglbCopy);
		CloseClipboard();
		return true;
	#endif

	#if APL
		ScrapRef	scrap;
		if (::ClearCurrentScrap() != noErr) return false;
		if (::GetCurrentScrap(&scrap) != noErr) return false;
		return ::PutScrapFlavor(scrap, kScrapFlavorTypeText, kScrapFlavorMaskNone, inText.size(), &*inText.begin()) == noErr;
	#endif
}

std::vector<string> splitStringBy(std::string line, std::string delimiter) {
	std::vector<std::string> words;
	size_t pos = 0;
	std::string token;
	while ((pos = line.find(delimiter)) != std::string::npos) {
		token = line.substr(0, pos);
		words.push_back(token);
		line.erase(0, pos + delimiter.length());
	}
	words.push_back(line);
	return words;
}

CommandConfigPauseNavaid interpretCommandConfigPauseNavaId(std::string line) {
	std::vector<string> words = splitStringBy(line, "|");
	std::vector<string>::iterator it;
	std::string navaid, type, distance;
	CommandConfigPauseNavaid command;
	int i = 0;
	for (it = words.begin(); it != words.end(); it++, i++) {
		std::string word = *it;
		if (i == 1) {
			command.navaid = word;
		}
		else
			if (i == 2) {
				command.type = word;
			}
			else
				if (i == 3) {
					command.distance = word;
				}
	}
	return command;
}

CommandConfigPauseLowerUpper interpretCommandConfigPauseLowerUpper(std::string line) {
	std::vector<string> words = splitStringBy(line, "|");
	std::vector<string>::iterator it;
	std::string navaid, type, distance;
	CommandConfigPauseLowerUpper command;
	int i = 0;
	for (it = words.begin(); it != words.end(); it++, i++) {
		std::string word = *it;
		if (i == 0) {
			command.command = word;
		}
		else
		if (i == 1) {
			command.parameter = word;
		}
		else
		if (i == 2) {
			command.lower = word;
		}
		else
		if (i == 3) {
			command.upper = word;
		}
	}
	return command;
}

CommandConfigPauseTime interpretCommandConfigPauseTime(std::string line) {
	std::vector<string> words = splitStringBy(line, "|");
	std::vector<string>::iterator it;
	std::string navaid, type, distance;
	CommandConfigPauseTime command;
	int i = 0;
	for (it = words.begin(); it != words.end(); it++, i++) {
		std::string word = *it;
		if (i == 0) {
			command.command = word;
		}
		else
			if (i == 1) {
				command.time = word;
			}
	}
	return command;
}

