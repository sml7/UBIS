#pragma once
/*************************************************************
  A system to manage communication to a server via WiFi.
*************************************************************/

//===========================================================
// included dependencies
#include "Arduino.h"
#include <WiFiClient.h>
#include <HTTPClient.h>

//===========================================================
// Definitons
#define CONN_TIMEOUT 20000        //< Timeout until the system tries to reconnect.

//===========================================================
// forward declared dependencies
enum class CommSysState: uint8_t;
extern WiFiClient client;
extern HTTPClient http;

//===========================================================
// Data Types

/**
 * Represents WiFi credentials.
 */
struct WifiCredentials{
  String ssid = "";                  //< The configured WiFi SSID
  String pass = "";                  //< The configured password        
};

/**
 * The states the connection status can be in.
 */
enum class ConnectionStatus: uint8_t {
  connected,                         //< A connection is currently established.
  disconnected,                      //< No connection is currently established.
  noSSIDAvail,                       //< The WiFi SSID is not available. 
  wifiAuthFailed,                    //< Authentication to WiFi failed.
  wifiFailed,                        //< WiFi connection failed for an unknown reason.
  connectionLost,                    //< Lost the connection to the server.
  connectionTimeout,                 //< Failed to connect to the server after timeout.
  connectionRequest                  //< Communication system request a connect.
};

/**
 * Manages connection establishement, their configuration and status displaying
 * of server and WiFi connections and communication tasks to the server.
 */
class CommunicationSystem {
  private: 
    hw_timer_t* connStatusTimer = NULL;                        //< Hardware timer to handle connection status LED update task.
    unsigned long lastConnButtonTime = 0;                      //< Records the last time the connection button was not pressed.
    const unsigned long connButtonInterval = 500;              //< Duration the connection button needs to be pressed in milli seconds.
    CommSysState state;                                        //< The current state of the communication system FSM.
    bool online = false;                                       //< Online state
    bool connButton = false;                                   //< Registeres a press of the connection button.
    unsigned long lastTimeOnline = 0;                          //< Record of last time the system was online.
    String serverUrl = "";                                     //< Url to the web server.
    bool statusMessages = false;                               //< If status  messages should be printed over serial.
    unsigned long lastConnStatusMessage = 0;                   //< To record the timestamp of the last connection status message.
    const unsigned long connStatusMessageInterval = 3000;      //< Time interval between two connection status messages in milli seconds.
    const uint8_t connButtonPin;                               //< Connection button pin.
    const uint8_t connLEDPin;                                  //< Connection LED pin.

    /**
    * Starts the blinking process of the connection status LED.
    * Uses a hardware timer, which executes the LED toggling routine periodically.
    * Can execute the LED blinking in parallel to other tasks.
    */  
    void startConnLEDBlink();

    /**
    * Stops the blinking process of the connection status LED.
    */
    void endConnLEDBlink();

    /**
     * Checks whether there was an unhandled press of the connection button.
     * @return Is there an unhandled press?
     *  -true: If yes.
     *  -false: otherwise.
     */
     bool checkConnButton();

    /**
     * Checks whether there exists a connection to wifi and server.
     * @return Are we connected?
     *  -true: If yes.
     *  -false: otherwise.
     */
     bool isConnected();
    
  public:

    /**
     * Constructs a CommunicationSystem with the used hardware pins and 
     * a reference to the WiFi credentials.
     * @param connButtonPin The connection button pin.
     * @param connLEDPin The connection status LED pin.
     */
    CommunicationSystem( uint8_t connButtonPin, 
                         uint8_t connLEDPin);

    /**
     * Sets the url of the web server.
     * @param url The url of the server.
     */
    void setServerUrl(const String& url);

    /**
     * Gets the url of the web server.
     * @param url The url of the server.
     */
    const String& getServerUrl() const;

    /**
     * Set whether status messages should be printed over serial.
     * Can be activated for debugging.
     * @param val Enables printing or not.
     */
    void setPrintStatus(bool val);

    /**
     * Returns the online state of the communication system.
     * @return
     *  -true: If oline.
     *  -false: otherwise.
     */
    bool isOnline() const;

    /**
     * Connects to WiFi and to the server.
     * Blocks until connection was established.
     * @param wifiCred The WiFi credentials.
     * @return The connection status.
     */
    ConnectionStatus connect(const WifiCredentials& wifiCred);

    /**
     * Disconnects from the server and updates status LED.
     */
    void disconnect();

    /**
     * Logs an event with a description to the server if online.
     * @param eventName The identifier name of the event.
     * @param description A discription which is send with the event.
     */
    void logEvent(const String& eventName, const String& description) const;

    /**
     * Sends data to the connected server.
     * @param jsonData The data which should be send. Data is expected to be in json format.
     * @return Was the sending of data successful?
     *  -true: If yes.
     *  -false: otherwise.
     */
    bool sendData(const String& jsonData);

    /**
     * Executes the communication system state machine.
     * Monitores the connection button and changes.
     * Gives a connection request status back if pressed in offline mode otherwise it will disconnect.
     * Needs the WiFi credentials in case the system wants to connect after connection button press.
     * System tries to reconnect after connection is lost until timeout is reached.
     * @return The connection status.
     */
    ConnectionStatus run();

    /**
     * Brings the system back in initial state.
     * Disconnects from all connections and goes offline.
     */
    void reset();
};

#include "comm_sys_inline.h"