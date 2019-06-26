#include "DS18.h"
#include <string.h>

DS18::DS18(uint16_t pin, bool parasitic)
  :
  _wire{pin},
  _parasitic{parasitic},
   // maybe 750ms is enough, maybe not, wait 1 sec for conversion
  _conversionTime{1000}
{
  init();
}

void DS18::init() {
  _raw = 0;
  _celsius = 0;
  memset(_addr, 0, sizeof(_addr));
  memset(_data, 0, sizeof(_data));
  _type = WIRE_UNKNOWN;
  _searchDone = false;
  _crcError = false;
}

bool DS18::read() {
  init();

  // Search for the next chip on the 1-Wire bus
  if (!_wire.search(_addr)) {
    _searchDone = true;
    _wire.reset_search();
    return false;
  }

  // Check the CRC
  if (OneWire::crc8(_addr, 7) != _addr[7]) {
    _crcError = true;
    return false;
  }

  // Read the temperature from that chip
  return read(_addr);
}

bool DS18::read(uint8_t addr[8]) {
  // Save the chip ROM information for later
  memcpy(_addr, addr, sizeof(_addr));

  // Identify the type of chip

  // the first ROM byte indicates which chip
  // Return if this is an unknown chip
  switch (addr[0]) {
    case 0x10: _type = WIRE_DS1820; break;
    case 0x28: _type = WIRE_DS18B20; break;
    case 0x22: _type = WIRE_DS1822; break;
    case 0x26: _type = WIRE_DS2438; break;
    default:   _type = WIRE_UNKNOWN; return false;
  }

  // Read the actual temperature!!!

  _wire.reset();               // first clear the 1-wire bus
  _wire.select(_addr);          // now select the device we just found
  int power = _parasitic ? 1 : 0; // whether to leave parasite power on at the end of the conversion
  _wire.write(0x44, power);    // tell it to start a conversion

  // just wait a second while the conversion takes place
  // different chips have different conversion times, check the specs, 1 sec is worse case + 250ms
  // you could also communicate with other devices if you like but you would need
  // to already know their address to select them.

  delay(_conversionTime); // wait for conversion to finish

  // we might do a _wire.depower() (parasite) here, but the reset will take care of it.

  // first make sure current values are in the scratch pad

  _wire.reset();
  _wire.select(_addr);
  _wire.write(0xB8,0);         // Recall Memory 0
  _wire.write(0x00,0);         // Recall Memory 0

  // now read the scratch pad

  _wire.reset();
  _wire.select(_addr);
  _wire.write(0xBE,0);         // Read Scratchpad
  if (_type == WIRE_DS2438) {
    _wire.write(0x00,0);       // The DS2438 needs a page# to read
  }

  // transfer the raw values
  for (unsigned i = 0; i < sizeof(_data); i++) {           // we need 9 bytes
    _data[i] = _wire.read();
  }

  // Check if the CRC matches
  if (OneWire::crc8(_data, 8) != _data[8]) {
    _crcError = true;
    return false;
  }

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  _raw = (_data[1] << 8) | _data[0];
  if (_type == WIRE_DS2438) {
    _raw = (_data[2] << 8) | _data[1];
  }
  byte cfg = (_data[4] & 0x60);

  switch (_type) {
    case WIRE_DS1820:
      _raw = _raw << 3; // 9 bit resolution default
      if (_data[7] == 0x10) {
        // "count remain" gives full 12 bit resolution
        _raw = (_raw & 0xFFF0) + 12 - _data[6];
      }
      _celsius = (float)_raw * 0.0625;
      break;
    case WIRE_DS18B20:
    case WIRE_DS1822:
      // at lower res, the low bits are undefined, so let's zero them
      if (cfg == 0x00) _raw = _raw & ~7;  // 9 bit resolution, 93.75 ms
      if (cfg == 0x20) _raw = _raw & ~3; // 10 bit res, 187.5 ms
      if (cfg == 0x40) _raw = _raw & ~1; // 11 bit res, 375 ms
      // default is 12 bit resolution, 750 ms conversion time
      _celsius = (float)_raw * 0.0625;
      break;

    case WIRE_DS2438:
      _data[1] = (_data[1] >> 3) & 0x1f;
      if (_data[2] > 127) {
        _celsius = (float)_data[2] - ((float)_data[1] * .03125);
      } else {
        _celsius = (float)_data[2] + ((float)_data[1] * .03125);
      }
  }

  // Got a good reading!
  return true;
}

int16_t DS18::raw() {
  return _raw;
}

float DS18::celsius() {
  return _celsius;
}

float DS18::fahrenheit() {
  return _celsius * 1.8 + 32.0;
}

void DS18::addr(uint8_t dest[8]) {
  memcpy(dest, _addr, sizeof(_addr));
}

void DS18::data(uint8_t data[9]) {
  memcpy(data, _data, sizeof(_data));
}

DS18Type DS18::type() {
  return _type;
}

bool DS18::searchDone() {
  return _searchDone;
}

bool DS18::crcError() {
  return _crcError;
}

void DS18::setConversionTime(uint16_t ms) {
  _conversionTime = ms;
}
