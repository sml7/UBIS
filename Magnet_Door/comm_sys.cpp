/*************************************************************
  The implementation of a system to manage communication to a server via WiFi.
*************************************************************/

//===========================================================
// Blynk definitons
/* Uncomment this line to enable Serial debug prints */
#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL4AnEFpL5p"
#define BLYNK_TEMPLATE_NAME "Quickstart Device"
#define BLYNK_AUTH_TOKEN "qEjFtvtSFtEUyFgUdcyLCil66z_tXCc_"


//===========================================================
// included dependencies
#include "comm_sys.h"
#include <BlynkSimpleEsp32.h>
#include <WiFi.h>

/**
 * Represents the states the communication system FSM can be in.
 */
enum class CommSysState: uint8_t {
  offline,        //< System is offline.
  online,         //< Does online tasks. 
  reconnect       //< Tries to reconnect.
};

//===========================================================
// Static function implementations

/**
 * Tries to establish a connection to a WiFi access point
 * Can fail after a certain timeout.
 * @param ssid The SSID of the WiFi.
 * @param pass The password of the WiFi.
 * @return The Status of the WiFi connection.
 * -WL_CONNECTED: If a connection could be successfully established.
 */
wl_status_t connectWiFi(const char* ssid, const char* pass) {
    BLYNK_LOG2(BLYNK_F("Connecting to "), ssid);
    WiFi.mode(WIFI_STA);
    if (pass && strlen(pass)) {
        WiFi.begin(ssid, pass);
    } 
    else {
        WiFi.begin(ssid);
    }
    uint16_t wait = 0;
    const uint16_t delay = 500;
    wl_status_t wifiStatus = WiFi.status();
    while (wifiStatus != WL_CONNECTED) {
        //TODO: Make timeout more accurate
        if(wait > CONN_TIMEOUT) {
          return wifiStatus;
        }
        wait += delay;
        BlynkDelay(delay);
        wifiStatus = WiFi.status();
    }
    BLYNK_LOG1(BLYNK_F("Connected to WiFi"));

    IPAddress myip = WiFi.localIP();
    (void)myip; // Eliminate warnings about unused myip
    BLYNK_LOG_IP("IP: ", myip);
    return WL_CONNECTED;
}

/**
 * Toggles the connection status LED. 
 * Is called with each interrupt of the connStatusTimer,
 * so it will blink in the configured timer interval.
 * @param connLEDPin Pointer to the pin number of the connection status LED.
 */
inline static void connStatusLEDToggle(const uint8_t connLEDPin) {
  digitalWrite(connLEDPin, !digitalRead(connLEDPin));
}


//===========================================================
// Member function implementations

/**
 * Constructs a CommunicationSystem with the used hardware pins and 
 * a reference to the WiFi credentials.
 * @param connButtonPin The connection button pin.
 * @param connLEDPin The connection status LED pin.
 */
CommunicationSystem::CommunicationSystem( uint8_t connButtonPin, 
                                          uint8_t connLEDPin): state(CommSysState::offline),
                                                               connButtonPin(connButtonPin), 
                                                               connLEDPin(connLEDPin) {
  //Setup  
  pinMode(connButtonPin, INPUT);
  pinMode(connLEDPin, OUTPUT);
}

/**
 * Sets the url of the server.
 * @param url The url of the server.
 */
void CommunicationSystem::setServerUrl(const String& url) {
  serverUrl = url;
}

/**
 * Set whether status messages should be printed over serial.
 * @param val Enables printing or not.
 */
void CommunicationSystem::setPrintStatus(bool val) {
  statusMessages = val;
}

/**
 * Executes the communication system state machine.
 * Monitores the connection button and changes.
 * Gives a connection request status back if pressed in offline mode otherwise it will disconnect.
 * Needs the WiFi credentials in case the system wants to connect after connection button press.
 * System tries to reconnect after connection is lost until timeout is reached.
 * @return The connection status.
 */
ConnectionStatus CommunicationSystem::run() {
  ConnectionStatus status;
  switch(state) {
    case CommSysState::offline:
      if(online) {
        status = ConnectionStatus::connected;
        state = CommSysState::online;
      }
      else {
        if(checkConnButton()) {
          status = ConnectionStatus::connectionRequest;
        }
        else {
          status = ConnectionStatus::disconnected;
        }
      }
      break;
    case CommSysState::online:
      if(online) {
        //Connected
        if(isConnected()) {
          Blynk.run();
          if(checkConnButton()) {
            disconnect();
            status = ConnectionStatus::disconnected;
          }
          status = ConnectionStatus::connected;
        }
        else {
          //Connection lost. Try to reconnect.
          state = CommSysState::reconnect;
          startConnLEDBlink();
          status = ConnectionStatus::connectionLost;
        }
      }
      else {
        state = CommSysState::offline;
        status = ConnectionStatus::disconnected;
      }
      break;
    case CommSysState::reconnect:
      if(checkConnButton()) {
        disconnect();
        status = ConnectionStatus::disconnected;
        state = CommSysState::offline;
      }
      else {
        if(millis() - lastTimeOnline <= CONN_TIMEOUT) {
          if(isConnected()) {
            //Connection reestablished
            lastTimeOnline = millis();
            state = CommSysState::online;
            status = ConnectionStatus::connected;
            endConnLEDBlink();
            digitalWrite(connLEDPin, LOW); //Setting connection status LED on
          }
          else {
            //Connection still lost. Try to reconnect.
            Blynk.run();
            status = ConnectionStatus::connectionLost;
          }
        }
        else {
          //Connection still lost after timeout. Falling back to offline mode.
          disconnect();
          state = CommSysState::offline;
          status = ConnectionStatus::connectionTimeout;
        }
      }
      break;
  }
  return status;
}

void CommunicationSystem::reset() {
  disconnect();
  state = CommSysState::offline;
}

/**
 * Connects to WiFi and to the server.
 * Updates status LED accordingly.
 * Blocks until connection was established.
 * @param wifiCred The WiFi credentials.
 * @return The connection status.
 */
ConnectionStatus CommunicationSystem::connect(const WifiCredentials& wifiCred) {
  startConnLEDBlink();
  wl_status_t wifiStatus = connectWiFi(wifiCred.ssid.c_str(), wifiCred.pass.c_str());
  //httpClient.connect()
  ConnectionStatus connStatus = ConnectionStatus::connectionTimeout; //start assumtion connection failes with timeout
  switch(wifiStatus) {
    case WL_CONNECTED:
      Blynk.config(BLYNK_AUTH_TOKEN);
      if(Blynk.connect(CONN_TIMEOUT)) {
        //Successfully connected
        lastTimeOnline = millis();
        online = true;
        endConnLEDBlink();
        digitalWrite(connLEDPin, LOW); //Setting connection status LED on
        return ConnectionStatus::connected;
      }
      break;
    case WL_NO_SSID_AVAIL:
      connStatus = ConnectionStatus::noSSIDAvail;
      break;
    case WL_DISCONNECTED:
      connStatus = ConnectionStatus::wifiAuthFailed;
      break;
    default:
      connStatus = ConnectionStatus::wifiFailed;
      break;
  }
  //Failed to connect
  online = false;
  endConnLEDBlink();
  return connStatus;
}

/**
 * Disconnects from the server and updates status LED.
 */
void CommunicationSystem::disconnect() {
  Blynk.disconnect();
  WiFi.disconnect();
  online = false;
  endConnLEDBlink();
}

/**
 * Checks whether there was an unhandled press of the connection button.
 * @return Is there an unhandled press?
 *  -true: If yes.
 *  -false: otherwise.
 */
bool CommunicationSystem::checkConnButton() {
  if(digitalRead(connButtonPin) == HIGH) {//Check if connection button pressed
    //If not pressed
    lastConnButtonTime = millis();
    connButton = false;
  }
  else { //If pressed
    //Checks if there was no registered press before and if it was pressed for at least connButtonInterval milli seconds
    if(!connButton && millis() - lastConnButtonTime > connButtonInterval) { 
      connButton = true;
      return true;
    }
  }
  return false; //No unhandled button press
}

/**
 * Logs an event with a description to the server if online.
 * @param eventName The identifier name of the event.
 * @param description A discription which is send with the event.
 */
void CommunicationSystem::logEvent(const String& eventName, const String& description) const {
  Blynk.logEvent(eventName, description);
}

/**
 * Sends data to the connected server.
 * @param jsonData The data which should be send. Data is expected to be in json format.
 * @return Was the sending of data successful?
 *  -true: If yes.
 *  -false: otherwise.
 */
bool CommunicationSystem::sendData(const String& jsonData) {
  if(online && state != CommSysState::reconnect) {
    // Preparing HTTP post request
    http.begin(client, serverUrl.c_str());
    http.addHeader("Content-Type", "application/json");

    // send data
    int httpResponseCode = http.POST(jsonData);

    if(statusMessages) {
      if (httpResponseCode > 0) {
        Serial.printf("[CommSys]: HTTP Response Code: %d\n", httpResponseCode);
        String response = http.getString();
        Serial.println("[CommSys]: Server responds:");
        Serial.println(response);
        http.end();
        return true;
      } 
      else {
        Serial.printf("[CommSys]: HTTP POST error: %s\n", http.errorToString(httpResponseCode).c_str());
      }
    }
    else {
      if (httpResponseCode > 0) {
        http.end();
        return true;
      } 
    }
    http.end();
  }
  return false;
}

/**
 * Starts the blinking process of the connection status LED.
 * Uses a hardware timer, which executes the LED toggling routine periodically.
 * Can execute the LED blinking in parallel to other tasks.
 */
void CommunicationSystem::startConnLEDBlink() {
  uint64_t alarmLimit = 1500000; 
  connStatusTimer = timerBegin(1000000); // timer frequency
  timerAttachInterruptArg(connStatusTimer, 
                       [](void* commSys){
                        const uint8_t connLEDPin = static_cast<CommunicationSystem*>(commSys)->connLEDPin;
                        connStatusLEDToggle(connLEDPin);
                       }, 
                       this);
  timerAlarm(connStatusTimer, alarmLimit, true, 0);
}

/**
 * Stops the blinking process of the connection status LED.
 */
void CommunicationSystem::endConnLEDBlink() {
  if(connStatusTimer) {
    timerEnd(connStatusTimer); //Stop blinking
    connStatusTimer = nullptr;
  }
  digitalWrite(connLEDPin, HIGH); //Setting connection status LED off
}

/**
 * Checks whether there exists a connection to WiFi and server.
 * @return Are we connected?
 *  -true: If yes.
 *  -false: otherwise.
 */
inline bool CommunicationSystem::isConnected() {
  bool conn = true;
  bool longEnough = false; //If the last status message was long enough ago.
  if(millis() - lastConnStatusMessage >= connStatusMessageInterval) {
    longEnough = true;
  }

  if(WiFi.status() == WL_CONNECTED) {
      if(!Blynk.connected()) {
        conn = false;
        if(statusMessages && longEnough) {
          Serial.println("[CommSys]: Lost connection to Blynk server.");
          lastConnStatusMessage = millis();
        }
      }
      http.begin(client, serverUrl.c_str());
      if(!http.connected()) {
        conn = false;
        if(statusMessages && longEnough) {
          Serial.println("[CommSys]: Lost connection to web server.");
          lastConnStatusMessage = millis();
        }
      }
      http.end();
  }
  else {
    conn = false;
    if(statusMessages && longEnough) {
      Serial.println("[CommSys]: Lost connection to WiFi.");
      lastConnStatusMessage = millis();
    }
  }
  return conn;
}

// void printWifiStatus() {
//   // prints the SSID of the attached network:
//   Serial.print("SSID: ");
//   Serial.println(WiFi.SSID());

//   // prints board's IP address:
//   IPAddress ip = WiFi.localIP();
//   Serial.print("IP Address: ");
//   Serial.println(ip);

//   // prints the received signal strength:
//   long rssi = WiFi.RSSI();
//   Serial.print("signal strength (RSSI):");
//   Serial.print(rssi);
//   Serial.println(" dBm");
// }



// void connectWiFi(){
//   // check for the WiFi module:
//   if (WiFi.status() == WL_NO_MODULE) {
//     Serial.println("Communication with WiFi module failed!");
//     // don't continue
//     while (true);
//   }

//   String fv = WiFi.firmwareVersion();

//   if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
//     Serial.println("Please upgrade the firmware");
//   }

//   // attempt to connect to WiFi network:
//   while (wifiStatus != WL_CONNECTED) {
//     Serial.print("Attempting to connect to SSID: ");
//     Serial.println(ssid);
//     // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
//     wifiStatus = WiFi.begin(ssid, pass);
//     // wait 10 seconds for connection:
//     delay(10000);
//   }
//   Serial.println("Connected to WiFi");
//   printWifiStatus();
// }