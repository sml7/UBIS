#pragma once
/*************************************************************
  Some helper functions to process input on the serial interface.
*************************************************************/

//===========================================================
// included dependencies
#include "Arduino.h"

//===========================================================
// Function declarations

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
            String enterMsg = "Please enter something: ", 
            bool enteredMsg = true, 
            bool trim = true);


/**
 * Reads a string input from serial if available.
 * @param read The read string.
 * @param trim If set to true all leading and ending white spaces will be removed from the read string.
 * @return 
 *   -true: If string was available and read.
 *   -false: otherwise.
 */
bool readStringFromSerial(String &read, bool trim = true);