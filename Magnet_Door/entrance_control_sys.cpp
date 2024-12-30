/*************************************************************
  The implementation of a system to handle the access of an entrance.
*************************************************************/

//===========================================================
// included dependencies
#include "entrance_control_sys.h"
#include "persistence.h"

//===========================================================
// Member function implementations

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

bool EntranceControlSystem::resetWifiConfig() {
  if(deleteWifiConfig()) { //Try to delete config
    loadWifiConfig(subSys.connSys.wifiCred); //update WiFi configuration
    return true;
  }
  return false;
}