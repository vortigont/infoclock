#pragma once

// Set of flash-strings that might be reused multiple times within the code

// General
static const char T_HEADLINE[] PROGMEM = "ИнфоЧасики";    // имя проекта

//////////////////////
// Our variables names  - V_ prefix for 'Variable'
static const char V_WAPI_KEY[] = "w_ak";                // API key for OpenWeather
static const char V_WAPI_CITY_ID[] = "w_cid";           // Новороссийск - 518255, Санкт-Петербург - "519690"
static const char V_WAPI_CITY_NAME[] = "w_csn";	        // Короткое имя города для дисплея
static const char V_W_UPD_TIME[] = "w_up_h";	        // weather update, hours
static const char V_W_UPD_RTR[] = "w_up_r";	            // weather update, minutes

// Matrix control
static const char V_MX_W[]  = "mx_w";	                // Matrix WIDTH (number of 8x8 MAX modules)
static const char V_MX_H[]  = "mx_h";	                // Matrix HIIGHT (number of 8x8 MAX modules)
static const char V_MX_VF[] = "mx_vf";	                // Canvas V-flip
static const char V_MX_HF[] = "mx_hf";	                // Canvas H-flip
static const char V_MX_OS[] = "mx_os";	                // Modules order - Serpentine
static const char V_MX_OV[] = "mx_ov";	                // Modules order - Vertical
static const char V_MX_MR[] = "mx_mr";	                // Module rotation
//static const char V_MX_RST[] = "mx_rst";	            // Module rotation


// UI blocks    - B_ prefix for 'web Block'
static const char B_CLOCK[] PROGMEM = "b_clock";
static const char B_WEATHER[] PROGMEM = "b_wthr";
static const char B_MORE[] PROGMEM = "b_more";
static const char B_MATRIX[] PROGMEM = "b_mtx";


// UI handlers - A_ prefix for 'Action'
static const char A_UPD_WEATHER[] PROGMEM = "a_updw";       //  weather data refresh
static const char A_SET_WEATHER[] PROGMEM = "a_setwth";     //  weather settings set 
static const char A_SET_MATRIX[] PROGMEM = "a_setmx";       //  Matrix settings set 


// other constants
static const char T_GRAY[] PROGMEM = "gray";
