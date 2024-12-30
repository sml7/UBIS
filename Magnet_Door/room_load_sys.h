#pragma once
/*************************************************************
  A system to manage the current room load.
*************************************************************/

#include <cstdint>

//===========================================================
// included dependencies
struct DoorStatusSystem;

#define ROOM_CAP_DEFAULT 5

//===========================================================
// Pin Definitons
#define innerLBarrPin 16         // inner light barrier pin
#define outerLBarrPin 17         // outer light barrier pin

//===========================================================
// Data Types

/**
 * Represents the states the door passing FSM can be in.
 */
enum PassState {
  Idle,
  StartEntering,
  Entering1,
  Entering2,
  Entered,
  StartLeaving,
  Leaving1,
  Leaving2,
  Left
};

/**
 * Manages the room load by providing methods to detect entering and leaving events.
 */
struct RoomLoadSystem {
  PassState passState;        //Current door passing state
  uint8_t personCount = 0;    //number of persons
  bool roomFull = false;      //If number of persons inside the room has reached the maximum.
  uint8_t roomCap = ROOM_CAP_DEFAULT;        //Capacity of the room

  bool isPassingOuter();
  bool isPassingInner();
  bool isPassingBoth();
  bool noPassing();
  void doDoorPassingCheck(DoorStatusSystem &statSys, bool online = false);
};

#include "room_load_sys_inline.h"