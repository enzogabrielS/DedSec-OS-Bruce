#ifndef __AP_INFO_H__
#define __AP_INFO_H__
#ifdef DEDSEC_OS
#include <WiFi.h>
String autoMode2String(wifi_auth_mode_t authMode);
#endif

void displayAPInfo();

#endif
