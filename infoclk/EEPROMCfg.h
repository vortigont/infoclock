/*
 *  InfoClock - ESP8266 based informeter/wall-clock
 *  ESP8266 contoller uses Max72xx modules as a display
 *  Internal/external sensors/message brokers could be used as an information source
 *
 *  Author      : Emil Muratov
 *
 *  This file        : EEPROMCfg.h
 *  This file Author : Alexey Shtykov, Emil Muratow
 *
 *  Description      : Save/Restore Configuration in EEPROM class implementation
 *
 */

#ifndef _INSTANCECONFIG_H_
#define _INSTANCECONFIG_H_
//#include <Arduino.h>
#include <config.h>
#include "Globals.h"
#include <EEPROM.h>

#ifndef WIFI_CFG_STR_LEN
#define WIFI_CFG_STR_LEN    16
#endif
#ifndef WIFI_OTA_URL_LEN
#define WIFI_OTA_URL_LEN    80
#endif

#define EEPROM_CFG_OFFSET	0
#define EEPROM_CFG_EXTRASIZE 2


// This vars are stored in EEPROM
typedef struct _cfg {
    char chostname[WIFI_CFG_STR_LEN];// WiFi hostname
    uint8_t cWmode;                    // WiFi mode 0: Station/Auto, 1: AccessPoint
    char cWssid[WIFI_CFG_STR_LEN];  // WiFi SSID
    char cWpwd[WIFI_CFG_STR_LEN];   // WiFi passwd
    char cOTAurl[WIFI_OTA_URL_LEN]; // OTA URL
}cfg;

class EEPROMCfg {
public:
    virtual ~EEPROMCfg();
    static  void    Reset(cfg& conf);
    static  void    Reset();
    static  void    Load(); 			// Load config from EEPROM into a cfg struct
    static  void    Save(const cfg &conf);	// Saves config from cfg struct into a EEPROM
    static  void    Save();
    static  bool    Update( const String& json);	// Parse json config data and update EEPROM
    static  uint8_t    crc8( uint8_t crc, uint8_t ch );
    static  const cfg&  getConfig() {return EEPROMCfg::m_config;};
    static        cfg&  setConfig() {return EEPROMCfg::m_config;};
    static  void    initFrom(const JsonObject& jsoncfg);
    static  void    cfg2json(const char* fmt, char* buf, size_t bufLen);
private:
    EEPROMCfg();
    EEPROMCfg(const EEPROMCfg& orig);
    static  void    copyTo(cfg& conf);
    static  void    initFrom(const cfg& conf);
    static  cfg m_config;
};

// Write structs to EEPROM
template <class T> size_t EEPROM_writeAny(size_t ee, const T& value)
{
    const uint8_t* p = (const uint8_t*)(const void*)&value;
    size_t i;
    for (i = 0; i != sizeof(value); i++)
          EEPROM.write(ee++, *p++);
    EEPROM.commit();
    return i;
}

// Read structs from EEPROM
template <class T> size_t EEPROM_readAny(size_t ee, T& value)
{
    uint8_t* p = (uint8_t*)(void*)&value;
    size_t i;
    for (i = 0; i < sizeof(value); i++)
        *p++ = EEPROM.read(ee++);
    return i;
}

// Calculate CRC for structs
template <class T> uint8_t CRC_Any( const T& value)
{
    const uint8_t* p = (const uint8_t*)(const void*)&value;
    uint8_t _crc=0;
    size_t i;
    for (i = 0; i != sizeof(value); i++)
          _crc = EEPROMCfg::crc8(_crc, *p++);
    return _crc;
}

#endif /* _INSTANCECONFIG_H_ */
