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

#include "sensors.h"

sensor_t sensor_model = sensor_t::NA;

/**
 * default constructor
**/
Sensors::Sensors(){
  Wire.begin();
}

/**
 * default destructor
**/
Sensors::~Sensors(){}

BME280I2C s_bme;
HTU21D s_si7021(HTU21D_RES_RH12_TEMP14);

sensor_t Sensors::begin(){
  if (s_bme.begin()) {
    _sensor_model = sensor_t::bme280;
  } else if(s_si7021.begin())  {
    _sensor_model = sensor_t::si7021;
  }

  return _sensor_model;
}

void Sensors::readbme280(float& t, float& h, float& p, float& dew) {
    BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
    BME280::PresUnit presUnit(BME280::PresUnit_torr);
    s_bme.read(p, t, h, tempUnit, presUnit);
     //EnvironmentCalculations::TempUnit     envTempUnit =  EnvironmentCalculations::TempUnit_Celsius;
     //dew = EnvironmentCalculations::DewPoint(t, h, envTempUnit);
 }

void Sensors::readsi7021(float& t, float& h) {
   h = s_si7021.readHumidity();
   t = s_si7021.readTemperature(SI70xx_TEMP_READ_AFTER_RH_MEASURMENT);
}

 // Update string with sensor's data
bool Sensors::getFormattedValues(char* str) {
 	float temp, pressure, humidity, dew = NAN;
    switch(_sensor_model) {
      case sensor_t::bme280 :
            readbme280(temp, humidity, pressure, dew);
            snprintf_P(str, SENSOR_DATA_BUFSIZE, PSTR("T:%.1f Rh:%.f%% P:%.fmmHg"), temp, humidity, pressure);
            break;
      case sensor_t::si7021 :
            readsi7021(temp, humidity);
            snprintf_P(str, SENSOR_DATA_BUFSIZE, PSTR("T:%.1f Rh:%.f%%"), temp, humidity);
            break;
      case sensor_t::NA :
            snprintf_P(str, SENSOR_DATA_BUFSIZE, PSTR("No sensors not found!"));
            return false;
    }
   //_SPLN(str);		//debug, print data to serial
   return true;
}
