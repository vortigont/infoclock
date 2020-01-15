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
//#include <ESP8266WiFi.h>
//#include <ESP8266WebServer.h>
//#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
// esp8266 OTA Updates
#include <ESP8266HTTPClient.h>
//#include <ESP8266httpUpdate.h>

// Task Scheduler lib	https://github.com/arkhipenko/TaskScheduler
#define _TASK_SLEEP_ON_IDLE_RUN
#include <TaskSchedulerDeclarations.h>

// NTP time
//#include <TimeLib.h>
#include <NtpClientLib.h>

// Adafruit_GFX
#include <Adafruit_GFX.h>	// need to override bundled "glcdfont.c" font with rus version
#include <Max72xxPanel.h>

// i2c lib
#include <Wire.h>

//Baro sensor
//#include <EnvironmentCalculations.h>
#include <BME280I2C.h>       //https://github.com/finitespace/BME280

// eeprom configuration class
#include "EEPROMCfg.h"

// PROGMEM strings
#include "http.h"

// Macro
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

// Defines
#define HTTP_VER_BUFSIZE 150
#define SENSOR_DATA_BUFSIZE 100 // chars for sensor data
#define UPD_RESTART_DELAY 15    // restart delay when updating firmware

//void wifibegin(EEPROMCfg::getConfig());
// callback functions
void wifibegin(const cfg &conf);   // Initialize WiFi

// HTTP server callbacks
bool cfgupdate( const String& json);		// Parse json config data and update EEPROM
void wotareq(AsyncWebServerRequest *request);	// Web OTA callbask
void wotaupl(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
//void otaclient();                  		// try OTA Update
void wcfgget(AsyncWebServerRequest *request);   // Web: Provide json encoded config data
void wcfgset(AsyncWebServerRequest *request);   // Web: Update config in EEPROM
void wver(AsyncWebServerRequest *request);	// return json with device status & sw version


// TaskScheduler
//Let the runner object be a global, single instance shared between object files.
extern Scheduler ts;

// Task Callback methods prototypes
void GetWeather();		// Update weather info with HTTP client
void updsensstr();		// Update data from sensors and build text string
void panescroller();		// Scroll text over pane
void doSeconds();		// every second pulse task (tSecondsPulse)
bool drawticks();		// Draw hh:mm ticks
void clearticks();		// clear hh:mm ticks

// Display manipulation functions
uint8_t brightness_calc(void);		// calculate display brightness for current time of day
void bigClk ();				// Draw clock with big font
void sectick(uint16_t x, uint16_t y);	// Draw seconds ticks

// work with data sources
void getsensordata(float& t, float& h, float& p, float& dew);	// retreive data from BME280 sensor
void ParseWeather(String s);		// Parse response from weather server

// recode UTF8 rus strings
// http://arduino.ru/forum/programmirovanie/rusifikatsiya-biblioteki-adafruit-gfx-i-vyvod-russkikh-bukv-na-displei-v-kodi
String utf8toCP1251(String source);
String utf8rus(String source);


template <typename T> void scroll( const T& str, int y, int& scrollptr);
