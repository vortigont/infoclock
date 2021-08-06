// localization resources goes here

#pragma once

/**
 *  Dictionary size
 *  must be more or equal to the number of messages in TD Enum
 */
#define UI_DICT_SIZE 50

/**
 * Clock Text-Dictionary Enums for language resources
 * the order of enums must match with elements in dictionary
 *
 */
enum CD : uint8_t {
    Clock = (0U),
    MX_Reset,
    OPT_Weath,
    UPD_Weath,
    Weather,
    WAPIKEY,
    WthNote,
    WthCID,
    WthSrtName,
    mtx,
    snsrs
};


// Infoclock - English Strings (order does not matther)
// ИнфоЧасики - Русские тексты (порядок значения не имеет)
static const char T_EN_Clock[] PROGMEM = "Clock";
static const char T_RU_Clock[] PROGMEM = "Часы";
static const char T_EN_Weather[] PROGMEM = "Weather";
static const char T_RU_Weather[] PROGMEM = "Погода";
static const char T_EN_Matrix[] PROGMEM = "Matrix";
static const char T_RU_Matrix[] PROGMEM = "Матрица";
static const char T_EN_Sensors[] PROGMEM = "Sensors";
static const char T_RU_Sensors[] PROGMEM = "Сенсоры";


static const char T_EN_Weather_note[] PROGMEM = "Obtain your API-key at http://api.openweathermap.org/";
static const char T_RU_Weather_note[] PROGMEM = "Получите свой API-ключ на http://api.openweathermap.org/";
static const char T_EN_Weather_cid[] PROGMEM = "City-ID. Pls, find City-ID at https://openweathermap.org/city/";
static const char T_RU_Weather_cid[] PROGMEM = "Код города. Найдите код города на https://openweathermap.org/city/";
static const char T_EN_Weather_shname[] PROGMEM = "short name for city in output string (optional)";
static const char T_RU_Weather_shname[] PROGMEM = "короткое имя города в строке вывода (опционально)";


static const char T_EN_OPT_Weath[] PROGMEM = "Настройки погоды";
static const char T_RU_OPT_Weath[] PROGMEM = "Weather settings";
static const char T_EN_UPD_Weath[] PROGMEM = "Refresh weather data";
static const char T_RU_UPD_Weath[] PROGMEM = "Обновить данные о погоде";
static const char T_EN_WAPIKEY[] PROGMEM = "OpenWeather API-key";

static const char T_EN_MX_Reset[] PROGMEM = "Matrix reset";
static const char T_RU_MX_Reset[] PROGMEM = "Сбросить матрицу";


/**
 *  Dictionary with references to all text resources
 *  it is a two-dim array of pointers to flash strings.
 *  Each row is a set of messages of a given language
 *  Each colums is a language index
 *  Messages indexes of each lang must match each other
 *  it is possible to reuse untraslated mesages from other lang's
 */
static const char *const C_DICT[][UI_DICT_SIZE] PROGMEM = {
// Index 0 - Russian lang
  { T_RU_Clock,
    T_RU_MX_Reset,
    T_RU_OPT_Weath,
    T_RU_UPD_Weath,
    T_RU_Weather,
    T_EN_WAPIKEY,
    T_RU_Weather_note,
    T_RU_Weather_cid,
    T_RU_Weather_shname,
    T_RU_Matrix,
    T_RU_Sensors
  },
// Index 1 - English lang
  { T_EN_Clock,
    T_EN_MX_Reset,
    T_EN_OPT_Weath,
    T_EN_UPD_Weath,
    T_EN_Weather,
    T_EN_WAPIKEY,
    T_EN_Weather_note,
    T_EN_Weather_cid,
    T_EN_Weather_shname,
    T_EN_Matrix,
    T_EN_Sensors
  }
};