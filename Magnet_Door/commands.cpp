#pragma once
/*************************************************************
  Implementation of command parsing and execution behavior.
*************************************************************/

#define NO_GLOBAL_BLYNK 1   //To prevent multiple definitions of blynk

//===========================================================
// included dependencies
#include "commands.h"
#include "WString.h"
#include "entrance_control_sys.h"
#include "blynk_credentials.h"
#include <BlynkSimpleEsp32.h>
#include "persistence.h"

//===========================================================
// Function implementations

/**
 * Tries to pars a given command.
 * @param cmdStr The command string which should be parsed.
 * @param[out] cmd The command which was parsed.
 * @return 
 * -true: If command was parsed successfully.
 * -false: otherwise.
 */
bool parseCommand(const String &cmdStr, Command *&cmd) {
  int indexFrom = 0;
  int indexTo = 0;
  indexTo = cmdStr.indexOf(" ");
  Serial.print("Parsing Command: ");
  Serial.println(cmdStr);
  if(indexTo == -1) { 
    //In case no further parameters
    if(cmdStr == "Connect") {
      Serial.println(cmdStr);
      cmd = new Command(CMD_CONN, cmdStr); return true;
    }
    else if(cmdStr == "Disconnect") {
      cmd = new Command(CMD_DISCONN, cmdStr); return true;
    }
    else if(cmdStr == "Reset") {
      cmd = new Command(CMD_RESET, cmdStr); return true;
    }
  }
  else {
    if(cmdStr == "Reset Wifi") {
      cmd = new Command(CMD_RESET_WIFI, cmdStr); return true;
    }
    else if(cmdStr == "Config Wifi") {
      cmd = new Command(CMD_CONF_WIFI, cmdStr); return true;
    }
    else {
      String subCmd;
      subCmd = cmdStr.substring(indexFrom, indexTo - 1);
      if(subCmd == "Config") {
        indexFrom = indexTo + 1;
        indexTo = cmdStr.indexOf(indexTo);
        if(indexTo == -1) {
          subCmd = cmdStr.substring(indexFrom);
          cmd = new ArgCommand<long int>(CMD_CONF_ROOM_CAP, cmdStr, subCmd.toInt());
        }
      }
    }
  }

  Serial.println("Error: Invalid command!");
  return false;
}

/**
 * Executes a given command.
 * Implements the command functionality
 * @param system A reference to the main entrance control system.
 * @param cmd The command which should be executed.
 * @return
 *   -true: If a command was executed successfully.
 *   -false: otherwise.
 */
bool executeCommand(EntranceControlSystem &entCtrlSys, const Command &cmd) {
  ConnectionSystem &connSys = entCtrlSys.subSys.connSys;
  RoomLoadSystem &roomLoadSys = entCtrlSys.subSys.roomLoadSys;

  switch(cmd.type) {
    case CMD_CONN:
      if(entCtrlSys.state != Online) {
        entCtrlSys.state = Connect; return true;
      }
      else {
        Serial.print("Info: Command unnecessary: ");
        Serial.println(cmd.cmdStr);
        Serial.println("  >> Reason: Already in Online mode.");
      }
      return true;
    case CMD_DISCONN:
      if(entCtrlSys.state != Offline) {
        Blynk.disconnect();
        Serial.println("-----------Going offline-----------");
        digitalWrite(connLEDPin, HIGH); //update connection status led
        entCtrlSys.state = Offline; return true;
      }
      else {
        Serial.print("Info: Command unnecessary: ");
        Serial.println(cmd.cmdStr);
        Serial.println("  >> Reason: Already in Offline mode.");
      }
      return true;
    case CMD_CONF_WIFI:
      entCtrlSys.state = Config; return true;
    case CMD_CONF_ROOM_CAP:
      if(0 < static_cast<const ArgCommand<long int>&>(cmd).arg <= 255) {
        roomLoadSys.roomCap = static_cast<const ArgCommand<long int>&>(cmd).arg;
        return true;
      }
      else {
        Serial.println("Error: Parameter out of bounds. Should be between 0 and 255.");
        return false;
      }
    case CMD_RESET_WIFI:
      if(entCtrlSys.resetWifiConfig()) { //Try to delete config
        Serial.println("  >> WiFi credentials successfully resetted.");
      }
      else {
        Serial.println("Error: Failed to reset WiFi credentials!.");
      }
      return true;
    case CMD_RESET:
      if(entCtrlSys.restoreFactorySettings()) { //Try to clear memory
        Serial.println("  >> Configuration restored to factory settings.");
      }
      else {
        Serial.println("Error: Failed to restored factory settings!.");
      }
      return true;
    default:
      return false;
  }
}