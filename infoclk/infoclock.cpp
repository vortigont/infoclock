/*
 * This file is part of Infoclock project
 * a controller for dotmatrix display used as a home informer panel
 * in some of my other automation projects
 *
 *  © 2019 Emil Muratov (vortigont)
 */

#include "config.h"     // defaults and constants
#include "infoclock.h"
#include "sensors.h"
#include "EmbUI.h"      // EmbUI framework

//#include <Fonts/FreeSans9pt7b.h>	//good plain
//#include <Fonts/FreeSansBold9pt7b.h>	// good but too bold
//#include <Fonts/FreeSerifBold9pt7b.h> // maybe (bad small i)
//#include <Fonts/TomThumb.h>		// nice very small 5x3 (no rus)
#include "mfFbsd8x16monoGFX.h"

// Time
#include <time.h>                       // time() ctime()
#include <sys/time.h>                   // struct timeval


// Parola lib
//https://github.com/MajicDesigns/MD_Parola

// Defines
#define WEATHER_INIT_DELAY 20   // seconds to delay weather update after WiFi connection 
#define WEATHER_API_BUFSIZE 1024


#define UNICODE_CP1251_OFFSET_D0 0x2f   // symbol offset is 0x30, but font index adds -1
#define UNICODE_CP1251_OFFSET_D1 0x6f   // symbol offset is 0x70, but font index adds -1


// weather API URL
static const char PGwapireq1[] PROGMEM = WAPI_URL "?id=";
static const char PGwapireq2[] PROGMEM = "&units=metric&lang=" COUNTRY "&APPID=";   // WAPI_KEY;


// Informer class constructor
Infoclock::Infoclock() : w(DEFAULT_DIM_X), h(DEFAULT_DIM_Y) {

}

/** @brief init - initialize Informer object, create dynamic valuies and objects
 * 
 *  @param _x - display panel dimensions, absolute width
 *  @param _y - display panel dimensions, absolute height
 */
void Infoclock::init(const int16_t _x, const int16_t _y){
  setDimensions(_x, _y);

  tape=String(F("Connecting to WiFi..."));
  // create display object
  matrix = std::unique_ptr<Max72xxPanel>(new Max72xxPanel(PIN_CS, w, h));

  //matrix->setFont(&TomThumb);
  //matrix->setFont(&FreeMono9pt7b);
  matrix->setTextWrap(false);

  tWeatherUpd.set(WEATHER_UPD_PERIOD * TASK_HOUR, TASK_FOREVER, std::bind(&Infoclock::GetWeather, this));
  tSensorUpd.set(SENSOR_UPD_PERIOD * TASK_SECOND, TASK_FOREVER, std::bind(&Infoclock::updsensstr, this));
  tDrawTicks.set(TICKS_TIME, TASK_ONCE, NULL, std::bind(&Infoclock::drawticks, this), std::bind(&Infoclock::clearticks, this));

  tScroller.set(SCROLL_RATE, TASK_FOREVER, std::bind(&Infoclock::panescroller, this));
  tScroller.setSchedulingOption(TASK_SCHEDULE_NC);

  tSecondsPulse.set(TASK_SECOND, TASK_FOREVER, std::bind(&Infoclock::doSeconds, this));
  tSecondsPulse.setSchedulingOption(TASK_INTERVAL);

  ts.addTask(tWeatherUpd);
  ts.addTask(tScroller);
  ts.addTask(tSecondsPulse);
  ts.addTask(tDrawTicks);

  if (clksensor.begin() == sensor_t::NA) {
    updsensstr();
  } else {
    ts.addTask(tSensorUpd);
    tSensorUpd.enableDelayed();
    clksensor.getSensorModel(sensorstr);
  }

  tScroller.enableDelayed();
  tSecondsPulse.enableDelayed();
}


/**
 *  Set Pane rotation according to settings
 */
void Infoclock::mxPaneSetup(const bool serp,  const bool vert, const bool vflip, const bool hflip, const int mr){

  LOG(printf_P, PSTR("Change PaneRotation params: serpentine=%d, vertical=%d, v-flip=%d, h-flip=%d, rot=%d\n"), serp, vert, vflip, hflip, mr);

  if(!mr){
      LOG(println, F("negative rotation values are forbidden"));
      return;
  }

  uint8_t x,y;
  unsigned int _mr;
  bool _vflip, _hflip;

  for ( uint8_t i = 0;  i != w*h;  ++i ) {
    _mr = mr;
    _vflip = vflip;
    _hflip = hflip;

    if ( vert ){          // verticaly ordered modules
      x = hflip ? w-i/h-1 : i/h;
      if (serp && x%2){   // for snake-shaped displays
        _vflip = !vflip;  // invert vertical flip for odd rows
        _mr = (mr+2)%4;   // and rotate each module 180 degrees
      }
      y = _vflip ? h-i%h-1 : i%h;
    } else {              // verticaly ordered modules
      y = vflip ? h-i/w-1 : i/w;
      if (serp && y%2){   // for snake-shaped displays
        _hflip = !hflip;  // invert h-flip for odd rows
        _mr = (mr+2)%4;   // and rotate each module 180 degrees
      }
      x = _hflip ? w-i%w-1 : i%w;
    }

    matrix->setPosition(i, x, y);
    LOG(printf, "Positioning: %d - %d %d\n", i, x, y);

    matrix->setRotation(i, _mr);
    LOG(printf, "Rotating: %d %d\n", i, _mr);
  }

  matrix->fillScreen(LOW);			// clear screen
}

template <typename T> void Infoclock::mtxprint( const T& str, uint16_t x, uint16_t y) {
    matrix->setCursor(x, y);
    matrix->print(str);
    matrix->write();
}

// template for display text scroller
template <typename T> void Infoclock::scroll( const T& str, int y, int& scrollptr) {

	int16_t  x1, y1;
	uint16_t w, h;
	matrix->setFont();

	matrix->getTextBounds(str, 0, y, &x1, &y1, &w, &h);
	//_SP("Text: "); _SP(w); _SP("x"); _SPLN(h);
	//_SP("Matrix: "); _SP(matrix->width()); _SP("x"); _SPLN(matrix->height());

	if ( scrollptr < -1*w )		// reset scroll pointer when text moves out of pane
		scrollptr = matrix->width();

	matrix->fillRect(0, y, matrix->width(), y+8, 0);	// blank 1 row of 8x8 modules
	matrix->setCursor(scrollptr--,y);
	matrix->print(str);
	matrix->write();
}

// callback function for every second pulse task (tSecondsPulse)
void Infoclock::doSeconds() {
	tDrawTicks.restartDelayed();   //run task that draws one pulse of a ticks

  clksensor.sgp30poll();

  if ( TimeProcessor::getInstance().getMinutes() == lastmin )
    return;

	// update clock display every new minute
  lastmin = TimeProcessor::getInstance().getMinutes();
  uint8_t _brt = brightness_calc();
  matrix->reset();             // reset matrix to clear possible garbage
  matrix->setIntensity(_brt);	 // set screen brightness
  wscroll = (bool)_brt;		     // disable weather scroll at nights
  matrix->fillScreen(LOW);		 // clear screen all screen (must be replaced to a clock region only)
  bigClk();                    // print time on screen
  LOG(print, ctime(TimeProcessor::getInstance().now()));     // print date/time to serial if debug


/*
    if (wscroll){
      tScroller.enableIfNot();
    } else {
      tScroller.disable();
     	matrix->setFont();
      mtxprint(sensorstr, 0, STR_SENSOR_OFFSET_Y);
    }
*/
}

// print big font clock
void Infoclock::bigClk () {
    matrix->setFont(&mfFbsd8x16mono);
    matrix->fillRect(0, 0, matrix->width(), CLK_FONT_HEIGHT, 0);

    char buf[3];
    sprintf(buf, "%2d", embui.timeProcessor.getHours());
    mtxprint(buf, 0, CLK_FONT_OFFSET_Y);
    sprintf(buf, "%02d", embui.timeProcessor.getMinutes());
    mtxprint(buf, CLK_MINUTE_OFFSET_X, CLK_FONT_OFFSET_Y);
}


// Draw hh:mm ticks
bool Infoclock::drawticks() {
    uint16_t x = CLK_TICK_OFFSET_X;
    uint16_t y = CLK_TICK_OFFSET_Y;
    //upper tick
    matrix->drawPixel(x++, y++, 1);
    matrix->drawPixel(x--, y++, 1);
    matrix->drawPixel(x++, y++, 1);
    //lower tick
    matrix->drawPixel(x--, ++y, 1);
    matrix->drawPixel(x++, ++y, 1);
    matrix->drawPixel(x, ++y, 1);

    matrix->write();
    return true;
}

// clears display segments for ticks
void Infoclock::clearticks() {
	matrix->fillRect(CLK_TICK_OFFSET_X, 0, CLK_TICK_OFFSET_Y, CLK_TICK_HEIGHT, 0);
	matrix->write();
}

// update weather info via http req
void Infoclock::GetWeather(){

  if (!wscroll) // exit if weather string is not scrolling
    return;

  WiFiClient tcpclient;
  HTTPClient httpreq;

  String url = FPSTR(PGwapireq1);
  url += embui.param(FPSTR(V_WAPI_CITY_ID));
  url += FPSTR(PGwapireq2);
  url += embui.param(FPSTR(V_WAPI_KEY));

  LOG(print, F("Updating weather via: "));
  LOG(println, url);
  if (httpreq.begin(tcpclient, url)){
  	int httpCode = httpreq.GET();
	  if( httpCode == HTTP_CODE_OK ){
		  String respdata = httpreq.getString();
		  parseWeather(respdata);
		  tWeatherUpd.setInterval(embui.param(FPSTR(V_W_UPD_TIME)).toInt() * TASK_HOUR);
  	} else {
	  	LOG(print, F("Weather update code: "));
		  LOG(print, httpCode);
		  tWeatherUpd.setInterval(embui.param(FPSTR(V_W_UPD_RTR)).toInt() * TASK_MINUTE);
	  }
  }

  httpreq.end();
  tcpclient.stop();
}

void Infoclock::updsensstr(){
    clksensor.getFormattedValues( sensorstr );
    LOG(println, sensorstr);
}

// получить строку для дисплея из json-ответа
void Infoclock::parseWeather(String& result){
  DynamicJsonDocument root(WEATHER_API_BUFSIZE);
  DeserializationError error = deserializeJson(root, result);
  result.clear();

  if (error){
  	LOG(print, F("Json parsing failed! Error: "));
  	LOG(println, error.c_str());
    weather = F("погода недоступна: ");
    weather += error.c_str();
    utf8toCP1251(weather, tape);
    return;
  }

// Погода
   weather = embui.param(FPSTR(V_WAPI_CITY_NAME));
   weather += root[F("weather")][0][F("description")].as<String>();
   weather += ", ";
// Температура
   weather += root["main"]["temp"].as<String>();

// Влажность
   weather += ", H:";
   weather += root["main"]["humidity"].as<String>();
// Ветер
   weather += "% Ветер ";
   double deg = root["wind"]["deg"];
   if( deg >22.5 && deg <=67.5 ) weather += "сев-вост ";
   else if( deg >67.5 && deg <=112.5 ) weather += "вост. ";
   else if( deg >112.5 && deg <=157.5 ) weather += "юг-вост ";
   else if( deg >157.5 && deg <=202.5 ) weather += "юж. ";
   else if( deg >202.5 && deg <=247.5 ) weather += "юг-зап ";
   else if( deg >247.5 && deg <=292.5 ) weather += "зап. ";
   else if( deg >292.5 && deg <=337.5 ) weather += "сев-зап ";
   else weather += "сев,";
   weather += root["wind"]["speed"].as<String>();
   weather += " м/с";

   LOG(print, F("Weather: "));
   LOG(println, weather);

  // Перекодируем из UNICODE в кодировку дисплейного шрифта
  utf8toCP1251(weather, tape);
}


// calculate brightness for the hour
uint8_t Infoclock::brightness_calc(void){
	uint8_t hr = embui.timeProcessor.getHours();
	if (hr>=MAX_BRT_HOUR_START && hr<MAX_BRT_HOUR_END)
		return MAX_BRIGHTNESS;

	// incr brt
	if (hr>=MIN_BRT_HOUR_END && hr<MAX_BRT_HOUR_START)
		return 	map(hr, MIN_BRT_HOUR_END, MAX_BRT_HOUR_START, MIN_BRIGHTNESS, MAX_BRIGHTNESS);

	// decr brt
	if (hr >= MAX_BRT_HOUR_END )
		return map(hr, MAX_BRT_HOUR_END, MIN_BRT_HOUR_START, MAX_BRIGHTNESS, MIN_BRIGHTNESS);

	return MIN_BRIGHTNESS;
}

// scroll text on a pane
void Infoclock::panescroller(void){
	scroll(sensorstr, STR_SENSOR_OFFSET_Y, strp1);
	if ( wscroll ) scroll(tape, STR_WEATHER_OFFSET_Y, strp2);
}

/* Recode russian string from UTF-8 to CP-1251 LCD Font
   http://arduino.ru/forum/programmirovanie/rusifikatsiya-biblioteki-adafruit-gfx-i-vyvod-russkikh-bukv-na-displei-v-kodi
   https://i.voenmeh.ru/kafi5/Kam.loc/inform/UTF-8.htm
*/
bool Infoclock::utf8toCP1251(const String& source, String& dst, bool concat){
  unsigned int i =0, t=0, k = source.length();
  char *buff = (char *)malloc( (k+1) * sizeof *buff );
  if (!buff)
    return false;

  unsigned char n;

  while (i < k) {
    n = source[i++];
    switch (n) {
      case 0xD0: {
        n = source[i++];
        if (n == 0x81) { n = 0xA8; break; }    // 0xd081 U+0401  'Ё' => cp1251 0xa8
        if (n > 0x8F && n < 0xC0) n += UNICODE_CP1251_OFFSET_D0;    // +47
        break;
      }
      case 0xD1: {
        n = source[i++];
        if (n == 0x91) { n = 0xB8; break; }       // 0xd191 U+0451  'ё' => cp1251 0xb8
        if (n > 0x7f && n < 0x90) n += UNICODE_CP1251_OFFSET_D1;    // +111
        break;
      }
    }

    buff[t++] = n;
  }

  buff[t] = '\0';
  concat ? dst += buff : dst = buff;
  delete buff;
  return true;
}


// reschedule weather update in a second
void Infoclock::refreshWeather(){
  tWeatherUpd.setInterval(TASK_SECOND);
  tWeatherUpd.restartDelayed();
}

/** @brief onNetIfUp - коллбек для внешнего события "сеть доступна"
 * 
 */
void Infoclock::onNetIfUp(){
  tape = F("WiFi ip:");
  tape += WiFi.localIP().toString();
  LOG(println, tape);

  //start weather updates
  tWeatherUpd.enableDelayed(WEATHER_INIT_DELAY * TASK_SECOND);
}

/** @brief onNetIfDown - коллбек для внешнего события "сеть НЕ доступна"
 * 
 */
void Infoclock::onNetIfDown(){
    tWeatherUpd.disable();
    tape=F("No Internet connection");
};

// Set display brightness, returns resulting brightness
void Infoclock::brightness(const uint8_t b){
  return matrix->setIntensity(b > MAX_BRIGHTNESS ? MAX_BRIGHTNESS : b);
}
