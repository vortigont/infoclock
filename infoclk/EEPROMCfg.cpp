/*
 *  InfoClock - ESP8266 based informeter/wall-clock
 *  ESP8266 contoller uses Max72xx modules as a display
 *  Internal/external sensors/message brokers could be used as an information source
 *
 *  Author      : Emil Muratov
 *
 *  This file        : EEPROMCfg.cpp
 *  This file Author : Alexey Shtykov, Emil Muratow
 *
 *  Description      : Save/Restore Configuration in EEPROM class implementation
 *
 */

//#include <cstdio>
//#include <cstdlib>
//#include <cstring>
#include "EEPROMCfg.h"

cfg EEPROMCfg::m_config;

/**
 * default constructor
 **/
EEPROMCfg::EEPROMCfg(){}

/**
 * copy constructor
 **/
EEPROMCfg::EEPROMCfg(const EEPROMCfg& orig){}

/**
 * default destructor
 **/
EEPROMCfg::~EEPROMCfg(){}

/**
 * Reset configuration to default
 * @param conf - configuration to reset
 */
void EEPROMCfg::Reset(cfg& conf)
{
    snprintf(conf.chostname, WIFI_CFG_STR_LEN, WIFI_DEF_HOSTNAME"-%06X", ESP.getChipId());
    snprintf(conf.cWssid, WIFI_CFG_STR_LEN, WIFI_DEF_HOSTNAME"_%06X", ESP.getChipId());
    snprintf(conf.cWpwd, WIFI_CFG_STR_LEN, "%s", WIFI_DEF_PASSWD);
    snprintf(conf.cOTAurl, WIFI_OTA_URL_LEN, "%s", OTA_URL);
    conf.cWmode = 0;
}

/**
 * reset default configuration
 */
void EEPROMCfg::Reset()
{
    Reset(m_config);
}

/**
 * Load config from EEPROM into a cfg struct
 * If EEPROM data is damaged, than reset to default configuration and update EEPROM
 */
void EEPROMCfg::Load()
{
    EEPROM.begin(sizeof (EEPROMCfg::m_config) + EEPROM_CFG_EXTRASIZE);
    size_t cfgsize = EEPROM_readAny(EEPROM_CFG_OFFSET, EEPROMCfg::m_config); // Load EEPROM data
    byte ee_crc;
    EEPROM_readAny(cfgsize, ee_crc); // read crc from eeprom
    EEPROM.end();

    byte _crc = CRC_Any(EEPROMCfg::m_config); // calculate crc for data
    if (_crc != ee_crc) {
        //Globals::SerialPrintln("Config CRC error, loading defaults");
        Reset(); // reset config to defaults
        Save(); // save default config to eeprom
    }
}

/**
 * Saves configuration from input into a EEPROM
 * @param conf
 */
void EEPROMCfg::Save(const cfg &conf)
{
    byte _crc = CRC_Any(conf);
    size_t cfgsize = sizeof (conf);
    EEPROM.begin(cfgsize + EEPROM_CFG_EXTRASIZE); //reserve some extra bytes for crc and...
    EEPROM_writeAny(EEPROM_CFG_OFFSET, conf);
    EEPROM_writeAny(cfgsize, _crc);
    EEPROM.end();
    //initFrom(conf);
}

/**
 * save default configuration
 */
void EEPROMCfg::Save()
{
    Save(m_config);
}

/**
 * compute crc8 sum on byte
 * @param crc - crc to update
 * @param ch  - byte to add
 * @return new crc8 value
 */
byte EEPROMCfg::crc8(byte crc, byte ch) {
    for (uint8_t i = 8; i; i--) {
        uint8_t mix = crc ^ ch;
        crc >>= 1;
        if (mix & 0x01) crc ^= 0x8C;
        ch >>= 1;
    }
    return crc;
}

/**
 * update configuration from JSON string
 * @param json - JSON input string
 * @return true if configuration set successfully
 */
bool EEPROMCfg::Update( const String& json)
{
    return false;
}

