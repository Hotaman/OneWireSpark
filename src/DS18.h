#ifndef DS18_h
#define DS18_h

#include "OneWire.h"

enum DS18Type {
  WIRE_UNKNOWN,
  WIRE_DS1820,
  WIRE_DS18B20,
  WIRE_DS1822,
  WIRE_DS2438
};

class DS18 {
public:
  DS18(uint16_t pin, bool parasitic = false);

  bool read();
  bool read(uint8_t addr[8]);
  int16_t raw();
  float celsius();
  float fahrenheit();
  void addr(uint8_t dest[8]);
  void data(uint8_t dest[9]);
  DS18Type type();

  bool searchDone();
  bool crcError();

  void setConversionTime(uint16_t ms);

private:
  void init();

  OneWire _wire;
  bool _parasitic;
  uint16_t _conversionTime;
  int16_t _raw;
  float _celsius;
  uint8_t _addr[8];
  uint8_t _data[9];
  DS18Type _type;
  bool _searchDone;
  bool _crcError;
};

#endif // DS18_h
