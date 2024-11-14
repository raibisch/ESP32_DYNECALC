#include "LuxWebsocket.h"

//----------------------------------------------------public Functions----------------------------------------------------

//default constructor
LuxWebsocket::LuxWebsocket(){}

LuxWebsocket::LuxWebsocket(const char luxurl[])
{
  _sLuxURL= luxurl;

  debug_printf("EPEX-URL: %s\r\n", _sLuxURL);
}


/*-----------------------------
destructor for class, not needed by Arduino but for complete class. Calls Arduino end function
*/
LuxWebsocket::~LuxWebsocket()
{

}


bool LuxWebsocket::init()
{
  _sLuxURL.reserve(20);
  //_sEPEXbuf.reserve(100);

  debug_println("LuxWebsocket-InitOK!");
  return true;
}


void LuxWebsocket::loop(time_t* time_now)
{
   struct tm *now_tm = localtime(time_now);

}

/* private functions */





