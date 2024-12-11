
#include "EEPROM.h"

#define WIFI_START_ADRR 0
#define SSID_MAX_SIZE 256
#define PASS_MAX_SIZE 256
#define EEPROM_SIZE (SSID_MAX_SIZE+PASS_MAX_SIZE)

void initMemory() {
  EEPROM.begin(EEPROM_SIZE);
}

/*
* Loads the wifi configuration from the memory
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
* Stores the wifi configuration into the memory
*/
bool storeWifiConfig(WifiCredentials &wifiCred) {
  EEPROM.writeString(WIFI_START_ADRR,wifiCred.ssid);
  EEPROM.writeString(SSID_MAX_SIZE,wifiCred.pass);
  return EEPROM.commit();
}

void clearMemory() {
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
}