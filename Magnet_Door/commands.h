#pragma once
/*************************************************************
  Command processing over serial is implemented here.
*************************************************************/

#include "Arduino.h"
//===========================================================
// forward declared dependencies
struct EntranceControlSystem;

//===========================================================
// Data Types

/**
 * Defines the different types of commands of the entrance control system.
 */
enum class CommandType: uint16_t {
  confWifi,                   //< To configure wifi
  confRoomCap,                //< To configure room capacity
  connect,                    //< To establish a connection to wifi and server
  disconnect,                 //< To disconnect from server
  reset,                      //< To reset the entire system
  resetWifi,                  //< To reset the wifi configuration
  confVerbose,                //< To configure verbose status messaging
  confServerUrl,              //< To configure the url of the web server
  showConfig                  //< To show the current configuration in terminal
};

/**
 * Defines commands which can be executed to control the entrance control system.
 */
class Command {
  CommandType type;
  String cmdStr;

  public:

    /**
     * Constructs a command with its type and its string representation.
     * @param type The type of the command.
     * @param cmdStr The command as a string.
     */
    Command(CommandType type, String cmdStr): type(type), cmdStr(cmdStr) {}
    ~Command() {}
    friend bool executeCommand(EntranceControlSystem &entCtrlSys, const Command &cmd);
};

/**
 * A subtype of command with one argument.
 */
template <typename argType>
class ArgCommand: public Command {
  argType arg;

  public:
    /**
     * Constructs a command with its type, its string representation and its argument.
     * @param type The type of the command.
     * @param cmdStr The command as a string.
     * @param arg The argument of the command.
     */
    ArgCommand(CommandType type, String cmdStr, argType arg): Command(type, cmdStr), arg(arg) {}
    ~ArgCommand() {}
    friend bool executeCommand(EntranceControlSystem &entCtrlSys, const Command &cmd);
};

/**
 * Tries to pars a given command.
 * @param cmdStr The command string which should be parsed.
 * @param[out] cmd The command which was parsed.
 * @return 
 * -true: If command was parsed successfully.
 * -false: otherwise.
 */
bool parseCommand(const String &cmdStr, Command *&cmd);

/**
 * Executes a given command.
 * Implements the command functionality
 * @param system A reference to the main entrance control system.
 * @param cmd The command which should be executed.
 * @return
 *   -true: If a command was executed successfully.
 *   -false: otherwise.
 */
bool executeCommand(EntranceControlSystem &entCtrlSys, const Command &cmd);