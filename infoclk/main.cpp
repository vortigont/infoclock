/*  Infoclk
 *
 *  (c) Emil Muratov 2019
 *
 */

// Main headers
#include "main.h"
#include "infoclock.h"

#if defined ESP8266
#include <ESP8266HTTPClient.h>
#elif defined ESP32 
#include <HTTPClient.h>
#endif
#include <EmbUI.h>

extern "C" int clock_gettime(clockid_t unused, struct timespec *tp);

// Global Task Scheduler
// Our instance of Infoclock
Infoclock informer;

// ----
// MAIN Setup
void setup() {
  Serial.begin(BAUD_RATE);
  LOG(println, F("Starting InfoClock..."));

  // цепляем крючки наших обработчиков на WiFi события от фреймворка
  embui.set_callback(CallBack::attach, CallBack::STAGotIP, std::bind(onSTAGotIP));
  embui.set_callback(CallBack::attach, CallBack::STADisconnected, std::bind(onSTADisconnected));

  // start framework - this loads config data and initializes WiFi
  embui.begin();

  // read matrix w,h,cs from config and initialize object
#ifdef ESP32
  informer.init(embui.paramVariant(FPSTR(V_MX_W)), embui.paramVariant(FPSTR(V_MX_H)), embui.paramVariant(V_CLKPIN), embui.paramVariant(V_DATAPIN), embui.paramVariant(V_CSPIN));
#else
  informer.init(embui.paramVariant(FPSTR(V_MX_W)), embui.paramVariant(FPSTR(V_MX_H)), embui.paramVariant(FPSTR(V_CSPIN)));
#endif

  // restore display and modules orientation from config
  informer.mxPaneSetup(
    embui.paramVariant(FPSTR(V_MX_OS)), embui.paramVariant(FPSTR(V_MX_OV)),
    embui.paramVariant(FPSTR(V_MX_VF)), embui.paramVariant(FPSTR(V_MX_HF)),
    embui.paramVariant(FPSTR(V_MX_MR))
  );

  // restore sensor update rate, if defined
  if (embui.paramVariant(FPSTR(V_SN_UPD_RATE)))
    informer.snsupdrate(embui.paramVariant(FPSTR(V_SN_UPD_RATE)));

  // restore sensor temp compensation, if defined
  if (embui.paramVariant(FPSTR(V_SN_TCOMP)))
    informer.clksensor.tempoffset(embui.paramVariant(FPSTR(V_SN_UPD_RATE)));

  // firmware info
  embui.server.on(PSTR("/fw"), HTTP_GET, [](AsyncWebServerRequest *request){
    wver(request);
  });

}


// MAIN loop
void loop() {
  embui.handle();
} // end of main loop



//////////////////
// Other functions

// WiFi connection callback
void onSTAGotIP() {
  informer.onNetIfUp();
}

// Manage network disconnection
void onSTADisconnected() {
  informer.onNetIfDown();
}

// send HTTP responce, json with controller/fw versions and status info
void wver(AsyncWebServerRequest *request) {
  char buff[HTTP_VER_BUFSIZE];
  timespec tp;
  clock_gettime(0, &tp);

  uint32_t chipId = 0;
#if defined ESP8266
		chipId = ESP.getChipId(),
#elif defined ESP32
    // emulate getChipId() like in 8266
		for(int i=0; i<17; i=i+8) {
	    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
	  }
#endif



  snprintf_P(buff, sizeof(buff), PGverjson,
    chipId,
		ESP.getFlashChipSize(),
#if defined ESP8266
		ESP.getCoreVersion().c_str(),
#elif defined ESP32
    "n/a",
#endif

#if defined ESP8266
		system_get_sdk_version(),
#elif defined ESP32
    ESP.getSdkVersion(),
#endif

#ifdef GIT_REV
    GIT_REV,
#else
    "-",
#endif
		ESP.getCpuFreqMHz(),
		ESP.getFreeHeap(),
    (uint32_t)tp.tv_sec);

  request->send(200, FPSTR(PGmimejson), buff );
}
