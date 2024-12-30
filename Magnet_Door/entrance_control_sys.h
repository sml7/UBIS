#pragma once
/*************************************************************
  A system to handle the access of an entrance.
*************************************************************/

//===========================================================
// included dependencies
#include "conn_sys.h"
#include "room_load_sys.h"
#include "door_status_sys.h"

//===========================================================
// Data Types

/**
 * Represents the states the system FSM can be in.
 */
enum SystemState {
  Init,
  Offline,
  Config,
  Connect,
  Online,
  Reconnect
};

/**
 * Groups all subsystems of the entrance control system
 */
struct Subsystems {
  ConnectionSystem connSys;
  DoorStatusSystem doorStatSys;
  RoomLoadSystem roomLoadSys;
};

/**
 * Manages access to an entrance.
 */
struct EntranceControlSystem {
  SystemState state;    //Current state of the system FSM
  Subsystems subSys;    //All subsystems of the system

  /**
   * Resets the system to factory Settings.
   * @return 
   *  -true: On success.
   *  -false: otherwise.
   */
  bool restoreFactorySettings();

  /**
   * Resets the WiFi configuration of the system.
   * @return 
   *  -true: On success.
   *  -false: otherwise.
   */
  bool resetWifiConfig();
};