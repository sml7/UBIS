/*************************************************************
  The implementation of a system to handle the access of an entrance.
*************************************************************/

//===========================================================
// included dependencies
#include "entrance_control_sys.h"
#include "persistence.h"
#include "commands.h"
#include "serial_access.h"
#include <ArduinoJson.h>

//===========================================================
// Data Types

/**
 * Represents the states the system FSM can be in.
 */
enum class EntranceControlState: uint8_t {
  offline,          //< Offline mode
  online,           //< Online mode
  reconnect         //< Connection lost. Communication System tries to reconnect.
};


//===========================================================
// Static function implementations

/**
 * Handler for room load events.
 * @param e The event to be handled.
 * @param commSys Reference to the communication system.
 */
inline static void roomLoadEventHandler(RoomLoadEvent e, CommunicationSystem& commSys) {
    switch(e) {
    case RoomLoadEvent::roomFull:
      commSys.logEvent("room_full", "Alert: Room is full now.");
      break;
    case RoomLoadEvent::roomNotFull:
      commSys.logEvent("room_not_full", "Info: Room is no longer full.");
      break;
    }
}

/**
 * Handler for door status events.
 * @param e The event to be handled.
 * @param commSys Reference to the communication system.
 */
inline static void doorStatusEventHandler(DoorStatusEvent e, CommunicationSystem& commSys) {
  switch(e) {
    case DoorStatusEvent::doorClosed:
      commSys.logEvent("door_closed", "Info: Room was closed.");
      break;
    case DoorStatusEvent::doorOpened:
      commSys.logEvent("door_opened", "Info: You can come in.");
      break;
    case DoorStatusEvent::PersonsInRoom:
      commSys.logEvent("persons_in_room", "Alert: There are still persons in the room!");
      break;
  }
}

//===========================================================
// Member function implementations

/**
 * Constructs a EntranceControlSystem with the used hardware pins.
 * @param commSys Reference to the used communication system.
 * @param openLEDPin The door open status LED pin.
 * @param closedLEDPin The door closed status LED pin.
 * @param magSwitchPin The magnatic switch pin.
 * @param outerDetPin The outer detector pin.
 * @param innerDetPin The inner detector pin.
 */
EntranceControlSystem::EntranceControlSystem( CommunicationSystem& commSys,
                                              uint8_t openLEDPin, 
                                              uint8_t closedLEDPin, 
                                              uint8_t magSwitchPin, 
                                              uint8_t buzzerPin,
                                              uint8_t outerDetPin, 
                                              uint8_t innerDetPin): state(EntranceControlState::offline),
                                                                    commSys(commSys),
                                                                    doorSys(openLEDPin, closedLEDPin, magSwitchPin, buzzerPin),
                                                                    roomLoadSys(outerDetPin, innerDetPin) {
  initMemory();                       
}

/**
 * Sets whether verbose status messages should be printed.
 * Can be used for debugging.
 * @param val Verbose value to be set.
 */
void EntranceControlSystem::activateVerboseMessaging(bool val) {
  commSys.setPrintStatus(val);
  if(val) {
    Serial.println(" >> Verbose Messaging activated.");
  }
  else {
    Serial.println(" >> Verbose Messaging deactivated.");
  }
}

/**
 * Executes the system state machine.
 */
void EntranceControlSystem::run() {
  //Determine next state of the system FSM
  switch(state) {
    case EntranceControlState::offline: {
      doMainRoutine();
      ConnectionStatus status = commSys.run();
      switch(status) {
        case ConnectionStatus::connectionRequest:
          doConnect();
          break;
        case ConnectionStatus::connected:
          Serial.println("-----------Going online-----------");
          state = EntranceControlState::online;
          break;
        case ConnectionStatus::connectionLost:
          Serial.println("-----------Going online-----------");
          Serial.println(" >> Info: Connection lost! Try to reconnect.");
          state = EntranceControlState::reconnect;
          break;
      }
      break;
    }
    case EntranceControlState::online: {
      doMainRoutine();
      ConnectionStatus status = commSys.run();
      switch(status) {
        case ConnectionStatus::connectionTimeout:
          Serial.println("Alert: Connection Timout. Falling back to offline mode.");
          Serial.println("-----------Going offline-----------");
          state = EntranceControlState::offline;
          break;
        case ConnectionStatus::connectionLost:
          Serial.println(" >> Info: Connection lost! Try to reconnect.");
          state = EntranceControlState::reconnect;
          break;
        case ConnectionStatus::disconnected:
          Serial.println("-----------Going offline-----------");
          state = EntranceControlState::offline;
          break;
      }
      break;
    }
    case EntranceControlState::reconnect: {
      doMainRoutine();
      ConnectionStatus status = commSys.run();
      switch(status) {
        case ConnectionStatus::connectionTimeout:
          Serial.println("Alert: Connection Timout. Falling back to offline mode.");
          Serial.println("-----------Going offline-----------");
          state = EntranceControlState::offline;
          break;
        case ConnectionStatus::connected:
          Serial.println("Info: Connection reestablished.");
          Serial.println("-----------Back online-----------");
          state = EntranceControlState::online;
          break;
        case ConnectionStatus::disconnected:
          Serial.println("-----------Going offline-----------");
          state = EntranceControlState::offline;
          break;
      }
      break;
    }
  }
}

/**
 * Returns the online state of the used communication system.
 * @return
 *  -true: If oline.
 *  -false: otherwise.
 */
bool EntranceControlSystem::isOnline() const {
  return commSys.isOnline();
}

/**
 * Tells the communication system to connect WiFi and server.
 */
void EntranceControlSystem::doConnect() const {
  if(!wifiCred.ssid.isEmpty()) {
    Serial.println("-----------Connecting to WiFi and Server-----------");
    ConnectionStatus connStatus = commSys.connect(wifiCred);
    switch(connStatus) {
      case ConnectionStatus::connected:
        //Successfully connected
        Serial.println(" >> Connection established.");
        return;
      case ConnectionStatus::connectionTimeout:
        Serial.println("Alert: Connection to server failed.");
        break;
      case ConnectionStatus::noSSIDAvail:
        Serial.println("Alert: Connection to WiFi failed.");
        Serial.println("  >> Reason: SSID not available.");
        break;
      case ConnectionStatus::wifiAuthFailed:
        Serial.println("Alert: Connection to WiFi failed.");
        Serial.println("  >> Reason: Authentication failed.");
        break;
      default:
        Serial.println("Alert: Connection to WiFi failed.");
        Serial.println("  >> Reason: Unknown.");
        break;
    }
  //Connecting failed
  Serial.println("  >> Result: Falling back to offline mode.");
  }
  else {
    Serial.println("Error: Connecting failed.");
    Serial.println(" >> Reason: No WiFi SSID configured yet.");
    Serial.println(" >> Please configure a WiFi SSID first.");
  }
}

/**
 * Tells the communication system to disconnect from the server.
 */
void EntranceControlSystem::doDisconnect() const {
  commSys.disconnect();
}

/**
 * The main routine of the entrance control system.
 * Performing processing of commands, doing door status and passing checks
 * and doing communication tasks using the communication system.
 * @param online If set to true, will be in online mode.
 */
void EntranceControlSystem::doMainRoutine() {
  if(!processCommand()) { //Check if command is inputted and process it
    //In case no command to process
    doorSys.doDoorStatusCheck(roomLoadSys, [this](DoorStatusEvent e) {doorStatusEventHandler(e, commSys);});
    if(doorSys.isDoorOpen()) {
      roomLoadSys.doDoorPassingCheck(doorSys, [this](RoomLoadEvent e) {roomLoadEventHandler(e, commSys);});
    }
    sendSensorData();
  }
}

/**
 * Brings the system back in initial state.
 */
void EntranceControlSystem::reset() {
  //Set inital state
  state = EntranceControlState::offline;

  //load currently saved config
  loadConfig();

  //Reset all controlled systems
  commSys.reset();
  roomLoadSys.reset();
  doorSys.setStatusLEDs(false); //resetting of door status LEDs
}

/**
 * Configures and saves the new room capacity into flash memory.
 * @param val Room capacity value which should be configured.
 * @return 
 *  -true: On success.
 *  -false: otherwise.
 */
bool EntranceControlSystem::configRoomCap(uint8_t val) {
  if(storeRoomCapConfig(val)) { //Try to set the room capacity
    roomLoadSys.setRoomCap(val);
  }
  else {
    Serial.println("Error: Failed set room capacity!");
    return false;
  }
  Serial.print(" >> Successfully set room capacity to: ");
  Serial.println(val);
  return true;
}

/**
 * Configures and saves the new server URL into flash memory.
 * @param val Server URL value which should be configured.
 * @return 
 *  -true: On success.
 *  -false: otherwise.
 */
bool EntranceControlSystem::configServerUrl(const String& val) {
  if(val.length() > SERVER_URL_MAX_SIZE) {
    Serial.printf("Error: URL is to long! The maximum allowed length is %d characters.\n", 
                  SERVER_URL_MAX_SIZE);
    return false;
  }
  if(storeServerUrlConfig(val)) { //Try to set the room capacity
    commSys.setServerUrl(val);
  }
  else {
    Serial.println("Error: Failed to set server url!");
    return false;
  }
  Serial.print(" >> Successfully set server url to: ");
  Serial.println(val);
  return true;
}

/**
 * Performs a WiFi configuration over serial terminal.
 * Stores the new configuration into flash memory.
 * @return 
 * -true: If configuration could be successfully stored.
 * -false: otherwise.
 */
bool EntranceControlSystem::configWifi() {
  Serial.println("-----------WiFi Configuration-----------");
  //Reading SSID from terminal
  inputStringFromSerial(
      wifiCred.ssid, 
      SSID_MAX_SIZE, 
      "Enter WiFi SSID:");

  //Reading password from terminal
  inputStringFromSerial(
      wifiCred.pass, 
      PASS_MAX_SIZE, 
      "Enter WiFi password:");

  if(storeWifiConfig(wifiCred)) { //Try to store the WiFi configuration
    Serial.println(" >> WiFi configuration successfully stored.");
    return true;
  }
  Serial.println("Error: Failed to store WiFi configuration!");
  return false;
}

/**
 * Loads the system configuration from flash memory.
 */
inline void EntranceControlSystem::loadConfig() {
  loadWifiConfig(wifiCred);
  roomLoadSys.setRoomCap(loadRoomCapConfig());
}

/**
 * Helper method to processes commands inputted over serial terminal.
 * @return
 *   -true: If a command was processed successfully.
 *   -false: otherwise.
 */
inline bool EntranceControlSystem::processCommand() {
  String cmdStr;
  Command *command = nullptr;

  if(readStringFromSerial(cmdStr)) { //Check serial input
    if(parseCommand(cmdStr, command)) {
      if(executeCommand(*this, *command)) {
        return true;
      }
    }
    Serial.print("Error: Command could not be processed: ");
    Serial.println(cmdStr);
  }
  if(command) {
    delete command;
  }
  return false;
}

void EntranceControlSystem::sendSensorData() {
  // Allocate the JSON document
  JsonDocument doc;
  if(millis() - lastPost >= postInterval) {
    // Add values in the document
    lastPost = millis();
    // doc["time"] = String(lastPost);
    doc["room"] = "Conference";
    // doc["door_state"] = String(doorSys.isDoorOpen());
    doc["people_count"] = String(roomLoadSys.getPersonCount());
    // doc["temperature"] = "23";

    String jsonData; //Data to be send
    serializeJson(doc, jsonData);
    commSys.sendData(jsonData);
  }
}

/**
 * Resets the system to factory Settings.
 * @return 
 *  -true: On success.
 *  -false: otherwise.
 */
bool EntranceControlSystem::restoreFactorySettings() {
  Serial.println(" >> Restore factory settings.");
  resetWifiConfig();
  bool success = true;
  if(storeRoomCapConfig(ROOM_CAP_DEFAULT)) { //Try to set the room capacity to default value
    roomLoadSys.setRoomCap(ROOM_CAP_DEFAULT);
    Serial.println(" >> Room capacity successfuly restored to default value.");
  }
  else {
    Serial.println("Error: Failed restore room capacity to default value!");
    success = false;
  }
  if(!success) {
    Serial.println("Error: Failed to restored factory settings!");
    return false;
  }
  reset();
  Serial.println(" >> Configuration successfully restored to factory settings.");
  return true; 
}

/**
 * Resets the WiFi configuration of the system.
 * @return 
 *  -true: On success.
 *  -false: otherwise.
 */
bool EntranceControlSystem::resetWifiConfig() {
  if(deleteWifiConfig()) { //Try to delete config
    loadWifiConfig(wifiCred); //update WiFi configuration
    Serial.println(" >> WiFi configuration successfully resetted.");
    return true;
  }
  Serial.println("Error: Failed to reset WiFi configuration!");
  return false;
}