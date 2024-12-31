/*************************************************************
  The implementation of a system to handle the access of an entrance.
*************************************************************/

//===========================================================
// included dependencies
#include "entrance_control_sys.h"
#include "persistence.h"

//===========================================================
// Member function implementations

/**
 * Configures and saves the new room capacity.
 * @param val Room capacity value which should be configured.
 * @return 
 *  -true: On success.
 *  -false: otherwise.
 */
bool EntranceControlSystem::configRoomCap(uint8_t val) {
  if(storeRoomCapConfig(val)) { //Try to set the room capacity
    subSys.roomLoadSys.roomCap = val;
  }
  else {
    Serial.println("Error: Failed set room capacity!.");
    return false;
  }
  
  return true;
}

/**
 * Resets the system to factory Settings.
 * @return 
 *  -true: On success.
 *  -false: otherwise.
 */
bool EntranceControlSystem::restoreFactorySettings() {
  if(deleteWifiConfig()) { //Try to reset wifi config
    loadWifiConfig(subSys.connSys.wifiCred); //update WiFi configuration
  }
  else {
    Serial.println("Error: Failed reset WiFi configuration!.");
    return false;
  }  
  
  if(storeRoomCapConfig(ROOM_CAP_DEFAULT)) { //Try to set the room capacity to default value
    subSys.roomLoadSys.roomCap = ROOM_CAP_DEFAULT;
  }
  else {
    Serial.println("Error: Failed set room capacity to default value!.");
    return false;
  }
  
  return true;
}

/**
 * Resets the WiFi configuration of the system.
 * @return 
 *  -true: On success.
 *  -false: otherwise.
 */
bool EntranceControlSystem::resetWifiConfig() {
  if(deleteWifiConfig()) { //Try to delete config
    loadWifiConfig(subSys.connSys.wifiCred); //update WiFi configuration
    return true;
  }
  return false;
}