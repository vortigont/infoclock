/*  Infoclk
 *
 *  (c) Emil Muratov 2019
 *
 */

// Main headers
#include "main.h"

// Time
#include <time.h>                       // time() ctime()
#include <sys/time.h>                   // struct timeval
//-#include <sntp.h>
extern "C" int clock_gettime(clockid_t unused, struct timespec *tp);

#include "sensors.h"

#include <ESP8266HTTPClient.h>


#include <EmbUI.h>

//#include <Fonts/FreeSans9pt7b.h>	//good plain
//#include <Fonts/FreeSansBold9pt7b.h>	// good but too bold
//#include <Fonts/FreeSerifBold9pt7b.h> // maybe (bad small i)
//#include <Fonts/TomThumb.h>		// nice very small 5x3 (no rus)

#include "mfFbsd8x16monoGFX.h"

// ----
// Constructs
std::unique_ptr<Max72xxPanel> matrix = nullptr;   // указатель для обеъкта матрицы, будет инициализирован позже

// Parola lib
//https://github.com/MajicDesigns/MD_Parola


//Task Scheduler
Scheduler ts;

// Tasks
Task tWeatherUpd(WAPI_DEFAULT_UPDATE_TIME * TASK_HOUR, TASK_FOREVER, &GetWeather, &ts, false);
Task tSensorUpd(SENSOR_UPD_PERIOD * TASK_SECOND, TASK_FOREVER, &updsensstr, &ts, false);
Task tScroller(SCROLL_RATE, TASK_FOREVER, &panescroller, &ts, false);
Task tSecondsPulse(TASK_SECOND, TASK_FOREVER, &doSeconds, &ts, false);
// Draw pulsing ticks
Task tDrawTicks(TICKS_TIME, TASK_ONCE, NULL, &ts, false, &drawticks, &clearticks);

static time_t now;
uint8_t lastmin = 0;
bool wscroll = 1;	// do weather scroll
uint8_t w,h;    // matrix width, height

// scroll y pointers
int strp1, strp2 = 0;

// Sensors
char sensorstr[SENSOR_DATA_BUFSIZE];      // sensor data
Sensors clksensor;    // sensor object

// String for weather info
String tape = "Connecting to WiFi...";

// ----
// MAIN Setup
void setup() {
  Serial.begin(BAUD_RATE);	    // start hw serial for debugging
  LOG(println, F("Starting InfoClock..."));

  embui.set_callback(CallBack::attach, CallBack::STAGotIP, std::bind(onSTAGotIP));
  embui.set_callback(CallBack::attach, CallBack::STADisconnected, std::bind(onSTADisconnected));

  embui.begin();

  // read matrix w,h from config
  w = embui.param(FPSTR(V_MX_W)).toInt();
  h = embui.param(FPSTR(V_MX_H)).toInt();

  // create display object
  matrix = std::unique_ptr<Max72xxPanel>(new Max72xxPanel(PIN_CS, w, h));

  // Set matrix rotations
  //mxRotation(embui.param(FPSTR(V_MX_MR)).toInt());

    mxPaneRotation(embui.param(FPSTR(V_MX_OS)) == FPSTR(P_true),
        embui.param(FPSTR(V_MX_OV)) == FPSTR(P_true),
        embui.param(FPSTR(V_MX_VF)) == FPSTR(P_true),
        embui.param(FPSTR(V_MX_HF)) == FPSTR(P_true),
        embui.param(FPSTR(V_MX_MR)).toInt()
    );

  // rotate pane 90 cw
  //mxPaneRotation(embui.param(FPSTR(V_MX_CR)).toInt());

/*
  // Make pane Serpent with normal orientation
  #ifdef MATRIX_ZIGZAG
  // modules position (n,x,y) x,y(0,0) is from the top left corner of the pane
  matrix->setPosition(0, 3, 0); matrix->setRotation(0, Rotation::CCW90);
  matrix->setPosition(1, 2, 0); matrix->setRotation(1, Rotation::CCW90);
  matrix->setPosition(2, 1, 0); matrix->setRotation(2, Rotation::CCW90);
  matrix->setPosition(3, 0, 0); matrix->setRotation(3, Rotation::CCW90);
  matrix->setPosition(4, 0, 1);
  matrix->setPosition(5, 1, 1);
  matrix->setPosition(6, 2, 1);
  matrix->setPosition(7, 3, 1);
  matrix->setPosition(8, 3, 2); matrix->setRotation(8, Rotation::CCW90);
  matrix->setPosition(9, 2, 2); matrix->setRotation(9, Rotation::CCW90);
  matrix->setPosition(10, 1, 2); matrix->setRotation(10, Rotation::CCW90);
  matrix->setPosition(11, 0, 2); matrix->setRotation(11, Rotation::CCW90);
  matrix->setPosition(12, 0, 3);
  matrix->setPosition(13, 1, 3);
  matrix->setPosition(14, 2, 3);
  matrix->setPosition(15, 3, 3); // The last display is at <3, 3>
  #endif
*/

    //matrix->setFont(&TomThumb);
    //matrix->setFont(&FreeMono9pt7b);
    matrix->setTextWrap(false);
    //matrix->fillScreen(LOW);

    ts.startNow();    // start scheduler

    if (clksensor.begin() == sensor_t::NA) {
        ts.deleteTask(tSensorUpd);
        updsensstr();
    } else {
    	tSensorUpd.enableDelayed();
      clksensor.getSensorModel(sensorstr);
    }

    tScroller.enableDelayed();
    tSecondsPulse.enableDelayed();
}


// MAIN loop
void loop() {
  embui.handle();
	ts.execute();		// run task scheduler
} // end of main loop



template <typename T> void mtxprint( const T& str, uint16_t x, uint16_t y) {
/*
    int16_t  x1, y1;
    uint16_t w, h;

    matrix->getTextBounds(str, x, y, &x1, &y1, &w, &h);

    //_SP("Text: "); _SP(w); _SP("x"); _SPLN(h);
    //_SP("Matrix: "); _SP(matrix->width()); _SP("x"); _SPLN(matrix->height());

    matrix->fillRect(x1, y1, w, h, 0);
*/
    matrix->setCursor(x, y);
    matrix->print(str);
    matrix->write();
}

// template for display text scroller
template <typename T> void scroll( const T& str, int y, int& scrollptr) {

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
void doSeconds() {
	// update clock display every new minute

	tDrawTicks.restartDelayed();   //run task that draws one pulse of a ticks

  time(&now);
  if ( localtime(&now)->tm_min == lastmin )
    return;

    lastmin = localtime(&now)->tm_min;
    uint8_t _brt = brightness_calc();
    matrix->reset();             // reset matrix to clear possible garbage
    matrix->setIntensity(_brt);	// set screen brightness
    wscroll = (bool)_brt;		    // disable weather scroll at nights
    matrix->fillScreen(LOW);			// clear screen all screen (must be replaced to a clock region only)
    bigClk();                   //simpleclk();   print time on screen
  	LOG(print, ctime(&now));    // print date/time to serial if debug

    if (wscroll){
      tScroller.enableIfNot();
    } else {
      tScroller.disable();
     	matrix->setFont();
      mtxprint(sensorstr, 0, STR_SENSOR_OFFSET_Y);
    }
}

// print big font clock
void bigClk () {
    matrix->setFont(&mfFbsd8x16mono);
    matrix->fillRect(0, 0, matrix->width(), CLK_FONT_HEIGHT, 0);

    char buf[3];
    sprintf(buf, "%2d", embui.timeProcessor.getHours());
    mtxprint(buf, 0, CLK_FONT_OFFSET_Y);
    sprintf(buf, "%02d", embui.timeProcessor.getMinutes());
    mtxprint(buf, CLK_MINUTE_OFFSET_X, CLK_FONT_OFFSET_Y);
}


// onEnable callback func, must retrun true
// Draw hh:mm ticks
bool drawticks() {
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

// onDisable callback func
// clears display segments for ticks
void clearticks() {
	matrix->fillRect(CLK_TICK_OFFSET_X, 0, CLK_TICK_OFFSET_Y, CLK_TICK_HEIGHT, 0);
	matrix->write();
}

// update weather info via http req
void GetWeather(){

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
		  ParseWeather(respdata);
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

void updsensstr(){
    clksensor.getFormattedValues( sensorstr );
    LOG(println, sensorstr);
}

// получить строку для дисплея из json-ответа
void ParseWeather(String result){
  DynamicJsonDocument root(WEATHER_API_BUFSIZE);
  DeserializationError error = deserializeJson(root, result);
  result = "";

  if (error){
  	LOG(print, F("Json parsing failed! Error: "));
  	LOG(println, error.c_str());
	  tape = F("погода недоступна: ");
    tape += error.c_str();
    tape = utf8rus(tape);
    return;
  }

// Погода
   tape = embui.param(FPSTR(V_WAPI_CITY_NAME));
   tape += root[F("weather")][0][F("description")].as<String>();
   tape += ", ";
// Температура
   int t = root["main"]["temp"].as<int>();
   tape += String(t);
// Влажность
   tape += ", H:";
   tape += root["main"]["humidity"].as<String>();
// Ветер
   tape += "% Ветер ";
   double deg = root["wind"]["deg"];
   if( deg >22.5 && deg <=67.5 )tape += "сев-вост ";
   else if( deg >67.5 && deg <=112.5 )tape += "вост. ";
   else if( deg >112.5 && deg <=157.5 )tape += "юг-вост ";
   else if( deg >157.5 && deg <=202.5 )tape += "юж. ";
   else if( deg >202.5 && deg <=247.5 )tape += "юг-зап ";
   else if( deg >247.5 && deg <=292.5 )tape += "зап. ";
   else if( deg >292.5 && deg <=337.5 )tape += "сев-зап ";
   else tape += "сев,";
   tape += root["wind"]["speed"].as<String>();
   tape += " м/с";

   LOG(print, F("Weather: "));
   LOG(println, tape);

  // Перекодируем из UNICODE в кодировку дисплейного шрифта
  tape = utf8rus(tape);
}


// calculate brightness for the hour
uint8_t brightness_calc(void){
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
void panescroller(void){
	scroll(sensorstr, STR_SENSOR_OFFSET_Y, strp1);
	if ( wscroll ) scroll(tape, STR_WEATHER_OFFSET_Y, strp2);
}


//////////////////
// Other functions

// WiFi connection callback
//void onSTAGotIP(WiFiEventStationModeGotIP ipInfo) {
void onSTAGotIP() {
  tape = "WiFi ip:";
  tape += WiFi.localIP().toString();
  LOG(println, tape);

  //start weather updates
  tWeatherUpd.enableDelayed(WEATHER_INIT_DELAY * TASK_SECOND);
}

// Manage network disconnection
//void onSTADisconnected(WiFiEventStationModeDisconnected event_info) {
void onSTADisconnected() {
    // disabe weather update
	  tWeatherUpd.disable();
}


/* Recode russian fonts from UTF-8 to Windows-1251 */
// http://arduino.ru/forum/programmirovanie/rusifikatsiya-biblioteki-adafruit-gfx-i-vyvod-russkikh-bukv-na-displei-v-kodi
String utf8toCP1251(String source)
{
  int i,k;
  String target;
  unsigned char n;
  char m[2] = { '0', '\0' };

  k = source.length(); i = 0;

  while (i < k) {
    n = source[i]; i++;

    if (n >= 0xC0) {
      switch (n) {
        case 0xD0: {
          n = source[i]; i++;
          if (n == 0x81) { n = 0xA8; break; }
          if (n >= 0x90 && n <= 0xBF) n = n + 0x30;
	    //if (n >= 0x90 && n <= 0xBF) n = n + 0x2f;
          break;
        }
        case 0xD1: {
          n = source[i]; i++;
          if (n == 0x91) { n = 0xB8; break; }
          if (n >= 0x80 && n <= 0x8F) n = n + 0x70;
          //if (n >= 0x80 && n <= 0x8F) n = n + 0x6f;
          break;
        }
      }
    }
    m[0] = n; target = target + String(m);
  }
return target;
}


/* Recode russian fonts from UTF-8 to HZ LCD Font */
// http://arduino.ru/forum/programmirovanie/rusifikatsiya-biblioteki-adafruit-gfx-i-vyvod-russkikh-bukv-na-displei-v-kodi
String utf8rus(String source)
{
  int i,k;
  String target;
  unsigned char n;
  char m[2] = { '0', '\0' };

  k = source.length(); i = 0;

  while (i < k) {
    n = source[i]; i++;

    if (n >= 0xC0) {
      switch (n) {
        case 0xD0: {
          n = source[i]; i++;
	    if (n == 0x81) { n = 0xA8; break; }
	    if (n >= 0x90 && n <= 0xBF) n = n + 0x2f;
          break;
        }
        case 0xD1: {
          n = source[i]; i++;
          if (n == 0x91) { n = 0xB8; break; }
          if (n >= 0x80 && n <= 0x8F) n = n + 0x6f;
          break;
        }
      }
    }
    m[0] = n; target = target + String(m);
  }
return target;
}



// reboot esp task
void espreboot() {
  	Task *t = new Task(0, TASK_ONCE, [](){ESP.restart();}, &ts, false);
    t->enableDelayed(UPD_RESTART_DELAY * TASK_SECOND);
}


// send HTTP responce, json with controller/fw versions and status info
void wver(AsyncWebServerRequest *request) {
  char buff[HTTP_VER_BUFSIZE];
  //char* firmware = (char*) malloc(strlen_P(PGver)+1);
  //strcpy_P(firmware, PGver);
  timespec tp;
  clock_gettime(0, &tp);

  snprintf_P(buff, sizeof(buff), PGverjson,
		ESP.getChipId(),
		ESP.getFlashChipSize(),
		ESP.getCoreVersion().c_str(),
		system_get_sdk_version(),
		FW_NAME,
		TOSTRING(FW_VER),
		ESP.getCpuFreqMHz(),
		ESP.getFreeHeap(),
    (uint32_t)tp.tv_sec);

  request->send(200, FPSTR(PGmimejson), buff );
}

void refreshWeather(){
  tWeatherUpd.setInterval(TASK_SECOND);
  tWeatherUpd.restartDelayed();
}

// Set MAX modules rotations
void mxRotation(const int r){
  for ( uint8_t i = 0;  i != w*h;  ++i ) {
    matrix->setRotation(i, r);
  }
}

// Set Pane rotation
void mxPaneRotation(const bool serp,  const bool vert, const bool vflip, const bool hflip, const unsigned int mr){
  LOG(printf, "Pos params: %d %d %d %d %d\n", serp, vert, vflip, hflip, mr);

  uint8_t x,y;
  unsigned int _mr;
  bool _vflip, _hflip;

  for ( uint8_t i = 0;  i != w*h;  ++i ) {
    _mr = mr;
    _vflip = vflip;
    _hflip = hflip;

    if ( vert ){
      x = hflip ? w-i/h-1 : i/h;
      if (serp && x%2){
        _vflip = !vflip;
        _mr = (mr+2)%4;
      }
      //bool _vflip = (serp && x%2) ? !vflip : vflip; // V-flip every odd row
      y = _vflip ? h-i%h-1 : i%h;
    } else {
      y = vflip ? h-i/w-1 : i/w;
      if (serp && y%2){
        _hflip = !hflip;
        _mr = (mr+2)%4;
      }
      //bool _hflip = (serp && y%2) ? !hflip : hflip; // H-flip every odd col
      x = _hflip ? w-i%w-1 : i%w;
    }

    matrix->setPosition(i, x, y);
    LOG(printf, "Positioning: %d - %d %d\n", i, x, y);

    matrix->setRotation(i, _mr);
    LOG(printf, "Rotating: %d %d\n", i, _mr);
  }

  matrix->fillScreen(LOW);			// clear screen all screen (must be replaced to a clock region only)
  //bigClk();                     //simpleclk();   print time on screen
/*
  for ( uint8_t i = 0;  i != w*h;  ++i ) {
//    LOG(printf, "Rotating: %d %d\n", i, r);
    switch (r) {
      default:
      case 0:   // normal orientation
        matrix->setPosition(i, i%w, i/w);
        break;
      case 1:   // 90 CW
        matrix->setPosition(i, i/w, (w*h-i-1)%w);
        LOG(printf, "Canvas: %d %d %d\n", i, i/w, (w*h-i-1)%w);
        break;
      case 2:   // 180 CW
        matrix->setPosition(i, w-i%w-1, h-i/h-1);
        break;
      case 3:   // 90 CCW
        matrix->setPosition(i, w-i%w-1, i%w);
        break;
    }
  }
*/
}
