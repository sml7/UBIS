//===========================================================
// included dependencies
#include "room_load_sys.h"
#include "Arduino.h"

//===========================================================
// Inline member function implementations

/**
 * Gives the room capacity.
 * @return room capacity
 */
inline uint8_t RoomLoadSystem::getRoomCap() const {
  return roomCap;
}

/**
 * Sets the room capacity.
 * @param val The room capacity which should be set.
 */
inline void RoomLoadSystem::setRoomCap(uint8_t val) {
  roomCap = val;
}

/**
 * Returns whether room is full or not.
 * @param
 *  -true: on room full
 *  -false: otherwise
 */
inline bool RoomLoadSystem::isRoomFull() const {
  return roomFull;
}

/**
 * Returns the current person count in the room.
 * @return person count
 */
inline uint8_t RoomLoadSystem::getPersonCount() const {
  return personCount;
}

/**
 * If the outer detector is passed.
 * @return
 * -true: If so
 * -false: otherwise
 */
inline bool RoomLoadSystem::isPassingOuter() const {
  if(!digitalRead(outerDetPin) && digitalRead(innerDetPin)) 
    return true;
  return false;
}

/**
 * If the inner detector is passed.
 * @return
 * -true: If so
 * -false: otherwise
 */
inline bool RoomLoadSystem::isPassingInner() const {
  if(digitalRead(outerDetPin) && !digitalRead(innerDetPin)) 
    return true;
  return false;
}

/**
 * If both detector are passed.
 * @return
 * -true: If so
 * -false: otherwise
 */
inline bool RoomLoadSystem::isPassingBoth() const {
  if(!digitalRead(outerDetPin) && !digitalRead(innerDetPin)) 
    return true;
  return false;
}

/**
 * If no detector is passed.
 * @return
 * -true: If so
 * -false: otherwise
 */
inline bool RoomLoadSystem::noPassing() const {
  if(digitalRead(outerDetPin) && digitalRead(innerDetPin)) 
    return true;
  return false;
}