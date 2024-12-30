/*************************************************************
  Implementation of the system control.
*************************************************************/

//===========================================================
// Definitons

/* Uncomment this line to enable Serial debug prints */
#define BLYNK_PRINT Serial

//===========================================================
// included dependencies
#include "entrance_control_sys.h"
#include "persistence.h"
#include "serial_access.h"
#include "commands.h"
#include "blynk_credentials.h"
#include <BlynkSimpleEsp32.h>

//===========================================================
// Globals

EntranceControlSystem mainSys;

//===========================================================
// Static function declarations

/**
 * Processes commands inputted over serial terminal.
 * @param system A reference to the main entrance control system.
 * @return
 *   -true: If a command was processed successfully.
 *   -false: otherwise.
 */
static bool processCommand(EntranceControlSystem &entCtrlSys);

/**
 * The main routine of the entrance control system.
 * @param online If set to true, will be in online mode.
 */
static void doMainRoutine(bool online = false);


//===========================================================
// Static function implementations
static bool processCommand(EntranceControlSystem &entCtrlSys) {
  String cmdStr;
  Command *command = nullptr;

  if(readStringFromSerial(cmdStr)) { //Check serial input
    if(parseCommand(cmdStr, command)) {
      if(executeCommand(entCtrlSys, *command)) {
        return true;
      }
    }
    Serial.print("Error: Command could not be processed: ");
    Serial.println(cmdStr);
  }
  return false;
}

static void doMainRoutine(bool online) {
  ConnectionSystem &connSys = mainSys.subSys.connSys;
  DoorStatusSystem &doorStatSys = mainSys.subSys.doorStatSys;
  RoomLoadSystem &roomLoadSys = mainSys.subSys.roomLoadSys;

  if(!processCommand(mainSys)) { //Check if command is inputted and process it
    // //In case no command to process
    if(digitalRead(connButtonPin) == HIGH) {//Check if connection button pressed
      //If not pressed
      connSys.lastConnUnpressed = millis();
    }
    else {
      //If pressed
      if(millis() - connSys.lastConnUnpressed > 500) { //Checks if button is pressed for at least 0.5 seconds
        connSys.lastConnUnpressed = millis();
        if(online) {
          Serial.println("-----------Going offline-----------");
          mainSys.state = Offline;
        }
        else {
          mainSys.state = Connect;
        }
      }
    }
    doorStatSys.doDoorStatusCheck(roomLoadSys, online);
    if(doorStatSys.doorOpen) {
      roomLoadSys.doDoorPassingCheck(doorStatSys, online);
    }
  }
}

//===========================================================
// Function implementations

void setup() {
  Serial.begin(115200); //Initialize Serial
  delay(1000); //Wait for serial to become ready
  Serial.println("-----------Program started-----------");
  initMemory();

  //Set inital mainSys state
  mainSys.state = Offline;

  //Setup pins
  pinMode(magSwitchPin, INPUT);
  pinMode(outerLBarrPin, INPUT);
  pinMode(innerLBarrPin, INPUT);
  pinMode(connButtonPin, INPUT);
  pinMode(redLEDPin, OUTPUT);
  pinMode(greenLEDPin, OUTPUT);
  pinMode(connLEDPin, OUTPUT);

  ConnectionSystem &connSys = mainSys.subSys.connSys;
  DoorStatusSystem &doorStatSys = mainSys.subSys.doorStatSys;
  RoomLoadSystem &roomLoadSys = mainSys.subSys.roomLoadSys;
  
  //Set initial LED state
  doorStatSys.updateEntranceStatusLEDs(false);
  digitalWrite(connLEDPin, HIGH);

  //load currently saved config
  loadWifiConfig(connSys.wifiCred);
  roomLoadSys.roomCap = loadRoomCapConfig();
}

void loop() {
  ConnectionSystem &connSys = mainSys.subSys.connSys;
  
  //Determine next state of the system FSM
  switch(mainSys.state) {
    case Offline:
      doMainRoutine();
      break;
    case Config:
      Serial.println("-----------WiFi Configuration-----------");
      if(connSys.doWifiConfig()) {
        Serial.println("  >> WiFi credentials stored successfully.");
        loadWifiConfig(connSys.wifiCred); //load new config
        mainSys.state = Connect;
      }
      else {
        Serial.println("Error: Storing of WiFi credentials failed!");
        mainSys.state = Offline;
      }
      break;
    case Connect:
      if(!connSys.wifiCred.pass.isEmpty() && !connSys.wifiCred.ssid.isEmpty()) {
        Serial.println("-----------Connecting to WiFi and Blynk-----------");
        connSys.startConnLEDBlink();
        wl_status_t wifiStatus = connSys.connectWiFi(connSys.wifiCred.ssid.c_str(), connSys.wifiCred.pass.c_str());
        if(wifiStatus == WL_CONNECTED) {
          Blynk.config(BLYNK_AUTH_TOKEN);
          if(Blynk.connect(CONN_TIMEOUT)) {
            //Successfully connected
            Serial.println("-----------Going online-----------");
            connSys.endConnLEDBlink();
            digitalWrite(connLEDPin, LOW); //update connection status led
            connSys.lastTimeOnline = millis();
            mainSys.state = Online;
            break;
          }
          else {
            //Failed to connect to server
            Serial.println("Alert: Connection to server failed.");
          }
        }
        else {
          //Failed to connect to WiFi
          Serial.println("Alert: Connection to WiFi failed.");
          if(wifiStatus == WL_NO_SSID_AVAIL) {
            Serial.println("  >> Reason: SSID not available.");
          }
          else if(wifiStatus == WL_DISCONNECTED) {
            Serial.println("  >> Reason: Authentication failed.");
          }
          else {
            Serial.print("  >> Reason: WiFi status code: ");
            Serial.println(wifiStatus);
          }
        }
        Serial.println("  >> Result: Falling back to offline mode.");
        Serial.println("-----------Going offline-----------");
        mainSys.state = Offline;
        connSys.endConnLEDBlink();
        digitalWrite(connLEDPin, HIGH); //update connection status led
        break;
      }
      else {
        Serial.println("Warning: No WiFi configuration set yet.");
        Serial.println("  >> Do you want to configure? (Y/n)");
        String answer;
        do {
          if(readStringFromSerial(answer)) {
            if(answer == "" || answer == "Y" || answer == "y") { //Yes is default answer
              Serial.println("  >> Answer: Yes");
              mainSys.state = Config; 
              break;
            }
            else if(answer == "N" || answer == "n") {
              Serial.println("  >> Answer: No");
              Serial.println("-----------Going offline-----------");
              digitalWrite(connLEDPin, 0x01); //update connection status led
              mainSys.state = Offline;
              break;
            }
            else {
              Serial.println("Error: This is not the right answer! Try again.");
            }
          }
        }
        while(true); //Waits until answer was given
      }
      break;
    case Online:
      if(Blynk.run()) {
        //Connected
        connSys.lastTimeOnline = millis();
        doMainRoutine(true);
      }
      else {
        //Connection lost. Try to reconnect.
        Serial.println("Alert: Connection lost. Try to reconnect.");
        connSys.startConnLEDBlink();
        mainSys.state = Reconnect;
      }
      break;
    case Reconnect:
      if(millis() - connSys.lastTimeOnline <= CONN_TIMEOUT) {
        if(Blynk.connected()) {
          //Connection reestablished
          Serial.println("Info: Connection reestablished.");
          connSys.lastTimeOnline = millis();
          mainSys.state = Online;
          connSys.endConnLEDBlink();
          digitalWrite(connLEDPin, LOW); //update connection status led
        }
        else {
          //Connection still lost. Try to reconnect.
          Blynk.run();
          doMainRoutine(true);
        }
      }
      else {
        //Connection still lost after timeout. Falling back to offline mode.
        Serial.println("Alert: Connection Timout. Falling back to offline mode.");
        Serial.println("-----------Going offline-----------");
        mainSys.state = Offline;
        connSys.endConnLEDBlink();
        digitalWrite(connLEDPin, HIGH); //update connection status led
      }
      break;
  }
}
