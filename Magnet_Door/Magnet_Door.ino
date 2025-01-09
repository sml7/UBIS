/*************************************************************
  The main system will be setup and executed here.
*************************************************************/

//===========================================================
// included dependencies
#include "entrance_control_sys.h"
#include "comm_sys.h"
#include "system_config.h"
#include <WiFiClient.h>
#include <HTTPClient.h>

//===========================================================
// Globals
EntranceControlSystem* mainCtrlSys; //< The main control system.
CommunicationSystem* commSys;       //< The communication system.
WiFiClient client;
HTTPClient http;

//===========================================================
// Function implementations

void setup() {
  Serial.begin(115200); //Initialize Serial
  delay(1000); //Wait for serial to become ready
  Serial.println("-----------Program started-----------");

  commSys = new CommunicationSystem(CONN_BUTTON_PIN, 
                                    CONN_LED_PIN);

  mainCtrlSys = new EntranceControlSystem( *commSys,
                                           OPENED_LED_PIN, 
                                           CLOSED_LED_PIN, 
                                           MAG_SWITCH_PIN, 
                                           BUZZER_PIN,
                                           OUTER_DET_PIN, 
                                           INNER_DET_PIN);
     
  mainCtrlSys->reset();
}

void loop() {
  mainCtrlSys->run();
}
