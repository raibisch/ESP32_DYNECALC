#include "AsyncWebApp.h"


#ifdef DEBUG_PRINT
#define debug_begin(...) Serial.begin(__VA_ARGS__);
#define debug_print(...) Serial.print(__VA_ARGS__);
#define debug_write(...) Serial.write(__VA_ARGS__);
#define debug_println(...) Serial.println(__VA_ARGS__);
#define debug_printf(...) Serial.printf(__VA_ARGS__);
#else
#define debug_begin(...)
#define debug_print(...)
#define debug_printf(...)
#define debug_write(...)
#define debug_println(...)
#endif


/* 
include this script im Webpage 'log.html' or include it in PROG_MEM definition of HTML-Code
...todo discribe !!
*/

// **************************************************************
// Include this before server.
//
void AsyncWebAppClass::begin(AsyncWebServer *server)
{
    _server = server;
   
  /* template from OTA
  // SPIFFS (DATA) Formular-Seite

  _server->on("/ota.html",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
     request->send(SPIFFS, "/ota.html", String(), false, NULL);
  });
  */
  
  //* lösung für setHtmlVar suchen ----------------- !!!
  //Route for root / web page
  /// das geht evt schon so (Frage was ist mit den Aufrufen innerhalb von parseHtmlTemplate ?)
  // oder mit construktor den pointer auf parseHtmlTemplate übergeben und im main definieren ?
  // oder Class ableiten und "begin" überschreiben !! ..> vieleicht die eleganteste Lösung ?
  /*
  _server->on("/",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/index.html", String(), false, AsyncWebAppClass::parseHtmlTemplate);
  });
  */
 
  // setup.html GET
  _server->on("/setup.html",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/setup.html", "text/html", false);
  });

  // config.txt GET
  _server->on("/config.txt", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/config.txt", "text/html", false);
  });

  // config.txt GET
  _server->on("/reboot.html", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(404, "text/plain", "RESTART !");
    //saveHistory();
    ESP.restart();
  });

  // Route for style-sheet
  _server->on("/style.css",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/style.css", String(), false);
  });
  
   //.. some code for the navigation icons
  _server->on("/home.png",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/home.png", String(), false);
  });
  _server->on("/file-list.png",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/file-list.png", String(), false);
  });
  _server->on("/settings.png",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/settings.png", String(), false);
  });
 
  // ...a lot of code only for icons and favicons ;-))
  _server->on("/manifest.json",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/manifest.json", String(), false);
  });
  _server->on("/favicon.ico",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/favicon.ico", String(), false);
  });
  _server->on("/apple-touch-icon.png",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/apple-touch-icon.png", String(), false);
  });
  _server->on("/android-chrome-192x192.png",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/android-chrome-192x192.png", String(), false);
  });
  _server->on("/android-chrome-384x384.png",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/android-chrome-384x384.png", String(), false);
  });
}


AsyncWebAppClass AsyncWebApp;