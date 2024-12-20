/*************************************************************
  Implementation of the system control.
*************************************************************/

/* Uncomment this line to enable Serial debug prints */
#define BLYNK_PRINT Serial

#include "credentials.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "persistence.h"
#include "serial_access.h"

//======Pin Definitons======
#define magSwitchPin 18          // magnetic switch pin
#define redLEDPin 22             // door closed status LED pin
#define greenLEDPin 19           // door opened status LED pin
#define connLEDPin 5             // Connection status LED pin
#define buzzerPin 23             // buzzer pin
#define innerLBarrPin 16         // inner light barrier pin
#define outerLBarrPin 17         // outer light barrier pin
#define connButtonPin 4          // the connection button pin

//======Data Types======

//Represents the states the system FSM can be in.
enum SystemState {
  Init,
  Offline,
  Config,
  Connect,
  Online,
  Reconnect
};

enum PassState {
  Idle,
  StartEntering,
  Entering1,
  Entering2,
  Entered,
  StartLeaving,
  Leaving1,
  Leaving2,
  Left
};

//======Globals======

//Saves current wifi credentials
WifiCredentials wifiCred;

//Notification
unsigned long lastNotificationTime = 0; // For timer control
const unsigned long notificationInterval = 1000; // 1 second

//Door state
bool doorOpen = false;
bool lastDoorOpen = false;

hw_timer_t * connStatusTimer = NULL; //Hardware timer to handle connection status LED update task

//Current state of the system FSM
SystemState sysState;

//Current door passing state
PassState passState;

//To monitor number of persons in the room
uint8_t personCount = 0;    //number of persons
uint8_t maxPersonCount = 3; //maximum number of persons
bool roomFull = false;      //If number of persons inside the room has reached the maximum.


unsigned long lastTimeOnline = 0; //Record of last time the system was online.
#define CONN_TIMEOUT 20000        //Timeout until the system tries to reconnect.

//Records the last time the connection button was pressed
unsigned long lastConnUnpressed = 0;

//======Function implementations======

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

wl_status_t connectWiFi(const char* ssid, const char* pass)
{
    BLYNK_LOG2(BLYNK_F("Connecting to "), ssid);
    WiFi.mode(WIFI_STA);
    if (pass && strlen(pass)) {
        WiFi.begin(ssid, pass);
    } else {
        WiFi.begin(ssid);
    }
    uint16_t wait = 0;
    const uint16_t delay = 500;
    wl_status_t wifiStatus = WiFi.status();
    while (wifiStatus != WL_CONNECTED) {
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

// void connectToWiFi(){
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

/*
* Performs a WiFi configuration over serial terminal.
* Stores the new configuration into flash memory.
* @param wifiCred The WiFi configuration in memory, which has to be updated.
*/
static bool doWifiConfig(WifiCredentials &wifiCred) {
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

  return storeWifiConfig(wifiCred);
}

inline static void updateEntranceStatusLEDs(bool ent) {
  //inverted logic
  digitalWrite(redLEDPin, ent);
  digitalWrite(greenLEDPin, !ent);
}

/*
* Prints the current configured WiFi credentials and
* asks the user if a connection should be established 
* over the serial terminal.
*/
static inline void printConnectionQuestion() {
  Serial.print("Current Configured SSID: ");
  Serial.println(wifiCred.ssid);
  Serial.print("Current Configured password: ");
  Serial.println(wifiCred.pass);
  Serial.println("Do you want to connect? (Y/n)"); //Yes is default answer
}

/*
* Performs an acoustic signalling for door state change events.
*/
static void doDoorStateChangedAcousticSignal() {
  tone(buzzerPin, 100);
  delay(1000);
  noTone(buzzerPin);
}

/*
* Determines the current door state by reading the magnatic switch input
* and registeres door state change events. Updates status LEDs accordingly.
* Logs events to the server if in online mode
* @param online If set to true, will be in online mode.
*/
static void doDoorStatusCheck(bool online = false) {
  doorOpen = digitalRead(magSwitchPin);

  // Check if sensor value is below threshold to indicate door is opened
  if (lastDoorOpen != doorOpen && (millis() - lastNotificationTime >= notificationInterval)) {
    lastDoorOpen = doorOpen;
    if(doorOpen) {
      Serial.println("Door state change event: opened");

      if(online) {
        // Trigger the event for notification in Blynk
        Blynk.logEvent("door_opened", "Info: You can come in.");
      }

      if(!roomFull) {
        updateEntranceStatusLEDs(true);
      }

      //acoustic signal
      doDoorStateChangedAcousticSignal();
    }
    else {
      Serial.println("Door state change event: closed");

      if(online) {
        // Trigger the event for notification in Blynk
        Blynk.logEvent("door_closed", "Info: Room was closed.");
      }

      if(personCount > 0) {
        Serial.println("Alert: There are still persons in the room!");
        if(online) {
          Blynk.logEvent("alert_persons", "Alert: There are still persons in the room!");
        }
      }

      updateEntranceStatusLEDs(false);
      doDoorStateChangedAcousticSignal();
    }
    // Update the time of the last notification
    lastNotificationTime = millis();
  }
}

/*
* Processes commands inputted over serial terminal.
* return
*   -true: If a command was processed.
*   -false: otherwise.
*/
bool processCommand() {
  String command;
  if(readStringFromSerial(command)) { //Check serial input
    //If command was received from serial process it
    if(command == "Config") {
      sysState = Config; return true;
    }
    else if(command == "Connect" && sysState != Online) {
      if(sysState != Online) {
        sysState = Connect; return true;
      }
      else {
        Serial.print("Info: Command unnecessary: ");
        Serial.println(command);
        Serial.println("  >> Reason: Already in Online mode.");
      }
    }
    else if(command == "Disconnect" && sysState != Offline) {
      if(sysState != Offline) {
        Blynk.disconnect();
        Serial.println("-----------Going offline-----------");
        digitalWrite(connLEDPin, 0x01); //update connection status led
        sysState = Offline; return true;
      }
      else {
        Serial.print("Info: Command unnecessary: ");
        Serial.println(command);
        Serial.println("  >> Reason: Already in Offline mode.");
      }
    }
    else if(command == "Clear") {
      if(clearMemory()) { //Try to clear memory
        Serial.println("  >> Configuration cleared successfully.");
        loadWifiConfig(wifiCred); //update WiFi configuration
      }
      else {
        Serial.println("Error: Failed to clear configuration!.");
      }
      return true;
    }
    else {
      Serial.print("Error: Invalid command: ");
      Serial.println(command);
    }
  }
  return false;
}

/*
* Starts the blinking process of the connection status LED.
* Uses a hardware timer, which executes the LED toggling routine periodically.
* Can execute the LED blinking in parallel to other tasks.
*/
static void startConnLEDBlink() {
  uint64_t alarmLimit = 1500000; 
  connStatusTimer = timerBegin(1000000); // timer frequency
  timerAttachInterrupt(connStatusTimer, &doConnLEDBBlink);
  timerAlarm(connStatusTimer, alarmLimit, true, 0);
}

/*
* Stops the blinking process of the connection status LED.
*/
static void endConnLEDBlink() {
  timerEnd(connStatusTimer); //Stop blinking
  connStatusTimer = NULL;
}

/*
* Implements LED toggling.
*/
void doConnLEDBBlink() {
  digitalWrite(connLEDPin, !digitalRead(connLEDPin));
}

void setup() {
  Serial.begin(115200); //Initialize Serial
  delay(1000); //Wait for serial to become ready
  Serial.println("-----------Program started-----------");
  initMemory();

  //Set inital system state
  sysState = Offline;

  //Setup pins
  pinMode(magSwitchPin, INPUT);
  pinMode(outerLBarrPin, INPUT);
  pinMode(innerLBarrPin, INPUT);
  pinMode(connButtonPin, INPUT);
  pinMode(redLEDPin, OUTPUT);
  pinMode(greenLEDPin, OUTPUT);
  pinMode(connLEDPin, OUTPUT);

  //Set initial LED state
  updateEntranceStatusLEDs(false);
  digitalWrite(connLEDPin, HIGH);

  //load currently saved config
  loadWifiConfig(wifiCred);
}

/*
* Assumes that only one person can pass the door at the same time.
*/
void doDoorPassingCheck(bool online = false) {
  // Serial.print("Is passing: ");
  // Serial.print(isPassingOuter());
  // Serial.print(" ");
  // Serial.println(isPassingInner());
  switch(passState) {
    case Idle:
      if(isPassingOuter()) {
        //Someone starts to enter
        passState = StartEntering;
      }
      else if(isPassingInner()) {
        //Someone starts to leave
        passState = StartLeaving;
      }
      break;
    case StartEntering:
     if(isPassingBoth()) {
      //Someone who enters steped further in and now passes both barriers at the same time
      passState = Entering1;
     }
     else if(noPassing()) {
      //Someone decided to step back from entering
      passState = Idle;
     }
     else if(isPassingInner()) {
      //Passing only the inner barrier is not allowed in this state.
      Serial.println("Error: During entering event!");
      Serial.println("  >> Reason: Only inner barrier is passed, but both barriers were not passed before.");
      Serial.println("  >> Result: Could not detect entering correctly. Falling back to idle.");
      passState = Idle;
     }
     break;
    case Entering1:
      if(isPassingInner()) {
        //Someone who enters steped further in and now passes only the inner barrier
        passState = Entering2;
      }
      else if(isPassingOuter()) {
        //Someone decided to step back
        passState = StartEntering;
      }
      else if(noPassing()) {
        //No passing of any barrier is not allowed in this state.
        Serial.println("Error: During entering event!");
        Serial.println("  >> Reason: Both barriers were passed, but now no barrier is passed.");
        Serial.println("  >> Result: Could not detect entering correctly. Falling back to idle.");
        passState = Idle;
      }
      break;
    case Entering2:
     if(noPassing()) {
      //Someone finished entering
      passState = Entered;
     }
     else if(isPassingBoth()) {
      //Someone decided to step back
      passState = Entering1;
     }
     else if(isPassingOuter()) {
      //Passing only the outer barrier is not allowed in this state.
      Serial.println("Error: During entering event!");
      Serial.println("  >> Reason: Someone passed the inner barrier, but now only the outer barrier is passed.");
      Serial.println("  >> Result: Could not detect entering correctly. Falling back to idle.");
      passState = Idle;
     }
     break;
    case Entered:
      if(personCount < maxPersonCount) {
        personCount++;
        Serial.println("Passing event: Someone entered.");
        Serial.print("  >> Current load: ");
        Serial.println(personCount);
      }
      else {
        Serial.println("Error: During entering event!");
        Serial.println("  >> Reason: Room is already full.");
        Serial.println("  >> Result: Falling back to idle.");
      }
      passState = Idle;
      break;
    case StartLeaving:
      if(isPassingBoth()) {
        //Someone who is leaving steped further out and now passes both barriers at the same time
        passState = Leaving1;
      }
      else if(noPassing()) {
        //Someone decided to step back from leaving
        passState = Idle;
      }
      else if(isPassingOuter()) {
        //Passing only the outer barrier is not allowed in this state.
        Serial.println("Error: During leaving event!");
        Serial.println("  >> Reason: Only outer barrier is passed, but both barriers were not passed before.");
        Serial.println("  >> Result: Could not detect leaving correctly. Falling back to idle.");
        passState = Idle;
      }
      break;
    case Leaving1:
      if(isPassingOuter()) {
        //Someone who is leaving steped further out and now passes only the outer barrier
        passState = Leaving2;
      }
      else if(isPassingInner()) {
        //Someone decided to step back
        passState = StartLeaving;
      }
      else if(noPassing()) {
        //No passing of any barrier is not allowed in this state.
        Serial.println("Error: During leaving event!");
        Serial.println("  >> Reason: Both barriers were passed, but now no barrier is passed.");
        Serial.println("  >> Result: Could not detect leaving correctly. Falling back to idle.");
        passState = Idle;
      }
      break;
    case Leaving2:
      if(noPassing()) {
        //Someone finished leaving
        passState = Left;
      }
      else if(isPassingBoth()) {
        //Someone decided to step back
        passState = Leaving1;
      }
      else if(isPassingInner()) {
        //Passing only the inner barrier is not allowed in this state.
        Serial.println("Error: During leaving event!");
        Serial.println("  >> Reason: Someone passed the outer barrier, but now only the inner barrier is passed.");
        Serial.println("  >> Result: Could not detect entering correctly. Falling back to idle.");
        passState = Idle;
      }
      break;
    case Left:
      if(!(personCount == 0)) {
      personCount--;
        Serial.println("Passing event: Someone left.");
        Serial.print("  >> Current load: ");
        Serial.println(personCount);
      }
      else {
        Serial.println("Error: During leaving event!");
        Serial.println("  >> Reason: Room was already empty.");
        Serial.println("  >> Result: Falling back to idle.");
      }
      passState = Idle;
      break;
  }

  if(!roomFull && personCount >= maxPersonCount) {
    Serial.println("Alert: Room is full.");
    roomFull = true;
    if(online) {
        // Trigger the event for notification in Blynk
        Blynk.logEvent("room_full", "Alert: Room is full now.");
    }
    updateEntranceStatusLEDs(false);
  }
  else if(roomFull && personCount < maxPersonCount) {
    Serial.println("Info: Room is no longer full.");
    roomFull = false;
    if(online) {
        // Trigger the event for notification in Blynk
        Blynk.logEvent("room_not_full", "Info: Room is no longer full.");
    }
    updateEntranceStatusLEDs(true);
  }
}

inline bool isPassingOuter() {
  if(!digitalRead(outerLBarrPin) && digitalRead(innerLBarrPin)) 
    return true;
  return false;
}

inline bool isPassingInner() {
  if(digitalRead(outerLBarrPin) && !digitalRead(innerLBarrPin)) 
    return true;
  return false;
}

inline bool isPassingBoth() {
  if(!digitalRead(outerLBarrPin) && !digitalRead(innerLBarrPin)) 
    return true;
  return false;
}

inline bool noPassing() {
  if(digitalRead(outerLBarrPin) && digitalRead(innerLBarrPin)) 
    return true;
  return false;
}

static void doMainRoutine(bool online = false) {
  if(!processCommand()) { //Check if command is inputted and process it
    // //In case no command to process
    if(digitalRead(connButtonPin) == HIGH) {//Check if connection button pressed
      //If not pressed
      lastConnUnpressed = millis();
    }
    else {
      //If pressed
      if(millis() - lastConnUnpressed > 500) { //Checks if button is pressed for at least 0.5 seconds
        lastConnUnpressed = millis();
        if(online) {
          Serial.println("-----------Going offline-----------");
          sysState = Offline;
        }
        else {
          sysState = Connect;
        }
      }
    }
    doDoorStatusCheck(online);
    if(doorOpen) {
      doDoorPassingCheck(online);
    }
  }
}

void loop() {
  //Determine next state of the system FSM
  switch(sysState) {
    case Offline:
      doMainRoutine();
      break;
    case Config:
      Serial.println("-----------WiFi Configuration-----------");
      if(doWifiConfig(wifiCred)) {
        Serial.println("  >> WiFi credentials stored successfully.");
        loadWifiConfig(wifiCred); //load new config
        sysState = Connect;
      }
      else {
        Serial.println("Error: Storing of WiFi credentials failed!");
        sysState = Offline;
      }
      break;
    case Connect:
      if(!wifiCred.pass.isEmpty() && !wifiCred.ssid.isEmpty()) {
        Serial.println("-----------Connecting to WiFi and Blynk-----------");
        startConnLEDBlink();
        wl_status_t wifiStatus = connectWiFi(wifiCred.ssid.c_str(), wifiCred.pass.c_str());
        if(wifiStatus == WL_CONNECTED) {
          Blynk.config(BLYNK_AUTH_TOKEN);
          if(Blynk.connect(CONN_TIMEOUT)) {
            //Successfully connected
            Serial.println("-----------Going online-----------");
            endConnLEDBlink();
            digitalWrite(connLEDPin, LOW); //update connection status led
            lastTimeOnline = millis();
            sysState = Online;
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
        sysState = Offline;
        endConnLEDBlink();
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
              sysState = Config; 
              break;
            }
            else if(answer == "N" || answer == "n") {
              Serial.println("  >> Answer: No");
              Serial.println("-----------Going offline-----------");
              digitalWrite(connLEDPin, 0x01); //update connection status led
              sysState = Offline;
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
        lastTimeOnline = millis();
        doMainRoutine(true);
      }
      else {
        //Connection lost. Try to reconnect.
        Serial.println("Alert: Connection lost. Try to reconnect.");
        startConnLEDBlink();
        sysState = Reconnect;
      }
      break;
    case Reconnect:
      if(millis() - lastTimeOnline <= CONN_TIMEOUT) {
        if(Blynk.connected()) {
          //Connection reestablished
          Serial.println("Info: Connection reestablished.");
          lastTimeOnline = millis();
          sysState = Online;
          endConnLEDBlink();
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
        sysState = Offline;
        endConnLEDBlink();
        digitalWrite(connLEDPin, HIGH); //update connection status led
      }
      break;
  }
}
