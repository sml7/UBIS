//===========================================================
// included dependencies
#include "door_status_sys.h"
#include "Arduino.h"

//===========================================================
// Inline member function implementations

/**
 * Returns the current door state.
 * @return
 *  -true: Door is open.
 *  -false: otherwise.
 */
inline bool DoorStatusSystem::isDoorOpen() const {
  return doorOpen;
}

/**
 * Sets the state of the door status LEDs to signal if someone can enter or not.
 * @param ent If entering is allowed or not.
 */
inline void DoorStatusSystem::setStatusLEDs(bool ent) const {
  //The logic of the LEDs is inverted
  digitalWrite(closedLEDPin, ent);
  digitalWrite(openLEDPin, !ent);
}