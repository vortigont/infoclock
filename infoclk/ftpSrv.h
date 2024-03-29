#ifdef ESP8266
 #include <ESP8266WiFi.h>
 #include <ESPAsyncTCP.h>
 #include <LittleFS.h>
#endif  // def ESP8266

#ifdef ESP32
 #include <AsyncTCP.h>
 #include <LITTLEFS.h>
 #define FORMAT_LITTLEFS_IF_FAILED true
 #define LittleFS LITTLEFS
#endif

#define FTP_DEBUG

#include <FTPServer.h>

FTPServer ftpSrv(LittleFS); // construct with LittleFS

void ftp_setup(void){
 
  /////FTP Setup, ensure LittleFS is started before ftp;  /////////
  if (LittleFS.begin()) {
    ftpSrv.begin(F("ftp"), F("ftp")); //username, password for ftp.  set ports in ESP8266FtpServer.h  (default 21, 50009 for PASV)
  }   
}

void ftp_loop(void){
  ftpSrv.handleFTP();        //make sure in loop you call handleFTP()!!  
}

//extern void ftp_setup(void);
//extern void ftp_loop(void);
//extern FTPServer ftpSrv; 