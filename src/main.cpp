#include <Arduino.h>
#include <Esp.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_rom_gpio.h"
#include <FS.h>                        // Build-in
#include <SPIFFS.h>                    // Build-in
#include <WiFi.h>                      // Built-in
#include <ESPmDNS.h>                   // Built-in
#include <ESPAsyncWebServer.h>         // Built-in
#include <HTTPClient.h>                // Build-in  
#include <base64.h>                    // Build-in
#include <Preferences.h>               // Build-in
        
#include "FileVarStore.h"

#ifdef WEB_APP
#include "AsyncWebLog.h"
#include "AsyncWebOTA.h"
#endif

#include "ESP32ntp.h"
#include "XPString.h"
#include "SmartGrid.h"
#include "SMLdecode.h"

#if defined ESP_S2_MINI || ESP_S3_ZERO
#include "driver/temp_sensor.h"
#endif

#ifdef M5_COREINK
#include <Wire.h>
#include <SPI.h>
#include "M5CoreInk.h"  
#include <esp_adc_cal.h>
#endif

// now set in platformio.ini
//#define DEBUG_PRINT 1   
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
    
#ifdef ESP32_RELAY_X2
#pragma message("Info : ESP32_RELAY_X2")
#define LED_GPIO 23
#define RELAY_1  16
#define RELAY_2  17
#endif

#ifdef ESP32_RELAY_X4
#pragma message("Info : ESP32_RELAY_X4")
#define LED_GPIO 23
#define RELAY_1  26
#define RELAY_2  25
#define RELAY_3  33
#define RELAY_4  32     
#endif

#ifdef ESP32_DEVKIT1
#pragma message("Info : ESP32_DEVKIT")
#define LED_GPIO 2
#endif

#ifdef ESP32_S2_MINI
#pragma message("Info : ESP32_S2_MINI")
#define LED_GPIO LED_BUILTIN    
#endif

#ifdef ESP32_S3_ZERO
 #pragma message("Info : ESP32_S3_ZERO")
 #define NEOPIXEL 21
#endif

#ifdef M5_COREINK
 #pragma message("Info : M5Stack CoreInk Module")
 #define LED_GPIO 10
#endif

const char* SYS_Version = "V 0.8.5";
const char* SYS_CompileTime =  __DATE__ ;
static String  SYS_IP = "0.0.0.0";

#ifdef WEB_APP
// internal Webserver                                
AsyncWebServer webserver(80);
#endif

// ntp client
const char* TimeServerLocal = "192.168.2.1";
const char* TimeServer      = "europe.pool.ntp.org";
const char* Timezone        = "CET-1CEST,M3.5.0,M10.5.0/3";       // Central Europe
ESP32ntp ntpclient(TimeServerLocal,Timezone);

WiFiClient wificlient;
SMLdecode smldecoder;

// fetch String;
static String sFetch;

const long   TimerFastDuration = 300;
long   TimerFast = 0;
const long   TimerSlowDuration   = 5000;   
long   TimerSlow = 0;     

#ifdef ESP32_S3_ZERO
static char neopixel_color = 'w';
#define  setcolor(...) neopixel_color = __VA_ARGS__
#else
#define setcolor(...)
#endif


////////////////////////////////////////////
/// @brief init builtin LED
////////////////////////////////////////////
void  inline initLED()
{
 #ifndef ESP32_S3_ZERO
  pinMode(LED_GPIO, OUTPUT);
  digitalWrite(LED_GPIO, HIGH);
#else
  neopixel_color='w';
#endif
}

/// @brief  set builtin LED
/// @param i = HIGH / LOW
void setLED(uint8_t i)
{
#ifndef ESP32_S3_ZERO
#ifndef M5_COREINK
 digitalWrite(LED_GPIO, i);
#endif
#else
  if (i==0)
  {
    neopixelWrite(NEOPIXEL,0,0,0); // off
  }
  else
  {
    switch (neopixel_color)
    {
    case 'r': 
      neopixelWrite(NEOPIXEL,6,0,0); // red
      break;
    case 'g':
      neopixelWrite(NEOPIXEL,0,6,0); // green
      break;
    case 'b':
      neopixelWrite(NEOPIXEL,0,0,6); // blue
      break;
    case 'y':
       neopixelWrite(NEOPIXEL,4,2,0); // yellow
      break;
    case 'w':
      neopixelWrite(NEOPIXEL,2,2,2); // white
      break;
    default:
       break;
    }
  }
#endif
}

uint8_t blnk=0;
void blinkLED()
{
  blnk = !blnk;
  setLED(blnk);
}

void inline initSPIFFS()
{
  if (!SPIFFS.begin())
  {
   debug_println("*** ERROR: SPIFFS Mount failed");
  } 
  else
  {
   debug_println("* INFO: SPIFFS Mount succesfull");
  }
}
 

void inline initRelay()
{
#ifdef ESP32_RELAY_X2
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  digitalWrite(RELAY_1, LOW);
  digitalWrite(RELAY_2, LOW);
#elseifdef ESP_RELAY_X4
  // ESP_RELAY_X4
  // we need only RELAY_1 and RELAY_2 for SGready
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);

  pinMode(RELAY_3, OUTPUT);
  pinMode(RELAY_4, OUTPUT);

  digitalWrite(RELAY_1, LOW);
  digitalWrite(RELAY_2, LOW);

  digitalWrite(RELAY_3, LOW);
  digitalWrite(RELAY_4, LOW);
#else
 /* for other boards without relais output */
#endif
}


/// @brief set Relay
/// @param no 1..2
/// @param on_off true..false
void setRelay(uint8_t no, bool on_off)
{
#if defined ESP32_RELAY_X2 || defined ESP_RELAY_X4
  switch (no)
  {
   case 1:
    digitalWrite(RELAY_1,on_off);
    break;
   case 2:
    digitalWrite(RELAY_2,on_off);
    break; 
   default:
     break;
  }
#else
 /* for other boards witout relais output*/
 #endif
}


//////////////////////////////////////////////////////
/// @brief  expand Class "FileVarStore" with variables
//////////////////////////////////////////////////////
class WPFileVarStore : public FileVarStore 
{
 public:  
  // Device-Parameter
   String varDEVICE_s_Name  = "ESP_SMARTGRID";
  // Wifi-Parameter
   String varWIFI_s_Mode    = "AP"; // STA=client connect with Router,  AP=Access-Point-Mode (needs no router)
   String varWIFI_s_Password= "";
   String varWIFI_s_SSID    = "espsmartgrid";

   int    varEPEX_i_low     = 22;   // cent per kwh
   int    varEPEX_i_high    = 26;   // cent per kwh
   int    varCOST_i_mwst    = 19;   // MwSt (=tax percent) of hour price
   float  varCOST_f_fix     = 17.2; // fix part of hour price in cent
#if defined SML_TASMOTA || defined SML_TIBBER
   String varSML_s_url      = "";
   String varSML_s_user     = "";
   String varSML_s_password = "";
#endif
#ifdef CALC_HOUR_ENERGYPRICE
   float varCOST_f_kwh_fix  = 31.0;  // kWh Price fix
#endif
   String varSG_s_rule1     = "00,00,0,0,FIX"; // start_hour, stop_hour, val1, val2, rulemode ("FIX, "EPEX_LOWHOUR", "EPEX_HIGHHOUR", "EPEX_HIGHLIMIT" EPEX_LOWLIMIT)
   String varSG_s_rule2     = "00,00,0,0,FIX";
   String varSG_s_rule3     = "00,00,0.0,FIX";
   String varSG_s_rule4     = "00,00,0,0,FIX";
   String varSG_s_rule5     = "00,00,0,0,FIX";
   String varSG_s_rule6     = "00,00,0,0,FIX";
#ifdef SG_READY
   String varSG_s_out1      = "GPIO_12";
   String varSG_s_out2      = "GPIO_10";
   String varSG_s_sg1       = "";        //  http://192.168.2.108/fetch?imax=6; for my ABL-Wallbox ESP Project     
   String varSG_s_sg2       = "";        //  http://192.168.2.137/cm?cmnd=Power1%201 Tasmota Relais1 1 on    (replace % with # in config)
   String varSG_s_sg3       = "";        //  http://192.168.2.137/cm?cmnd=Power1%200 Tasmota Relais1 2 off   ("                             ")
   String varSG_s_sg4       = "";        //  http://192.168.2.137/cm?cmnd=Backlog%20Power1%201%3BPower2%200   Tasmota Relais1=on, Relais2=off (replace % with # in config) switch Relais 1 and Relais 2 
#endif

#ifdef LUX_WEBSOCKET
   String varLuxtronik_s_IP = "192.168.2.101";
#endif
   
 protected:
   void GetVariables()
   {
     varDEVICE_s_Name     = GetVarString(GETVARNAME(varDEVICE_s_Name));
     varWIFI_s_Mode       = GetVarString(GETVARNAME(varWIFI_s_Mode)); //STA or AP
     varWIFI_s_Password   = GetVarString(GETVARNAME(varWIFI_s_Password));
     varWIFI_s_SSID       = GetVarString(GETVARNAME(varWIFI_s_SSID));
   
     varEPEX_i_low        = GetVarInt(GETVARNAME(varEPEX_i_low));
     varEPEX_i_high       = GetVarInt(GETVARNAME(varEPEX_i_high));

     varSG_s_rule1         = GetVarString(GETVARNAME(varSG_s_rule1));
     varSG_s_rule2         = GetVarString(GETVARNAME(varSG_s_rule2));
     varSG_s_rule3         = GetVarString(GETVARNAME(varSG_s_rule3));
     varSG_s_rule4         = GetVarString(GETVARNAME(varSG_s_rule4));
     varSG_s_rule5         = GetVarString(GETVARNAME(varSG_s_rule5));
     varSG_s_rule6         = GetVarString(GETVARNAME(varSG_s_rule6));
#ifdef SG_READY
     varSG_s_out1          = GetVarString(GETVARNAME(varSG_s_out1));
     varSG_s_out2          = GetVarString(GETVARNAME(varSG_s_out2));
     varSG_s_sg1           = GetVarString(GETVARNAME(varSG_s_sg1));
     varSG_s_sg2           = GetVarString(GETVARNAME(varSG_s_sg2));
     varSG_s_sg3           = GetVarString(GETVARNAME(varSG_s_sg3));
     varSG_s_sg4           = GetVarString(GETVARNAME(varSG_s_sg4));
#endif
#if defined SML_TASMOTA || defined SML_TIBBER
     varSML_s_url         = GetVarString(GETVARNAME(varSML_s_url));
     varSML_s_password    = GetVarString(GETVARNAME(varSML_s_password));
     varSML_s_user        = GetVarString(GETVARNAME(varSML_s_user));
#endif 
     varCOST_f_fix         = GetVarFloat(GETVARNAME(varCOST_f_fix),17.2);
     varCOST_i_mwst        = GetVarInt(GETVARNAME(varCOST_i_mwst), 19);
#ifdef CALC_HOUR_ENERGYPRICE   
     varCOST_f_kwh_fix     = GetVarFloat(GETVARNAME(varCOST_f_kwh_fix), 31.0);
#endif


#ifdef LUX_WEBSOCKET
       varLuxtronik_s_IP    = GetVarString(GETVARNAME(varLuxtronik_s_IP));
#endif
   }
};
WPFileVarStore varStore;

void setSGreadyOutput(uint8_t mode);
//////////////////////////////////////////////////////
/// @brief  expand Class "FileVarStore" with variables
//////////////////////////////////////////////////////
class SmartGridEPEX : public SmartGrid
{ 
  public:
  SmartGridEPEX(const char* epex) : SmartGrid(epex) {}

#ifndef M5_COREINK
  bool getAppRules() final
  {
    for (size_t i = 0; i < SG_HOURSIZE; i++)
    {
        setHourVar1(i, DEFAULT_SGREADY_MODE);
    }
    calcSmartGridfromConfig(varStore.varSG_s_rule1.c_str());
    calcSmartGridfromConfig(varStore.varSG_s_rule2.c_str());
    calcSmartGridfromConfig(varStore.varSG_s_rule3.c_str());
    calcSmartGridfromConfig(varStore.varSG_s_rule4.c_str());
    calcSmartGridfromConfig(varStore.varSG_s_rule5.c_str());
    calcSmartGridfromConfig(varStore.varSG_s_rule6.c_str());  
   return true;;
  }

  /// @brief set hour output for Application
  /// @param hour 
  void setAppOutputFromRules(uint8_t hour) final
  {
#ifdef SG_READY
    setSGreadyOutput(this->getHourVar1(hour));
#endif
  }
#endif


  /// @brief your individual calculation for Enduserprice (here: Tibber DE)
  /// @param rawprice 
  /// @return 
  float calcUserkWhPrice(float rawprice)
  {
    float f = (rawprice / 10.0) + (rawprice*varStore.varCOST_i_mwst/1000.0) +  varStore.varCOST_f_fix;
    //debug_printf("Epex-price:%f\r\n",rawprice);
    //debug_printf("User-price:%f\r\n",f);
    return f;
  }

 #ifdef CALC_HOUR_ENERGYPRICE
  float getUserkWhFixPrice()
  { 
   return varStore.varCOST_f_kwh_fix;
  }
#endif
};


#define URL_ENERGY_CHARTS "api.energy-charts.info" 
SmartGridEPEX smartgrid(URL_ENERGY_CHARTS);

#ifdef SG_READY
/// @brief  Post a REST message to a web-device instaed of switching a Relais-Output-Pin
/// @param s  REST url
/// @return 
bool sendSGreadyURL(String s)
{
  HTTPClient http;
  
  if (s.startsWith("http:") == false)
  {
    AsyncWebLog.println("No HTTP sendSGreadString:" + s);
     return false;   
  }

 
  AsyncWebLog.println("sendSGreadURL:" + s);
  int getlength;
  s.replace('#','%'); // because % % is used as html insert marker
  http.begin(wificlient, s);
  int httpResponseCode = http.GET();
  
  if (httpResponseCode > 0) 
  {
    getlength = http.getSize();
    if (getlength > 0)
    {
      AsyncWebLog.println("send OK: SG ready url:" + s);
      http.end();
      return true;
    }
  }
  else 
  {
    AsyncWebLog.println("ERROR: SG ready url:" + s);
    debug_printf("httpResponseCode: %d\r\n", httpResponseCode);
  }
  // Free resources
  http.end();

  return false;
}

void setSGreadyOutput(uint8_t mode)
{
  uint8_t h = ntpclient.getTimeInfo()->tm_hour;
  debug_printf("SGreadyMode: %d\r\n",mode);
  smartgrid.setHourVar1(h, mode); // override if value has changed
  String sURL;
  sURL.reserve(50);
  
 #ifdef WEB_APP
  AsyncWebLog.println("SGreadyMode:"+ String(mode));   
#endif  
#ifdef SG_READY
  switch (mode)
  {
  case 1:
    setcolor('b');
    sendSGreadyURL(varStore.varSG_s_sg1);
   
    setRelay(1,1);
    setRelay(2,0);
    
    break;
  case 2:
    setcolor('g');
    sendSGreadyURL(varStore.varSG_s_sg2);
   
    setRelay(1,0);
    setRelay(2,0);
    break;
  case 3:
    setcolor('y');
    sendSGreadyURL(varStore.varSG_s_sg3);
  
    setRelay(1,0);
    setRelay(2,1);
    break;
  case 4:
    setcolor('r');
    sendSGreadyURL(varStore.varSG_s_sg4);
    setRelay(1,1);
    setRelay(2,1);
    break;  
  default:
    break;
  }
#elif defined LED_PRICE
   if (smartgrid.getUserkWhPrice(h) > varStore.varEPEX_i_high)
   {
     setcolor('r');
   }
   else if (smartgrid.getUserkWhPrice(h) > varStore.varEPEX_i_low)
   {
     setcolor('y');
   }
   else
  {
    setcolor('g');
  }
#endif
}
#endif

/*
#ifdef SML_TASMOTA
// special version to get the values from the main energy-meter 
// from tasmota device to read the optical SML-Signal fom the main energy-meter
String httpGETRequest(const char* serverName) 
{
  HTTPClient http;
    
  // Your Domain name with URL path or IP address with path
  http.begin(wificlient, serverName);
  
  // If you need Node-RED/server authentication, insert user and password below
  //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = ""; 
  
  if (httpResponseCode>0) {
    //debug_print("HTTP Response code: ");
    //debug_println(httpResponseCode);
    payload = http.getString();
    //debug_printf("httpGetRequest: %s\r\n",payload);
  }
  else {
    debug_print("Error code: ");
    debug_println(httpResponseCode);
  }
  // Free resources
  http.end();

  payload.trim();
  return payload;
}

/// @brief my implementation for reading a TASMOTA device...adapt to your needs
void getSmlMeter()
{
  // by JG : todo: replace hardcoded url with config-value
  String s = httpGETRequest("http://192.168.2.88/cm?cmnd=status%2010");
  //Format of Tasmota SML
  //{
   //"StatusSNS": {
   // "Time": "2024-04-29T18:28:19",
   // "SML": {
   //  "Total_in": 5953.3436,
   //   "Total_out": 8620.3801,
   //   "Power_curr": -27
   // }
  // }
  //}
 
 JsonDocument doc;
 deserializeJson(doc, s);
 valSML_kwh_in  = doc["StatusSNS"]["SML"]["Total_in"];
 valSML_kwh_out = doc["StatusSNS"]["SML"]["Total_out"];
 valSML_watt   = doc["StatusSNS"]["SML"]["Power_curr"];
 String sml = "SML-Meter-Watt: ";
 sml += String(valSML_watt);
 AsyncWebLog.println(sml);
}
#endif


#ifdef SML_TIBBER
#define SMLPAYLOADMAXSIZE 300
byte smlpayload[SMLPAYLOADMAXSIZE] {0}; 
// my special version to get the values from the tibber-host 
// ...quick and dirty ;-) only testet for Meter: ISKRA MT631 !!!
// write your own routine if you need this values !!!
byte * httpGETRequest(const char* serverName) 
{
  HTTPClient http;

  // for TEST !!!
  //varStore.varSML_s_user     = "admin"; 
  //varStore.varSML_s_password = "xxxx-xxxx";

  int getlength = 0;
  http.begin(wificlient, serverName);
  http.setAuthorization(varStore.varSML_s_user.c_str(), varStore.varSML_s_password.c_str());
  int httpResponseCode = http.GET();
  
  if (httpResponseCode > 0) 
  {
    getlength = http.getSize();
    if ((getlength > SMLPAYLOADMAXSIZE) || (getlength < 200))
    {
      http.end();
      return NULL;
    }

    WiFiClient * w = http.getStreamPtr();
    w->readBytes(smlpayload, getlength);

    if (getlength != 272)
    {
     AsyncWebLog.println("Error tibber smlpayload");
     debug_printf("smlpayload length:%d Data: ", getlength);
     for (size_t i = 0; i < 10; i++)
     {
       debug_printf("%02x ",smlpayload[i]);
     }
     debug_println("...");
     getlength = 0;
    }
    
  }
  else 
  {
    AsyncWebLog.print("TIBBER httpGETRequest Error code");
    AsyncWebLog.println(String(httpResponseCode));
    debug_print("TIBBER httpGETRequest Error code: ");
    debug_println(httpResponseCode);
  }
  // Free resources
  http.end();

  if (getlength <= 0)
  {
    return NULL;
  }

  return smlpayload;
}

uint32_t decodeSMLval(byte * payload, byte* smlcode, uint smlsize,  uint offset)
{
  byte *loc = (byte*)memmem(payload, SMLPAYLOADMAXSIZE, smlcode, smlsize);
  if (loc == NULL)
  {
    return 0;
  }

  //Test fake "-1"
  //if (nlen == 2){ loc[offset]  = 0xff; loc[offset+1] = 0xff;}
  
  // 10/2024 neu: 'nlen' aus SML lesen (aendert sich bei Leistung je nach Größe des Wertes !!!)
  uint8_t nlen = (loc[offset-1] & 0x0F) -1;

  if ((nlen < 1) ||(nlen > 8))
  {
    debug_println("decodeSMLval unvalid length");
    return 0;
  }
  
  // for extradebugging
  
  //debug_printf(" nlen=%d\r\n", nlen);
  //debug_print("HEX: ");
  //for (size_t i = 0; i < offset+nlen+1; i++)
  //{debug_printf("%02x ", loc[i]);}
  

  byte* pval = loc + offset;
  uint32_t value=0;

 if (nlen == 1)
 {
   value = (int8_t) *(pval);
 }
 else
 {
  nlen=nlen+1; 
  while (--nlen) 
  {
      //value<<=8;
      value = value << 8;
      value|=*(pval)++;
  }
 }

  return value;
}

/// @brief my implementation for reading a Tibber device...adapt to your needs
void getSmlMeter()
{
  // http://192.168.2.87/data.json?node_id=1  ( for test with webbrowser)
  // varStore.varSML_s_url="192.168.2.87";
  String sURL = "http://";
  sURL += varStore.varSML_s_url;
  sURL += "/data.json?node_id=1";
  byte * payload = httpGETRequest(sURL.c_str());
  
  if (payload == NULL)
  {
    return;
  }

  // Energy IN (1.8.0)
  byte sml_1_8_0[] {0x77, 0x07, 0x01, 0x00, 0x01, 0x08, 0x00, 0xff};
  valSML_kwh_in = double(decodeSMLval(payload, sml_1_8_0, sizeof(sml_1_8_0), 19)) / 10000.0;
  

// for Power:
// length is variable !!! (2 byte= for big values)
//                                           53=16bit int
// 77 07 01 00 10 07 00 ff 01 01 62 1b 52 00 53 00 91 01 145W
// 77 07 01 00 10 07 00 ff 01 01 62 1b 52 00 53 00 95 01 149W
// 77 07 01 00 10 07 00 ff 01 01 62 1b 52 00 53 00 8d 01 141W
// length is variable !!! (1 byte for small values)
//                                           52=8bit int 
// 77 07 01 00 10 07 00 ff 01 01 62 1b 52 00 52 76 01 01 
// 77 07 01 00 10 07 00 ff 01 01 62 1b 52 00 52 66 01 01

  // Power (in= pos. out= neg)
  byte sml_16_7_0[] {0x77, 0x07, 0x01, 0x00, 0x10, 0x07, 0x00, 0xFF};
  valSML_watt = int16_t(decodeSMLval(payload, sml_16_7_0, sizeof(sml_16_7_0), 15));

  String s1 = "SML:";
  s1 += String(valSML_kwh_in);
  s1 += "kWh ";
  s1 += String(valSML_watt);
  s1 += "W";
  AsyncWebLog.println(s1);
  debug_println(s1);
}
#endif
*/

static String readString(File s) 
{
  String ret;
  int c = s.read();
  while (c >= 0) {
    ret += (char)c;
    c = s.read();
  }
  return ret;
}   

void initFileVarStore()
{
  varStore.Load();
}   


//////////////////////////////////////////
/// @brief Init Wifi
/////////////////////////////////////////
void initWiFi()
{
  // Test mit AP !!!!!!!!!!!!!!!!!!!!!
  //varStore.varWIFI_s_Mode="AP";
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // API Info: https://docs.espressif.com/projects/esp-idf/en/v4.4.6/esp32/api-reference/network/esp_wifi.html
#ifdef MINI_32
   WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG,0); // brownout problems
#endif
   if (varStore.varWIFI_s_Mode == "AP")
   {
    delay(100);
    Serial.println("INFO-WIFI:AP-Mode");
    WiFi.softAP(varStore.varDEVICE_s_Name.c_str());   
    Serial.print("IP Address: ");
    SYS_IP = WiFi.softAPIP().toString();
    Serial.println(SYS_IP);
   }
   else
   {
    debug_printf("INFO-WIFI:STA-Mode\r\n");
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(varStore.varDEVICE_s_Name.c_str());
    WiFi.begin(varStore.varWIFI_s_SSID.c_str(), varStore.varWIFI_s_Password.c_str());
    #if defined ESP32_S3_ZERO || defined MINI_32 || defined M5_COREINK
    WiFi.setTxPower(WIFI_POWER_7dBm);// brownout problems with some boards or low battery load for M5_COREINK
    //WiFi.setTxPower(WIFI_POWER_15dBm);// Test 15dB
    #endif
    #if defined DEBUG_PRINT && (defined ESP32_RELAY_X4 || defined ESP32_RELAY_X2)
    WiFi.setTxPower(WIFI_POWER_MINUS_1dBm); // decrease power over serial TTY-Adapter
    #endif
    int i = 0;
    delay(200);
    debug_printf("SSID:%s connecting\r\n", varStore.varWIFI_s_SSID);
    ///debug_printf("Passwort:%s\r\n", varStore.varWIFI_s_Password);
    while (!WiFi.isConnected())
    {
        debug_print(".");
        blinkLED();
        i++;  
        delay(400);
        if (i > 20)
        {
          ESP.restart();
        }
    }
    if (!WiFi.isConnected())
    {
      ESP.restart();
    }
    delay(300);
    debug_println("CONNECTED!");
    debug_printf("WiFi-Power:%d\r\n",WiFi.getTxPower())
    debug_printf("WiFi-RSSI:%d\r\n",WiFi.RSSI());
      
    SYS_IP = WiFi.localIP().toString();
    debug_print("IP Address: ");
    debug_println(SYS_IP);
   }
  return;
}     

inline void testWiFiReconnect()
{
  // Test if wifi is lost from router
  if ((varStore.varWIFI_s_Mode == "STA") && (WiFi.status() != WL_CONNECTED))
    {
     debug_println("Reconnecting to WiFi...");
     delay(300);
    if (!WiFi.reconnect())
    {
      //saveHistory();
      //hist.putInt("restart",SYS_RestartCount++);
      delay(200);
      ESP.restart();
    } 
   }
}

#ifdef WEB_APP
// -------------------- WEBSERVER -------------------------------------------
// --------------------------------------------------------------------------

/// @brief replace placeholder "%<variable>%" in HTML-Code
/// @param var 
/// @return String
String setHtmlVar(const String& var)
{
  
  debug_print("func:setHtmlVar: ");
  debug_println(var);
  sFetch = "";
 
  if (var == "CONFIG") // read config.txt
  {
    if (!SPIFFS.exists("/config.txt")) 
    {
     return String(F("Error: File 'config.txt' not found!"));
    }
    // read "config.txt" 
    fs::File configfile = SPIFFS.open("/config.txt", "r");   
    String sConfig;     
    if (configfile) 
    {
      sConfig = readString(configfile);
      configfile.close();
    }
    else 
    { // no "config.txt"
      sConfig = "";
    }
    return sConfig;
  } 
  else
  if (var== "DEVICEID")
  {
    return varStore.varDEVICE_s_Name;
  }
  else
  if (var == "INFO")
  {
     sFetch = "Version      :";
     sFetch += SYS_Version;

     sFetch += "\nBuild-Date :";
     sFetch +=  F(__DATE__);

     sFetch += "\nIP-Addr    :";
     sFetch += SYS_IP;

     sFetch += "\nRSSI       :";
     sFetch += WiFi.RSSI();

     sFetch += "\nTemp.      :";
     sFetch += String(temperatureRead());
     
     sFetch += "\nFreeHeap   :";
     sFetch += String(ESP.getFreeHeap());
     sFetch += "\nMinFreeHeap:";
     sFetch += String(ESP.getMinFreeHeap());

//*
     sFetch += "\n\nEPEX NEXT DATE:";
     sFetch +=  smartgrid.getWebData(true); //sEPEXdateNext;
     sFetch +=  "\n";
     sFetch += smartgrid.getWebHourValueString(true);

     sFetch += "\n\nEPEX TODAY:";
     sFetch +=  "\n";
     sFetch += smartgrid.getWebHourValueString(false); //sEPEXPriceToday;
     sFetch += "\n---\n\n";
//*/
     /*
     for (size_t i = 0; i < SG_HOURSIZE; i++)
     {
       if (smartgrid.getHourVar1(i) > 0) //[i].var1 > 0)
       {
        sFetch += "\n SG-SETTING_";
        sFetch += " Hour:";
        sFetch += String(i);
        sFetch += " mode:";
        sFetch += String(smartgrid.getHourVar1(i));  //[i].var1);
       }
     }
     //*/
     return sFetch;
  }
  else
  if (var =="PRICE_LOW")
  {
     return String(varStore.varEPEX_i_low);      
  }
 else
  if (var =="PRICE_HIGH")
  {
     return String(varStore.varEPEX_i_high);     
  }
  else
  if (var == "COST_MWST")
  {
    return String(varStore.varCOST_i_mwst);
  }
  else
  if (var == "COST_FIX")
  {
    return String(varStore.varCOST_f_fix);
  }
  else
  if (var == "EPEX_ARRAY")
  {
    //return  smartgrid.getWebHourValueString(false);                      //EPEXPriceToday; // jetzt TODAY !!!
    String ret = smartgrid.getWebHourValueString(false);
    if (smartgrid.getWebHourValueString(true).length() > 0)
    {
     ret += ", ";
     ret += smartgrid.getWebHourValueString(true);
    }
    return ret;
  }
  else
  if (var == "EPEX_DATE")
  {
    String ret = smartgrid.getWebDate(false);
    ret += "   -    ";
    ret += smartgrid.getWebData(true);
    return ret;
  }
  else
  if (var == "SGMODE")
  {
    //switch (SmartGridReadyStatus)
    switch (smartgrid.getHourVar1(ntpclient.getTimeInfo()->tm_hour))
    {
    case 1:
      return "sg1";
      break;
    case 2:
      return "sg2";
      break;
    case 3:
      return "sg3";
      break;
    case 4:
      return "sg4";
      break;
    default:
      break;
    }
    return "";
  }
  else
  if (var == "SGHOURMODE")
  {
    for (size_t i = 0; i < SG_HOURSIZE; i++)
    {
      sFetch += smartgrid.getHourVar1(i); //String(smartgrid_hour[i].var1);
      if (i < SG_HOURSIZE-1)
      {
        sFetch += ',';
      }
    }
    //sFetch += '0';
    return sFetch;
  }
#ifdef CALC_HOUR_ENERGYPRICE
  else
  if (var == "COSTINFO")
  { 
     sFetch =     "Flex(ct):";
    sFetch += String((smartgrid.getUserkWhPrice(ntpclient.getTimeInfo()->tm_hour)),1);
    sFetch +=   "  Fix (cnt):";
    sFetch += String(smartgrid.getUserkWhFixPrice(),1);
   
    sFetch +=  " Hour-kWh : ";
    sFetch +=  String((smldecoder.getInputkWh() /* valSML_kwh_in */ - smartgrid.hourprice_kwh_start),3);

    sFetch += "\r\n\r\nHour-flex:";
    sFetch += String(smartgrid.getFlexprice_hour(ntpclient.getTimeInfo()->tm_hour),2);
  
    sFetch +=   "  Day-flex :";
    sFetch += String(smartgrid.getFlexprice_monthday(ntpclient.getTimeInfo()->tm_mday),2);
    
    sFetch +=    " Month-flex:";
    sFetch += String(smartgrid.getFlexprice_month(ntpclient.getTimeInfo()->tm_mon),2);

    //sFetch +=     "\r\nSum-flex     : ";
    //sFetch += String(smartgrid.Flexprice_sum + smartgrid.getAktFlexprice(),2);

    sFetch +=    "\r\nHour-fix: ";
    sFetch += String(smartgrid.getFixprice_hour(ntpclient.getTimeInfo()->tm_hour),2);

    sFetch +=     "  Day-fix  :";
    sFetch += String(smartgrid.getFixprice_monthday(ntpclient.getTimeInfo()->tm_mday),2);

    sFetch +=     "  Month-fix:";
    sFetch += String(smartgrid.getFixprice_month(ntpclient.getTimeInfo()->tm_mon) ,2);

    //sFetch +=     "\r\nSum-fix     : ";
    //sFetch += String(smartgrid.fixprice_sum + smartgrid.akt_fixprice,2);
    sFetch += "";

    return sFetch;
  }

  else
  if (var == "PRICE_FIX")
  {
     return String(smartgrid.getUserkWhFixPrice(),1);
  }

  else 
  if  (var == "PRICE_HOUR_FLEX")
  {
     for (size_t i = 0; i < 24; i++)
     {
      //sFetch += String(smartgrid.flexprice_hour[i],2);
      sFetch += String(smartgrid.getFlexprice_hour(i),2);
      if (i < 23)
      {
       sFetch += ", ";
      }
     }
     debug_println(sFetch);
     return sFetch;
  }
  else
  if (var == "PRICE_HOUR_FIX")
  {
     for (size_t i = 0; i <24; i++)
     {
      //sFetch += String(smartgrid.fixprice_hour[i],2);
      sFetch += String(smartgrid.getFixprice_hour(i),2);
      if (i < 23)
      {
       sFetch += ", ";
      }
     }
     return sFetch;
  }
  else
  if (var == "PRICE_MONTHDAY_FLEX")
  {
     for (size_t i = 1; i < 32; i++)
     {
      //sFetch += String(smartgrid.flexprice_monthday[i],2);
      sFetch += String(smartgrid.getFlexprice_monthday(i),2);
      if (i < 31)
      {
       sFetch += ", ";
      }
     }
     return sFetch;
  }
  else
  if (var == "PRICE_MONTHDAY_FIX")
  {
     for (size_t i = 1; i < 32; i++) // begin with "1" !!
     {
      //sFetch += String(smartgrid.fixprice_monthday[i],2);
      sFetch += String(smartgrid.getFixprice_monthday(i),2);
      if (i < 31)
      {
       sFetch += ", ";
      }
     }
     return sFetch;
  }
  else
  if (var == "PRICE_MONTH_FLEX")
  {
     for (size_t i = 0; i < 12; i++)
     {
      //sFetch += String(smartgrid.flexprice_month[i],2);
      sFetch += String(smartgrid.getFlexprice_month(i),2);
      if (i < 11)
      {
       sFetch += ", ";
      }
     }
     return sFetch;
  }
  else
  if (var == "PRICE_MONTH_FIX")
  {
     for (size_t i = 0; i < 12; i++)
     {
      //sFetch += String(smartgrid.fixprice_month[i],2);
      sFetch += String(smartgrid.getFixprice_month(i),2);
      if (i < 11)
      {
       sFetch += ", ";
      }
     }
     return sFetch;
  }
  else
  if (var == "COST_AKT_MONTH") // read config.txt
  {
    if (!SPIFFS.exists("/cost_akt_month.txt")) 
    {
     return String(F("Error: File 'cost_akt_month.txt' not found!"));
    }
    // read "config.txt" 
    fs::File cfile = SPIFFS.open("/cost_akt_month.txt", "r");   
    String sData;     
    if (cfile) 
    {
      sData = readString(cfile);
      cfile.close();
    }
    else 
    { // no "config.txt"
      sData = "";
    }
    return sData;
  }
  else
  if (var == "COST_AKT_YEAR") // read config.txt
  {
    if (!SPIFFS.exists("/cost_akt_year.txt")) 
    {
     return String(F("Error: File 'cost_akt_year.txt' not found!"));
    }
    // read "config.txt" 
    fs::File cfile = SPIFFS.open("/cost_akt_year.txt", "r");   
    String sData;     
    if (cfile) 
    {
      sData = readString(cfile);
      cfile.close();
    }
    else 
    { // no "config.txt"
      sData = "";
    }
    return sData;
  }
  #endif
  
  return "";
}

void notFound(AsyncWebServerRequest *request) 
{
    request->send(404, "text/plain", "Not found");
}

// for "/" and "/index" handle post
void Handle_Index_Post(AsyncWebServerRequest *request)
{
  debug_println("Argument: " + request->argName(0));
  uint8_t i = 0;
  debug_printf("Value: %s\r\n", request->arg(i));
  request->send(SPIFFS, "/index.html", String(), false, setHtmlVar);
}
#endif
  

void initWebServer()
{ 
#ifdef WEB_APP
  //Route for root / web page
  webserver.on("/",          HTTP_GET, [](AsyncWebServerRequest *request)
  {https://
   request->send(SPIFFS, "/index.html", String(), false, setHtmlVar);
  });
  //Route for root /index web page
  webserver.on("/index.html",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/index.html", String(), false, setHtmlVar);
  });
  //Route for setup web page
  webserver.on("/meter.html",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/meter.html", String(), false, setHtmlVar);
  });
  //Route for setup web page
  webserver.on("/smartgrid.html",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/smartgrid.html", String(), false, setHtmlVar);
  });
  
  webserver.on("/sgready.html",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/sgready.html", String(), false, setHtmlVar);
  });


  //Route for setup web page
  webserver.on("/setup.html",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/setup.html", String(), false, setHtmlVar);
  });
  //Route for config web page
  webserver.on("/config.html",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/config.html", String(), false, setHtmlVar);
  });


  //Route for stored values
  webserver.on("/savevalues.html",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/savevalues.html", String(), false, setHtmlVar);
  });

 #ifdef LUX_WEBSOCKET
 //Route for Luxtronik values
  webserver.on("/luxtronik.html",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/luxtronik.html", String(), false, setHtmlVar);
  });
#endif

  //Route for Energy Cost
  webserver.on("/energycost.html",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/energycost.html", String(), false, setHtmlVar);
  });


  //Route for Info-page
  webserver.on("/info.html",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/info.html", String(), false, setHtmlVar);
  });



  // Route for style-sheet
  webserver.on("/style.css",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/style.css", String(), false);
  });


  webserver.on("/fetch", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    if (request->args() > 0)
    {
    debug_println("GET-Argument: " + request->argName(0));
    debug_print("Value: ");
    uint8_t i = 0;
    //String s  = request->arg(i);
    //debug_println(s);
    }
                                                                                 // Index:
    sFetch = ntpclient.getTimeString();                                          // 0 
    sFetch += ',';

    sFetch += smartgrid.getHourVar1(ntpclient.getTimeInfo()->tm_hour);           // 1
    sFetch += ',';
  #if defined SML_TASMOTA || defined SML_TIBBER
    sFetch += String(smldecoder.getWatt());                                               // 2
    sFetch += ',';
    sFetch += String(smldecoder.getInputkWh()) ;                                            // 3
  #else
    sFetch += "0,0",
  #endif
    sFetch += ',';
    sFetch += String((smartgrid.getUserkWhPrice(ntpclient.getTimeInfo()->tm_hour)),1); //4
    sFetch += ",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0";
  
    request->send(200, "text/plain", sFetch);
    //debug_println("server.on /fetch: "+ s);
  });

  // config.txt GET
  webserver.on("/config.txt", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/config.txt", "text/html", false);
  });
  // config.txt GET
  webserver.on("/reboot.html", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(404, "text/plain", "RESTART !");
    //saveHistory();
    ESP.restart();
  });

  // datafiles for direct access in Website
  webserver.on("/cost_akt_month.txt",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/cost_akt_month.txt", String(), false);
  });
  // datafiles for direct access in Website
  webserver.on("/cost_akt_year.txt",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/cost_akt_year.txt", String(), false);
  });

  //.. some code for the navigation icons
  webserver.on("/home.png",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/home.png", String(), false);
  });
  webserver.on("/file-list.png",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/file-list.png", String(), false);
  });
  webserver.on("/settings.png",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/settings.png", String(), false);
  });
  webserver.on("/current.png",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/current.png", String(), false);
  });
   webserver.on("/sg.png",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/sg.png", String(), false);
  });
  
  webserver.on("/sgready.png",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/sgready.png", String(), false);
  });
  /*
  webserver.on("/heatpump.png",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/heatpump.png", String(), false);
  });
  */
  webserver.on("/energycost.png",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/energycost.png", String(), false);
  });

  // ...a lot of code only for icons and favicons ;-))
  webserver.on("/manifest.json",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/manifest.json", String(), false);
  });
  webserver.on("/favicon.ico",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/favicon.ico", String(), false);
  });
  webserver.on("/apple-touch-icon.png",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/apple-touch-icon.png", String(), false);
  });
  webserver.on("/android-chrome-192x192.png",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/android-chrome-192x192.png", String(), false);
  });
  webserver.on("/android-chrome-384x384.png",          HTTP_GET, [](AsyncWebServerRequest *request)
  {
   request->send(SPIFFS, "/android-chrome-384x384.png", String(), false);
  });

  // ------------ POSTs --------------------------------------------------------------
  // root (/) POST
  webserver.on("/",          HTTP_POST, [](AsyncWebServerRequest *request)
  {
    Handle_Index_Post(request);
  });
  // index.html POST
  webserver.on("/index.html", HTTP_POST, [](AsyncWebServerRequest *request)
  {
    Handle_Index_Post(request);
  });


  // sgready.html POST
  webserver.on("/sgready.html",          HTTP_POST, [](AsyncWebServerRequest *request)
  {
   uint8_t i = 0;
   String s  = request->arg(i);
   //AsyncWebLog.println("Requestarg:" + s);
#ifdef SG_READY
   if (s == "mode1")
   {
      setSGreadyOutput(1);
   }
   else
   if (s == "mode2")
   {
      setSGreadyOutput(2);
   }
   else
   if (s == "mode3")
   {
      setSGreadyOutput(3);
   }
   else
   if (s == "mode4")
   {
      setSGreadyOutput(4);
   }
  #endif
   request->send(SPIFFS, "/sgready.html", String(), false, setHtmlVar); 
  });
     

  // config.html POST
  webserver.on("/config.html",          HTTP_POST, [](AsyncWebServerRequest *request)
  {
   //debug_println("Argument: " + request->argName(0));
   //debug_println("Value: ");
   uint8_t i = 0;
   String s  = request->arg(i);
   debug_println(s);
   if (request->argName(0) == "saveconfig")
   {
       varStore.Save(s);
       varStore.Load();
       smartgrid.refreshWebData(true); // new: 30.10.2024
       //smartgrid.getAppRules();   // old: setSmartGridRules(); // calulate rules from config
   }
   //debug_println("Request /index3.html");
   request->send(SPIFFS, "/config.html", String(), false, setHtmlVar);
  });
 
   // config.html POST
  webserver.on("/savevalues.html",          HTTP_POST, [](AsyncWebServerRequest *request)
  {
   //debug_println("Argument: " + request->argName(0));
   //debug_println("Value: ");
   uint8_t i = 0;
   String s  = request->arg(i);
   debug_println("--- POST-arg savevelues.html -----");
   debug_println(s);
   if (request->argName(0) == "savemonthday")
   {
      debug_println("\r\nMONTH_DAY");
      smartgrid.saveCost("/cost_akt_month.txt", s);
      smartgrid.loadCost_monthday();
   }
   else
   if (request->argName(0)== "savemonth")
   {
      debug_println("MONTH");
      smartgrid.saveCost("/cost_akt_year.txt", s);
      smartgrid.loadCost_month();
   }
   //debug_println("Request /index3.html");

   request->send(SPIFFS, "/savevalues.html", String(), false, setHtmlVar);
  });

  

  // init Webserver 
  sFetch.reserve(150);
  AsyncWebLog.begin(&webserver);
  AsyncWebOTA.begin(&webserver);
  webserver.onNotFound(notFound);
  webserver.begin();
#endif
}


// extra handling for M5Core-Ink Module
#ifdef M5_COREINK
Ink_Sprite TimePageSprite(&M5.M5Ink);


#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
uint32_t timeToSleep = 3600;    // =1 hour 

RTC_SLOW_ATTR int iInit = 1;
RTC_SLOW_ATTR uint16_t rtc_userhour_price[24];

/// @brief Draw only akt. Hour and date
/// @param time 
/// @param date 
void drawTimeAndDate(RTC_TimeTypeDef time, RTC_DateTypeDef date)
{
    char buf[14] = {0};
    uint8_t next_hour = 0;
   

    //snprintf(buf, 6, "%02d:%02d", time.Hours, time.Minutes);
    //TimePageSprite.drawString(40, 20, buf, &AsciiFont24x48);

    snprintf(buf,11, "%02d.%02d.%d", (date.Date), date.Month, date.Year);
    TimePageSprite.drawString(20, 58, buf, &AsciiFont8x16);

    if (time.Hours < 23)
    { next_hour = time.Hours+1;}
    sprintf(buf, "%02d:00 - %02d:00", time.Hours, next_hour);
    TimePageSprite.drawString(20, 74, buf, &AsciiFont8x16);
}


float getBatVoltage()
{
    analogSetPinAttenuation(35,ADC_11db);
    esp_adc_cal_characteristics_t *adc_chars = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_12, 3600, adc_chars);
    uint16_t ADCValue = analogRead(35);
    
    uint32_t BatVolmV  = esp_adc_cal_raw_to_voltage(ADCValue,adc_chars);
    float BatVol = float(BatVolmV) * 25.1 / 5.1 / 1000;
    return BatVol;
}

void drawBatterylevel()
{
    char batteryStrBuff[64];
    sprintf(batteryStrBuff,"Batt.:%.2fV",getBatVoltage());
    TimePageSprite.drawString(20,0,batteryStrBuff,&AsciiFont8x16);
}


void drawHourPrice(uint8_t hour)
{
    char buf[20];
    //sprintf(buf, "Tibber");
    //TimePageSprite.drawString(20, 62, buf, &AsciiFont24x48); 
    sprintf(buf, "%.1f ct", float(rtc_userhour_price[hour] / 10.0));
    TimePageSprite.drawString(20, 90, buf, &AsciiFont24x48); 

    if (hour < 23)
    {
      sprintf(buf, "hour+1: %.1f ct", float(rtc_userhour_price[hour+1] / 10.0));
      TimePageSprite.drawString(20, 134, buf, &AsciiFont8x16); 
    }
}


void initM5CoreInk()
{
  RTC_TimeTypeDef time;
  RTC_DateTypeDef date;

  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();
  
  M5.begin();
  digitalWrite(LED_EXT_PIN, HIGH);

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : debug_println("** Wakeup caused by external signal using RTC_IO"); break;

    case ESP_SLEEP_WAKEUP_EXT1 : debug_println("** Wakeup caused by external signal using RTC_CNTL"); break;

    case ESP_SLEEP_WAKEUP_TIMER :debug_println("** Wakeup caused by timer");
    iInit = 0;
    break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : debug_println("Wakeup caused by touchpad"); break;

    case ESP_SLEEP_WAKEUP_ULP : debug_println("Wakeup caused by ULP program"); break;

    default : debug_printf("**Wakeup was not caused by deep sleep: %d\n",wakeup_reason); 
     iInit = 1;
    break;
  }

  esp_sleep_pd_config(  ESP_PD_DOMAIN_RTC_SLOW_MEM,   ESP_PD_OPTION_ON );
  esp_sleep_pd_config(  ESP_PD_DOMAIN_RTC_FAST_MEM,   ESP_PD_OPTION_OFF);

   
 

    /*
    if(M5.M5Ink.isInit() == false)
    {
        debug_println("Ink Init failed");
        while(1) delay(100);
    }
    */

   M5.M5Ink.clear(1);
   TimePageSprite.creatSprite(0,0,200,160,false);
   
  M5.rtc.GetDate(&date);
  M5.rtc.GetTime(&time);

  // Update um Mitternacht
  if (time.Hours==0 && time.Minutes==0) 
  {
    iInit = 1;
  }
        
  if ((iInit == 1))
  {
          iInit = 0;
          initSPIFFS();
          initFileVarStore();
          initWiFi();
          if (!ntpclient.begin())
          {
            ESP.restart();
          }
          smartgrid.init();
          time_t t = ntpclient.getUnixTime();
          smartgrid.downloadWebData(&t,false);
          for (size_t i = 0; i < 24; i++)
          {
           rtc_userhour_price[i] = smartgrid.unsorted_userhour_price[i];
           debug_printf("price: %d, hour: %d\r\n", rtc_userhour_price[i], i);
          }
          
          debug_println("\r\ninit ready disconnect WIFI !")
          WiFi.disconnect(true);
          WiFi.mode(WIFI_OFF); 
          time.Hours = ntpclient.getTimeInfo()->tm_hour;
          time.Minutes = ntpclient.getTimeInfo()->tm_min;
          time.Seconds = ntpclient.getTimeInfo()->tm_sec;
          date.Date   = ntpclient.getTimeInfo()->tm_mday;
          date.Month  = ntpclient.getTimeInfo()->tm_mon+1;
          date.Year   = ntpclient.getTimeInfo()->tm_year+1900;
          date.WeekDay= ntpclient.getTimeInfo()->tm_wday;
          M5.rtc.SetTime(&time);
          M5.rtc.SetDate(&date);
  }

    M5.M5Ink.clear(); 
  
    drawBatterylevel();
    drawTimeAndDate(time, date);
    drawHourPrice(time.Hours);
    TimePageSprite.pushSprite(); 
    //delay(1000);

    timeToSleep = 3600 - (time.Minutes*60);
    esp_sleep_enable_timer_wakeup(timeToSleep * uS_TO_S_FACTOR);
    debug_println("Setup ESP32 to sleep in  " + String(timeToSleep) + " Seconds");

    debug_println("before M5.I5Ink.deepsleep()");
    pinMode(POWER_HOLD_PIN,OUTPUT);
    digitalWrite(POWER_HOLD_PIN,HIGH);
    gpio_hold_en(GPIO_NUM_12); // tell the esp to keep pin high even in sleep
   
    M5.rtc.clearIRQ();
    //M5.rtc.SetAlarmIRQ(3600-(time.Minutes*60));
    M5.M5Ink.deepSleep();

/*
    M5.rtc.disableIRQ();
    M5.rtc.SetAlarmIRQ(3600-(time.Minutes*60));
    gpio_wakeup_enable(GPIO_NUM_19, gpio_int_type_t::GPIO_INTR_LOW_LEVEL);
    esp_sleep_enable_gpio_wakeup();
    M5.shutdown();
*/
  
    pinMode(POWER_HOLD_PIN,OUTPUT);
    digitalWrite(POWER_HOLD_PIN,HIGH);
    gpio_hold_en(GPIO_NUM_12); // tell the esp to keep pin high even in sleep
   
    Serial.flush(); 
    esp_deep_sleep_start();           
}


#endif
      
////////////////////////////////////////////////////
/// @brief setup
////////////////////////////////////////////////////
void setup() 
{
#ifdef M5_COREINK
  initM5CoreInk();
#else
  initLED();
  setcolor('w'); //white
  setLED(1);
  initRelay();
  Serial.begin(115200);                                           
  delay(500);

  initSPIFFS();
  initFileVarStore();
  Serial.print("--------------------------------------------------");
  smldecoder.init(varStore.varSML_s_url.c_str(), varStore.varSML_s_user.c_str(), varStore.varSML_s_password.c_str());
  smartgrid.init();
  setLED(0);
  setcolor('b'); // blue
  initWiFi();
  if (!ntpclient.begin())
  {
    ESP.restart();
  }
  initWebServer(); 
  delay(200);
  setLED(0);
#endif
}

///////////////////////////////////// TimePageSprite.drawString(20,10,buf, &Font0);///////////
/// @brief loop
////////////////////////////////////////////////
void loop() 
{
#ifndef M5_COREINK
   if (millis() - TimerSlowDuration > TimerSlow) 
   {
    TimerSlow = millis();                      // Reset time for next event
    testWiFiReconnect();

    ntpclient.update();
    time_t tt = ntpclient.getUnixTime();
    String s = "Time: "; s+= ntpclient.getTimeString(); 
#ifdef WEB_APP
    AsyncWebLog.println(s); 
#endif

#if defined SML_TASMOTA || defined SML_TIBBER
    smldecoder.read();
#ifdef WEB_APP
    AsyncWebLog.printf("SML %dW  %.2f kWh\r\n", smldecoder.getWatt(), smldecoder.getInputkWh());
#endif
#endif
    smartgrid.loop(&tt);
#ifdef CALC_HOUR_ENERGYPRICE
    smartgrid.calcHourPrice(&tt, smldecoder.getInputkWh());
#endif
  }
  
  // fast blink
  if (millis() - TimerFastDuration > TimerFast)
  {
    TimerFast = millis();
    blinkLED();
  }
#endif // no CoreInk
#ifndef WEB_APP
    debug_printf("Tibber-Price:%s\r\n", String((smartgrid.getUserkWhPrice(ntpclient.getTimeInfo()->tm_hour)),1));
#endif
}

