// This #include statement was automatically added by the Spark IDE.
#include "OneWire.h"

// OneWire DS18S20, DS18B20, DS1822, DS2438 Temperature Example
//
// https://github.com/Hotaman/OneWireSpark
//
// Thanks to all who have worked on this demo!
// I just made some minor tweeks for the spark core
// and added support for the DS2438 battery monitor
// 6/2014 - Hotaman

// Define the pins we will use
int ow = D0;    // put the onewire bus on D0

OneWire  ds(ow);  // a 1 - 4.7K resistor to 3.3v is necessary

void setup(void) {
  Serial.begin(57600);  // local hardware test only
}

void loop(void) {
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;
  
  if ( !ds.search(addr)) {
    Serial.println("No more addresses.");
    Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }
  
  Serial.print("ROM =");
  for( i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
  Serial.println();
 
  // the first ROM byte indicates which chip
  // includes debug output of chip type
  switch (addr[0]) {
    case 0x10:
      Serial.println("  Chip = DS18S20");  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
      Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    case 0x26:
      Serial.println("  Chip = DS2438");
      type_s = 2;
      break;
    default:
      Serial.println("Device is not a DS18x20/DS1822/DS2438 device. Skipping...");
      return;
  } 

  ds.reset();
  ds.select(addr);      // Just do one at a time for testing
                        // change to skip if you already have a list of addresses
                        // then loop through them below for reading
                        
  ds.write(0x44);        // start conversion, with parasite power on at the end
  
  delay(900);     // maybe 750ms is enough, maybe not, I'm shooting for 1 reading per second
                    // prob should set to min reliable and check the timer for 1 second intervals
                    // but that's a little fancy for test code
                    
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xB8,0);         // Recall Memory 0
  ds.write(0x00,0);         // Recall Memory 0

  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE, 0);         // Read Scratchpad 0
  ds.write(0x00,0);         // Recall Memory 0

  Serial.print("  Data = ");
  Serial.print(present, HEX);
  Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.print(" CRC=");
  Serial.print(OneWire::crc8(data, 8), HEX);
  Serial.println();

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    if (type_s==1) {    // DS18S20
      raw = raw << 3; // 9 bit resolution default
      if (data[7] == 0x10) {
        // "count remain" gives full 12 bit resolution
        raw = (raw & 0xFFF0) + 12 - data[6];
      }
      celsius = (float)raw / 16.0;
    }else{ // type_s==2 for DS2438
      if (data[2] > 127) data[2]=0;
      data[1] = data[1] >> 3;
      celsius = (float)data[2] + ((float)data[1] * .03125);
    }
  } else {  // DS18B20 and DS1822
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
    celsius = (float)raw / 16.0;
  }
  fahrenheit = celsius * 1.8 + 32.0;
  // end of test code
  
  // debug output
  Serial.print("  Temperature = ");
  Serial.print(celsius);
  Serial.print(" Celsius, ");
  Serial.print(fahrenheit);
  Serial.println(" Fahrenheit");
}