//===========================================================
// included dependencies
#include "room_load_sys.h"
#include "Arduino.h"

//===========================================================
// Inline member function implementations

/**
 * If the outer light barrier is passed.
 * @return
 * -true: If so
 * -false: otherwise
 */
inline bool RoomLoadSystem::isPassingOuter() {
  if(!digitalRead(outerLBarrPin) && digitalRead(innerLBarrPin)) 
    return true;
  return false;
}

/**
 * If the inner light barrier is passed.
 * @return
 * -true: If so
 * -false: otherwise
 */
inline bool RoomLoadSystem::isPassingInner() {
  if(digitalRead(outerLBarrPin) && !digitalRead(innerLBarrPin)) 
    return true;
  return false;
}

/**
 * If both light barriers are passed.
 * @return
 * -true: If so
 * -false: otherwise
 */
inline bool RoomLoadSystem::isPassingBoth() {
  if(!digitalRead(outerLBarrPin) && !digitalRead(innerLBarrPin)) 
    return true;
  return false;
}

/**
 * If no light barrier is passed.
 * @return
 * -true: If so
 * -false: otherwise
 */
inline bool RoomLoadSystem::noPassing() {
  if(digitalRead(outerLBarrPin) && digitalRead(innerLBarrPin)) 
    return true;
  return false;
}