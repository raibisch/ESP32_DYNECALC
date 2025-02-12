#include "LuxWebsocket.h"



// Wandelt Source-Code Variablen-Nam varVALUE_i_ABL_minAmpere en in String um :-))
#define GETVARNAME(Variable) (#Variable)


#define LUXPRINT_BUF_LEN 100
char luxprintbuf[LUXPRINT_BUF_LEN];

#define LUX_LOG_STRING2(...) sprintf(luxprintbuf,"[LUX] %s:\t %s\r\n",__VA_ARGS__, __VA_ARGS__);
#define LUX_LOG_STRING1(...) sprintf(luxprintbuf,"[LUX] %s\r\n",__VA_ARGS__, __VA_ARGS__);

#define WSRX_BUF_LEN 8600
char wsbufrx[WSRX_BUF_LEN]; 
XPString luxRxString(wsbufrx, WSRX_BUF_LEN);

#define WSTX_BUF_LEN 20
char wsbuftx[WSTX_BUF_LEN];
XPString luxTxString(wsbuftx, WSTX_BUF_LEN);

#define LUXID_BUF_LEN 20
char luxbufid[LUXID_BUF_LEN];
XPString luxIdString(luxbufid, LUXID_BUF_LEN);

#define LUXVALUE_BUF_LEN 10

char lxbufValueWWIst[LUXVALUE_BUF_LEN];
XPString luxValueWWIst(lxbufValueWWIst, LUXVALUE_BUF_LEN);

char lxbufValueVLIst[LUXVALUE_BUF_LEN];
XPString luxValueVLIst(lxbufValueVLIst, LUXVALUE_BUF_LEN);

char lxbufValueRLIst[LUXVALUE_BUF_LEN];
XPString luxValueRLIst(lxbufValueRLIst, LUXVALUE_BUF_LEN);

char lxbufValueRLSoll[LUXVALUE_BUF_LEN];
XPString luxValueRLSoll(lxbufValueRLSoll, LUXVALUE_BUF_LEN);

char lxbufValueAussentemp[LUXVALUE_BUF_LEN];
XPString luxValueOuttemp(lxbufValueAussentemp, LUXVALUE_BUF_LEN);


char lxbufValueAussentempmit[LUXVALUE_BUF_LEN];
XPString luxValueOuttempmid(lxbufValueAussentempmit, LUXVALUE_BUF_LEN);


char lxbufValueHeizGastemp[LUXVALUE_BUF_LEN];
XPString luxValueHeizGastemp(lxbufValueHeizGastemp, LUXVALUE_BUF_LEN);


char lxbufValueVDIntemp[LUXVALUE_BUF_LEN];
XPString luxValueVDIntemp(lxbufValueVDIntemp, LUXVALUE_BUF_LEN);



char lxbufValueVDHeiztemp[LUXVALUE_BUF_LEN];
XPString luxValueVDHeiztemp(lxbufValueVDHeiztemp, LUXVALUE_BUF_LEN);


char lxbufValueLuftEintemp[LUXVALUE_BUF_LEN];
XPString luxValueLuftEintemp(lxbufValueLuftEintemp, LUXVALUE_BUF_LEN);



char lxbufValueFlow[LUXVALUE_BUF_LEN];
XPString luxValueFlow(lxbufValueFlow, LUXVALUE_BUF_LEN);


char lxbufValueWWSwitch[LUXVALUE_BUF_LEN];
XPString luxValueWWSwitch(lxbufValueWWSwitch, LUXVALUE_BUF_LEN);

char lxbufValueHUPpwm[LUXVALUE_BUF_LEN];
XPString luxValueHUPpwm(lxbufValueHUPpwm, LUXVALUE_BUF_LEN);


char lxbufValueVDrpm[LUXVALUE_BUF_LEN];
XPString luxValueVDrpm(lxbufValueVDrpm, LUXVALUE_BUF_LEN);

char lxbufValueVDheatSwitch[LUXVALUE_BUF_LEN];
XPString luxValueVDheatSwitch(lxbufValueVDheatSwitch, LUXVALUE_BUF_LEN);


char lxbufValuePowerOut[LUXVALUE_BUF_LEN];
XPString luxValuePowerOut(lxbufValuePowerOut, LUXVALUE_BUF_LEN);

// neu ab V 3.89.1
char lxbufValuePowerIn[LUXVALUE_BUF_LEN];
XPString luxValuePowerIn(lxbufValuePowerIn, LUXVALUE_BUF_LEN);

char lxbufValueDefrostPercent[LUXVALUE_BUF_LEN];
XPString luxValueDefrostPercent(lxbufValueDefrostPercent, LUXVALUE_BUF_LEN);


char lxbufValueEnergyHEOut[LUXVALUE_BUF_LEN];
XPString luxValueEnergyHEOut(lxbufValueEnergyHEOut, LUXVALUE_BUF_LEN);

char lxbufValueEnergyWWOut[LUXVALUE_BUF_LEN];
XPString luxValueEnergyWWOut(lxbufValueEnergyWWOut, LUXVALUE_BUF_LEN);

char lxbufValueEnergySumIn[LUXVALUE_BUF_LEN];
XPString luxValueEnergySumIn(lxbufValueEnergySumIn, LUXVALUE_BUF_LEN);


char lxbufValueEnergyHEIn[LUXVALUE_BUF_LEN];
XPString luxValueEnergyHEIn(lxbufValueEnergyHEIn, LUXVALUE_BUF_LEN);

char lxbufValueEnergyWWIn[LUXVALUE_BUF_LEN];
XPString luxValueEnergyWWIn(lxbufValueEnergyWWIn, LUXVALUE_BUF_LEN);

char lxbufValueEnergySumOut[LUXVALUE_BUF_LEN];
XPString luxValueEnergySumOut(lxbufValueEnergySumOut, LUXVALUE_BUF_LEN);

char _bufurl[16];
XPString _url(_bufurl, sizeof(_bufurl));
char _bufpassword[32];
XPString _password(_bufpassword, sizeof(_bufpassword));
char _bufuser[32];
XPString _user(_bufuser, sizeof(_bufuser));

// only way to define static in header --> init in cpp-file !!
WS_POLL_STATUS       LuxWebsocket::poll_state = WS_POLL_STATUS::NOT_CONNECTED;
WStype_t             LuxWebsocket::wstype      = WStype_t::WStype_DISCONNECTED;
LUX_HEATPUMP_STATUS  LuxWebsocket::hp_state     = LUX_HEATPUMP_STATUS::INAKTIVE;

//----------------------------------------------------public Functions----------------------------------------------------

//default constructor
LuxWebsocket::LuxWebsocket()
{
    
}


/*-----------------------------
destructor for class, not needed by Arduino but for complete class. Calls Arduino end function
*/
LuxWebsocket::~LuxWebsocket()
{

}


/// @brief 
/// @param type 
/// @param payload 
/// @param length 
void LuxWebsocket::wsEvent(WStype_t type, uint8_t * payload, size_t length) 
{
  wstype = type;
	switch(type) {
		case WStype_DISCONNECTED: 
      //AsyncWebLog.println("[Websocket] Disconnected!");
      debug_println("[Websocket] Disconnected")
      poll_state = NOT_CONNECTED;
			break;
		case WStype_CONNECTED:
      //AsyncWebLog.println("\r\n[Websocket] connected");
      debug_println("\r\n[Websocket] connected!");
			//debug_printf("\r\n[Websocket] Connected to url: %s\n", (char*) payload);
      poll_state = SEND_LOGIN;
			break;
		case WStype_TEXT:
      luxRxString = (char*) payload;
			// der gesammte payload im debug_print !!
      //debug_printf("[Websocket] get text: %s\r\n", payload);
      DataParser();
			break;
    default:
		//case WStype_BIN:
      // no binary Data from Luxtronic
		//case WStype_ERROR:			
		//case WStype_FRAGMENT_TEXT_START:
		//case WStype_FRAGMENT_BIN_START:
		//case WStype_FRAGMENT:
		//case WStype_FRAGMENT_FIN:
      debug_printf("***Websocket ERROR: %d", type);
      //AsyncWebLog.println("[WSc] **ERROR**: " + String(type));
      poll_state = NOT_CONNECTED;
		 break;
	}
}



bool LuxWebsocket::wsConnect()
{
  debug_println("*** connect Websocket");
  if (poll_state == NOT_CONNECTED)
  {
     // server address, port and URL
     String sIP = _url.c_str();
     debug_printf("try to connect to Luxtronik-IP:%s\r\n",_url.c_str());
     
     LUX_LOG_STRING2("connect to", sIP);
     //logPrint(luxprintbuf);
     debug_println(luxprintbuf);
     //AsyncWebLog.println("connect to:" + sIP);
	  _wsClient.begin(_url.c_str(), 8214, "/", "Lux_WS");
     poll_state = SEND_LOGIN;
    return true;
  }
  return false;
}


bool LuxWebsocket::init(const char * url,  const char* password)
{
  _url = url;
  _password = password;
  hp_state = INAKTIVE;
  poll_state = NOT_CONNECTED;
	// event handler
	_wsClient.onEvent(wsEvent);
	//_wsClient.setReconnectInterval(5000);
  //_wsClient.enableHeartbeat(6000,500,2);
  wsConnect();

  //delay(200);
  
  debug_println("LuxWebsocket-Ini OK!");
  return true;
}

void LuxWebsocket::setpolling(bool on)
{
  if (poll_state < WS_POLL_STATUS::IS_CONNECTED)
  return;

  if (on)
  {
     poll_state = WS_POLL_STATUS::NO_POLLING;
  }
  else
  {
    poll_state = WS_POLL_STATUS::SEND_LOGIN;
  }
}

char csvfetchbuffer [100];
XPString sCSVfetch(csvfetchbuffer, sizeof(csvfetchbuffer));
const char* LuxWebsocket::getCSVfetch(bool all)
{
  sCSVfetch = ',';
  sCSVfetch += getval(LUX_VAL_TYPE::LUX_STATUS_HEATPUMP); // 3
  sCSVfetch += ',';
  sCSVfetch += getval(LUX_VAL_TYPE::LUX_TEMP_WW_IST);     // 4
  sCSVfetch += ',';
  sCSVfetch += getval(LUX_VAL_TYPE::LUX_TEMP_AT_IST);     // 5
  sCSVfetch += ',';
  sCSVfetch += getval(LUX_VAL_TYPE::LUX_TEMP_RL_IST);     // 6
  sCSVfetch += ',';
  sCSVfetch += getval(LUX_VAL_TYPE::LUX_TEMP_RL_SOLL);    // 7
  sCSVfetch += ',';
  sCSVfetch += getval(LUX_VAL_TYPE::LUX_TEMP_VL_IST);     // 8
  sCSVfetch += ',';
  sCSVfetch += getval(LUX_VAL_TYPE::LUX_COP_DAY_HEAT);    // 9
  sCSVfetch += ',';
  sCSVfetch += getval(LUX_VAL_TYPE::LUX_COP_DAY_WW);      // 10
  sCSVfetch += ',';
  sCSVfetch += getval(LUX_VAL_TYPE::LUX_POWER_IN);        // 11
  sCSVfetch += ',';
  sCSVfetch += getval(LUX_VAL_TYPE::LUX_POWER_OUT);       // 12
  if (all)
  {
  sCSVfetch += ',';
  sCSVfetch += getval(LUX_VAL_TYPE::LUX_PUMP_FLOW);       // 12 Pumpe l/h
  sCSVfetch += ','; 
  sCSVfetch += getval(LUX_VAL_TYPE::LUX_PUMP_PWM);        // 13 Pumpe PWM
  sCSVfetch += ',';
  sCSVfetch += getval(LUX_VAL_TYPE::LUX_PUMP_WW_SWITCH);  // 14 WW-Ventil
  sCSVfetch += ',';
  sCSVfetch += getval(LUX_VAL_TYPE::LUX_VD_HEIZ);        // 19 VD-Heiz-ON
  sCSVfetch += ',';
  sCSVfetch += getval(LUX_VAL_TYPE::LUX_VD_RPM);        // 20 Verdichter RPM
  sCSVfetch += ',';
  sCSVfetch += getval(LUX_VAL_TYPE::LUX_TEMP_VD_IN);    // 21 Verdichter Ansaug Temp
  sCSVfetch += ',';
  sCSVfetch += getval(LUX_VAL_TYPE::LUX_VD_ND);        // 22 Verdichter ND Bar 
  sCSVfetch += ',';
  sCSVfetch += getval(LUX_VAL_TYPE::LUX_VD_EEV);        // 23 EEV %
  sCSVfetch += ',';
  sCSVfetch += getval(LUX_VAL_TYPE::LUX_DEFROST_PERCENT); // 12 Abtaubedarf %
  sCSVfetch += ',';
  /* todo 
  sCSVfetch += getval(LUX_VAL_TYPE::LUX_TEMP_OUT);        // 24 Laufzeit WW 
  sCSVfetch += ',';
  sCSVfetch += getval(LUX_VAL_TYPE::LUX_TEMP_OUT);        // 25 Laufzeit Heiz
  sCSVfetch += ',';
  sCSVfetch += getval(LUX_VAL_TYPE::LUX_TEMP_OUT);        // 26 Laufzeit ges
  sCSVfetch += ',';
  sCSVfetch += getval(LUX_VAL_TYPE::LUX_TEMP_OUT);        // 27 Takte je Tag
  */
  }
  return sCSVfetch.c_str();
}

char valbuffer [20];
XPString sVal(valbuffer, sizeof(valbuffer));
const char* LuxWebsocket::getval(LUX_VAL_TYPE valtype)
{
   sVal = "--";
 
   switch (valtype)
   {
   case LUX_TEMP_VL_IST:
    sVal = luxValueVLIst.c_str();
    break;
   case LUX_TEMP_RL_IST:
     sVal = luxValueRLIst.c_str();
     break;
   case LUX_TEMP_RL_SOLL:
     sVal = luxValueRLSoll.c_str();
     break;
   case LUX_TEMP_WW_IST:
     sVal = luxValueWWIst.c_str();
     break;
   case LUX_TEMP_AT_IST:
     sVal = luxValueOuttemp.c_str();
     break;
   case LUX_TEMP_AT_MID:
     sVal = luxValueOuttempmid.c_str();
     break;
   case LUX_TEMP_VDGAS:
     sVal = luxValueHeizGastemp.c_str();
     break; 
   case LUX_TEMP_VD_IN:
     sVal = luxValueVDIntemp.c_str();
     break;
   case LUX_ENERGY_INSUM:
     sVal = luxValueEnergySumIn.c_str();
     break;
   case LUX_ENERGY_OUTSUM:
     sVal = luxValueEnergySumOut.c_str();
     break;
   case LUX_ENERGY_IN_WW:
     sVal = luxValueEnergyWWIn.c_str();
     break;
   case LUX_ENERGY_OUT_WW:
     sVal = luxValueEnergyWWOut.c_str();
     break;
   case LUX_ENERGY_IN_HEAT:
     sVal = luxValueEnergyHEIn.c_str();
     break;
   case LUX_ENERGY_OUT_HEAT:
     sVal = luxValueEnergyHEOut.c_str();
     break;
   case LUX_POWER_IN:
     sVal = luxValuePowerIn.c_str();
     break;
   case LUX_POWER_OUT:
     sVal = luxValuePowerOut.c_str();
     break;
   case LUX_VD_EEV:
     sVal = "x.xx";// todo !!
     break;
   case LUX_DEFROST_PERCENT:
     sVal = luxValueDefrostPercent.c_str();
     break;
   case LUX_DEFROST_LASTTIME:
     sVal = "00.00.00 xx:xx";
     break;
   // intern calculated:
   case LUX_COP_SUM:
     sVal = (energySumOut) / float(energySumIn);
     break;
   case LUX_COP_HEAT:
     sVal = (energyHEOut) / float(energyHEIn);
    break;
   case LUX_COP_WW:
     sVal = (energyWWOut) / float(energyWWIn);
     break;
   case LUX_COP_DAY_SUM:
     sVal = COPdaySumLux;
     break;
   case LUX_COP_DAY_HEAT:
     sVal = COPdayHE;
     break;
   case LUX_COP_DAY_WW:
     sVal = COPdayWW;
     break;
   case LUX_COP_ACTUAL:
     sVal = atof(luxValuePowerOut.c_str()) / atof(luxValuePowerIn.c_str());
     break;
    
   // webservice polling status
   case LUX_STATUS_POLL:  
     switch (poll_state)
     {
       case WS_POLL_STATUS::SEND_LOGIN:
         sVal = "LOGIN";
         break;
       case WS_POLL_STATUS::SEND_GET_INFO:
        sVal = "GET_INFO";
        break;
       case WS_POLL_STATUS::SEND_REFRESH:
        sVal = "REFRESH";
        break;
       case WS_POLL_STATUS::NO_POLLING:
        sVal = "NO_POLLING";
        break;
       default:
        sVal = "NOT_CONNECTED";
        break;
     }
     break;

   // heatpump status
   case LUX_STATUS_HEATPUMP:
     switch (hp_state)
     {
     case LUX_HEATPUMP_STATUS::INAKTIVE:
       sVal = "INAKTIV";
       break;
     case LUX_HEATPUMP_STATUS::HEATING:
       sVal = "HEIZUNG";
       break;
     case LUX_HEATPUMP_STATUS::WW:
       sVal = "WW";
       break;
     case LUX_HEATPUMP_STATUS::DEFROST:
       sVal = "ABTAUEN";
       break;
     default:
      break;
     }
     break;
   default:
     break;
   }
   
   return sVal.c_str();
}

 float LuxWebsocket::getvalf(LUX_VAL_TYPE valtype)
 {
   return atof(getval(valtype));
 }

 uint LuxWebsocket::getvalui(LUX_VAL_TYPE valtype)
 {
   return atoi(getval(valtype));
 }


/* private functions */

/// @brief calculate kwh values and save energy per day
/// @param bDayOld 
void LuxWebsocket::calcEnergyValues(bool bDayOld)
{
   energySumIn    = atof(luxValueEnergySumIn.c_str())  * 1000;
   energySumOut   = atof(luxValueEnergySumOut.c_str()) * 1000;

   energyWWIn     = atof(luxValueEnergyWWIn.c_str())   * 1000;
   energyWWOut    = atof(luxValueEnergyWWOut.c_str())  * 1000;

   energyHEIn     = atof(luxValueEnergyHEIn.c_str())   * 1000;
   energyHEOut    = atof(luxValueEnergyHEOut.c_str())  * 1000;

   if (bDayOld)
   {
    energyOldSumIn    = energySumIn;
    energyOldSumOut   = energySumOut;
    
    energyOldWWIn     = energyWWIn;
    energyOldWWOut    = energyWWOut;
    
    energyOldHEIn     = energyHEIn;
    energyOldHEOut    = energyHEOut;
   }
}


/// @brief  calculate die COP of last Day
void LuxWebsocket::calcCopDay()
{  
   if (energySumIn > 0) //luxtronic data availible
   {
     if (energySumIn > energyOldSumIn)
     {
       COPdaySumLux = (energySumOut - energyOldSumOut) / float(energySumIn - energyOldSumIn);
     }
     //else
     //{
       // alten Wert unverändert lassen !!
     //}
     //debug_printf("COP-DAY-SUM: %.2f\r\n",luxdata.COPdaySumLux);
     {
      COPdayWW = (energyWWOut - energyOldWWOut) / float(energyWWIn - energyOldWWIn);
     }
     //debug_printf("COP-DAY-WW : %.2f\r\n",luxdata.COPdayWW);
     //AsyncWebLog.println("COP-Day-WW: " + String(luxdata.COPdayWW,2));


     if (energyHEIn > energyOldHEIn)
     {
      COPdayHE = (energyHEOut - energyOldHEOut) / float(energyHEIn - energyOldHEIn);
      //debug_printf("COP-DAY-HE: %.2f\r\n",luxdata.COPdayHE); 
     }
   }
}

/// **********************************************************************
/// @brief parse Luxtronik Webservice (XML with strange structure ;-))
/// ACHTUNG: der Parser ist sprachabhaenig (hier: Deutsch) !! 
/// **********************************************************************
void LuxWebsocket::DataParser()
{
  int xml_pos0, xml_pos1, xml_pos2 = -1;
  bool bRet = false;

  switch (poll_state)
  {
  case SEND_LOGIN:
    xml_pos1 = luxRxString.index_of(LUX_XMLDEF_INFO);
    if (luxRxString.index_of(LUX_XMLDEF_INFO) > 0)
    {
       luxIdString.reset();
       luxRxString.substringBeetween(luxIdString, "<item id=", 10, "<name>Informationen</name>", -2);
       debug_printluxval("ID", luxIdString.c_str());
       //debug_printf("[LUX] ID:\t %s\r\n", luxIdString.c_str());
       //AsyncWebLog.println("Lux-Websocket:Info-ID:"+ String(luxIdString.c_str()));
       //AsyncWebLog.println(String(luxIdString));
       poll_state = SEND_GET_INFO;
    }
    break;
  case SEND_REFRESH:
    xml_pos1 = luxRxString.index_of(LUX_XMLDEF_INFO);
    debug_printf("REFRESH: %s\r\n", luxRxString.c_str());
    // geht so noch nicht !!!
    // sendet nur noch die item id's ohne Text (...)
    break;
  case SEND_GET_INFO:                                        
    /*                                                                             used fetch-Index:
     <item id='0x910164'><name>Temperaturen</name>
       <item id='0x98992c'><name>Vorlauf</name><value>20.2°C</value></item>           x = 
       <item id='0x9cfb4c'><name>Rücklauf</name><value>19.9°C</value></item>          x =
       <item id='0x9cfb94'><name>Rückl.-Soll</name><value>15.0°C</value></item>       x =
       <item id='0x9898e4'><name>Heissgas</name><value>28.2°C</value></item>          x =
       <item id='0x9cfc24'><name>Außentemperatur</name><value>19.4°C</value></item>   x =
       <item id='0x9cfc6c'><name>Mitteltemperatur</name><value>18.4°C</value></item>  x =
       <item id='0x9cfcb4'><name>Warmwasser-Ist</name><value>38.6°C</value></item>    x =
       <item id='0x98b1cc'><name>Warmwasser-Soll</name><value>40.0°C</value></item>
       <item id='0x98989c'><name>Wärmequelle-Ein</name><value>21.1°C</value></item>   x =
       <item id='0x9cfabc'><name>Vorlauf max.</name><value>70.0°C</value></item>   
       <item id='0x9e807c'><name>Ansaug VD</name><value>28.5°C</value></item>         x =
       <item id='0x9e810c'><name>VD-Heizung</name><value>41.6°C</value></item>        x =
       <item id='0x98978c'><name>Überhitzung</name><value>8.6 K</value></item>
       <item id='0x9897c4'><name>Überhitzung Soll</name><value>20.0 K</value></item>
       <item id='0x9e78e4'><name>TFL1</name><value>22.7°C</value></item>
       <item id='0x9e792c'><name>TFL2</name><value>24.6°C</value></item>
    <name>Temperaturen</name></item>
    */
     if (luxRxString.index_of("<name>Temperaturen</name>") > 0)
     { 
        //debug_println("Temperaturen");
        bRet = luxRxString.substringBeetween(luxValueWWIst, LUX_XMLDEF_TEMP_WW_IST, sizeof(LUX_XMLDEF_TEMP_WW_IST)-1, "</value>", -3);
        if (bRet)
        {
          debug_printluxval(GETVARNAME(LUX_TEMP_WW_IST), luxValueWWIst.c_str());
        }
        else
        {
          debug_println("*********** xmlValue not Valid");
        }
        
        bRet = luxRxString.substringBeetween(luxValueRLIst, LUX_XMLDEF_TEMP_RL_IST, sizeof(LUX_XMLDEF_TEMP_RL_IST)-1, "</value>", -3);
        if (bRet > 0)
        {
          debug_printluxval(GETVARNAME(LUX_TEMP_RL_IST), luxValueRLIst.c_str());
        }
        else
        {
          debug_println("*********** xmlValue not Valid");
        }

        // neu 10/2024
        bRet = luxRxString.substringBeetween(luxValueRLSoll, LUX_XMLDEF_TEMP_RL_SOLL, sizeof(LUX_XMLDEF_TEMP_RL_SOLL)-1, "</value>", -3);
        if (bRet > 0)
        {
          debug_printluxval(GETVARNAME(LUX_TEMP_RL_SOLL), luxValueRLSoll.c_str());
        }
        else
        {
          debug_println("*********** xmlValue not Valid");
        }

        bRet = luxRxString.substringBeetween(luxValueVLIst, LUX_XMLDEF_TEMP_VL_IST, sizeof(LUX_XMLDEF_TEMP_VL_IST)-1, "</value>", -3);
        if (bRet > 0)
        {
          //debug_printf("******* Vorlauf value:       %s\r\n", luxValueVLIst.c_str());
        }
        else
        {
          debug_println("*********** xmlValue not Valid");
        }
        
        bRet = luxRxString.substringBeetween(luxValueOuttemp, LUX_XMLDEF_TEMP_OUT, sizeof(LUX_XMLDEF_TEMP_OUT)-1, "</value>", -3);
        if (bRet > 0)
        {
          debug_printluxval(GETVARNAME(LUX_TEMP_OUT), luxValueRLIst.c_str());
        }
        else
        {
          debug_println("*********** xmlValue not Valid");
        }
        
        bRet = luxRxString.substringBeetween(luxValueOuttempmid, LUX_XMLDEF_TEMP_OUTM, sizeof(LUX_XMLDEF_TEMP_OUTM)-1, "</value>", -3);
        if (bRet > 0)
        {
          debug_printluxval(GETVARNAME(LUX_TEMP_OUTM), luxValueOuttempmid.c_str());
        }
        else
        {
          debug_println("*********** xmlValue not Valid");
        }

        bRet = luxRxString.substringBeetween(luxValueLuftEintemp, LUX_XMLDEF_TEMP_WQ_IN, sizeof(LUX_XMLDEF_TEMP_WQ_IN)-1, "</value>", -3);
        if (bRet > 0)
        {
          //debug_printf("******* Luft-Eing.Temp. value:    %s\r\n", luxValueLuftEintemp.c_str());
        }
        else
        {
          debug_println("*********** xmlValue not Valid");
        }

         bRet = luxRxString.substringBeetween(luxValueVDIntemp, LUX_XMLDEF_TEMP_VD_IN, sizeof(LUX_XMLDEF_TEMP_VD_IN)-1, "</value>", -3);
        if (bRet > 0)
        {
          //debug_printf("******* Verdichter-Eing.Temp. value:    %s\r\n", luxValueVDIntemp.c_str());
        }
        else
        {
          debug_println("*********** xmlValue not Valid");
        }

        bRet = luxRxString.substringBeetween(luxValueVDHeiztemp, LUX_XMLDEF_TEMP_VDHEI, sizeof(LUX_XMLDEF_TEMP_VDHEI)-1, "</value>", -3);
        if (bRet > 0)
        {
          //debug_printf("******* Verdichter Heiz.Temp. value:    %s\r\n", luxValueVDHeiztemp.c_str());
        }
        else
        {
          debug_println("*********** xmlValue not Valid");
        }

         bRet = luxRxString.substringBeetween(luxValueHeizGastemp, LUX_XMLDEF_TEMP_HEGAS, sizeof(LUX_XMLDEF_TEMP_HEGAS)-1, "</value>", -3);
        if (bRet > 0)
        {
          //debug_printf("******* Heizgas-Temp. value:    %s\r\n", luxValueHeizGastemp.c_str());
        }
        else
        {
          debug_println("*********** Heizgas xmlValue not Valid");
        }
     }

    /*
     char lxbufValueHeizGastemp[LUXVALUE_BUF_LEN];
     XPString luxValueHeizGastemp(lxbufValueHeizGastemp, LUXVALUE_BUF_LEN);


     char lxbufValueVDHeiztemp[LUXVALUE_BUF_LEN];
     XPString luxValueVDHeiztemp(lxbufValueVDHeiztemp, LUXVALUE_BUF_LEN);


     char lxbufValueLuftEintemp[LUXVALUE_BUF_LEN];
     XPString luxValueLuftEintemp(lxbufValueLuftEintemp, LUXVALUE_BUF_LEN);
    */

    /*
    <item id='0x9ce384'><name>Eingänge</name>
     <item id='0x9adfc4'><name>EVU</name><value>Ein</value></item>
     <item id='0x9add54'><name>HD</name><value>Aus</value></item>
     <item id='0x9ae00c'><name>MOT</name><value>Ein</value></item>
     <item id='0x9add9c'><name>HD</name><value>7.24 bar</value></item>
     <item id='0x9adde4'><name>ND</name><value>7.23 bar</value></item>
     <item id='0x9ae1bc'><name>Durchfluss</name><value>--- l/h</value></item>      x fetch-index = 
     <item id='0x9ae30c'><name>EVU 2</name><value>Aus</value></item>
     <item id='0x9ae354'><name>STB E-Stab</name><value>Aus</value></item>
    <name>Eingänge</name></item> 
    */
    if (luxRxString.index_of("Durchfluss</name>") > 0) //  Durchfluss
    {
      xml_pos1 = luxRxString.index_of("Durchfluss</name><value>") + 24;
      xml_pos2 = luxRxString.index_of("</value>", xml_pos1) -4;
      bRet = luxRxString.substring(luxValueFlow, xml_pos1, xml_pos2-xml_pos1);
      if (bRet > 0)
      {
          //debug_printf("******* Flow value:   %s\r\n", luxValueFlow.c_str());
          if (luxValueFlow.ends_with("-"))
          {
            luxValueFlow = "0";
          }
      }
      else
      {
          debug_println("*********** Druchfluss xmlValue not Valid");
      }
    }
    else
    {
      luxValueFlow = "0"; // for Hardware with without flow-Sensor
    } 

    /*
     <item id='0x98a0ec'><name>Ausgänge</name>
      <item id='0x9ad3b4'><name>AV-Abtauventil</name><value>Aus</value></item>
      <item id='0x9ad5ac'><name>BUP</name><value>Aus</value></item>                xx  Umschalt-Heiz/WW 0/1
      <item id='0x9ad63c'><name>HUP</name><value>Aus</value></item>                 () nur pos ermitteln da "HUP" doppelt vorkommt           
      <item id='0x9ad564'><name>Ventil.-BOSUP</name><value>Aus</value></item>
      <item id='0x9ad3fc'><name>Verdichter 1</name><value>Aus</value></item>      
      <item id='0x9ad7a4'><name>ZIP</name><value>Aus</value></item>
      <item id='0x9ad8c4'><name>ZUP</name><value>Aus</value></item>
      <item id='0x9ad444'><name>ZWE 1</name><value>Aus</value></item>
      <item id='0x9ad48c'><name>ZWE 2 - SST</name><value>Aus</value></item>
      <item id='0x9aefac'><name>AO 1</name><value>10.00 V</value></item>
      <item id='0x9aefe4'><name>AO 2</name><value>10.00 V</value></item>
      <item id='0x9ad51c'><name>VD-Heizung</name><value>Ein</value></item>         xx Verdichter Heizung 0/1
      <item id='0x9af594'><name>HUP</name><value>0.0 %</value></item>              xx Haupt-Umwaelzpumpe PWM-Wert %
      <item id='0x9af65c'><name>Freq. Sollwert</name><value>0 RPM</value></item> 
      <item id='0x9af6f4'><name>Freq. aktuell</name><value>0 RPM</value></item>    xx Frequ. Verdichter RPM
      <item id='0x9af89c'><name>Ventilatordrehzahl</name><value>0 RPM</value></item>
      <item id='0x9af8e4'><name>EEV Heizen</name><value>0.0 %</value></item>
      <item id='0x9af92c'><name>EEV Kühlen</name><value>0.0 %</value></item>
     <name>Ausgänge</name></item>
    */
    if (luxRxString.index_of("<name>Ausg") > 0) // Ausgänge
    {
      //debug_println("Ausgaenge");
      xml_pos1 = luxRxString.index_of("BUP</name><value>", luxRxString.index_of("<name>Ausg")) +17;  // "BUP" = WW Ventil 0/1
      xml_pos2 = luxRxString.index_of("</value>", xml_pos1) ;
      bRet = luxRxString.substring(luxValueWWSwitch, xml_pos1, xml_pos2-xml_pos1);
      if (bRet > 0)
      {
          //debug_printf("******* WW-Ventil Switch value:   %s\r\n", luxValueWWSwitch.c_str());
          if (luxValueWWSwitch.starts_with("E")) // Ein
          {
            luxValueWWSwitch = "1";
          }
          else
          {
            luxValueWWSwitch = "0";
          }
      }
      else
      {
          debug_println("*********** WW-Ventil Value not valid");
      }

      xml_pos1 = luxRxString.index_of("VD-Heizung</name><value>", luxRxString.index_of("<name>Ausg"))+ 24;  // "VD-Heizung" = Verdichterheizung 0/1
      xml_pos2 = luxRxString.index_of("</value>", xml_pos1) ;
      bRet = luxRxString.substring(luxValueVDheatSwitch, xml_pos1, xml_pos2-xml_pos1);
      if (bRet > 0)
      {
          //debug_printf("******* VD-Heat Switch  value:   %s\r\n", luxValueVDheatSwitch.c_str());
          if (luxValueVDheatSwitch.starts_with("E")) // Ein
          {
            luxValueVDheatSwitch = "1";
          }
          else
          {
            luxValueVDheatSwitch = "0";
          }
      }
      else
      {
          debug_println("*********** VD-Heat value not Valid");
      }

      
      xml_pos1 = luxRxString.index_of("Freq. aktuell</name><value>") + 27; // Verdichter Frequenz RPM
      xml_pos2 = luxRxString.index_of("</value>", xml_pos1) -4;
      bRet = luxRxString.substring(luxValueVDrpm, xml_pos1, xml_pos2-xml_pos1);
      if (bRet > 0)
      {
          //debug_printf("******* VD Freq. aktuell value:   %s\r\n", luxValueVDrpm.c_str());
      } 
      else
      {
          debug_println("*********** VD-Freq value not Valid");
      }


      xml_pos1 = luxRxString.index_of("HUP</name><value>", luxRxString.index_of("ZIP</name><value>")) + 17; // Heizungs-Pumpe PWM  !! tricky: 1. "HUP" String ignorieren !!
      xml_pos2 = luxRxString.index_of("</value>", xml_pos1) -2;
      bRet = luxRxString.substring(luxValueHUPpwm, xml_pos1, xml_pos2-xml_pos1);
      if (bRet > 0)
      {
          //debug_printf("******* HUP Pump PWM value:   %s\r\n", luxValueHUPpwm.c_str());
      } 
      else
      {
          debug_println("*********** HUP Pump PMW value not Valid");
      }
    }

    /*
    <item id='0x91008c'><name>Anlagenstatus</name>
     <item id='0x9affd4'><name>Wärmepumpen Typ</name><value>LP8V</value></item>
     <item id='0x9b0004'><name>Wärmepumpen Typ</name><value>HMDc</value></item>
     <item id='0x9b0034'><name>Hydraulikcode</name><value>000306211</value></item>
     <item id='0x9b00e4'><name>Softwarestand</name><value>V3.89.5</value></item>
     <item id='0x9b0114'><name>Revision</name><value>11451</value></item>
     <item id='0x9b0194'><name>HZ/IO</name><value><![CDATA[<div class='boardInfoLine'><span>  Revision:</span><span>9393</span></div><div class='boardInfoLine'><span>  CPU:</span><span>3</span></div>]]></value></item>
     <item id='0x9b0374'><name>ASB</name><value><![CDATA[<div class='boardInfoLine'><span>  Revision:</span><span>9562</span></div><div class='boardInfoLine'><span>  HW Revision:</span><span>15</span></div>]]></value></item>
     <item id='0x9b05e4'><name>Bedienteil</name><value><![CDATA[<div class='boardInfoLine'><span>  Revision:</span><span>2003</span></div>]]></value></item>
     <item id='0x9b06e4'><name>Inverter SW Version</name><value>52</value></item>
     <item id='0x9b075c'><name>Bivalenz Stufe</name><value>1</value></item>
     <item id='0x9b0794'><name>Betriebszustand</name><value></value></item>
     <item id='0x9b07d4'><name>Heizleistung Ist</name><value>0.00 kW</value></item>  xx (new)
     <item id='xxxxxxxx'><name>Leistungsaufnahme</name>value>0.00 kW</value></item>  xx (V3.90.2 NEW)
     <item id='0x9b098c'><name>Abtaubedarf</name><value>0.0%</value></item>
     <item id='0x9b09c4'><name>Letzte Abt.</name><value>---</value></item>
   <name>Anlagenstatus</name></item>
   */
    
    if (luxRxString.index_of("Anlagenstatus") > 0)  // Anlagenstatus
    {
      //debug_println("Anlagenstatus");
      bRet = luxRxString.substringBeetween(luxValuePowerOut, LUX_XML_DEF_POWER_OUT, sizeof(LUX_XML_DEF_POWER_OUT)-1, "</value>", -3);
      if (bRet)
      {
          //debug_printf("******* PowerOut value:%s\r\n", luxValuePowerOut.c_str());
          float f = String(luxValuePowerOut.c_str()).toFloat();
          if (f > 0)
          {
            if (luxValueWWSwitch.starts_with("0"))
            {
             hp_state = HEATING;
            }
            else
            {
              hp_state = WW;
            }

          }
          else
          if (f < 0)
          {
             hp_state = DEFROST;
          }
          else 
          {
            hp_state = INAKTIVE;
          }
      }
      else 
      { debug_println("*********** xmlValue not Valid");}

#ifndef Luxtronik_V3_89
      xml_pos1 = luxRxString.index_of(LUX_XML_DEF_POWER_IN, luxRxString.index_of("<name>Anlagenstatus")) +17;  // "BUP" = WW Ventil 0/1
      xml_pos2 = luxRxString.index_of("</value>", xml_pos1) -3 ;
      bRet = luxRxString.substring(luxValuePowerIn, xml_pos1, xml_pos2-xml_pos1);
      if (bRet)
      {
         // debug_println()
      }
      else
      { debug_println("******* xmlValue PowerIn not valid");}
      
      bRet = luxRxString.substringBeetween(luxValueDefrostPercent, LUX_XML_DEF_DEFROST_PERCENT, sizeof(LUX_XML_DEF_DEFROST_PERCENT)-1, "</value>", -2);
      if (bRet)
      {}
      else
      { debug_println("********* xmlValue DefrostPercent not valid")};
#endif        
    }
    //debug_println("Anlagenstatus");
     

    /*
    <item id='0x9b0cc4'><name>Wärmemenge</name>
      <item id='0x9c2324'><name>Heizung</name><value>12.5 kWh</value></item>
      <item id='0x9c23fc'><name>Warmwasser</name><value>60.1 kWh</value></item>
      <item id='0x9c26dc'><name>Gesamt</name><value>72.6 kWh</value></item>
     <name>Wärmemenge</name>
    */
    if (luxRxString.index_of("rmemenge") > 0)  // Wärmemenge
    {
      //debug_println("Waermemenge");
      xml_pos1 = luxRxString.index_of("Heizung</name><value>", luxRxString.index_of("rmemenge")) +21;
      xml_pos2 = luxRxString.index_of("</value>", xml_pos1) -4;
      //debug_printf("----------POS1WMHEIZ:%d, POS2WMHEIZ:%d\r\n", xml_pos1, xml_pos2);
      bRet = luxRxString.substring(luxValueEnergyHEOut, xml_pos1, xml_pos2-xml_pos1);
      if (bRet > 0)
      {
          //debug_printf("******* Energy Heiz Out value:   %s\r\n", luxValueEnergyHEOut.c_str());
           debug_printluxval(GETVARNAME(LUX_ENERGY_OUT_HEAT), luxValueEnergyHEOut.c_str());
      }
      else
      {
          debug_println("*********** xmlValue not Valid");
      }
      xml_pos1 = luxRxString.index_of("Warmwasser</name><value>", luxRxString.index_of("rmemenge")) +24;
      xml_pos2 = luxRxString.index_of("</value>", xml_pos1) -4;
      //debug_printf("----------POS1WMHEIZ:%d, POS2WMHEIZ:%d\r\n", xml_pos1, xml_pos2);
      bRet = luxRxString.substring(luxValueEnergyWWOut, xml_pos1, xml_pos2-xml_pos1);
      if (bRet > 0)
      {
          debug_printluxval(GETVARNAME(LUX_ENERGY_OUT_WW), luxValueEnergyWWOut.c_str());
          //debug_printf("******* Energy WW Out value:    %s\r\n", luxValueEnergyWWOut.c_str());
      }
      else
      {utoa (0, valbuffer, 10);
          debug_println("*********** xmlValue not Valid");
      }

      xml_pos1 = luxRxString.index_of("Gesamt</name><value>", luxRxString.index_of("rmemenge")) + 20;
      xml_pos2 = luxRxString.index_of("</value>", xml_pos1) -4;
      //debug_printf("----------POS1WMHEIZ:%d, POS2WMHEIZ:%d\r\n", xml_pos1, xml_pos2);
      bRet = luxRxString.substring(luxValueEnergySumOut, xml_pos1, xml_pos2-xml_pos1);
      if (bRet > 0)
      {
          debug_printluxval(GETVARNAME(LUX_ENERGY_OUT_SUM), luxValueEnergySumOut.c_str());
          //debug_printf("******* Energy Sum Out value:    %s\r\n", luxValueEnergySumOut.c_str());
      }
      else
      {
          debug_println("*********** xmlValue not Valid");
      }
    }

    /*
     </item><item id='0x9b0c2c'><name>Eingesetzte Energie</name>
       <item id='0x9c20ac'><name>Heizung</name><value>3.5 kWh</value></item>
       <item id='0x9c2b0c'><name>Warmwasser</name><value>20.9 kWh</value></item>
       <item id='0x9c341c'><name>Gesamt</name><value>24.4 kWh</value></item>
     <name>Eingesetzte Energie</name></item>


     neu:

   
    <item id='0xa50964'><name>Leistungsaufnahme</name>
      <item id='0xa56adc'><name>Heizung</name><value>6.1 kWh</value></item>
      <item id='0xa56b9c'><name>Warmwasser</name><value>61.7 kWh</value></item>
      <item id='0xa56e7c'><name>Gesamt</name><value>67.8 kWh</value></item>
    <name>Leistungsaufnahme</name></item>
 
    */
    xml_pos0 = luxRxString.index_of(LUX_XMLDEF_ENERGY_IN, luxRxString.index_of("rmemenge"));
    if (xml_pos0 > 0)  // 'Leistungsaufnahme'  alt(< Luxtronik V2.90): 'Eingesetzte Energie'
    {
      //xml_pos1 = luxRxString.index_of("rmemenge");
      //debug_println(LUX_XMLDEF_ENERGY_IN);
      xml_pos1 = luxRxString.index_of("Heizung</name><value>", xml_pos0) +21;
      xml_pos2 = luxRxString.index_of("</value>", xml_pos1) -4;
      //debug_printf("----------POS1EnergyHeizIn:%d, POS2EnergyHeizIn:%d\r\n", xml_pos1, xml_pos2);
      bRet = luxRxString.substring(luxValueEnergyHEIn, xml_pos1, xml_pos2-xml_pos1);
      if (bRet > 0)
      {
          debug_printluxval(GETVARNAME(LUX_ENERGY_IN_HEAT), luxValueEnergyHEIn.c_str());
          //debug_printf("******* Power In-Heiz value: %s\r\n", luxValueEnergyHEIn.c_str());
      }
      else
      {
          debug_println("*********** xmlValue not Valid");
      }
      xml_pos1 = luxRxString.index_of("Warmwasser</name><value>", xml_pos0) +24;
      xml_pos2 = luxRxString.index_of("</value>", xml_pos1) -4;
      //debug_printf("----------POS1EnergyWWIn:%d, POS2EnergyWWIn:%d\r\n", xml_pos1, xml_pos2);
      bRet = luxRxString.substring(luxValueEnergyWWIn, xml_pos1, xml_pos2-xml_pos1);
      if (bRet > 0)
      {
          //debug_printf("******* Energy In-WW value:   %s\r\n", luxValueEnergyWWIn.c_str());
      }
      else
      {
          debug_println("*********** xmlValue not Valid");
      }

      xml_pos1 = luxRxString.index_of("Gesamt</name><value>", xml_pos0) + 20;
      xml_pos2 = luxRxString.index_of("</value>", xml_pos1) -4;
      //debug_printf("----------POS1WMHEIZ:%d, POS2WMHEIZ:%d\r\n", xml_pos1, xml_pos2);
      bRet = luxRxString.substring(luxValueEnergySumIn, xml_pos1, xml_pos2-xml_pos1);
      if (bRet > 0)
      {
          //debug_printf("******* Energy In-Sum value:    %s\r\n", luxValueEnergySumIn.c_str());
      }
      else
      {
          debug_println("*********** xmlValue not Valid");
      }

      // todo: brauch ich das hier (von altem Code übernommen)
      //getEnergyValues(false);
    }
    else
    {
      debug_println("*********** xmlValues 'Leistungsaufnahme' not found\r\n");
    }

    //poll_state = SEND_REFRESH; geht so noch nicht !!!
    //poll_state = SEND_LOGIN; 
    poll_state = SEND_GET_INFO;
    break;
  default:
    break;
  }
} 


void LuxWebsocket::poll()
{  
    switch (poll_state)
    {
    case OPEN_CONNECTION:
       debug_println("luxdata.poll_status= OPEN_CONNECTION");
       wsConnect();
       break;
    case SEND_LOGIN:
        debug_println("[Lux] Send: 'LOGIN;999999'");
       _wsClient.sendTXT("LOGIN;999999");
       break;
    case SEND_GET_INFO:
       luxTxString = "GET;";
       luxTxString += luxIdString;
       debug_printf("[Lux] Send: 'GET;%s'\r\n",luxIdString.c_str());
       _wsClient.sendTXT(luxTxString,luxTxString.length());
       break;
     case SEND_REFRESH:
       _wsClient.sendTXT("REFRESH"); // not used !!
       break; 
    case CLOSE_CONNECTION:
        _wsClient.disconnect();
    case NO_POLLING:
        //AsyncWebLog.println("Luxtronik: NO_POLLING"); // activated in "luxtronik.html" Webpage
        break;
    default:
      break;
    }

    calcEnergyValues(false);
    calcCopDay();
}








