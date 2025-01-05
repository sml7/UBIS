/*************************************************************
  The implementation of a system to handle door state change events.
*************************************************************/

//===========================================================
// included dependencies
#include "door_status_sys.h"
#include "room_load_sys.h"
#include "Arduino.h"

//===========================================================
// Member function implementations

/**
 * Constructs a DoorStatusSystem with the used hardware pins.
 * @param openLEDPin The door open status LED pin.
 * @param closedLEDPin The door closed status LED pin.
 * @param magSwitchPin The magnatic switch pin.
 * @param buzzerPin The buzzer pin.
 */
DoorStatusSystem::DoorStatusSystem( uint8_t openLEDPin, 
                                    uint8_t closedLEDPin, 
                                    uint8_t magSwitchPin, 
                                    uint8_t buzzerPin): openLEDPin(openLEDPin),
                                                        closedLEDPin(closedLEDPin),
                                                        magSwitchPin(magSwitchPin),
                                                        buzzerPin(buzzerPin) {
  //Setup pins
  pinMode(openLEDPin, OUTPUT);
  pinMode(closedLEDPin, OUTPUT);
  pinMode(magSwitchPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  //Set initial LED state
  setStatusLEDs(false);
}

/**
 * Determines the current door state by reading the magnatic switch input
 * and registeres door state change events. Updates status LEDs accordingly 
 * and performs acoustic signalling on door state change events.
 * Also checks if persons are still in the room if the door was closed.
 * Calls the callback on door closed and door opened events.
 * @param passSys A reference to the door passing system.
 * @param eventCallback A callback which will be called if a door status event was registered.
 */
void DoorStatusSystem::doDoorStatusCheck(RoomLoadSystem& loadSys, std::function<void (DoorStatusEvent)> eventCallback) {
  bool doorReading = digitalRead(magSwitchPin);

  // Check if sensor value is below threshold to indicate door is opened
  if (doorOpen != doorReading && (millis() - lastDoorEventTime >= DoorEventInterval)) {
    doorOpen = doorReading;
    if(doorOpen) {
      Serial.println("Door state change event: opened");
      eventCallback(DoorStatusEvent::doorOpened); //Register the event

      if(!loadSys.isRoomFull()) {
        setStatusLEDs(true);
      }

      //acoustic signal
      doDoorStateChangedAcousticSignal();
    }
    else {
      Serial.println("Door state change event: closed");
      eventCallback(DoorStatusEvent::doorClosed); //Register the event

      if(loadSys.getPersonCount() > 0) {
        Serial.println("Alert: There are still persons in the room!");
        eventCallback(DoorStatusEvent::PersonsInRoom); //Register the event
      }

      setStatusLEDs(false);
      doDoorStateChangedAcousticSignal();
    }
    // Update the time of the last door event
    lastDoorEventTime = millis();
  }
}

/**
 * Performs an acoustic signalling for door state change events.
 */
void DoorStatusSystem::doDoorStateChangedAcousticSignal() const {
  tone(buzzerPin, 100);
  delay(1000);
  noTone(buzzerPin);
}