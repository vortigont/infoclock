/*
 *  InfoClock - ESP8266 based informeter/wall-clock
 *  ESP8266 contoller uses Max72xx modules as a display
 *  Internal/external sensors/message brokers could be used as an information source
 *
 *  Author      : Emil Muratov
 *
 *  This file        : sensors.h
 *  This file Author : Emil Muratow
 *
 *  Description      : sensors poller/parser
 *
*/

#pragma once
#include <Wire.h>
//Baro sensor
//#include <EnvironmentCalculations.h>
#include <BME280I2C.h>       //https://github.com/finitespace/BME280

//Si7021 secsor
#include <HTU21D.h>

 // chars for sensors formatted data
#define SENSOR_DATA_BUFSIZE 25

// List of sensor types
static const char sname_0[] PROGMEM = "N/A";
static const char sname_1[] PROGMEM = "BME280";
static const char sname_2[] PROGMEM = "Si7021";

//Table of sensor names
const char* const sensor_types[] PROGMEM = { sname_0, sname_1, sname_2 };

// sensors type enum
enum class sensor_t{NA, bme280, si7021};

class Sensors {
private:
  void readbme280(float& t, float& h, float& p, float& dew);
  void readsi7021(float& t, float& h);
  sensor_t _sensor_model;

public:
   Sensors();
   virtual ~Sensors();
   sensor_t begin();
   bool getFormattedValues( char* str);
};
