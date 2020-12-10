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

#include <Arduino.h>
#include "sensors.h"

//Baro sensor
//#include <EnvironmentCalculations.h>
#include <BME280I2C.h>       //https://github.com/finitespace/BME280

//Si7021 secsor
#include <HTU21D.h>

BME280I2C s_bme;
HTU21D s_si7021(HTU21D_RES_RH12_TEMP14);

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

sensor_t Sensors::begin(){
  if (s_bme.begin()) {
    _sensor_model = s_bme.chipModel() == s_bme.ChipModel::ChipModel_BME280 ? sensor_t::bme280: sensor_t::bmp280;
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
      case sensor_t::bmp280 :
            readbme280(temp, humidity, pressure, dew);

            switch (s_bme.chipModel()){
              case BME280::ChipModel_BME280:
                snprintf_P(str, SENSOR_DATA_BUFSIZE, PSTR("T:%.1f Rh:%.f%% P:%.fmmHg"), temp, humidity, pressure);
                return true;
              case BME280::ChipModel_BMP280:
                snprintf_P(str, SENSOR_DATA_BUFSIZE, PSTR("T:%.1f P:%.fmmHg"), temp, pressure);
                return true;
              default:
                return false;
            }

            break;
      case sensor_t::si7021 :
            readsi7021(temp, humidity);
            snprintf_P(str, SENSOR_DATA_BUFSIZE, PSTR("T:%.1f Rh:%.f%%"), temp, humidity);
            return true;
      default:
            snprintf_P(str, SENSOR_DATA_BUFSIZE, PSTR("No sensors found!"));
            return false;
    }
}

void Sensors::getSensorModel(char* str){
  snprintf_P(str, SENSOR_DATA_BUFSIZE, PSTR("Sensor model: %s"), sensor_types[(uint8_t)_sensor_model]);
}
