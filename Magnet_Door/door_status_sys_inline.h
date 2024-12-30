//===========================================================
// included dependencies
#include "door_status_sys.h"
#include "Arduino.h"

//===========================================================
// Inline member function implementations

/**
 * Updates the state of the entrance status LEDs to signal if someone can enter or not.
 * @param ent If entering is allowed or not.
 */
inline void DoorStatusSystem::updateEntranceStatusLEDs(bool ent) {
  //inverted logic
  digitalWrite(redLEDPin, ent);
  digitalWrite(greenLEDPin, !ent);
}