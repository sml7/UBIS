#pragma once
/*************************************************************
  Implementation of command parsing and execution behavior.
*************************************************************/

//===========================================================
// included dependencies
#include "commands.h"
#include "entrance_control_sys.h"

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
  if(indexTo == -1) { 
    //In case no further parameters
    if(cmdStr == "Connect") {
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
      String subCmd = cmdStr.substring(indexFrom, indexTo);
      if(subCmd == "Config") {
        indexFrom = indexTo + 1;
        indexTo = cmdStr.indexOf(indexTo);
        if(indexTo == -1) {
          subCmd = cmdStr.substring(indexFrom);
          cmd = new ArgCommand<long int>(CMD_CONF_ROOM_CAP, cmdStr, subCmd.toInt());
          return true;
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
  switch(cmd.type) {
    case CMD_CONN:
      if(!entCtrlSys.isOnline()) {
        entCtrlSys.doConnect();
      }
      else {
        Serial.print("Info: Command unnecessary: ");
        Serial.println(cmd.cmdStr);
        Serial.println("  >> Reason: Already in Online mode.");
      }
      return true;
    case CMD_DISCONN:
      if(entCtrlSys.isOnline()) {
        entCtrlSys.doDisconnect();
      }
      else {
        Serial.print("Info: Command unnecessary: ");
        Serial.println(cmd.cmdStr);
        Serial.println("  >> Reason: Already in Offline mode.");
      }
      return true;
    case CMD_CONF_WIFI:
      entCtrlSys.configWifi();
      return true;
    case CMD_CONF_ROOM_CAP: {
      long int arg = static_cast<const ArgCommand<long int>&>(cmd).arg;
      if(arg > 0 && arg <= 255) {
        entCtrlSys.configRoomCap(arg);
        return true;
      }
      else {
        Serial.println("Error: Parameter out of bounds. Should be between 0 and 255.");
        return false;
      }
    }
    case CMD_RESET_WIFI:
      entCtrlSys.resetWifiConfig();
      return true;
    case CMD_RESET:
      entCtrlSys.restoreFactorySettings();
      return true;
    default:
      return false;
  }
}