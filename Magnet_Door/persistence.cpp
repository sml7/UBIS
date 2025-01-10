/*************************************************************
  The implementation of the memory management.
*************************************************************/

//===========================================================
// included dependencies
#include "persistence.h"
#include "comm_sys.h"

//===========================================================
// Function implementations

/**
 * Initilizes memory with a certain size.
 */
void initMemory() {
  EEPROM.begin(EEPROM_SIZE);
}

/**
 * Loads the WiFi configuration from the flash memory.
 * @param wifiCred The WiFi configuration to be loaded.
 * @return Number of the loaded bytes.
 */
unsigned int loadWifiConfig(WifiCredentials &wifiCred) {
  char readSSID[SSID_MAX_SIZE]; //read buffer for ssid
  unsigned int bytesRead = EEPROM.readString(WIFI_START_ADRR,
                                             readSSID,
                                             SSID_MAX_SIZE - 1);
  wifiCred.ssid = String(readSSID);

  char readPass[PASS_MAX_SIZE]; //read buffer for pass
  bytesRead += EEPROM.readString(SSID_MAX_SIZE,
                                 readPass,
                                 PASS_MAX_SIZE - 1);
  wifiCred.pass = String(readPass);
  return bytesRead;
}

/**
 * Stores the WiFi configuration into the flash memory.
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

/**
 * Deletes the WiFi configuration in the flash memory.
 */
bool deleteWifiConfig() {
  for (int i = WIFI_START_ADRR; i < WIFI_CONFIG_SIZE; i++) {
    EEPROM.write(i, 0);
  }
  return EEPROM.commit();
}

/**
 * Loads the room capacity configuration from the flash memory.
 * @param count The value to be loaded.
 * @return The loaded max person count.
 */
uint8_t loadRoomCapConfig() {
  return EEPROM.readByte(ROOM_CAP_START_ADRR);
}

/**
 * Stores the room capacity configuration into the flash memory.
 * @param count The value to be stored.
 * @return 
 * -true: On success.
 * -false: otherwise.
 */
bool storeRoomCapConfig(uint8_t count) {
  EEPROM.writeByte(ROOM_CAP_START_ADRR,count);
  return EEPROM.commit();
}

/**
 * Loads the server url from the flash memory.
 * @param serverUrl The sever url to be loaded.
 * @return Number of the loaded bytes.
 */
unsigned int loadServerUrlConfig(String& serverUrl) {
  char readUrl[SERVER_URL_MAX_SIZE]; //read buffer
  unsigned int bytesRead = EEPROM.readString(SERVER_URL_START_ADDR, 
                                             readUrl, 
                                             SERVER_URL_MAX_SIZE - 1);
  serverUrl = readUrl;
  return bytesRead;
}

/**
 * Stores the sever url into the flash memory.
 * @param serverUrl The sever url to be saved.
 * @return 
 * -true: On success.
 * -false: otherwise.
 */
bool storeServerUrlConfig(const String& serverUrl) {
  EEPROM.writeString(SERVER_URL_START_ADDR, serverUrl);
  return EEPROM.commit();
}


/**
 * Erases the complete flash memory.
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