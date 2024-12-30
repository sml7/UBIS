#pragma once
/*************************************************************
  A system to handle door state change events.
*************************************************************/

//===========================================================
// forward declared dependencies
struct RoomLoadSystem;

//===========================================================
// Pin Definitons
#define magSwitchPin 18          // magnetic switch pin
#define redLEDPin 22             // door closed status LED pin
#define greenLEDPin 19           // door opened status LED pin
#define buzzerPin 23             // buzzer pin

//===========================================================
// Data Types
struct RoomLoadSystem;

/**
 * Manages the door state by providing methods to detect door state change events and
 * signalling of those.
 */
struct DoorStatusSystem {
  bool doorOpen = false;                  //Current door state
  bool lastDoorOpen = false;              //Last door state
  unsigned long lastNotificationTime = 0; // For timer control
  const unsigned long notificationInterval = 1000; // 1 second

  void doDoorStatusCheck(RoomLoadSystem &loadSys, bool online = false);
  void updateEntranceStatusLEDs(bool ent);
};

#include "door_status_sys_inline.h"