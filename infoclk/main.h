/*
 *  InfoClock - ESP8266 based informeter/wall-clock
 *  ESP8266 contoller uses Max72xx modules as a display
 *  Internal/external sensors/message brokers could be used as an information source
 *
 *  Author		: Emil Muratov
 *
 *  This file        : main.h
 *  This file Author : Emil Muratow
 *
 *  Description      : Common includes and defines required by whole project
 *  (c) Emil Muratov 2017
 *
 */

// Sketch configuration
#include "Globals.h"

// Libs
//#include <ESPAsyncWebServer.h>
#include <ESP8266HTTPClient.h>

// Task Scheduler lib	https://github.com/arkhipenko/TaskScheduler
#define _TASK_SLEEP_ON_IDLE_RUN
#include <TaskSchedulerDeclarations.h>

// Time
#include <time.h>                       // time() ctime()
#include <sys/time.h>                   // struct timeval
//-#include <sntp.h>
extern "C" int clock_gettime(clockid_t unused, struct timespec *tp);


// Adafruit_GFX
#include <Adafruit_GFX.h>	// need to override bundled "glcdfont.c" font with rus version
#include <Max72xxPanel.h>

// Macro
//#define STRINGIFY(x) #x
//#define TOSTRING(x) STRINGIFY(x)

// Defines
#define HTTP_VER_BUFSIZE 200
#define WEATHER_INIT_DELAY 20   // seconds to delay weather update after WiFi connection 

#define UPD_RESTART_DELAY   5   // restart delay when updating firmware

#define BAUD_RATE	115200	// serial debug port baud rate

#define WEATHER_API_BUFSIZE 1024

// PROGMEM strings
//#include "http.h"
// sprintf template for json version data
static const char PGverjson[] PROGMEM = "{\"ChipID\":\"%x\",\"FlashSize\":%u,\"Core\":\"%s\",\"SDK\":\"%s\",\"firmware\":\"%s\",\"version\":\"%s\",\"CPUMHz\":%u,\"Heap\":%u,\"Uptime\":%u,}";
// weather API URL
static const char PGwapireq[] PROGMEM = WAPI_URL "?id=" WAPI_CITY_ID "&units=metric&lang=" COUNTRY "&APPID=" WAPI_KEY;

void create_parameters();       // декларируем для переопределения weak метода из фреймворка для WebUI

/*
void wver(AsyncWebServerRequest *request);	// return json with device status & sw version
void wf1(AsyncWebServerRequest *request);
void wf2(AsyncWebServerRequest *request);
*/

// TaskScheduler
//Let the runner object be a global, single instance shared between object files.
extern Scheduler ts;
extern Max72xxPanel matrix;

// Task Callback methods prototypes
void GetWeather();		// Update weather info with HTTP client
void updsensstr();		// Update data from sensors and build text string
void panescroller();		// Scroll text over pane
void doSeconds();		// every second pulse task (tSecondsPulse)
bool drawticks();		// Draw hh:mm ticks
void clearticks();		// clear hh:mm ticks
void refreshWeather();  // restart weather timer

// Display manipulation functions
uint8_t brightness_calc(void);		// calculate display brightness for current time of day
void bigClk ();				// Draw clock with big font
void sectick(uint16_t x, uint16_t y);	// Draw seconds ticks

// work with data sources
void readbme280(float& t, float& h, float& p, float& dew);	// retreive data from BME280 sensor
void readsi7021(float& t, float& h);	// retreive data from si7021 sensor

void ParseWeather(String s);		// Parse response from weather server

// recode UTF8 rus strings
// http://arduino.ru/forum/programmirovanie/rusifikatsiya-biblioteki-adafruit-gfx-i-vyvod-russkikh-bukv-na-displei-v-kodi
String utf8toCP1251(String source);
String utf8rus(String source);

// reboot esp with a delay
void espreboot(void);

// WiFi connection callback
void onSTAGotIP();
// Manage network disconnection
void onSTADisconnected();

template <typename T> void scroll( const T& str, int y, int& scrollptr);
