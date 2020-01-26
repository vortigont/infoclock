/*
 *  InfoClock - ESP8266 based informeter/wall-clock
 *  ESP8266 contoller uses Max72xx modules as a display
 *  Internal/external sensors/message brokers could be used as an information source
 *
 *  Author      : Emil Muratov
 *
 *  This file        : Globals.h
 *  This file Author : Emil Muratow
 *
 *  Description      : Common includes and defines required by whole project
 *
 */

#ifndef GLOBALS_H
#define GLOBALS_H

//#include <stdint.h>
//#include <sys/types.h>

// Sketch configuration
#include "config.h"

#include <Arduino.h>
// Libs
#include <ESP8266WiFi.h>

#include <ArduinoJson.h>


// ========= Defines

// Enable Serial.print if DEBUG
#ifdef _FWDEBUG_
  #define _SP(a) Serial.print(a);
  #define _SPLN(a) Serial.println(a);
  #define _SPF(a,b) Serial.printf(a,b);
  #define _SPTO(a) a;
#else
  #define _SP(a)
  #define _SPLN(a)
  #define _SPF(a,b)
  #define _SPTO(a)
#endif

/*
class Globals {
public:
    // public methods
    Globals();
    Globals(const Globals& orig);
    virtual ~Globals();
}
*/
#endif
