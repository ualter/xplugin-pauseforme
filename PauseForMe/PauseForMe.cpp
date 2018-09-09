/**
* PauseForMe
*
* This is a plugin just for help those who likes to put your plane on the correct flying heading and waiting (respecting the time, or not!)
* until certain event happens to PAUSE the game. Some options of events being tracking are:
* - Max/Min Distance to DME (Nav1/Nav2/GPS);
* - Max/Min Time to DME (Nav1/Nav2/GPS);
* - Max/Min Altitude;
* - Max/Min Airspeed; or�
* - Alignment with a radial signal (Nav1/Nav2)
*
* What comes first it will trigger the PAUSE. Then, you have the chance to get back the command from the autopilot from that
* point on.
*
* You could use it while doing another things on computer, like programming your favourite language, or check out what your 8-years-old-boy
* wants with you, or simply to have a shower, a snack! :-)  I hope it helps, I did it just for fun and because it was features that I was
* looking for on a plugin, and as I did not find� I decide to write this one.
*
* Any doubts or problems you can reach me at my e-mail(below), as soon as I have free time I promised to correct the bugs you found! ;-)
* Please in case you found any bugs, send me also information of the conditions that it happened. And of course, if you have any ideias to
* add more cool features, just let me know, right?
*
* @author: Ualter Otoni Azambuja Junior (ualter.junior@gmail.com)
* @where: Sao Paulo, Brazil
* @when: 09-Aug-2014
* @why: I liked! Both! Programming and Flying simulator, they are both great!
* @skills: Java Specialist, Integration and Software development specialist
* @somethingelse: like to learning new things, new languages and face daring things
*
* Aug-09-2014 Beta-v1.0
* Aug-14-2014 v1.5
* Aug-17-2014 v1.5 Win64
* Aug-28-2014 v2.0 Win64
* Aug-31-2014 v2.1 Win64
* Set-04-2014 v2.5 Win64
*
*/

#define APL 0
#define IBM 1

#pragma warning(disable: 4996)
#pragma warning(disable: 4244)

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
#include "Coordenada.h"
#include "NavaidManager.h"
#include "Navaid.h"

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

#define CHECKINTERVAL 1.5 //seconds between checks

static XPWidgetID wCaptionNav1, wCaptionNav2;
static XPWidgetID wMainWindow, wSubWindow, wBtnSave, wBtnReload, wBtnCancel;
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

int isDataRef1Selected, isDataRef2Selected, isDataRef3Selected;

static XPWidgetID wChkToUnSelect;

std::string dataRef1;
std::string dataRefValue1;
std::string dataRef2;
std::string dataRefValue2;
std::string dataRef3;
std::string dataRefValue3;

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
char debug_message[128];
char msgPause[128];
std::string fileName = "PauseForMe.ini";

Coordenada objCurrentLongitude(2), objCurrentLatitude(2), objUserLongitude(2), objUserLatitude(2);
int acceptableDifference = 2;

//float currentLongitude, currentLatitude;
//std::string  currentLongitudeStr, currentLatitudeStr;

float CallBackXPlane(float  inElapsedSinceLastCall,
	float  inElapsedTimeSinceLastFlightLoop,
	int    inCounter,
	void * inRefcon);
static void PauseForMeMenuHandler(void *, void *);

void checkPreferenceFile();

XPLMCommandRef SetupOnCommand = NULL;
XPLMCommandRef SetupOffCommand = NULL;

int SetupOnCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void * inRefcon);
int SetupOffCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void * inRefcon);

PLUGIN_API int XPluginStart(
	char *		outName,
	char *		outSig,
	char *		outDesc)
{
	checkPreferenceFile();

	strcpy(outName, "PauseForMe");
	strcpy(outSig, "br.sp.ualter.junior.PauseForMe");
	strcpy(outDesc, "A plug-in to pause at a specific time you want, while you are going to have a shower :-)");

	// Create our commands; these will increment and decrement our custom dataref.
	SetupOnCommand = XPLMCreateCommand("Ualter/PauseForMe/SetupOn", "Open Setup window");
	SetupOffCommand = XPLMCreateCommand("Ualter/PauseForMe/SetupOff", "Close Setup window");
	// Register our custom commands
	XPLMRegisterCommandHandler(SetupOnCommand, SetupOnCommandHandler, 1, (void *)0);
	XPLMRegisterCommandHandler(SetupOffCommand, SetupOffCommandHandler, 1, (void *)0);

	// Build menu
	int item;
	item = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "Pause For Me", NULL, 1);
	id = XPLMCreateMenu("Menu Pause For Me", XPLMFindPluginsMenu(), item, PauseForMeMenuHandler, NULL);
	XPLMAppendMenuItem(id, "Setup", (void *)"Setup", 1);

	// Used by widget to make sure only one widgets instance created
	MenuItem1 = 0;

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

// Callbacks we will register when we create our window
void				draw_hello_world(XPLMWindowID in_window_id, void * in_refcon);
int					dummy_mouse_handler(XPLMWindowID in_window_id, int x, int y, int is_down, void * in_refcon) { return 0; }
XPLMCursorStatus	dummy_cursor_status_handler(XPLMWindowID in_window_id, int x, int y, void * in_refcon) { return xplm_CursorDefault; }
int					dummy_wheel_handler(XPLMWindowID in_window_id, int x, int y, int wheel, int clicks, void * in_refcon) { return 0; }
void				dummy_key_handler(XPLMWindowID in_window_id, char key, XPLMKeyFlags flags, char virtual_key, void * in_refcon, int losing_focus) { }

void CreateWidgetWindow()
{
	int x = 150;
	int y = 900;
	int w = 840;
	int h = 555;

	int x2 = x + w;
	int y2 = y - h;

	wMainWindow = XPCreateWidget(x, y, x2, y2, 1, "Pause For Me / v2.8", 1, NULL, xpWidgetClass_MainWindow);
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
	int padX = 15;
	leftX = x + leftMargin + 43 + padX;
	rightX = leftX + widthCaption;
	bottomY = topY - heightFields;
	XPCreateWidget(leftX, topY, rightX, bottomY, 1, "Altitude", 0, wMainWindow, xpWidgetClass_Caption);
	// Altitude Checkbox
	tmpX = leftMargin - 88;
	wChkAltitude = XPCreateWidget(leftX + tmpX, topY - 5, leftX + tmpX + widthField, bottomY - 5, 1, "", 0, wMainWindow, xpWidgetClass_Button);
	XPSetWidgetProperty(wChkAltitude, xpProperty_ButtonType, xpRadioButton);
	XPSetWidgetProperty(wChkAltitude, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
	XPSetWidgetProperty(wChkAltitude, xpProperty_ButtonState, isAltitudeSelected);
	// Altitude Caption
	leftX = x + leftMargin + padX;
	topY -= 30;
	widthField = 50;
	bottomY = topY - heightFields;
	wCaptionAltitude = XPCreateWidget(leftX, topY, leftX + widthField, bottomY, 1, "0", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wCaptionAltitude, xpProperty_CaptionLit, 1);
	// Altitude Text Min
	leftX += 90;
	topY += 10;
	bottomY = topY - heightFields;
	widthField = 50;
	wTextAltitudeMin = XPCreateWidget(leftX, topY, leftX + widthField, bottomY, 1, convertToString(userAltitudeMin).c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextAltitudeMin, xpProperty_TextFieldType, xpTextEntryField);
	XPCreateWidget(x + leftMargin + 60 + padX, topY, x + leftMargin + 65 + padX, bottomY, 1, "Min:", 0, wMainWindow, xpWidgetClass_Caption);
	// Altitude Text Max
	leftX = x + leftMargin + 90 + padX;
	topY -= 20;
	bottomY = topY - heightFields;
	widthField = 50;
	wTextAltitudeMax = XPCreateWidget(leftX, topY, leftX + widthField, bottomY, 1, convertToString(userAltitudeMax).c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextAltitudeMax, xpProperty_TextFieldType, xpTextEntryField);
	XPCreateWidget(x + leftMargin + 60 + padX, topY, x + leftMargin + 65 + padX, bottomY, 1, "Max:", 0, wMainWindow, xpWidgetClass_Caption);

	// Airspeed
	topY = tmpY;
	tmpX = 170;
	leftX = x + leftMargin + tmpX + padX + padX;
	rightX = leftX + widthCaption;
	bottomY = topY - heightFields;
	XPCreateWidget(leftX, topY, rightX, bottomY, 1, "Airspeed", 0, wMainWindow, xpWidgetClass_Caption);
	// Airspeed Checkbox
	tmpX = tmpX + leftMargin - 88 + padX;
	wChkAirspeed = XPCreateWidget(leftX - 80, topY - 5, leftX + widthField, bottomY - 5, 1, "", 0, wMainWindow, xpWidgetClass_Button);
	XPSetWidgetProperty(wChkAirspeed, xpProperty_ButtonType, xpRadioButton);
	XPSetWidgetProperty(wChkAirspeed, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
	XPSetWidgetProperty(wChkAirspeed, xpProperty_ButtonState, isAirspeedSelected);
	// Airspeed Caption
	leftX = x + leftMargin + tmpX + padX;
	topY -= 30;
	widthField = 50;
	bottomY = topY - heightFields;
	wCaptionAirspeed = XPCreateWidget(leftX + 10, topY, leftX + widthField, bottomY, 1, "0", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wCaptionAirspeed, xpProperty_CaptionLit, 1);
	// Airspeed Text Min
	leftX += 90;
	topY += 10;
	bottomY = topY - heightFields;
	widthField = 50;
	wTextAirspeedMin = XPCreateWidget(leftX, topY, leftX + widthField, bottomY, 1, convertToString(userAirspeedMin).c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextAirspeedMin, xpProperty_TextFieldType, xpTextEntryField);
	XPCreateWidget(x + leftMargin + 60 + tmpX + padX, topY, x + leftMargin + 65 + tmpX + padX, bottomY, 1, "Min:", 0, wMainWindow, xpWidgetClass_Caption);
	// Airspeed Text Max
	leftX = x + leftMargin + 90 + tmpX + padX;
	topY -= 20;
	bottomY = topY - heightFields;
	widthField = 50;
	wTextAirspeedMax = XPCreateWidget(leftX, topY, leftX + widthField, bottomY, 1, convertToString(userAirspeedMax).c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextAirspeedMax, xpProperty_TextFieldType, xpTextEntryField);
	XPCreateWidget(x + leftMargin + 60 + tmpX + padX, topY, x + leftMargin + 65 + tmpX + padX, bottomY, 1, "Max:", 0, wMainWindow, xpWidgetClass_Caption);

	// GPS
	topY = tmpY + 18;
	tmpX = 480;
	leftX = x + leftMargin + tmpX + padX + padX;
	rightX = leftX + widthCaption;
	bottomY = topY - heightFields;
	XPCreateWidget(leftX, topY, rightX, bottomY, 1, "GPS", 0, wMainWindow, xpWidgetClass_Caption);
	// GPS Checkbox
	tmpX = tmpX + leftMargin - 88 + padX;
	wChkGPS = XPCreateWidget(leftX - 80, topY - 5, leftX + widthField, bottomY - 5, 1, "", 0, wMainWindow, xpWidgetClass_Button);
	XPSetWidgetProperty(wChkGPS, xpProperty_ButtonType, xpRadioButton);
	XPSetWidgetProperty(wChkGPS, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
	XPSetWidgetProperty(wChkGPS, xpProperty_ButtonState, isGPSSelected);
	// GPS Caption
	leftX = x + leftMargin + tmpX + padX;
	topY -= 22;
	widthField = 50;
	bottomY = topY - heightFields;
	wCaptionGPS = XPCreateWidget(leftX - 80, topY, (leftX - 80) + 150, bottomY, 1, "0", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wCaptionGPS, xpProperty_CaptionLit, 1);
	// GPS DME Distance Caption
	topY -= 34; bottomY = topY - heightFields;
	XPCreateWidget(leftX - 8, topY + 35, rightX + 10, bottomY, 1, "Min", 0, wMainWindow, xpWidgetClass_Caption);
	XPCreateWidget(leftX + 155, topY + 35, rightX + 165, bottomY, 1, "Max", 0, wMainWindow, xpWidgetClass_Caption);
	XPCreateWidget(leftX - 100, topY, rightX, bottomY, 1, "DME Distance:", 0, wMainWindow, xpWidgetClass_Caption);
	// GPS DME Distance Text Min.
	tmpX = leftX - 18;
	wTextGPSDmeDistanceMin = XPCreateWidget(tmpX, topY, tmpX + widthField, bottomY, 1, convertToString(userGPSDmeDistanceMin).c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextGPSDmeDistanceMin, xpProperty_TextFieldType, xpTextEntryField);
	XPCreateWidget(tmpX + widthField, topY, tmpX + widthField + 10, bottomY, 1, ">=", 0, wMainWindow, xpWidgetClass_Caption);
	tmpX = leftX + widthField + 3;
	wCaptionGPSDmeDistance = XPCreateWidget(tmpX, topY, tmpX + widthField, bottomY, 1, "0", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wCaptionGPSDmeDistance, xpProperty_CaptionLit, 1);
	// GPS DME Distance Text Max.
	tmpX = leftX + widthField + widthField + 45;
	wTextGPSDmeDistanceMax = XPCreateWidget(tmpX, topY, tmpX + widthField, bottomY, 1, convertToString(userGPSDmeDistanceMax).c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextGPSDmeDistanceMax, xpProperty_TextFieldType, xpTextEntryField);
	tmpX = leftX + widthField + 70;
	XPCreateWidget(tmpX, topY, tmpX + 10, bottomY, 1, ">=", 0, wMainWindow, xpWidgetClass_Caption);

	// GPS DME Time Min.
	topY -= 18; bottomY = topY - heightFields;
	XPCreateWidget(leftX - 100, topY, rightX, bottomY, 1, "DME Time:", 0, wMainWindow, xpWidgetClass_Caption);
	// GPS DME Time Text Min.
	tmpX = leftX - 18;
	wTextGPSDmeTimeMin = XPCreateWidget(tmpX, topY, tmpX + widthField, bottomY, 1, convertToString(userGPSDmeTimeMin).c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextGPSDmeTimeMin, xpProperty_TextFieldType, xpTextEntryField);
	XPCreateWidget(tmpX + widthField, topY, tmpX + widthField + 10, bottomY, 1, ">=", 0, wMainWindow, xpWidgetClass_Caption);
	tmpX = leftX + widthField + 3;
	wCaptionGPSDmeTime = XPCreateWidget(tmpX, topY, tmpX + widthField, bottomY, 1, "0", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wCaptionGPSDmeTime, xpProperty_CaptionLit, 1);
	// GPS DME Time Text Max.
	tmpX = leftX + widthField + widthField + 45;
	wTextGPSDmeTimeMax = XPCreateWidget(tmpX, topY, tmpX + widthField, bottomY, 1, convertToString(userGPSDmeTimeMax).c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextGPSDmeTimeMax, xpProperty_TextFieldType, xpTextEntryField);
	tmpX = leftX + widthField + 70;
	XPCreateWidget(tmpX, topY, tmpX + 10, bottomY, 1, ">=", 0, wMainWindow, xpWidgetClass_Caption);

	topY -= 30;

	// Latitude e Longitude
	leftX = x + leftMargin + 28;
	int leftXLatLong = leftX;
	rightX = leftX;
	bottomY = topY - 10;
	tmpY = topY;
	tmpX = leftX;
	int topYlatLong = topY;
	XPCreateWidget(leftX + 20, topY, rightX + 100, bottomY, 1, "Latitude / Longitude (Decimal degrees)", 0, wMainWindow, xpWidgetClass_Caption);
	leftX -= 30;
	topY -= 20;
	wChkLatLon = XPCreateWidget(leftX, topY, leftX + widthField, bottomY - 40, 1, "", 0, wMainWindow, xpWidgetClass_Button);
	XPSetWidgetProperty(wChkLatLon, xpProperty_ButtonType, xpRadioButton);
	XPSetWidgetProperty(wChkLatLon, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
	XPSetWidgetProperty(wChkLatLon, xpProperty_ButtonState, isLatLongSelected);
	leftX += 45;
	topY += 5;
	wCaptionLatLon = XPCreateWidget(leftX, topY, leftX + 100, bottomY - 40, 1, "00.000 / 00.000", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(wCaptionGPS, xpProperty_CaptionLit, 1);
	leftX += 100;
	bottomY -= 40;
	XPCreateWidget(leftX, topY, leftX + 100, bottomY, 1, "=", 0, wMainWindow, xpWidgetClass_Caption);
	leftX += 15;
	topY -= 3;
	bottomY -= 3;
	wTextLatitude = XPCreateWidget(leftX, topY, leftX + 55, bottomY, 1, objUserLatitude.getValorStr().c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextLatitude, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(wTextLatitude, xpProperty_MaxCharacters, 7);
	leftX += 58;
	topY += 3;
	bottomY += 3;
	XPCreateWidget(leftX, topY, leftX + 100, bottomY, 1, "/", 0, wMainWindow, xpWidgetClass_Caption);
	leftX += 12;
	topY -= 3;
	bottomY -= 3;
	wTextLongitude = XPCreateWidget(leftX, topY, leftX + 55, bottomY, 1, objUserLongitude.getValorStr().c_str(), 0, wMainWindow, xpWidgetClass_TextField);
	XPSetWidgetProperty(wTextLongitude, xpProperty_TextFieldType, xpTextEntryField);
	XPSetWidgetProperty(wTextLongitude, xpProperty_MaxCharacters, 7);
	

	// Navaids
	int hTextField = 22;
	leftX = x + leftMargin + 470;
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
	int dataReftopY    = topYlatLong - 60;
	int dataRefRightX  = dataRefleftX;
	int dataRefBottomY = topYlatLong - 80;
	XPCreateWidget(dataRefleftX + 30, dataReftopY, dataRefRightX + 130, dataRefBottomY, 1, " ------ Pause with DataRefs ------", 0, wMainWindow, xpWidgetClass_Caption);
	XPCreateWidget(dataRefleftX - 18, dataReftopY - 15, dataRefRightX + 50, dataRefBottomY - 25, 1, "DataRef...", 0, wMainWindow, xpWidgetClass_Caption);
	XPCreateWidget(dataRefleftX + 193, dataReftopY - 15, dataRefRightX + 217, dataRefBottomY - 25, 1, "Pause when...", 0, wMainWindow, xpWidgetClass_Caption);

	int sizeDataRefField     = 210;
	int sizeDataRefValueField = 45;
	int marginDataRefFields  = 30;
	int gapBetweenFields     = 2;
	int gapBetweenChks       = 5;

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


	//*********************************************************************************************************************************

	topY -= 50;

	// Button Save
	tmpX = 320;
	leftX = x + tmpX;
	bottomY = topY - heightFields - 5;
	wBtnSave = XPCreateWidget(leftX, topY, leftX + 60, bottomY, 1, "Save", 0, wMainWindow, xpWidgetClass_Button);
	XPSetWidgetProperty(wBtnSave, xpProperty_ButtonType, xpPushButton);
	// Button Exit
	leftX = x + (tmpX + 100);
	bottomY = topY - heightFields - 5;
	wBtnCancel = XPCreateWidget(leftX, topY, leftX + 60, bottomY, 1, "Exit", 0, wMainWindow, xpWidgetClass_Button);
	XPSetWidgetProperty(wBtnCancel, xpProperty_ButtonType, xpPushButton);

	topY -= 25;
	XPWidgetID email = XPCreateWidget(leftX + 210, topY, leftX + 310, bottomY, 1, "ualter.junior@gmail.com", 0, wMainWindow, xpWidgetClass_Caption);
	XPSetWidgetProperty(email, xpProperty_CaptionLit, 1);

	
	//*********************************************************************************************************************************
	//*********************************************************************************************************************************
	//*********************************************************************************************************************************
	//*********************************************************************************************************************************
	//*********************************************************************************************************************************
	// Button Reload
	leftX += 100;
	bottomY = topY-heightFields;
	wBtnReload = XPCreateWidget(leftX, topY-5, leftX+40, bottomY,1,"Reload",0,wMainWindow,xpWidgetClass_Button);
	XPSetWidgetProperty(wBtnReload,xpProperty_ButtonType,xpPushButton);

	// Just For Debug Purposes
	topY -= 15;
	bottomY = topY-heightFields;
	leftX   = x+leftMargin;
	widgetDebug1 = XPCreateWidget(leftX, topY, leftX+80, bottomY,1,"Debug1!",0,wMainWindow,xpWidgetClass_Caption);
	leftX += 160;
	widgetDebug2 = XPCreateWidget(leftX, topY, leftX+80, bottomY,1,"Debug2!",0,wMainWindow,xpWidgetClass_Caption);
	leftX += 160;
	widgetDebug3 = XPCreateWidget(leftX, topY, leftX+80, bottomY,1,"Debug3!",0,wMainWindow,xpWidgetClass_Caption);

	XPSetWidgetDescriptor(widgetDebug1, dataRef1.c_str());

	// AQUI
	//XPLMCreateWindow_t params;
	//params.structSize = sizeof(params);
	//params.visible = 1;
	//params.drawWindowFunc = draw_hello_world;
	//// Note on "dummy" handlers:
	//// Even if we don't want to handle these events, we have to register a "do-nothing" callback for them
	//params.handleMouseClickFunc = dummy_mouse_handler;
	//params.handleRightClickFunc = dummy_mouse_handler;
	//params.handleMouseWheelFunc = dummy_wheel_handler;
	//params.handleKeyFunc = dummy_key_handler;
	//params.handleCursorFunc = dummy_cursor_status_handler;
	//params.refcon = NULL;
	//params.layer = xplm_WindowLayerFloatingWindows;
	//// Opt-in to styling our window like an X-Plane 11 native window
	//// If you're on XPLM300, not XPLM301, swap this enum for the literal value 1.
	//params.decorateAsFloatingWindow = xplm_WindowDecorationRoundRectangle;
	//// Set the window's initial bounds
	//// Note that we're not guaranteed that the main monitor's lower left is at (0, 0)...
	//// We'll need to query for the global desktop bounds!
	//int left, bottom, right, top;
	//XPLMGetScreenBoundsGlobal(&left, &top, &right, &bottom);
	//params.left = left + 50;
	//params.bottom = bottom + 150;
	//params.right = params.left + 200;
	//params.top = params.bottom + 200;
	//g_window = XPLMCreateWindowEx(&params);
	//// Position the window as a "free" floating window, which the user can drag around
	//XPLMSetWindowPositioningMode(g_window, xplm_WindowPositionFree, -1);
	//// Limit resizing our window: maintain a minimum width/height of 100 boxels and a max width/height of 300 boxels
	//XPLMSetWindowResizingLimits(g_window, 200, 200, 300, 300);
	//XPLMSetWindowTitle(g_window, "Pause for Me");


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

			saveFileValues();
			return 1;
		}
		if (inParam1 == (intptr_t)wBtnAlertWindowClose)
		{
			XPHideWidget(wAlertWindow);
			AlertWindowShown = 0;
			XPLMCommandKeyStroke(xplm_key_pause);
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
			return 1;
		}
		if (inParam1 == (intptr_t)wChkDataRef2)
		{
			long isStateTrue = XPGetWidgetProperty(wChkDataRef2, xpProperty_ButtonState, 0);
			isStateTrue ? isDataRef2Selected = 1 : isDataRef2Selected = 0;
			return 1;
		}
		if (inParam1 == (intptr_t)wChkDataRef3)
		{
			long isStateTrue = XPGetWidgetProperty(wChkDataRef3, xpProperty_ButtonState, 0);
			isStateTrue ? isDataRef3Selected = 1 : isDataRef3Selected = 0;
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

	NavaidManager navManager;
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

	isBatteryOn = XPLMGetDatai(XPLMFindDataRef("sim/cockpit/electrical/battery_on"));
	isGamePaused = XPLMGetDatai(XPLMFindDataRef("sim/time/paused"));
	currentGPSDistDme = (int)XPLMGetDataf(XPLMFindDataRef("sim/cockpit/radios/gps_dme_dist_m"));
	currentGPSTimeDme = (int)XPLMGetDataf(XPLMFindDataRef("sim/cockpit/radios/gps_dme_time_secs"));

	// GPS Destination
	XPLMNavRef gpsDestination = XPLMGetGPSDestination();
	XPLMNavType gpsDestinationType = XPLMGetGPSDestinationType();

	int outFrequency;
	char outID[10];
	char outName[256];
	XPLMGetNavAidInfo(gpsDestination, &gpsDestinationType, NULL, NULL, NULL, &outFrequency, NULL, outID, outName, NULL);
	if (strcmp(outID, "----") != 0)  {
		std::string descripDestTypeGPS = getDescriptionGPSDestinationType(gpsDestinationType);
		sprintf(label, "%s: %s  (%s)", descripDestTypeGPS.c_str(), outName, outID);
	} else {
		std::string descripDestTypeGPS = "FMS";
		sprintf(label, "%s", "FMS");
	}
	XPSetWidgetDescriptor(wCaptionGPS, label);
	sprintf(label, "%dnm", currentGPSDistDme);
	XPSetWidgetDescriptor(wCaptionGPSDmeDistance, label);
	sprintf(label, "%dmin", currentGPSTimeDme);
	XPSetWidgetDescriptor(wCaptionGPSDmeTime, label);

	currentAltitude = (int)XPLMGetDataf(XPLMFindDataRef("sim/cockpit2/gauges/indicators/altitude_ft_pilot"));
	currentAirspeed = (int)XPLMGetDataf(XPLMFindDataRef("sim/cockpit2/gauges/indicators/airspeed_kts_pilot"));
	sprintf(label, "%dft", (int)currentAltitude);
	XPSetWidgetDescriptor(wCaptionAltitude, label);
	sprintf(label, "%dkts", (int)currentAirspeed);
	XPSetWidgetDescriptor(wCaptionAirspeed, label);

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

	// Latitude e Longitude
	double currentLatitude = XPLMGetDatad(XPLMFindDataRef("sim/flightmodel/position/latitude"));
	double currentLongitude = XPLMGetDatad(XPLMFindDataRef("sim/flightmodel/position/longitude"));
	objCurrentLatitude.setValor(currentLatitude);
	objCurrentLongitude.setValor(currentLongitude);
	sprintf(label, "%s / %s", objCurrentLatitude.getValorStr().c_str(), objCurrentLongitude.getValorStr().c_str());
	XPSetWidgetDescriptor(wCaptionLatLon, label);

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
			sprintf(msgPause, "DataRef %s", dataRef1.c_str());
				// convertToString(navaidDME.distance).c_str(), convertToString(userNavaidDMEDistance).c_str());
			wChkToUnSelect     = wChkDataRef1;
			isDataRef1Selected = 0;
		}
	}

	if (isDataRef2Selected) {
		result = checkDataRefs(2);
		if (result == 1) {
			sprintf(msgPause, "DataRef %s", dataRef2.c_str());
			// convertToString(navaidDME.distance).c_str(), convertToString(userNavaidDMEDistance).c_str());
			wChkToUnSelect = wChkDataRef2;
			isDataRef2Selected = 0;
		}
	}

	if (isDataRef3Selected) {
		result = checkDataRefs(3);
		if (result == 1) {
			sprintf(msgPause, "DataRef %s", dataRef3.c_str());
			// convertToString(navaidDME.distance).c_str(), convertToString(userNavaidDMEDistance).c_str());
			wChkToUnSelect = wChkDataRef3;
			isDataRef3Selected = 0;
		}
	}

	return result;
}

int checkDataRefs(int number) {
	XPWidgetID wDataRef;
	XPWidgetID wDataRefValue;
	XPWidgetID debugId;
	XPWidgetID wCheckbox;
	int        result = 0;

	if (number == 1) {
		wDataRef      = wDataRef1;
		wDataRefValue = wDataRefValue1;
		debugId       = widgetDebug1;
		wCheckbox     = wChkDataRef1;
	} else
	if (number == 2) {
		wDataRef      = wDataRef2;
		wDataRefValue = wDataRefValue2;
		debugId       = widgetDebug2;
		wCheckbox     = wChkDataRef2;
	} else
	if (number == 3) {
		wDataRef      = wDataRef3;
		wDataRefValue = wDataRefValue3;
		debugId       = widgetDebug3;
		wCheckbox     = wChkDataRef3;
	}

	char b1[256], b2[256];
	XPGetWidgetDescriptor(wDataRef, b1, sizeof(b1));
	XPGetWidgetDescriptor(wDataRefValue, b2, sizeof(b2));
	
	std::string s = b2;
	// Float or Int (if there's a point, so...  it is a Float value, otherwise it will be treated as a Integer)
	if (s.find('.') != std::string::npos) {
		float realDataRefValue   = XPLMGetDataf(XPLMFindDataRef(b1));
		float wishedDataRefValue = convertToNumber(b2);
		XPSetWidgetDescriptor(debugId, std::to_string(realDataRefValue).c_str());

		XPLMDebugString("---: number..: ");
		XPLMDebugString(std::to_string(number).c_str());
		XPLMDebugString("\n");

		XPLMDebugString("---: realDataRefValue..: ");
		XPLMDebugString(std::to_string(realDataRefValue).c_str());
		XPLMDebugString("\n");

		XPLMDebugString("---: wishedDataRefValue..: ");
		XPLMDebugString(std::to_string(wishedDataRefValue).c_str());
		XPLMDebugString("\n");

		if (realDataRefValue == wishedDataRefValue) {
			result = 1;
		}
	} else {
		int   realDataRefValue = (int)XPLMGetDataf(XPLMFindDataRef(b1));
		int   wishedDataRefValue = convertToNumber(b2);
		XPSetWidgetDescriptor(debugId, convertToString(realDataRefValue).c_str());

		XPLMDebugString("---: number..: ");
		XPLMDebugString(std::to_string(number).c_str());
		XPLMDebugString("\n");

		XPLMDebugString("---: realDataRefValue..: ");
		XPLMDebugString(std::to_string(realDataRefValue).c_str());
		XPLMDebugString("\n");

		XPLMDebugString("---: wishedDataRefValue..: ");
		XPLMDebugString(std::to_string(wishedDataRefValue).c_str());
		XPLMDebugString("\n");
		

		if (realDataRefValue == wishedDataRefValue) {
			result = 1;
		}
	}

	XPLMDebugString(" RESULT..: ");
	XPLMDebugString(std::to_string(result).c_str());
	XPLMDebugString("\n");
	XPLMDebugString("\n");

	return result;
}

float CallBackXPlane(float  inElapsedSinceLastCall,
	float  inElapsedTimeSinceLastFlightLoop,
	int    inCounter,
	void * inRefcon)
{
	getXPlaneDataInfos();

	float pause = pauseXPlane();
	if (pause) {
		if (!isGamePaused)
		{
			if (!AlertWindowShown) {
				int aX = 730, aY = 780;
				int aW = 320, aH = 150;
				wAlertWindow = XPCreateWidget(aX, aY, aX + aW, aY - aH, 1, "Pause For Me!!!", 1, NULL, xpWidgetClass_SubWindow);
				//XPSetWidgetProperty(wAlertWindow, xpProperty_MainWindowHasCloseBoxes, 1) (Erro quando ligado!!!);
				XPWidgetID c0 = XPCreateWidget(aX + 20, aY, aX + aW + 20, aY - aH + 120, 1, "                *-*-*-*-*-*  Pause For Me!!!  *-*-*-*-*-*"
					, 0, wAlertWindow, xpWidgetClass_Caption);
				XPSetWidgetProperty(c0, xpProperty_CaptionLit, 1);
				XPWidgetID c1 = XPCreateWidget(aX + 20, aY, aX + aW + 20, aY - aH + 25, 1, "Reason:", 0, wAlertWindow, xpWidgetClass_Caption);
				XPSetWidgetProperty(c1, xpProperty_CaptionLit, 0);
				XPWidgetID c2 = XPCreateWidget(aX + 20, aY, aX + aW + 20, aY - aH, 1, msgPause, 0, wAlertWindow, xpWidgetClass_Caption);
				XPSetWidgetProperty(c2, xpProperty_CaptionLit, 0);
				wBtnAlertWindowClose = XPCreateWidget(aX + 110, aY, aX + 210, aY - aH - 80, 1, "  Close  ", 0, wAlertWindow, xpWidgetClass_Button);
				XPSetWidgetProperty(wBtnAlertWindowClose, xpProperty_ButtonType, xpPushButton);
				XPAddWidgetCallback(wAlertWindow, widgetWidgetHandler);
				AlertWindowShown = 1;
			}
			XPLMCommandKeyStroke(xplm_key_pause);
			if (wChkToUnSelect != NULL)
				XPSetWidgetProperty(wChkToUnSelect, xpProperty_ButtonState, 0);
		}
	}
	return CHECKINTERVAL;
}



PLUGIN_API void	XPluginStop(void)
{
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

void	draw_hello_world(XPLMWindowID in_window_id, void * in_refcon)
{
	// Mandatory: We *must* set the OpenGL state before drawing
	// (we can't make any assumptions about it)
	XPLMSetGraphicsState(
		0 /* no fog */,
		0 /* 0 texture units */,
		0 /* no lighting */,
		0 /* no alpha testing */,
		1 /* do alpha blend */,
		1 /* do depth testing */,
		0 /* no depth writing */
	);

	int l, t, r, b;
	XPLMGetWindowGeometry(in_window_id, &l, &t, &r, &b);

	float col_white[] = { 1.0, 1.0, 1.0 }; // red, green, blue

	XPLMDrawString(col_white, l + 10, t - 20, "Hello world!", NULL, xplmFont_Proportional);
}