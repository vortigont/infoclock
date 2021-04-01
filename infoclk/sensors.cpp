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

#include <SparkFun_SGP30_Arduino_Library.h>

BME280I2C s_bme;
HTU21D s_si7021(HTU21D_RES_RH12_TEMP14);
SGP30 sgp30;

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
    readbme280(temp, humidity, pressure, dew);
  } else if(s_si7021.begin())  {
    _sensor_model = sensor_t::si7021;
    readsi7021(temp, humidity);
  }

  if (sgp30.begin()){
    issgp = true;
    sgp30.initAirQuality();

    //Convert relative humidity to absolute humidity
    double absHumidity = RHtoAbsolute(humidity, temp);
    //Convert the double type humidity to a fixed point 8.8bit number
    uint16_t sensHumidity = doubleToFixedPoint(absHumidity);
    sgp30.setHumidity(sensHumidity);
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
bool Sensors::getFormattedValues(String &str){
  char sensorstr[SENSOR_DATA_BUFSIZE];
  bool rth = getFormattedValues(sensorstr);
  if (rth){
    str = String(sensorstr);
  } else {
    str = "";
  }

  if (!issgp)
    return rth;

  readsgp30(co2, tvoc, temp, humidity);
  str += " CO2: ";
  str += co2;
  str += " ppm, tvoc: ";
  str += tvoc;
  str += " ppb";

  return true;
}

bool Sensors::getFormattedValues(char* str) {

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

void Sensors::getSensorModel(String &str){
  char sensorstr[SENSOR_DATA_BUFSIZE];
  getSensorModel(sensorstr);
  str = String(sensorstr);
}

void Sensors::getSensorModel(char* str){
  snprintf_P(str, SENSOR_DATA_BUFSIZE, PSTR("Sensor model: %s"), sensor_types[(uint8_t)_sensor_model]);
}


void Sensors::sgp30poll(){
  sgp30.measureAirQuality(); 
}

void Sensors::readsgp30(uint16_t &co2, uint16_t &tvoc, const float rh, const float t){
  //sgp30.measureAirQuality();
  co2 = sgp30.CO2;
  tvoc = sgp30.TVOC;

//
    //Convert relative humidity to absolute humidity
    double absHumidity = RHtoAbsolute(humidity, temp);
    //Convert the double type humidity to a fixed point 8.8bit number
    uint16_t sensHumidity = doubleToFixedPoint(absHumidity);
    sgp30.setHumidity(sensHumidity);
//
}


double Sensors::RHtoAbsolute (float relHumidity, float tempC) {
  double eSat = 6.11 * pow(10.0, (7.5 * tempC / (237.7 + tempC)));
  double vaporPressure = (relHumidity * eSat) / 100; //millibars
  double absHumidity = 1000 * vaporPressure * 100 / ((tempC + 273) * 461.5); //Ideal gas law with unit conversions
  return absHumidity;
}

uint16_t Sensors::doubleToFixedPoint( double number) {
  int power = 1 << 8;
  double number2 = number * power;
  uint16_t value = floor(number2 + 0.5);
  return value;
}