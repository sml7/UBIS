//===========================================================
// included dependencies
#include "comm_sys.h"

//===========================================================
// Inline member function implementations

/**
 * Returns the online state of the communication system.
 * @return
 *  -true: If oline.
 *  -false: otherwise.
 */
inline bool CommunicationSystem::isOnline() const {
  return online;
}