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

// BME Sensor setup
#define PRESSURE_UNITS 5        // unit: B001 = hPa, B010 = inHg, 5 = mmHg
#define METRIC_UNITS true       // measurement units
#define SENSOR_UPD_PERIOD 5     // Update rate in seconds
#define SENSOR_DATA_BUFSIZE 25  // chars for sensors formatted data

// List of sensor types
static const char sname_0[] PROGMEM = "N/A";
static const char sname_1[] PROGMEM = "BME280";
static const char sname_2[] PROGMEM = "BMP280";
static const char sname_3[] PROGMEM = "Si7021";

//Table of sensor names
const char* const sensor_types[] PROGMEM = { sname_0, sname_1, sname_2, sname_3 };

// sensors type enum
enum class sensor_t{NA, bme280, bmp280, si7021};

class Sensors {
private:
  void readbme280(float& t, float& h, float& p, float& dew);
  void readsi7021(float& t, float& h);
  sensor_t _sensor_model = sensor_t::NA;


public:
   Sensors();
   virtual ~Sensors();
   sensor_t begin();
   bool getFormattedValues( char* str);
   void getSensorModel(char* str);
};
