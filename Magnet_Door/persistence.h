#pragma once
/*************************************************************
  Manages access to memory.
*************************************************************/

#include "EEPROM.h"
#include "credentials.h"

#define WIFI_START_ADRR 0
#define SSID_MAX_SIZE 256
#define PASS_MAX_SIZE 256
#define EEPROM_SIZE (SSID_MAX_SIZE+PASS_MAX_SIZE)

/*
* Initilizes memory with a certain size.
*/
void initMemory() {
  EEPROM.begin(EEPROM_SIZE);
}

/*
* Loads the WiFi configuration from the memory
* @param wifiCred The WiFi configuration to be loaded.
* @return Number of the loaded bytes.
*/
unsigned int loadWifiConfig(WifiCredentials &wifiCred) {
  char readSSID[SSID_MAX_SIZE]; //read buffer for ssid
  unsigned int bytesRead = EEPROM.readString(WIFI_START_ADRR,readSSID,SSID_MAX_SIZE - 1);
  wifiCred.ssid = String(readSSID);

  char readPass[PASS_MAX_SIZE]; //read buffer for pass
  bytesRead += EEPROM.readString(SSID_MAX_SIZE,readPass,PASS_MAX_SIZE - 1);
  wifiCred.pass = String(readPass);
  return bytesRead;
}

/*
* Stores the WiFi configuration into the memory
* @param wifiCred The WiFi configuration to be saved.
* @return 
* -true: On success.
* -false: otherwise.
*/
bool storeWifiConfig(WifiCredentials &wifiCred) {
  EEPROM.writeString(WIFI_START_ADRR,wifiCred.ssid);
  EEPROM.writeString(SSID_MAX_SIZE,wifiCred.pass);
  return EEPROM.commit();
}

/*
* Erases memory.
* @return 
* -true: On success.
* -false: otherwise.
*/
bool clearMemory() {
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
  return EEPROM.commit();
}