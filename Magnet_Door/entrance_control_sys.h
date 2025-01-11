#pragma oncecommSys
/*************************************************************
  A system to handle the access of an entrance.
*************************************************************/

//===========================================================
// included dependencies
#include <cstdint>
#include "room_load_sys.h"
#include "door_status_sys.h"
#include "comm_sys.h"

//===========================================================
// forward declared dependencies
enum class EntranceControlState: uint8_t;

//===========================================================
// Data Types

/**
 * Manages access to an entrance.
 */
class EntranceControlSystem {
  private: 
    EntranceControlState state;                  //< Current state of the system FSM.
    CommunicationSystem& commSys;                //< A reference to the communication system.
    DoorStatusSystem doorSys;                    //< The door state sub system.
    RoomLoadSystem roomLoadSys;                  //< The room load sub system.
    WifiCredentials wifiCred;                    //< Saves the current WiFi credentials.
    bool verbose = false;                        //< Whether verbose status messaging is activated.
    unsigned long lastDataLog = 0;               //< records the last logging of data.
    const unsigned long dataLogInterval = 3000; //< Time interval between two data loggings in milli seconds.
    const uint8_t termPin;                       //< The pin of the thermistor.
    float temperature = 0;                       //< The current temperature in °C at the door position.

    /**
     * The main routine of the entrance control system.
     * Performing processing of commands, doing door status and passing checks
     * and doing communication tasks using the communication system.
     */
    void doMainRoutine();

    /**
     * Loads the system configuration from flash memory.
     */
    void loadConfig();

    /**
     * Processes commands inputted over serial terminal.
     * @return
     *   -true: If a command was processed successfully.
     *   -false: otherwise.
     */
    bool processCommand();

    /**
     * Logs all collected data to the web server.
     * Prints data information into serial if verbose messaging is enabled.
     */
    void logData();

    /**
     * Determines the temperature at the entrance.
     */
    void doTemperatureCheck();

  public:
    /**
     * Constructs a EntranceControlSystem with the used hardware pins.
     * @param commSys Reference to the used communication system.
     * @param termPin The thermistor pin
     * @param openLEDPin The door open status LED pin.
     * @param closedLEDPin The door closed status LED pin.
     * @param magSwitchPin The magnatic switch pin.
     * @param buzzerPin The buzzer pin.
     * @param outerDetPin The outer detector pin.
     * @param innerDetPin The inner detector pin.
     */
    EntranceControlSystem( CommunicationSystem& commSys,
                           uint8_t termPin,
                           uint8_t openLEDPin, 
                           uint8_t closedLEDPin, 
                           uint8_t magSwitchPin, 
                           uint8_t buzzerPin,
                           uint8_t outerDetPin, 
                           uint8_t innerDetPin);

    /**
     * Sets whether verbose status messages should be printed.
     * Can be used for debugging.
     * @param val Verbose value to be set.
     */
    void activateVerboseMessaging(bool val);

    /**
     * Executes the system state machine.
     */
    void run();

    /**
     *  Brings the system back in initial state.
     */
    void reset();

    /**
     * Tells the communication system to connect WiFi and server.
     */
    void doConnect() const;

    /**
     * Tells the communication system to disconnect from the server.
     */
    void doDisconnect() const;

    /**
     * Returns the online state of the used communication system.
     * @return
     *  -true: If oline.
     *  -false: otherwise.
     */
    bool isOnline() const;

    /**
     * Configures and saves the new room capacity into flash memory.
     * @param val Room capacity value which should be configured.
     * @return 
     *  -true: If configuration could be successfully stored.
     *  -false: otherwise.
     */
    bool configRoomCap(uint8_t val);

    /**
     * Performs a WiFi configuration over serial terminal.
     * Stores the new configuration into flash memory.
     * Disconnects the WiFi and server connections beforehand.
     * @return 
     * -true: If configuration could be successfully stored.
     * -false: otherwise.
     */
    bool configWifi();

    /**
     * Configures and saves the new server URL into flash memory.
     * @param val Server URL value which should be configured.
     * @return 
     *  -true: On success.
     *  -false: otherwise.
     */
    bool configServerUrl(const String& val);

    /**
     * To print the current configuration over serial.
     */
    void printConfig();

    /**
     * Resets the system to factory Settings.
     * @return 
     *  -true: On success.
     *  -false: otherwise.
     */
    bool restoreFactorySettings();

    /**
     * Resets the WiFi configuration of the system.
     * @return 
     *  -true: On success.
     *  -false: otherwise.
     */
    bool resetWifiConfig();
};