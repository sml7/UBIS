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
 * Tries to parse a given command.
 * @param cmdStr The command string which should be parsed.
 * @param[out] cmd The command which was parsed.
 * @return 
 * -true: If command was parsed successfully.
 * -false: otherwise.
 */
bool parseCommand(const String &cmdStr, Command *&cmd) {
  int indexFrom = 0;
  int indexTo = 0;
  bool done = false;
  indexTo = cmdStr.indexOf(" "); //search for space
  if(indexTo == -1) { //No space found
    //In case no further parameters
    if(cmdStr == "Connect") {
      cmd = new Command(CommandType::connect, cmdStr); done = true;
    }
    else if(cmdStr == "Disconnect") {
      cmd = new Command(CommandType::disconnect, cmdStr); done = true;
    }
    else if(cmdStr == "Reset") {
      cmd = new Command(CommandType::reset, cmdStr); done = true;
    }
  }
  else { //If there is a space
    if(cmdStr == "Reset Wifi") {
      cmd = new Command(CommandType::resetWifi, cmdStr); done = true;
    }
    if(cmdStr == "Show Config") {
      cmd = new Command(CommandType::showConfig, cmdStr); done = true;
    }
    else if(cmdStr == "Config Wifi") {
      cmd = new Command(CommandType::confWifi, cmdStr); done = true;
    }
    else { //If it can be a command with parameters
      String subCmd = cmdStr.substring(indexFrom, indexTo);
      if(subCmd == "Config") {
        indexFrom = indexTo + 1;
        indexTo = cmdStr.indexOf(" ", indexFrom);
        if(indexTo != -1) { //If there was a space found, read next token
          String subCmd = cmdStr.substring(indexFrom, indexTo);
          if(subCmd == "RoomCap") {
            indexFrom = indexTo + 1;
            indexTo = cmdStr.indexOf(" ", indexFrom);
            //Check if there follows something after expected parameter
            if(indexTo == -1) {
              subCmd = cmdStr.substring(indexFrom); //Read parameter
              cmd = new ArgCommand<long int>(CommandType::confRoomCap, cmdStr, subCmd.toInt());
              done = true;
            }
          }
          else if(subCmd == "Verbose") {
            indexFrom = indexTo + 1;
            indexTo = cmdStr.indexOf(" ", indexFrom);
            //Check if there follows something after expected parameter
            if(indexTo == -1) {
              subCmd = cmdStr.substring(indexFrom); //Read parameter
              if(subCmd == "true") {
                cmd = new ArgCommand<bool>(CommandType::confVerbose, cmdStr, true);
                done = true;
              }
              else if(subCmd == "false") {
                cmd = new ArgCommand<bool>(CommandType::confVerbose, cmdStr, false);
                done = true;
              }
            }
          }
          else if(subCmd == "ServerUrl") {
            indexFrom = indexTo + 1;
            indexTo = cmdStr.indexOf(" ", indexFrom);
            //Check if there follows something after expected parameter
            if(indexTo == -1) {
              subCmd = cmdStr.substring(indexFrom); //Read parameter
              cmd = new ArgCommand<String>(CommandType::confServerUrl, cmdStr, subCmd);
              done = true;
            }
          }
        }
      }
    }
  }
  //Check if it was a valid command
  if(done) {
    if(cmd) {
      return true;
    }
    else {
      Serial.println("Error: Command parsing failed!");
      Serial.println(" >> Reason: Could not allocate memory for command!");
    }
  } 
  else {
    Serial.println("Error: Invalid command!");
  }
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
    case CommandType::connect:
      if(!entCtrlSys.isOnline()) {
        entCtrlSys.doConnect();
      }
      else {
        Serial.print("Info: Command unnecessary: ");
        Serial.println(cmd.cmdStr);
        Serial.println("  >> Reason: Already in Online mode.");
      }
      return true;
    case CommandType::disconnect:
      if(entCtrlSys.isOnline()) {
        entCtrlSys.doDisconnect();
      }
      else {
        Serial.print("Info: Command unnecessary: ");
        Serial.println(cmd.cmdStr);
        Serial.println("  >> Reason: Already in Offline mode.");
      }
      return true;
    case CommandType::confWifi:
      entCtrlSys.configWifi();
      return true;
    case CommandType::confRoomCap: {
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
    case CommandType::confVerbose: {
      bool arg = static_cast<const ArgCommand<bool>&>(cmd).arg;
      entCtrlSys.activateVerboseMessaging(arg);
      return true;
    }
    case CommandType::confServerUrl: {
      const String& arg = static_cast<const ArgCommand<String>&>(cmd).arg;
      entCtrlSys.configServerUrl(arg);
      return true;
    }
    case CommandType::showConfig:
      entCtrlSys.printConfig();
      return true;
    case CommandType::resetWifi:
      entCtrlSys.resetWifiConfig();
      return true;
    case CommandType::reset:
      entCtrlSys.restoreFactorySettings();
      return true;
    default:
      return false;
  }
}