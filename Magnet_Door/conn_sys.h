#pragma once
/*************************************************************
  A system to manage connections to server and WiFi.
*************************************************************/

//===========================================================
// included dependencies
#include <WiFi.h>
#include <WiFiClient.h>
#include "Arduino.h"

//===========================================================
// Definitons
#define CONN_TIMEOUT 20000        //Timeout until the system tries to reconnect.

//===========================================================
// Pin Definitons
#define connButtonPin 4          // the connection button pin
#define connLEDPin 5             // Connection status LED pin

//===========================================================
// Data Types

/**
 * Represents the current configured WiFi credentials.
 */
struct WifiCredentials{
  String ssid;                  // The configured WiFi SSID
  String pass;                  // The configured password        
};

/**
 * Provides methods for connection establishement, their configuration and status displaying
 * of server and WiFi connections.
 */
struct ConnectionSystem {
  hw_timer_t * connStatusTimer = NULL; //Hardware timer to handle connection status LED update task
  unsigned long lastConnUnpressed = 0; //Records the last time the connection button was pressed
  WifiCredentials wifiCred;            //Saves current wifi credentials
  unsigned long lastTimeOnline = 0; //Record of last time the system was online.

  void startConnLEDBlink();
  void endConnLEDBlink();
  wl_status_t connectWiFi(const char* ssid, const char* pass);
  bool doWifiConfig();
};

