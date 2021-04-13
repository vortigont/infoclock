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
//extern Scheduler ts;
// Our instance of Infoclock
Infoclock informer;

// ----
// MAIN Setup
void setup() {
  Serial.begin(BAUD_RATE);	    // start hw serial for debugging
  LOG(println, F("Starting InfoClock..."));

  // цепляем крючки наших обработчиков на WiFi события от фреймворка
  embui.set_callback(CallBack::attach, CallBack::STAGotIP, std::bind(onSTAGotIP));
  embui.set_callback(CallBack::attach, CallBack::STADisconnected, std::bind(onSTADisconnected));

  // start framework - this loads config data and initializes WiFi
  embui.begin();

  // read matrix w,h from config and initialize object
  informer.init(embui.param(FPSTR(V_MX_W)).toInt(), embui.param(FPSTR(V_MX_H)).toInt());

  // restore display and modules orientation from config
  informer.mxPaneRotation(
    embui.param(FPSTR(V_MX_OS)).toInt(), embui.param(FPSTR(V_MX_OV)).toInt(),
    embui.param(FPSTR(V_MX_VF)).toInt(), embui.param(FPSTR(V_MX_HF)).toInt(),
    embui.param(FPSTR(V_MX_MR)).toInt()
  );

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



// reboot esp task
void espreboot() {
  	Task *t = new Task(0, TASK_ONCE, [](){ESP.restart();}, &ts, false);
    t->enableDelayed(UPD_RESTART_DELAY * TASK_SECOND);
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
		FW_NAME,
		TOSTRING(FW_VER),
		ESP.getCpuFreqMHz(),
		ESP.getFreeHeap(),
    (uint32_t)tp.tv_sec);

  request->send(200, FPSTR(PGmimejson), buff );
}


