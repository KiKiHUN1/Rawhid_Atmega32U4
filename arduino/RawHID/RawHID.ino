/*
  Copyright (c) 2014-2015 NicoHood
  See the readme for credit to other people.

  Advanced RawHID example

  Shows how to send bytes via RawHID.
  Press a button to send some example values.

  Every received data is mirrored to the host via Serial.

  See HID Project documentation for more information.
  https://github.com/NicoHood/HID/wiki/RawHID-API
*/

#include "HID-Project.h"

const int pinLed = LED_BUILTIN;
const int pinButton = 2;

// Buffer to hold RawHID data.
// If host tries to send more data than this,
// it will respond with an error.
// If the data is not read until the host sends the next data
// it will also respond with an error and the data will be lost.
uint8_t rawhidData[255];
uint8_t inBuff[64]= {0};
uint8_t outBuff[64]= {0};
void setup() {
  pinMode(pinLed, OUTPUT);
  pinMode(pinButton, INPUT_PULLUP);

  Serial.begin(115200);

  // Set the RawHID OUT report array.
  // Feature reports are also (parallel) possible, see the other example for this.
  RawHID.begin(rawhidData, sizeof(rawhidData));
}

void loop() {
  // Check if there is new data from the RawHID device
  uint8_t bytesAvailable = RawHID.available();
  if (bytesAvailable)
  {
     Serial.println("Reading data...");
    for(int i = 0; i < bytesAvailable; i++)
    {
      inBuff[i] = RawHID.read();
      // Mirror data via Serial
      Serial.print(i);
      Serial.print(" : ");
      Serial.println(inBuff[i]);
    }
    Serial.println("Init check");
    delay(100);
   if (inBuff[0] == 0x69) {  // Check for a specific identifier byte (0x69)
      Serial.println("init...");
      
      if (inBuff[1] == 0x01 && inBuff[2] == 0x02 && inBuff[3] == 0x03) {  // Check handshake values
        Serial.println("Ok");
        outBuff[0] = 0x01;  // Send acknowledgment (0x01)
      } else {
        Serial.println("Init Sending NACK");
        outBuff[0] = 0x02;  // Send negative acknowledgment (0x02)
      }
    } else {
      Serial.println("Init Sending Error Code");
      outBuff[0] = 0x03;  // Error code if the handshake is invalid
    }

    // Send the response back to the host (PC)
    RawHID.write(outBuff, 64);  // Send the entire buffer, not just a single byte
  }
}

