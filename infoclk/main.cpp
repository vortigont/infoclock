/*  Infoclk
 *
 *  (c) Emil Muratov 2019
 *
 */

// Main headers
#include "main.h"
#include "infoclock.h"

#include <ESP8266HTTPClient.h>
#include <EmbUI.h>

#ifdef USE_FTP
 #include "ftpSrv.h"
#endif

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
  informer.init(embui.paramVariant(FPSTR(V_MX_W)), embui.paramVariant(FPSTR(V_MX_H)), embui.paramVariant(FPSTR(V_CSPIN)));

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


#ifdef USE_FTP
    ftp_setup(); // запуск ftp-сервера
#endif
}


// MAIN loop
void loop() {
  embui.handle();

#ifdef USE_FTP
    ftp_loop(); // цикл обработки событий фтп-сервера
#endif
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

  snprintf_P(buff, sizeof(buff), PGverjson,
		ESP.getChipId(),
		ESP.getFlashChipSize(),
		ESP.getCoreVersion().c_str(),
		system_get_sdk_version(),
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
