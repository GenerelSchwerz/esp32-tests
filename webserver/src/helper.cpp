#include <Arduino.h>


String readSerialLine() {
  String line = "";
  while (true) {
    if (Serial.available() > 0) {
      char c = Serial.read();
      if (c == '\n') { // Check for newline character
       
        break; // Exit loop when newline is received
      }
      line += c; // Append character to line
    }
  }

  line.trim(); // Remove leading and trailing whitespaces
  return line;
}