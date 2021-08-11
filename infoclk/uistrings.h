#pragma once

// Set of flash-strings that might be reused multiple times within the code

// General
static const char T_HEADLINE[] PROGMEM = "ИнфоЧасики";    // имя проекта

//////////////////////
// Our variables names  - V_ prefix for 'Variable'

// Weather
static const char V_WAPI_KEY[] = "w_ak";                // API key for OpenWeather
static const char V_WAPI_CITY_ID[] = "w_cid";           // Новороссийск - 518255, Санкт-Петербург - "519690"
static const char V_WAPI_CITY_NAME[] = "w_csn";	        // Короткое имя города для дисплея
static const char V_W_UPD_TIME[] = "w_up_h";	        // weather update, hours
static const char V_W_UPD_RTR[] = "w_up_r";	            // weather update, minutes

// Matrix control
static const char V_MX_W[]  = "mx_w";	                // Matrix WIDTH (number of 8x8 MAX modules)
static const char V_MX_H[]  = "mx_h";	                // Matrix HEIGHT (number of 8x8 MAX modules)
static const char V_MX_VF[] = "mx_vf";	                // Canvas V-flip
static const char V_MX_HF[] = "mx_hf";	                // Canvas H-flip
static const char V_MX_OS[] = "mx_os";	                // Modules order - Serpentine/Zig-Zag
static const char V_MX_OV[] = "mx_ov";	                // Modules order - Vertical/Horizontal
static const char V_MX_MR[] = "mx_mr";	                // Module rotation (90 degree turns)
//static const char V_MX_RST[] = "mx_rst";	            //

// Sensors
static const char V_SN_UPD_RATE[]  = "sn_updr";	            // Sensors update rate, sec
static const char V_SN_TCOMP[]  = "sn_tcmp";	            // Temperature Sensor compensation, float
static const char V_SN_HCOMP[]  = "sn_hcmp";	            // Humidity Sensor compensation, float


// UI blocks    - B_ prefix for 'web Block'
static const char B_CLOCK[] PROGMEM = "b_clk";
static const char B_WEATHER[] PROGMEM = "b_wthr";
static const char B_MATRIX[] PROGMEM = "b_mtx";
static const char B_SENSORS[] PROGMEM = "b_sns";


// UI handlers - A_ prefix for 'Action'
static const char A_UPD_WEATHER[] PROGMEM = "a_updw";       //  weather data refresh
static const char A_SET_WEATHER[] PROGMEM = "a_setwth";     //  weather settings set 
static const char A_SET_MATRIX[] PROGMEM = "a_setmx";       //  Matrix settings set 


// other constants
static const char T_GRAY[] PROGMEM = "gray";
