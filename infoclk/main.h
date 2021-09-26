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
 *  Description      : Common includes and defines
 *  (c) Emil Muratov 2017
 *
 */

#pragma once
// Sketch configuration
#include "globals.h"    // EmbUI macro's for LOG
#include "config.h"
#include "uistrings.h"  // non-localized text-strings
#include "ts.h"         // task scheduler
#include <ESPAsyncWebServer.h>

#define FW_NAME "infoclock"

#define INFOCLOCK_VERSION_MAJOR     1
#define INFOCLOCK_VERSION_MINOR     2
#define INFOCLOCK_VERSION_REVISION  1

/* make version as integer*/
#define INFOCLOCK_VERSION ((INFOCLOCK_VERSION_MAJOR) << 16 | (INFOCLOCK_VERSION_MINOR) << 8 | (INFOCLOCK_VERSION_REVISION))

/* make version as string*/
#define INFOCLOCK_VERSION_STRING   TOSTRING(INFOCLOCK_VERSION_MAJOR) "." TOSTRING(INFOCLOCK_VERSION_MINOR) "." TOSTRING(INFOCLOCK_VERSION_REVISION)

#define UPD_RESTART_DELAY   5   // restart delay when updating firmware
#define BAUD_RATE	115200	// serial debug port baud rate

#define HTTP_VER_BUFSIZE 200

// PROGMEM strings
// sprintf template for json version data
static const char PGverjson[] PROGMEM = "{\"ChipID\":\"%x\",\"Flash\":%u,\"Core\":\"%s\",\"SDK\":\"%s\",\"firmware\":\"" FW_NAME "\",\"version\":\"" INFOCLOCK_VERSION_STRING "\",\"git\":\"%s\",\"CPUMHz\":%u,\"Heap\":%u,\"Uptime\":%u}";

// TaskScheduler
//Let the runner object be a global, single instance shared between object files.
extern Scheduler ts;

void create_parameters();       // декларируем для переопределения weak метода из фреймворка для WebUI

// reboot esp with a delay
void espreboot(void);

// WiFi connection callback
void onSTAGotIP();
// Manage network disconnection
void onSTADisconnected();
// firmware version
void wver(AsyncWebServerRequest *request);