/*************************************************************
  The implementation of a system to manage the current room load.
*************************************************************/

#define NO_GLOBAL_BLYNK 1   //To prevent multiple definitions of blynk

//===========================================================
// included dependencies
#include "room_load_sys.h"
#include "door_status_sys.h"
#include "blynk_credentials.h"
#include <BlynkSimpleEsp32.h>

//===========================================================
// Member function implementations

/**
 * Determines the current door passing state by reading the light barrier inputs
 * and registeres entering events. 
 * Updates status LEDs accordingly on room full and room not full respectively.
 * Also checks if persons are still in the room if the door was closed.
 * Logs room full, room not full and persons in the room events to the server if in online mode.
 * Assumes that only one person can pass the door at the same time.
 * @param statSys A reference to the door status system.
 * @param online If set to true, will be in online mode.
 */
void RoomLoadSystem::doDoorPassingCheck(DoorStatusSystem &statSys, bool online) {
  switch(passState) {
    case Idle:
      if(isPassingOuter()) {
        //Someone starts to enter
        passState = StartEntering;
      }
      else if(isPassingInner()) {
        //Someone starts to leave
        passState = StartLeaving;
      }
      break;
    case StartEntering:
     if(isPassingBoth()) {
      //Someone who enters steped further in and now passes both barriers at the same time
      passState = Entering1;
     }
     else if(noPassing()) {
      //Someone decided to step back from entering
      passState = Idle;
     }
     else if(isPassingInner()) {
      //Passing only the inner barrier is not allowed in this state.
      Serial.println("Error: During entering event!");
      Serial.println("  >> Reason: Only inner barrier is passed, but both barriers were not passed before.");
      Serial.println("  >> Result: Could not detect entering correctly. Falling back to idle.");
      passState = Idle;
     }
     break;
    case Entering1:
      if(isPassingInner()) {
        //Someone who enters steped further in and now passes only the inner barrier
        passState = Entering2;
      }
      else if(isPassingOuter()) {
        //Someone decided to step back
        passState = StartEntering;
      }
      else if(noPassing()) {
        //No passing of any barrier is not allowed in this state.
        Serial.println("Error: During entering event!");
        Serial.println("  >> Reason: Both barriers were passed, but now no barrier is passed.");
        Serial.println("  >> Result: Could not detect entering correctly. Falling back to idle.");
        passState = Idle;
      }
      break;
    case Entering2:
     if(noPassing()) {
      //Someone finished entering
      passState = Entered;
     }
     else if(isPassingBoth()) {
      //Someone decided to step back
      passState = Entering1;
     }
     else if(isPassingOuter()) {
      //Passing only the outer barrier is not allowed in this state.
      Serial.println("Error: During entering event!");
      Serial.println("  >> Reason: Someone passed the inner barrier, but now only the outer barrier is passed.");
      Serial.println("  >> Result: Could not detect entering correctly. Falling back to idle.");
      passState = Idle;
     }
     break;
    case Entered:
      if(personCount < roomCap) {
        personCount++;
        Serial.println("Passing event: Someone entered.");
        Serial.print("  >> Current load: ");
        Serial.println(personCount);
      }
      else {
        Serial.println("Error: During entering event!");
        Serial.println("  >> Reason: Room is already full.");
        Serial.println("  >> Result: Falling back to idle.");
      }
      passState = Idle;
      break;
    case StartLeaving:
      if(isPassingBoth()) {
        //Someone who is leaving steped further out and now passes both barriers at the same time
        passState = Leaving1;
      }
      else if(noPassing()) {
        //Someone decided to step back from leaving
        passState = Idle;
      }
      else if(isPassingOuter()) {
        //Passing only the outer barrier is not allowed in this state.
        Serial.println("Error: During leaving event!");
        Serial.println("  >> Reason: Only outer barrier is passed, but both barriers were not passed before.");
        Serial.println("  >> Result: Could not detect leaving correctly. Falling back to idle.");
        passState = Idle;
      }
      break;
    case Leaving1:
      if(isPassingOuter()) {
        //Someone who is leaving steped further out and now passes only the outer barrier
        passState = Leaving2;
      }
      else if(isPassingInner()) {
        //Someone decided to step back
        passState = StartLeaving;
      }
      else if(noPassing()) {
        //No passing of any barrier is not allowed in this state.
        Serial.println("Error: During leaving event!");
        Serial.println("  >> Reason: Both barriers were passed, but now no barrier is passed.");
        Serial.println("  >> Result: Could not detect leaving correctly. Falling back to idle.");
        passState = Idle;
      }
      break;
    case Leaving2:
      if(noPassing()) {
        //Someone finished leaving
        passState = Left;
      }
      else if(isPassingBoth()) {
        //Someone decided to step back
        passState = Leaving1;
      }
      else if(isPassingInner()) {
        //Passing only the inner barrier is not allowed in this state.
        Serial.println("Error: During leaving event!");
        Serial.println("  >> Reason: Someone passed the outer barrier, but now only the inner barrier is passed.");
        Serial.println("  >> Result: Could not detect entering correctly. Falling back to idle.");
        passState = Idle;
      }
      break;
    case Left:
      if(!(personCount == 0)) {
      personCount--;
        Serial.println("Passing event: Someone left.");
        Serial.print("  >> Current load: ");
        Serial.println(personCount);
      }
      else {
        Serial.println("Error: During leaving event!");
        Serial.println("  >> Reason: Room was already empty.");
        Serial.println("  >> Result: Falling back to idle.");
      }
      passState = Idle;
      break;
  }

  if(!roomFull && personCount >= roomCap) {
    Serial.println("Alert: Room is full.");
    roomFull = true;
    if(online) {
        // Trigger the event for notification in Blynk
        Blynk.logEvent("room_full", "Alert: Room is full now.");
    }
    statSys.updateEntranceStatusLEDs(false);
  }
  else if(roomFull && personCount < roomCap) {
    Serial.println("Info: Room is no longer full.");
    roomFull = false;
    if(online) {
        // Trigger the event for notification in Blynk
        Blynk.logEvent("room_not_full", "Info: Room is no longer full.");
    }
    statSys.updateEntranceStatusLEDs(true);
  }
}