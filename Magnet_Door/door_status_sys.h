#pragma once
/*************************************************************
  A system to handle door state change events.
*************************************************************/

//===========================================================
// included dependencies
#include <cstdint>
#include <functional>

//===========================================================
// forward declared dependencies
class RoomLoadSystem;

//===========================================================
// Data Types

/**
 * The events the door status system can register.
 */
enum DoorStatusEvent: uint8_t {
  doorOpened,                //< If the door was opened.
  doorClosed,                //< If the door was closed.
  PersonsInRoom              //< If the door was closed, but there were still persons in the room.
};

/**
 * Manages the door state by providing methods to detect door state change events and
 * signalling of those via status LEDs and sound through a buzzer.
 */
class DoorStatusSystem {
  private:
    unsigned long lastDoorEventTime = 0;          //< For registering last time a door event happened.
    const unsigned long DoorEventInterval = 1000; //< The time interval between two door events can happen in milli seconds.
    bool doorOpen = false;                        //< Current door state
    const uint8_t openLEDPin;                     //< The door open status LED pin.
    const uint8_t closedLEDPin;                   //< The door closed status LED pin.
    const uint8_t magSwitchPin;                   //< The magnatic switch pin.
    const uint8_t buzzerPin;                      //< The buzzer pin.

    /**
     * Performs an acoustic signalling for door state change events.
     */
    void doDoorStateChangedAcousticSignal() const;

  public:
    /**
     * Constructs a DoorStatusSystem with the used hardware pins.
     * @param openLEDPin The door open status LED pin.
     * @param closedLEDPin The door closed status LED pin.
     * @param magSwitchPin The magnatic switch pin.
     * @param buzzerPin The buzzer pin.
     */
    DoorStatusSystem( uint8_t openLEDPin, 
                      uint8_t closedLEDPin, 
                      uint8_t magSwitchPin, 
                      uint8_t buzzerPin);
    
    /**
     * Returns the current door state.
     * @return
     *  -true: Door is open.
     *  -false: otherwise.
     */
    bool isDoorOpen() const;

    /**
     * Determines the current door state by reading the magnatic switch input
     * and registeres door state change events. Updates status LEDs accordingly 
     * and performs acoustic signalling on door state change events.
     * Also checks if persons are still in the room if the door was closed.
     * Calls the callback on door closed and door opened events.
     * @param passSys A reference to the door passing system.
     * @param eventCallback A callback which will be called if a door status event was registered.
     */
    void doDoorStatusCheck(RoomLoadSystem& loadSys, std::function<void (DoorStatusEvent)> eventCallback);

    /**
    * Sets the state of the door status LEDs to signal if someone can enter or not.
    * @param ent If entering is allowed or not.
    */
    void setStatusLEDs(bool ent) const;
};

#include "door_status_sys_inline.h"