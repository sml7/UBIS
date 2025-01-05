/*************************************************************
  Implementation of some helper functions to process input on the serial interface.
*************************************************************/

//===========================================================
// included dependencies
#include "serial_access.h"
#include "Arduino.h"

//===========================================================
// Function implementations

/**
 * Prompts the user to input a string into the serial terminal.
 * Blocks until String was correctly inputted.
 * String is not allowed to be greater than maxLength.
 * @param input The string which was inputted.
 * @param maxLength The maximum allowed length of the string.
 * @param enterMsg The message wich should be printed before the input should be made.
 * @param enteredMsg If set to true prints the entered string into the serial.
 * @param trim If set to true all leading and ending white spaces will be removed from the inputted string.
 */
void inputStringFromSerial(
            String &input, 
            unsigned int maxLength, 
            String enterMsg, 
            bool enteredMsg, 
            bool trim) {

  bool processed = false;
  do {
    Serial.println(enterMsg);
    while (Serial.available() == 0) {}

    String read = Serial.readString();
    if(trim) {
      read.trim(); //Remove carriage return from string
    }
     
    if(read.length() <= maxLength) {
      input = read;
      break; //processed
    }
    //failed processing
    Serial.println("Error: Input exceeds maximum allowed size. Try again.");
  }
  while(true); //Repeat as long as processing did not succeed
  if(enteredMsg) {
    Serial.print("Entered: ");
    Serial.println(input);
  }
}


/**
 * Reads a string input from serial if available.
 * @param read The read string.
 * @param trim If set to true all leading and ending white spaces will be removed from the read string.
 * @return 
 *   -true: If string was available and read.
 *   -false: otherwise.
 */
bool readStringFromSerial(String &read, bool trim) {
  if(Serial.available() != 0) {
    read = Serial.readString();
    if(trim) {
      read.trim();
    }
    return true;
  }
  return false;
}