/*  Infoclk
 *
 *  (c) Emil Muratov 2019
 *
 */

// Main headers
#include "Globals.h"
#include "main.h"
#include "EEPROMCfg.h"
#include "sensors.h"

//#include <Fonts/FreeSans9pt7b.h>	//good plain
//#include <Fonts/FreeSansBold9pt7b.h>	// good but too bold
//#include <Fonts/FreeSerifBold9pt7b.h> // maybe (bad small i)
//#include <Fonts/TomThumb.h>		// nice very small 5x3 (no rus)

#include "mfFbsd8x16monoGFX.h"

// ----
// Constructs
Max72xxPanel matrix = Max72xxPanel(PIN_CS, MATRIX_W, MATRIX_H);

// Sensors
char sensorstr[SENSOR_DATA_BUFSIZE];      // sensor data
Sensors clksensor;    // sensor object

// Create an instance of the web-server
//ESP8266WebServer httpsrv(80);
AsyncWebServer httpsrv(80);

//Task Scheduler
Scheduler ts;

// Tasks
Task tWeatherUpd(WEATHER_UPD_PERIOD * TASK_HOUR, TASK_FOREVER, &GetWeather, &ts, false);
Task tSensorUpd(SENSOR_UPD_PERIOD * TASK_SECOND, TASK_FOREVER, &updsensstr, &ts, false);
Task tScroller(SCROLL_RATE, TASK_FOREVER, &panescroller, &ts, false);
Task tSecondsPulse(TASK_SECOND, TASK_FOREVER, &doSeconds, &ts, false);
// Draw pulsing ticks every second
Task tDrawTicks(TICKS_TIME, TASK_ONCE, NULL, &ts, false, &drawticks, &clearticks);

static time_t now;
uint8_t lastmin = 0;
bool wscroll = 0;	// do weather scroll

// scroll y pointers
int strp1, strp2 = 0;

// String for weather info
String tape = "Connecting to WiFi...";

// ----
// MAIN Setup
void setup() {
    _SPTO(Serial.begin(BAUD_RATE));	    // start hw serial for debugging
    _SPLN("Starting InfoClock...");

  EEPROMCfg::Load();	// Load config from EEPROM

  // set ntp opts
  configTime(TZONE, NTP_SERVER);

  wifibegin(EEPROMCfg::getConfig());    // Enable WiFi

  //Define server "pages"
  httpsrv.onNotFound( [](AsyncWebServerRequest *request){request->send_P(200, FPSTR(PGmimehtml), PGindex);});  //return index for non-ex pages
  //httpsrv.on("/ota",		wota);		// OTA firmware update
  httpsrv.on("/ver",		wver);		// version and status info
  httpsrv.on("/cfg", HTTP_GET,  wcfgget);	// get config (json)
  httpsrv.on("/cfg", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL, wcfgset);	// set config (json)
  httpsrv.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){request->send_P(200, FPSTR(PGmimehtml), PGotaform);});	// Simple Firmware Update Form
  httpsrv.on("/update", HTTP_POST, wotareq, wotaupl);	// OTA firmware update
  httpsrv.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){request->send_P(200, FPSTR(PGmimetxt), "Reboot in UPD_RESTART_DELAY"); espreboot();});
  httpsrv.on("/f1",		wf1);		// fix1
  httpsrv.on("/f2",		wf2);		// fix2

   // Set matrix rotations
   for ( uint8_t i = 0;  i < MATRIX_W*MATRIX_H;  i++ ) {
        matrix.setRotation(i, MATRIX_ROTATION);
   }

  // rotate pane 90 cw
  #ifdef MATRIX_PANEROT
  // modules position x,y from the top left corner
  matrix.setPosition(0, 0, 3); // The first display is at <4, 0>
  matrix.setPosition(1, 0, 2);
  matrix.setPosition(2, 0, 1);
  matrix.setPosition(3, 0, 0);
  matrix.setPosition(4, 1, 3);
  matrix.setPosition(5, 1, 2);
  matrix.setPosition(6, 1, 1);
  matrix.setPosition(7, 1, 0);
  matrix.setPosition(8, 2, 3);
  matrix.setPosition(9, 2, 2);
  matrix.setPosition(10, 2, 1);
  matrix.setPosition(11, 2, 0);
  matrix.setPosition(12, 3, 3);
  matrix.setPosition(13, 3, 2);
  matrix.setPosition(14, 3, 1);
  matrix.setPosition(15, 3, 0); // The last display is at <3, 0>
  #endif // MATRIX_PANEROT


  // Make pane Zig-Zag with normal orientation
  #ifdef MATRIX_ZIGZAG
  // modules position (n,x,y) x,y(0,0) is from the top left corner of the pane
  matrix.setPosition(0, 3, 0); matrix.setRotation(0,3);
  matrix.setPosition(1, 2, 0); matrix.setRotation(1,3);
  matrix.setPosition(2, 1, 0); matrix.setRotation(2,3);
  matrix.setPosition(3, 0, 0); matrix.setRotation(3,3);
  matrix.setPosition(4, 0, 1);
  matrix.setPosition(5, 1, 1);
  matrix.setPosition(6, 2, 1);
  matrix.setPosition(7, 3, 1);
  matrix.setPosition(8, 3, 2); matrix.setRotation(8,3);
  matrix.setPosition(9, 2, 2); matrix.setRotation(9,3);
  matrix.setPosition(10, 1, 2); matrix.setRotation(10,3);
  matrix.setPosition(11, 0, 2); matrix.setRotation(11,3);
  matrix.setPosition(12, 0, 3);
  matrix.setPosition(13, 1, 3);
  matrix.setPosition(14, 2, 3);
  matrix.setPosition(15, 3, 3); // The last display is at <3, 3>
  #endif

    //matrix.setFont(&TomThumb);
    //matrix.setFont(&FreeMono9pt7b);
    matrix.setTextWrap(false);
    //matrix.fillScreen(LOW);

    if (clksensor.begin() == sensor_t::NA) {
        ts.deleteTask(tSensorUpd);
        updsensstr();
    } else {
    	tSensorUpd.enableDelayed();
    }

    // Start the Web-server
    httpsrv.begin();
    ts.startNow();
    tScroller.enable();
    tSecondsPulse.enableDelayed();
}


// MAIN loop
void loop() {

	//matrix.setFont(&FreeSans9pt7b);	// хороший прямой, тонкий. расстояние м-ду цифрами и ":" большое
	ts.execute();		// run task scheduler
// end of main loop
}




template <typename T> void mtxprint( const T& str, uint16_t x, uint16_t y) {
/*
    int16_t  x1, y1;
    uint16_t w, h;

    matrix.getTextBounds(str, x, y, &x1, &y1, &w, &h);

    //_SP("Text: "); _SP(w); _SP("x"); _SPLN(h);
    //_SP("Matrix: "); _SP(matrix.width()); _SP("x"); _SPLN(matrix.height());

    matrix.fillRect(x1, y1, w, h, 0);
*/
    matrix.setCursor(x, y);
    matrix.print(str);
    matrix.write();
}

// template for display text scroller
template <typename T> void scroll( const T& str, int y, int& scrollptr) {

	int16_t  x1, y1;
	uint16_t w, h;
	matrix.setFont();

	matrix.getTextBounds(str, 0, y, &x1, &y1, &w, &h);
	//_SP("Text: "); _SP(w); _SP("x"); _SPLN(h);
	//_SP("Matrix: "); _SP(matrix.width()); _SP("x"); _SPLN(matrix.height());

	if ( scrollptr < -1*w )		// reset scroll pointer when text moves out of pane
		scrollptr = matrix.width();

	matrix.fillRect(0, y, matrix.width(), y+8, 0);	// blank 1 row of 8x8 modules
	matrix.setCursor(scrollptr--,y);
	matrix.print(str);
	matrix.write();
}

// callback function for every second pulse task (tSecondsPulse)
void doSeconds() {
	// update clock display every new minute
  time(&now);
  if ( localtime(&now)->tm_min != lastmin ) {
    //matrix.shutdown(true);    // attempt to mitigate random garbage at specific modules
    uint8_t _brt = brightness_calc();
    matrix.setIntensity(_brt);	// set screen brightness
    wscroll = (bool)_brt;		// disable weather scroll at nights
    matrix.fillScreen(LOW);			// clear screen all screen (must be replaced to a clock region only)
    //matrix.shutdown(false);    // attempt to mitigate random garbage at specific modules
    bigClk();                  //simpleclk();   print time on screen
    lastmin = localtime(&now)->tm_min;
  	_SP(ctime(&now));              // print date/time to serial if debug

    if (wscroll){
      tScroller.enableIfNot();
    } else {
      tScroller.disable();
     	matrix.setFont();
      mtxprint(sensorstr, 0, STR_SENSOR_OFFSET_Y);
    }

  }

	tDrawTicks.restartDelayed();   //run task that draws one pulse of a ticks
}

// print big font clock
void bigClk () {
    matrix.setFont(&mfFbsd8x16mono);
    matrix.fillRect(0, 0, matrix.width(), CLK_FONT_HEIGHT, 0);

    char buf[3];
    sprintf(buf, "%2d", localtime(&now)->tm_hour);
    mtxprint(buf, 0, CLK_FONT_OFFSET_Y);
    sprintf(buf, "%02d", localtime(&now)->tm_min);
    mtxprint(buf, CLK_MINUTE_OFFSET_X, CLK_FONT_OFFSET_Y);
}


// onEnable callback func, must retrun true
// Draw hh:mm ticks
bool drawticks() {
    uint16_t x = CLK_TICK_OFFSET_X;
    uint16_t y = CLK_TICK_OFFSET_Y;
    //upper tick
    matrix.drawPixel(x++, y++, 1);
    matrix.drawPixel(x--, y++, 1);
    matrix.drawPixel(x++, y++, 1);
    //lower tick
    matrix.drawPixel(x--, ++y, 1);
    matrix.drawPixel(x++, ++y, 1);
    matrix.drawPixel(x, ++y, 1);

    matrix.write();
    return true;
}

// onDisable callback func
// clears display segments for ticks
void clearticks() {
	matrix.fillRect(CLK_TICK_OFFSET_X, 0, CLK_TICK_OFFSET_Y, CLK_TICK_HEIGHT, 0);
	matrix.write();
}

// update weather info via http req
void GetWeather(){

  if (!wscroll) // exit if weather string is not scrolling
    return;

    WiFiClient tcpclient;
    HTTPClient httpreq;
    _SP(F("Updating weather via: ")); _SPLN(FPSTR(PGwapireq));
  if (httpreq.begin(tcpclient, FPSTR(PGwapireq))){
	int httpCode = httpreq.GET();
	if( httpCode == HTTP_CODE_OK ){
		String respdata = httpreq.getString();
		ParseWeather(respdata);
		tWeatherUpd.setInterval(WEATHER_UPD_PERIOD * TASK_HOUR);
	} else {
		_SP("Weather update code: ");
		_SPLN(httpCode);
		tWeatherUpd.setInterval(WEATHER_UPD_RETRY * TASK_MINUTE);
	}
  }
    httpreq.end();
    tcpclient.stop();
}

void updsensstr(){
    clksensor.getFormattedValues( sensorstr );
    _SPLN(sensorstr);
}

// получить строку для дисплея из json-ответа
void ParseWeather(String s){
   DynamicJsonBuffer jsonBuffer;
   JsonObject& root = jsonBuffer.parseObject(s);

   if (!root.success()) {
	_SPLN("Json parsing failed!");
	tape = "погода недоступна";
      return;
   }
// Погода
   tape = WAPI_CITY_NAME;
   tape += root["weather"][0]["description"].as<String>();
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

   _SP("Weather: ");
   _SPLN(tape);

// Перекодируем из UNICODE в кодировку дисплейного шрифта
   tape = utf8rus(tape);
}


// calculate brightness for the hour
uint8_t brightness_calc(void){
	uint8_t hr = localtime(&now)->tm_hour;
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
void onSTAGotIP(WiFiEventStationModeGotIP ipInfo) {

   tape = "WiFi ip:";
   tape += WiFi.localIP().toString();
   _SPLN(tape);
  WiFi.mode(WIFI_STA);        // Shutdown internal Access Point

  sntp_init();

  //start weather updates
  tWeatherUpd.enableDelayed(WEATHER_INIT_DELAY * TASK_SECOND);
}

// Manage network disconnection
void onSTADisconnected(WiFiEventStationModeDisconnected event_info) {
    WiFi.mode(WIFI_AP_STA);   // Enable internal AP if station connection is lost
    sntp_stop();              // NTP sync can be disabled while not connected
    // disabe weather update
	  tWeatherUpd.disable();
}



// Initialize WiFi
void wifibegin(const cfg &conf) {
  _SPLN("Enabling WiFi");
  if (!conf.cWmode) {   //Monitor station events only if in Client/Auto mode
    _SPLN("Listening for WiFi station events");
    static WiFiEventHandler e1, e2;
    e1 = WiFi.onStationModeGotIP(onSTAGotIP);   // WiFi client gets IP event
    e2 = WiFi.onStationModeDisconnected(onSTADisconnected); // WiFi client disconnected event
  }
  WiFi.hostname(conf.chostname);
  WiFi.mode(conf.cWmode ? WIFI_AP : WIFI_AP_STA);
  WiFi.begin();		// use internaly stored credentials for connection
}

/*
// OTA update
void otaclient( const String& url) {
  //timer.disable(poller_id);   // disable poller
  _SPLN("Trying OTA Update...");
  WiFiClient client;
  t_httpUpdate_return ret = ESPhttpUpdate.update(client, url, OTA_VER);
  switch(ret) {
    case HTTP_UPDATE_FAILED:
        _SPLN("[update] Update failed");
        break;
    case HTTP_UPDATE_NO_UPDATES:
        _SPLN("[update] No Updates");
        break;
    case HTTP_UPDATE_OK:
        _SPLN("[update] Update OK"); // may reboot the ESP
        break;
    }
}
*/


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


// HTTP related stuff
// web-pages
// Takes config struct and sends it as json via http
void cfg2json(const cfg &conf, AsyncWebServerRequest *request) {
  char buff[sizeof(conf) + 50];      // let's keep it simple and do some sprintf magic
  sprintf_P(buff, PGcfgjson, conf.chostname, conf.cWmode, conf.cWssid,  conf.cOTAurl);
  _SP("EEPROM cfg:"); _SPLN(buff);  //Debug
  request->send(200, FPSTR(PGmimejson), buff );
}


/*  Webpage: Provide json encoded config data
 *  Get data from EEPROM and return it in via json
 */
void wcfgget(AsyncWebServerRequest *request) {
    EEPROMCfg::Load();	// Load config from EEPROM
    cfg2json(EEPROMCfg::getConfig(), request);     // send it as json
}

void wotareq(AsyncWebServerRequest *request) {
    //bool shouldReboot = !Update.hasError();
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", Update.hasError()?"FAIL":"Update OK");
    response->addHeader("Connection", "close");
    request->send(response);
};

void wotaupl(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
    if(!index){
      _SPF("Update Start: %s\n", filename.c_str());
      Update.runAsync(true);
      if(!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)){
    	_SPTO(Update.printError(Serial));
      }
      tWeatherUpd.disable();
      tSensorUpd.disable();
      tape = F("Updating firmware...");
    }

    if(!Update.hasError()){
      if(Update.write(data, len) != len){
        _SPTO(Update.printError(Serial));
      }
    }

    if(final){
      if(Update.end(true)){
	       _SPF("Update Success: %uB, rebooting ESP\n", index+len);
          tape = F("Update success, rebooting...");
         espreboot();
      } else {
        _SPTO(Update.printError(Serial));
        tape=F("Update error");
      }
    }
};



/*
// try OTA Update
void wota(AsyncWebServerRequest *request) {
    String otaurl;
    if(request->hasParam("url")) {    // update via url arg if provided
        otaurl = request->getParam("url")->value();
    } else {                      // otherwise use url from EEPROM config
        EEPROMCfg::Load();	// Load config from EEPROM
        otaurl = EEPROMCfg::getConfig().cOTAurl;
    }
    _SP("OTA update URL:"); _SPLN(otaurl);
    request->send_P(200, FPSTR(PGmimetxt), PGota );
    otaclient(otaurl);
}
*/


// reboot esp task
void espreboot() {
  	Task *t = new Task(0, TASK_ONCE, [](){ESP.restart();}, &ts, false);
    t->enableDelayed(UPD_RESTART_DELAY * TASK_SECOND);
}

/*  Webpage: Update config in EEPROM
 *  Use form-posted json object to update data in EEPROM
 */
void wcfgset(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  const size_t bufferSize = 2*JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(7) + CFG_JSON_BUF_EXT;
  StaticJsonBuffer<bufferSize> buff;
  JsonObject& jsoncfg = buff.parseObject((const char*)data);

  if (!jsoncfg.success()) {
    request->send_P(500, FPSTR(PGmimejson), PGdre);   // return http-error if json is unparsable
      return;
  }
  //jsoncfg.printTo(Serial); //Debug

  EEPROMCfg::Load();	// Load config from EEPROM

  snprintf(EEPROMCfg::setConfig().chostname, sizeof EEPROMCfg::getConfig().chostname, "%s", jsoncfg["wH"].as<const char*>());
  snprintf(EEPROMCfg::setConfig().cOTAurl, sizeof(EEPROMCfg::getConfig().cOTAurl), "%s", jsoncfg["uU"].as<const char*>());

  if (jsoncfg.containsKey("wA")) {             //We have new WiFi settings
      EEPROMCfg::setConfig().cWmode = atoi(jsoncfg["wM"].as<const char*>());
      if (EEPROMCfg::setConfig().cWmode) {                      // we have non-station config => save SSID/passwd to eeprom
          snprintf(EEPROMCfg::setConfig().cWssid, sizeof(EEPROMCfg::getConfig().cWssid), "%s", jsoncfg["wS"]);
          snprintf(EEPROMCfg::setConfig().cWpwd,  sizeof(EEPROMCfg::getConfig().cWpwd),  "%s", jsoncfg["wP"]);  // save password only for internal AP-mode, but never for client
	  WiFi.softAP(jsoncfg["wS"].as<const char*>(), jsoncfg["wP"].as<const char*>());		// save new AP params to the internal config
      } else {                                // try to connect to the AP with a new settings
            WiFi.mode(WIFI_AP_STA);           // Make sure we are in a client mode
            WiFi.begin(jsoncfg["wS"].as<const char*>(), jsoncfg["wP"].as<const char*>()); // try to connect to the AP, event scheduler will
                                                                                          // take care of disabling internal AP-mode if success
      }
  }

  EEPROMCfg::Save();    // Update EEPROM
  cfg2json(EEPROMCfg::getConfig(), request);   // return current config as serialised json
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

void wf1(AsyncWebServerRequest *request) {
  matrix.shutdown(true);    // attempt to mitigate random garbage at specific modules
  request->send(200, FPSTR(PGmimetxt), "OK" );
}

void wf2(AsyncWebServerRequest *request) {
  matrix.shutdown(false);    // attempt to mitigate random garbage at specific modules
  request->send(200, FPSTR(PGmimetxt), "OK" );
}
