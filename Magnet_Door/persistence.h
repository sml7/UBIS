#pragma once
/*************************************************************
  Manages access to memory.
*************************************************************/

//===========================================================
// included dependencies
#include "EEPROM.h"

//===========================================================
// forward declared dependencies
struct WifiCredentials;

//===========================================================
// Definitons

#define WIFI_START_ADRR 0
#define SSID_MAX_SIZE 256
#define PASS_MAX_SIZE 256
#define WIFI_CONFIG_SIZE (SSID_MAX_SIZE+PASS_MAX_SIZE)
#define ROOM_CAP_START_ADRR WIFI_CONFIG_SIZE
#define ROOM_CAP_SIZE 1
#define SERVER_URL_MAX_SIZE 256
#define SERVER_URL_START_ADDR (WIFI_CONFIG_SIZE+ROOM_CAP_SIZE)
#define EEPROM_SIZE (WIFI_CONFIG_SIZE+ROOM_CAP_SIZE+SERVER_URL_MAX_SIZE)

//===========================================================
// Function Declarations

/**
 * Initilizes memory with a certain size.
 */
void initMemory();

/**
 * Loads the WiFi configuration from the flash memory.
 * @param wifiCred The WiFi configuration to be loaded.
 * @return Number of the loaded bytes.
 */
unsigned int loadWifiConfig(WifiCredentials& wifiCred);

/**
 * Stores the WiFi configuration into the flash memory.
 * @param wifiCred The WiFi configuration to be saved.
 * @return 
 * -true: On success.
 * -false: otherwise.
 */
bool storeWifiConfig(const WifiCredentials& wifiCred);

/**
 * Deletes the WiFi configuration in the flash memory.
 */
bool deleteWifiConfig();

/**
 * Loads the room capacity configuration from the flash memory.
 * @param count The value to be loaded.
 * @return The loaded max person count.
 */
uint8_t loadRoomCapConfig();

/**
 * Stores the room capacity configuration into the flash memory.
 * @param count The value to be stored.
 * @return 
 * -true: On success.
 * -false: otherwise.
 */
bool storeRoomCapConfig(uint8_t count);

/**
 * Loads the server url from the flash memory.
 * @param serverUrl The sever url to be loaded.
 * @return Number of the loaded bytes.
 */
unsigned int loadServerUrlConfig(String& serverUrl);

/**
 * Stores the sever url into the flash memory.
 * @param serverUrl The sever url to be saved.
 * @return 
 * -true: On success.
 * -false: otherwise.
 */
bool storeServerUrlConfig(const String& serverUrl);

/**
 * Erases the complete flash memory.
 * @return 
 * -true: On success.
 * -false: otherwise.
 */
bool clearMemory();