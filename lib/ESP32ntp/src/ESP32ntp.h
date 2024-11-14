#pragma once 

#include <Arduino.h>
#include <time.h>
#include <esp_sntp.h>


/*


Die time.h Bibliothek stellt verschiedenste Funktionen zum Abfragen und Manipulieren von Zeit und Datumswerten zur Verfügung.
Abfragen
Funktion 	Bedeutung
clock() 	Ticks seit dem Start des Programmes erhalten.
difftime() 	Den Unterschied zwischen zwei Zeiten erhalten
time() 	Die aktuelle Zeit erhalten
Manipulieren
Funktion 	Bedeutung
mktime() 	Konvertiert ein tm Struktur in einen Wert vom Typ time_t
asctime() 	Aus einer tm Struktur eine String generieren
ctime() 	Aus einem time_t Wert einen String generieren
gmtime() 	Konvertiert einen time_t Wert in eine tm Struktur als UTC Zeit.
localtime() 	Konvertiert einen time_t Wert in eine tm Struktur als lokale Zeit.
strftime() 	Formatiert einen tm Struktur in einen String.
Datentypen
Funktion 	Bedeutung
time_t 	Zeitstempel in Sekunden ab 1.1.1970, 0:00 Uhr
struct tm 	Zeitpunktsbeschreibung
clock_t 	Integer-Typ, hier als Zählvariable für System-Ticks 


Beispiel asctime():


#include <stdio.h>
#include <time.h>
#include <stdlib.h>
 
int main ()
{
  time_t rawtime;
  struct tm * timeinfo;
 
  time ( &rawtime );
  timeinfo = localtime ( &rawtime );
  printf ( "Das aktuelle Datum lautet: %s", asctime (timeinfo) );
 
  return EXIT_SUCCESS;
}
*/



// now set in platformio.ini
//#define DEBUG_PRINT 1    // SET TO 0 OUT TO REMOVE TRACES
#ifdef DEBUG_PRINT
#pragma message("Info : DEBUG_PRINT=1")
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
    


class ESp32ntp;

 
class ESP32ntp
{
  private:
  ESP32ntp(){};
  const char *_ntpserver = nullptr;
  const char *_timezone = nullptr;


  struct tm timeinfo;
  time_t now;
  time_t unixTime = 0;

/* z. Z. nicht genutzt      
 #define NumOfEvents 4 
 struct eventsettings {
  uint8_t sgMode    ;      
  uint8_t Start_hour;      
  uint8_t Start_min ;
  uint8_t Stop_hour ;
  uint8_t Stop_min  ;
 };

 eventsettings TimerEvents[7][NumOfEvents];       // TimerEvents NumOfEvents=4 for each day
 eventsettings DayAheadEvents[NumOfEvents];
*/

 public: 
    ESP32ntp(const char ntpserver[], const char timezone[]);
   ~ESP32ntp();
    bool begin();
    char* getTimeString();
    char* getDateString();
    int getWeekday();
    bool update(); 
    time_t getUnixTime();
    struct tm* getTimeInfo();
    //bool setSGrModeEvent(uint8_t eventIndex, uint8_t SGrMode, tm start, tm stop); // SGgMode (0= not_active 1=SG-readyMode1, 2= SG-readyMode2, 3= SG-readyMode3)
    //uint8_t getSGrModeActual(uint8_t eventIndex);   // return sg_mode (0= not_active 1=SG-readyMode1, 2= SG-readyMode2, 3= SG-readyMode3)
};