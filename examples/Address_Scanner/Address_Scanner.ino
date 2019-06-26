/*
Use this sketch to find the address(es) of any 1-Wire devices
you have attached to your Particle device (core, p0, p1, photon, electron)

It is an example of using the OneWire library directly to identify all devices attached
to the 1-wire bus. It will identify all 'known' devices like DS18B20.
Normally you would save all the found addresses in an array for later use. This
code just prints the info out to the serial port.

Pin setup:
These made it easy to just 'plug in' my 18B20

D3 - 1-wire ground, our just use regular pin and comment out below.
D4 - 1-wire signal, 2K-10K resistor to...
D5 - 1-wire power, ditto ground comment.

A pull-up resistor is required on the signal line. The spec calls for a 4.7K.
I have used 1K-10K depending on the bus configuration and what I had out on the
bench. If you are powering the device, they all work. If you are using parisidic
power it gets more picky about the value. I probably use 10K the most.

*/

#include "OneWire.h"

OneWire wire = OneWire(D4);  // 1-wire signal on pin D4

unsigned long lastUpdate = 0;

void setup() {
  Serial.begin(9600);
  // Set up 'power' pins, comment out if not used!
  pinMode(D3, OUTPUT);
  pinMode(D5, OUTPUT);
  digitalWrite(D3, LOW);
  digitalWrite(D5, HIGH);
}

// Every 3 seconwire check for the next address on the bus
// The scan resets when no more addresses are available

void loop() {
   unsigned long now = millis();
  // change the 3000(ms) to change the operation frequency
  // better yet, make it a variable!
  if ((now - lastUpdate) < 3000) {
    return;
  }
  lastUpdate = now;
  byte i;
  byte present = 0;
  byte addr[8];

  if (!wire.search(addr)) {
    Serial.println("No more addresses.");
    Serial.println();
    wire.reset_search();
    //delay(250);
    return;
  }

  // if we get here we have a valid address in addr[]
  // you can do what you like with it
  // see the Temperature example for one way to use
  // this basic code.

  // this example just identifies a few chip types
  // so first up, lets see what we have found

  // the first ROM byte indicates which chip family
  switch (addr[0]) {
    case 0x10:
      Serial.println("Chip = DS1820/DS18S20 Temp sensor");
      break;
    case 0x28:
      Serial.println("Chip = DS18B20 Temp sensor");
      break;
    case 0x22:
      Serial.println("Chip = DS1822 Temp sensor");
      break;
    case 0x26:
      Serial.println("Chip = DS2438 Smart Batt Monitor");
      break;
    default:
      Serial.println("Device type is unknown.");
      // Just dumping addresses, show them all
      //return;  // uncomment if you only want a known type
  }

  // Now print out the device address
  Serial.print("ROM = ");
  Serial.print("0x");
    Serial.print(addr[0],HEX);
  for( i = 1; i < 8; i++) {
    Serial.print(", 0x");
    Serial.print(addr[i],HEX);
  }

  // Show the CRC status
  // you should always do this on scanned addresses

  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }

  Serial.println();

  wire.reset(); // clear bus for next use
}
