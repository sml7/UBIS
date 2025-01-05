/*************************************************************
  The implementation of a system to manage the current room load.
*************************************************************/

//===========================================================
// included dependencies
#include "room_load_sys.h"
#include "door_status_sys.h"

//===========================================================
// Data Types

/**
 * Represents the states the door passing FSM can be in.
 */
enum class PassState: uint8_t {
  idle,           //< Nothing happens
  startEntering,  //< Someone starts to enter by passing the outer detector.
  entering1,      //< Someone who enters stepped further in and now passes both detectors at the same time.
  entering2,      //< Someone who enters stepped further in and now passes only the inner detector.
  entered,        //< Someone fully entered.
  startLeaving,   //< Someone starts to leave by passing the inner detector.
  leaving1,       //< Someone who leaves stepped further out and now passes both detectors at the same time.
  leaving2,       //< Someone who leaves stepped further out and now passes only the outer detector.
  left            //< Someone fully left.
};

//===========================================================
// Member function implementations

/**
 * Constructs a RoomLoadSystem with the used hardware pins.
 * @param outerDetPin The outer detector pin.
 * @param innerDetPin The inner detector pin.
 */
RoomLoadSystem::RoomLoadSystem(  uint8_t outerDetPin,
                                 uint8_t innerDetPin): outerDetPin(outerDetPin), 
                                                       innerDetPin(innerDetPin),
                                                       passState(PassState::idle) {
  //Setup pins
  pinMode(outerDetPin, INPUT);
  pinMode(innerDetPin, INPUT);
}

/**
 * Determines the current door passing state by reading the detector inputs
 * and registeres entering and leaving events. 
 * Assumes that only one person can pass the door at the same time.
 * Accesses a DoorStatusSystem to update status LEDs on room full and room not full events respectively and
 * also checks if persons are still in the room if the door was closed.
 * Calls the callback on room full, room not full and persons in the room events.
 * @param doorSys A reference to the door status system.
 * @param eventCallback A callback which will be called if a room load event was registered.
 */
void RoomLoadSystem::doDoorPassingCheck(DoorStatusSystem& doorSys, std::function<void (RoomLoadEvent)> eventCallback) {
  switch(passState) {
    case PassState::idle:
      if(isPassingOuter()) {
        //Someone starts to enter
        passState = PassState::startEntering;
      }
      else if(isPassingInner()) {
        //Someone starts to leave
        passState = PassState::startLeaving;
      }
      break;
    case PassState::startEntering:
     if(isPassingBoth()) {
      //Someone who enters stepped further in and now passes both detectors at the same time
      passState = PassState::entering1;
     }
     else if(noPassing()) {
      //Someone decided to step back from entering
      passState = PassState::idle;
     }
     else if(isPassingInner()) {
      //Passing only the inner detector is not allowed in this state.
      Serial.println("Error: During entering event!");
      Serial.println("  >> Reason: Only inner detector is passed, but both detectors were not passed before.");
      Serial.println("  >> Result: Could not detect entering correctly. Falling back to idle.");
      passState = PassState::idle;
     }
     break;
    case PassState::entering1:
      if(isPassingInner()) {
        //Someone who enters stepped further in and now passes only the inner detector
        passState = PassState::entering2;
      }
      else if(isPassingOuter()) {
        //Someone decided to step back
        passState = PassState::startEntering;
      }
      else if(noPassing()) {
        //No passing of any detector is not allowed in this state.
        Serial.println("Error: During entering event!");
        Serial.println("  >> Reason: Both detectors were passed, but now no detector is passed.");
        Serial.println("  >> Result: Could not detect entering correctly. Falling back to idle.");
        passState = PassState::idle;
      }
      break;
    case PassState::entering2:
     if(noPassing()) {
      //Someone finished entering
      passState = PassState::entered;
     }
     else if(isPassingBoth()) {
      //Someone decided to step back
      passState = PassState::entering1;
     }
     else if(isPassingOuter()) {
      //Passing only the outer detector is not allowed in this state.
      Serial.println("Error: During entering event!");
      Serial.println("  >> Reason: Someone passed the inner detector, but now only the outer detector is passed.");
      Serial.println("  >> Result: Could not detect entering correctly. Falling back to idle.");
      passState = PassState::idle;
     }
     break;
    case PassState::entered:
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
      passState = PassState::idle;
      break;
    case PassState::startLeaving:
      if(isPassingBoth()) {
        //Someone who is leaving steped further out and now passes both detectors at the same time
        passState = PassState::leaving1;
      }
      else if(noPassing()) {
        //Someone decided to step back from leaving
        passState = PassState::idle;
      }
      else if(isPassingOuter()) {
        //Passing only the outer detector is not allowed in this state.
        Serial.println("Error: During leaving event!");
        Serial.println("  >> Reason: Only outer detector is passed, but both detectors were not passed before.");
        Serial.println("  >> Result: Could not detect leaving correctly. Falling back to idle.");
        passState = PassState::idle;
      }
      break;
    case PassState::leaving1:
      if(isPassingOuter()) {
        //Someone who is leaving steped further out and now passes only the outer detector
        passState = PassState::leaving2;
      }
      else if(isPassingInner()) {
        //Someone decided to step back
        passState = PassState::startLeaving;
      }
      else if(noPassing()) {
        //No passing of any detector is not allowed in this state.
        Serial.println("Error: During leaving event!");
        Serial.println("  >> Reason: Both detectors were passed, but now no detector is passed.");
        Serial.println("  >> Result: Could not detect leaving correctly. Falling back to idle.");
        passState = PassState::idle;
      }
      break;
    case PassState::leaving2:
      if(noPassing()) {
        //Someone finished leaving
        passState = PassState::left;
      }
      else if(isPassingBoth()) {
        //Someone decided to step back
        passState = PassState::leaving1;
      }
      else if(isPassingInner()) {
        //Passing only the inner detector is not allowed in this state.
        Serial.println("Error: During leaving event!");
        Serial.println("  >> Reason: Someone passed the outer detector, but now only the inner detector is passed.");
        Serial.println("  >> Result: Could not detect entering correctly. Falling back to idle.");
        passState = PassState::idle;
      }
      break;
    case PassState::left:
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
      passState = PassState::idle;
      break;
  }

  if(!roomFull && personCount >= roomCap) {
    Serial.println("Alert: Room is full.");
    roomFull = true;
    eventCallback(RoomLoadEvent::roomFull); //Register the event
    doorSys.setStatusLEDs(false);
  }
  else if(roomFull && personCount < roomCap) {
    Serial.println("Info: Room is no longer full.");
    roomFull = false;
    eventCallback(RoomLoadEvent::roomNotFull); //Register the event
    doorSys.setStatusLEDs(true);
  }
}

/**
 * Brings the system back in initial state.
 */
void RoomLoadSystem::reset() {
  passState = PassState::idle;
}