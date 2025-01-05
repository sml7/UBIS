#pragma once
/*************************************************************
  A system to manage the current room load.
*************************************************************/

//===========================================================
// included dependencies
#include <cstdint>
#include <functional>

//===========================================================
// Included forward dependencies
class DoorStatusSystem;
enum class PassState: uint8_t;

//===========================================================
// Definitions
#define ROOM_CAP_DEFAULT 5

//===========================================================
// Data Types

/**
 * The events the room load system can register.
 */
enum class RoomLoadEvent: uint8_t {
  roomFull,                    //< If the room capacity was reached, so there is no room for more persons.
  roomNotFull                  //< If there is room for persons again.
};

/**
 * Manages the person load in a room.
 * Providing the functionality to detect entering and leaving events by
 * using a two detectors. 
 * With two detectors the direction of the door passing can be determined.
 */
class RoomLoadSystem {
  private:
    const uint8_t outerDetPin;            //< The inner detector pin.
    const uint8_t innerDetPin;            //< The outer detector pin.
    PassState passState;                  //< Current door passing state
    uint8_t personCount = 0;              //< number of persons
    bool roomFull = false;                //< If number of persons inside the room has reached the maximum.
    uint8_t roomCap = ROOM_CAP_DEFAULT;   //< Capacity of the room

  public:

    /**
     * Constructs a RoomLoadSystem with the used hardware pins.
     * @param outerDetPin The outer detector pin.
     * @param innerDetPin The inner detector pin.
     */
    RoomLoadSystem(uint8_t outerDetPin, uint8_t innerDetPin);

    /**
     * Gives the room capacity.
     * @return room capacity
     */
    uint8_t getRoomCap() const;

    /**
     * Sets the room capacity.
     * @param val The room capacity which should be set.
     */
    void setRoomCap(uint8_t val);

    /**
     * Returns whether room is full or not.
     * @param
     *  -true: on room full
     *  -false: otherwise
     */
    bool isRoomFull() const;

    /**
     * Returns the current person count in the room.
     * @return person count
     */
    uint8_t getPersonCount() const;

    /**
     * Brings the system back in initial state.
     */
    void reset();

    /**
    * If the outer detector is passed.
    * @return
    * -true: If so
    * -false: otherwise
    */
    bool isPassingOuter() const;

    /**
    * If the inner detector is passed.
    * @return
    * -true: If so
    * -false: otherwise
    */
    bool isPassingInner() const;

    /**
    * If both detector are passed.
    * @return
    * -true: If so
    * -false: otherwise
    */
    bool isPassingBoth() const;

    /**
    * If no detector is passed.
    * @return
    * -true: If so
    * -false: otherwise
    */
    bool noPassing() const;

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
    void doDoorPassingCheck(DoorStatusSystem& doorSys, std::function<void (RoomLoadEvent)> eventCallback);
};

#include "room_load_sys_inline.h"