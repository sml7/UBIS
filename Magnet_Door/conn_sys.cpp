/*************************************************************
  The implementation of a system to manage connections to server and WiFi.
*************************************************************/

#define NO_GLOBAL_BLYNK 1   //To prevent multiple definitions of blynk

//===========================================================
// included dependencies
#include "conn_sys.h"
#include "blynk_credentials.h"
#include <BlynkSimpleEsp32.h>
#include "Arduino.h"
#include "persistence.h"
#include "serial_access.h"

//===========================================================
// Static function implementations

/**
 * Implements LED togglings.
 */
static inline void doConnLEDBBlink() {
  digitalWrite(connLEDPin, !digitalRead(connLEDPin));
}

//===========================================================
// Member function implementations

/**
 * Tries to establish a connection to a WiFi access point
 * Can fail after a certain timeout.
 * @param ssid The SSID of the WiFi.
 * @param pass The password of the WiFi.
 * @return The Status of the WiFi connection.
 * -WL_CONNECTED: If a connection could be successfully established.
 */
wl_status_t ConnectionSystem::connectWiFi(const char* ssid, const char* pass) {
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
 * Performs a WiFi configuration over serial terminal.
 * Stores the new configuration into flash memory.
 * @param wifiCred The WiFi configuration in memory, which has to be updated.
 * @return 
 * -true: If configuration could be successfully stored.
 * -false: otherwise.
 */
bool ConnectionSystem::doWifiConfig() {
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

/**
 * Starts the blinking process of the connection status LED.
 * Uses a hardware timer, which executes the LED toggling routine periodically.
 * Can execute the LED blinking in parallel to other tasks.
 */
void ConnectionSystem::startConnLEDBlink() {
  uint64_t alarmLimit = 1500000; 
  connStatusTimer = timerBegin(1000000); // timer frequency
  timerAttachInterrupt(connStatusTimer, &doConnLEDBBlink);
  timerAlarm(connStatusTimer, alarmLimit, true, 0);
}

/**
 * Stops the blinking process of the connection status LED.
 */
void ConnectionSystem::endConnLEDBlink() {
  timerEnd(connStatusTimer); //Stop blinking
  connStatusTimer = NULL;
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