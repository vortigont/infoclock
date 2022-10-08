// Default options

// Enable debug messages via hw serail port0
// #define _FWDEBUG_


// NTP Options
#define COUNTRY "ru"           // Country double-letter code

// BME Sensor setup
#define PRESSURE_UNITS 5        // unit: B001 = hPa, B010 = inHg, 5 = mmHg
#define METRIC_UNITS true       // measurement units

// some constants
#define MIN_BRIGHTNESS 0
#define MAX_BRIGHTNESS 10
#define MAX_BRT_HOUR_START 11
#define MAX_BRT_HOUR_END 18
#define MIN_BRT_HOUR_START 24
#define MIN_BRT_HOUR_END 6

#define TICKS_TIME 250
#define SCROLL_RATE 50

// strings position
#define STR_SENSOR_OFFSET_Y 15
#define STR_WEATHER_OFFSET_Y 25

#define CLK_FONT_HEIGHT 12
#define CLK_FONT_OFFSET_Y 13
#define CLK_MINUTE_OFFSET_X 17

#define CLK_TICK_OFFSET_X 15
#define CLK_TICK_OFFSET_Y 2
#define CLK_TICK_HEIGHT 10
