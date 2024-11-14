#ifndef LUX_WEBSOCKET_H
#define LUX_WEBSOCKET_H

#include <Arduino.h>

#include <WebSocketsClient.h>          // https://github.com/Links2004/arduinoWebSockets
#include "ESP32ntp.h"
#include "XPString.h"


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


class LuxWebsocket
{
  private:
    int _i = 0;

  protected:
     int _p = 0;
     String _sLuxURL= "";
     WebSocketsClient _LuxSockClient;

  public:
    LuxWebsocket();
    LuxWebsocket(const char luxurl[]);
    ~LuxWebsocket ();

    bool init();
    void loop(time_t* time_now);
};

#endif