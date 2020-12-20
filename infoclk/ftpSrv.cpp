#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include <FTPServer.h>
#include <ftpSrv.h>

FTPServer ftpSrv(LittleFS); // construct with LittleFS

void ftp_setup(void){
 
  /////FTP Setup, ensure LittleFS is started before ftp;  /////////
  if (LittleFS.begin()) {
      ftpSrv.begin(F("esp8266"),F("esp8266"));    //username, password for ftp.  set ports in ESP8266FtpServer.h  (default 21, 50009 for PASV)
  }    
}

void ftp_loop(void){
  ftpSrv.handleFTP();        //make sure in loop you call handleFTP()!!  
}