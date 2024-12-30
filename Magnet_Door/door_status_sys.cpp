/*************************************************************
  The implementation of a system to handle door state change events.
*************************************************************/

#define NO_GLOBAL_BLYNK 1     //To prevent multiple definitions of blynk

//===========================================================
// included dependencies
#include "door_status_sys.h"
#include "room_load_sys.h"
#include "Arduino.h"
#include "blynk_credentials.h"
#include <BlynkSimpleEsp32.h>

//===========================================================
// Static function implementations

/**
 * Performs an acoustic signalling for door state change events.
 */
static void doDoorStateChangedAcousticSignal() {
  tone(buzzerPin, 100);
  delay(1000);
  noTone(buzzerPin);
}


//===========================================================
// Member function implementations

/**
 * Determines the current door state by reading the magnatic switch input
 * and registeres door state change events. Updates status LEDs accordingly 
 * and performs acoustic signalling on door state change events.
 * Also checks if persons are still in the room if the door was closed.
 * Logs events to the server if in online mode.
 * @param passSys A reference to the door passing system.
 * @param online If set to true, will be in online mode.
 */
void DoorStatusSystem::doDoorStatusCheck(RoomLoadSystem &loadSys, bool online) {
  doorOpen = digitalRead(magSwitchPin);

  // Check if sensor value is below threshold to indicate door is opened
  if (lastDoorOpen != doorOpen && (millis() - lastNotificationTime >= notificationInterval)) {
    lastDoorOpen = doorOpen;
    if(doorOpen) {
      Serial.println("Door state change event: opened");

      if(online) {
        // Trigger the event for notification in Blynk
        Blynk.logEvent("door_opened", "Info: You can come in.");
      }

      if(!loadSys.roomFull) {
        updateEntranceStatusLEDs(true);
      }

      //acoustic signal
      doDoorStateChangedAcousticSignal();
    }
    else {
      Serial.println("Door state change event: closed");

      if(online) {
        // Trigger the event for notification in Blynk
        Blynk.logEvent("door_closed", "Info: Room was closed.");
      }

      if(loadSys.personCount > 0) {
        Serial.println("Alert: There are still persons in the room!");
        if(online) {
          Blynk.logEvent("persons_in_room", "Alert: There are still persons in the room!");
        }
      }

      updateEntranceStatusLEDs(false);
      doDoorStateChangedAcousticSignal();
    }
    // Update the time of the last notification
    lastNotificationTime = millis();
  }
}