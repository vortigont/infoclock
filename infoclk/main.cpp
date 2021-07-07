/*  Infoclk
 *
 *  (c) Emil Muratov 2019
 *
 */

// Main headers
#include "Globals.h"
#include "main.h"
#include "EEPROMCfg.h"

//#include <WiFi.h>
//#include <AsyncTCP.h>
//#include <ESPAsyncWebServer.h>
// HTTP related def's
//#include "http.h"

//#include <Fonts/FreeSans9pt7b.h>	//good plain
//#include <Fonts/FreeSansBold9pt7b.h>	// good but too bold
//#include <Fonts/FreeSerifBold9pt7b.h> // maybe (bad small i)
//#include <Fonts/TomThumb.h>		// nice very small 5x3 (no rus)

#include "mfFbsd8x16monoGFX.h"

const unsigned char wifi_icon [] = {0x07, 0xfb, 0xfd, 0x1e, 0xee, 0xf6, 0x36, 0xb6 };
const unsigned char logo2 [] = {
0xff, 0xff, 0xdf, 0xfd, 0xcf, 0xf9, 0xc7, 0xf1, 0xc0, 0x01, 0xe0, 0x03, 0xe0, 0x03, 0xc2, 0x11,
0xc7, 0x39, 0xc2, 0x11, 0x80, 0x01, 0x00, 0xc1, 0x00, 0x03, 0x00, 0x03, 0x00, 0x07, 0x00, 0x0f };


// ----
// Constructs
Max72xxPanel matrix = Max72xxPanel(PIN_CS, MATRIX_W, MATRIX_H);

// Sensor
BME280I2C bme;

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

int lastmin = 0;
bool wscroll = 0;	// do weather scroll

// scroll y pointers
int strp1, strp2 = 0;

// sensor data
char sensstr[SENSOR_DATA_BUFSIZE];

// String for weather info
String tape = "Connecting to WiFi...";

// ----
// MAIN Setup
void setup() {
    #ifdef _CLKDEBUG_
	Serial.begin(115200);	    // start hw serial for debugging
    #endif

    EEPROMCfg::Load();	// Load config from EEPROM
    wifibegin(EEPROMCfg::getConfig());    // Enable WiFi

    //Define server "pages"
    httpsrv.onNotFound( [](AsyncWebServerRequest *request){request->send_P(200, FPSTR(PGmimehtml), PGindex);});  //return index for non-ex pages
    //httpsrv.on("/ota",		wota);		// OTA firmware update
    httpsrv.on("/ver",		wver);		// version and status info
    httpsrv.on("/cfg", HTTP_GET,  wcfgget);	// get config (json)
    httpsrv.on("/cfg", HTTP_POST, wcfgset);	// set config (json)
    httpsrv.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){request->send_P(200, FPSTR(PGmimehtml), PGotaform);});	// Simple Firmware Update Form
    httpsrv.on("/update", HTTP_POST, wotareq, wotaupl);	// OTA firmware update

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

   // Set matrix rotations
   for ( uint8_t i = 0;  i < MATRIX_W*MATRIX_H;  i++ ) {
        matrix.setRotation(i, MATRIX_ROTATION);
   }

    //matrix.setFont(&TomThumb);
    //matrix.setFont(&FreeMono9pt7b);
    matrix.setTextWrap(false);
    //matrix.setIntensity(7);
    //matrix.fillScreen(LOW);
    matrix.drawBitmap(0, 0, wifi_icon, 16, 16, 0, 1);
    //writestep(); matrix.fillScreen(LOW);

    Wire.begin();
    if (!bme.begin()) {
	ts.deleteTask(tSensorUpd);
	snprintf(sensstr, sizeof sensstr, "BME280 temp/huminidy sensor not found!");
	_SPLN(sensstr);
    } else {
    	tSensorUpd.enable();
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

// callback function for every second pulse task (tSecondsPulse)
void doSeconds() {
	// update clock display every new minute
        if ( minute() != lastmin ) {
            matrix.setIntensity(brightness_calc());	// set screen brightness
            wscroll = brightness_calc();		// disable weather scroll at nights
            matrix.fillScreen(LOW);			// clear screen all screen (must be replaced to a clock region only)
            bigClk(); //simpleclk();
            lastmin = minute();
        }

	tDrawTicks.restartDelayed();	//run task that draws one pulse of a ticks
	_SP(NTP.getDateStr()); _SP(" "); _SPLN(NTP.getTimeStr());	// print date/time to serial if debug
}


//print normal clock
void simpleclk() {
    matrix.setFont();
    matrix.fillRect(0, 16, matrix.width(), 8, 0);
    matrix.setCursor(0, 16);
    matrix.print(NTP.getTimeStr());
    matrix.write();
}


// print big font clock
void bigClk () {
    matrix.setFont(&mfFbsd8x16mono);
    matrix.fillRect(0, 0, matrix.width(), CLK_FONT_HEIGHT, 0);

    char buf[3];
    sprintf(buf, "%2d", hour());
    mtxprint(buf, 0, CLK_FONT_OFFSET_Y);
    sprintf(buf, "%02d", minute());
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

// Update string with sensor's data
void updsensstr() {
	float temp, pressure, humidity, dew = NAN;
	getsensordata(temp, humidity, pressure, dew);
	snprintf(sensstr, sizeof sensstr, "T:%.1f H:%.f%% P:%.fmmHg", temp, humidity, pressure);
	_SPLN(sensstr);		//debug, print data to serial
}

void getsensordata(float& t, float& h, float& p, float& dew) {

   BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
   BME280::PresUnit presUnit(BME280::PresUnit_torr);

    bme.read(p, t, h, tempUnit, presUnit);

    //EnvironmentCalculations::TempUnit     envTempUnit =  EnvironmentCalculations::TempUnit_Celsius;
    //dew = EnvironmentCalculations::DewPoint(t, h, envTempUnit);
}

// update weather info via http req
void GetWeather(){
    WiFiClient tcpclient;
    HTTPClient httpreq;
    String url = WAPI_REQURL;
    _SPLN(url);
    if (httpreq.begin(tcpclient, url)){
	int httpCode = httpreq.GET();
	if( httpCode == HTTP_CODE_OK ){
		String respdata = httpreq.getString();
		ParseWeather(respdata);
		tWeatherUpd.setInterval(WEATHER_UPD_PERIOD * TASK_HOUR);
	} else {
		tWeatherUpd.setInterval(WEATHER_UPD_RETRY * TASK_MINUTE);
	}
    }
    httpreq.end();
    tcpclient.stop();
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
	uint8_t hr = hour();
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
	scroll(sensstr, STR_SENSOR_OFFSET_Y, strp1);
	if ( wscroll ) scroll(tape, STR_WEATHER_OFFSET_Y, strp2);
}


//////////////////
// Other functions

// WiFi connection callback
void onSTAGotIP(WiFiEventStationModeGotIP ipInfo) {

  _SP("WiFi connected, ip:");
  _SPLN(WiFi.localIP());
  WiFi.mode(WIFI_STA);        // Shutdown internal Access Point

  NTP.begin(NTP_SERVER, TZ, TZ_DL); // Start NTP only after IP network is connected
  NTP.setInterval(NTP_INTERVAL);

  // Start the Web-server
  //httpsrv.begin();

  //start weather updates
  tWeatherUpd.enableDelayed(5 * TASK_SECOND);
}

// Manage network disconnection
void onSTADisconnected(WiFiEventStationModeDisconnected event_info) {
    WiFi.mode(WIFI_AP_STA);	// Enable internal AP if station connection is lost
    NTP.stop();			// NTP sync can be disabled to avoid sync errors
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
    }
    if(!Update.hasError()){
      if(Update.write(data, len) != len){
        _SPTO(Update.printError(Serial));
      }
    }
    if(final){
      if(Update.end(true)){
	_SPF("Update Success: %uB, rebooting ESP\n", index+len);
	Task *t = new Task(0, TASK_ONCE, [](){ESP.restart();}, &ts, false);
	t->enableDelayed(UPD_RESTART_DELAY * TASK_SECOND);
      } else {
        _SPTO(Update.printError(Serial));
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




/*
// dumb func for timer
void espreboot() {
  _SPLN("Reboot initiated");
  ESP.restart();
}
*/

/*  Webpage: Update config in EEPROM
 *  Use form-posted json object to update data in EEPROM
 */
void wcfgset(AsyncWebServerRequest *request) {

  const size_t bufferSize = 2*JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(7) + CFG_JSON_BUF_EXT;
  StaticJsonBuffer<bufferSize> buff;
  JsonObject& jsoncfg = buff.parseObject(request->arg("plain"));

  if (!jsoncfg.success()) {
    request->send_P(500, FPSTR(PGmimejson), PGdre);   // return http-error if json is unparsable
      return;
  }
  //jsoncfg.printTo(Serial); //Debug

  //cfg conf;           // struct for config data
  //cfgload(conf);      // Load config from EEPROM
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
/*
  if (jsoncfg.containsKey("uA")) {
      otaclient(jsoncfg["uU"].as<String>());	//Initiate OTA update
  } else {                                      // there was a config update, so I need either do all the tasks to update setup
	Task *t = new Task(0, TASK_ONCE, &espreboot, &ts, false);	// or reboot it all...
	t->enableDelayed(UPD_RESTART_DELAY * TASK_SECOND);			// just give it some time to try new WiFi setup if required
  }
*/
}


// send HTTP responce, json with controller/fw versions and status info
void wver(AsyncWebServerRequest *request) {
  char buff[HTTP_VER_BUFSIZE];
  //char* firmware = (char*) malloc(strlen_P(PGver)+1);
  //strcpy_P(firmware, PGver);

  snprintf_P(buff, sizeof(buff), PGverjson,
		ESP.getChipId(),
		ESP.getFlashChipSize(),
		ESP.getCoreVersion().c_str(),
		system_get_sdk_version(),
		FPSTR(PGver),
		ESP.getCpuFreqMHz(),
		ESP.getFreeHeap(),
		NTP.getUptime() );

  request->send(200, FPSTR(PGmimejson), buff );
}

