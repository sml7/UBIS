/*************************************************************
  Code for ESP32 to trigger Blynk notification when sensor detects door open.
*************************************************************/

/* Uncomment this line to enable Serial debug prints */
#define BLYNK_PRINT Serial

#include "credentials.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>


char auth[] = BLYNK_AUTH_TOKEN;
const char *ssid = WIFI_SSID;      // Your WiFi SSID
const char *pass = WIFI_PASS;      // Your WiFi password

WifiCredentials wifiCred;

//Pin Definitions
const int magSwitchPin = 18;         // magnetic switch pin
const int redLEDPin = 22;            // red pin
const int greenLEDPin = 19;          // green pin
const int buzzerPin = 23;            // buzzer pin

//Notification
unsigned long lastNotificationTime = 0; // For timer control
const unsigned long notificationInterval = 1000; // 15 seconds

//Door state
bool doorOpen = false;
bool lastDoorOpen = false;

const int buzzerFreq = 100;

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
* Performs a wifi configuration over serial terminal
*/
void doWifiConfig(WifiCredentials &wifiCred) {
  while (!Serial){} // wait for serial port to connect.

  //Reading SSID from terminal
  Serial.println("Enter Wifi SSID:");
  while (Serial.available() == 0) {}
  
  wifiCred.ssid = Serial.readString();
  wifiCred.ssid.trim(); //Remove carriage return from string
  Serial.print("Entered: ");
  Serial.println(wifiCred.ssid);

  //Reading password from terminal
  Serial.println("Enter Wifi password:");
  while (Serial.available() == 0) {}

  wifiCred.pass = Serial.readString();
  wifiCred.pass.trim(); //Remove carriage return from string
  Serial.print("Entered: ");
  Serial.println(wifiCred.pass);

  wifiCred.valid = true; //Indicate that stored credentials are valid
}

void setup() {
  // Initialize Serial for debugging
  Serial.begin(115200);
  delay(1000);
  Serial.println("Program started.........");

  //Setup pins
  pinMode(magSwitchPin, INPUT);
  pinMode(redLEDPin, OUTPUT);
  pinMode(greenLEDPin, OUTPUT);

  //Set initial LED state
  digitalWrite(redLEDPin, 0x00);
  digitalWrite(greenLEDPin, 0x01);

  doWifiConfig(wifiCred); //Perform wifi config

  // Connect to WiFi and Blynk
  Serial.println("Connecting to WiFi and Blynk...");
  // Blynk.begin(auth, ssid, pass);
  Blynk.begin(auth, wifiCred.ssid.c_str(), wifiCred.pass.c_str());

}

void loop() {
  // if(!Blynk.connected()) {
  //   Serial.println("Connection lost. Try to reconnect.");
  //   Blynk.connect();
  // }
  Blynk.run();

  doorOpen = digitalRead(magSwitchPin);
  
  // Serial.print("Sensor Value: ");
  // Serial.println(doorOpen);
  // delay(2000);

  // Check if sensor value is below threshold to indicate door is opened
  if (lastDoorOpen != doorOpen && (millis() - lastNotificationTime >= notificationInterval)) {
    lastDoorOpen = doorOpen;
    if(doorOpen) {
      Serial.println("Door was opened");

      // Trigger the event for notification in Blynk
      Blynk.logEvent("door_opened", "Looks like someone's opened the door");

      //Update status LEDs
      digitalWrite(redLEDPin, 0x01);
      digitalWrite(greenLEDPin, 0x00);

      //acoustic signal
      tone(buzzerPin, buzzerFreq);
      delay(1000);
      noTone(buzzerPin);
    }
    else {
      Serial.println("Door was closed");

      // Trigger the event for notification in Blynk
      Blynk.logEvent("door_closed", "Looks like someone's closed the door");

      //Update status LEDs
      digitalWrite(redLEDPin, 0x00);
      digitalWrite(greenLEDPin, 0x01);

      //acoustic signal
      tone(buzzerPin, buzzerFreq);
      delay(1000);
      noTone(buzzerPin);
    }
    // Update the time of the last notification
    lastNotificationTime = millis();
  }
}
