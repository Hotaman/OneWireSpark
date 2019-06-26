# One Wire

The One Wire (1-Wire) protocol is used in the popular DS18B20 temperature sensor and other devices from Maxim (formerly Dallas).

This library implements the Dallas One Wire (1-wire) protocol on the Particle Photon, Electron, Core, P0/P1, Red Bear Duo and compatible devices.

## Usage

If you are using a DS18B20, DS1820 or DS1822 temperature sensor, you can simply use the `DS18` object to read temperature.

Connect sensor:
- pin 1 (1-Wire ground) to ground.
- pin 2 (1-Wire signal) to `D0` (or another pin) with a 2K-10K resistor to pin 3.
- pin 3 (1-Wire power) to 3V3 or VIN.

```
#include "DS18.h"

DS18 sensor(D0);

void loop() {
  if (sensor.read()) {
    Particle.publish("temperature", String(sensor.celsius()), PRIVATE);
  }
}
```

If you use another chip or you want to customize the behavior you can copy-paste one of the examples and modify it.

## documentation

### `DS18`

```
DS18 sensor(pin);
DS18 sensor(pin, parasitic);
```

Create an object to interact with one or more DS18x20 sensors connected to `pin` (D0-D7, A0-A7, etc).

If `parasitic` is `true`, the power will be maintained at the end of a conversion so the sensor can parasitically draw power from the data pin. This mode is discourage since it can be harder to set up correctly. The value of the pull-up resistor is important in parasitic mode. See the [references](#references).

Save yourself some trouble, buy some DS18B20 (not DS18B20-PAR) and use the 3 pin powered mode.

## `read()`

`bool succes = sensor.read();`

Search for the next temperature sensor on the bus and start a conversion. Return `true` when the temperature is valid.

The default conversion time is 1 second.

Since it performs a 1-Wire search each time if you only have 1 sensor it's normal for this function to return `false` every other time.

If you have more than 1 sensor, check [`addr()`](#addr) to see which sensor was just read.

`bool succes = sensor.read(addr);`

Read a specific sensor, skiping the search. You could set up your code to `read()` once in `setup()`, save the `addr` and in `loop` always read this sensor only.

## `celsius()`
## `fahrenheit()`

```
float temperature = sensor.celsius();
float temperature = sensor.fahrenheit();
```

Return the temperature of the last read device. Only call after `read()` returns `true`.

_Note: on the DS18B20, 85 C is returned when there's a wiring issue, possibly not enough current to convert the temperature in parasitic mode. Check the pull up value._

## `searchDone()`

`bool done = sensor.searchDone();`

If `read()` returns `false`, check `searchDone()`. If `true`, this is not an error case. The next `read()` will start from the first temperature sensor again.

## `crcError()`

`bool error = sensor.crcError();`

Returns `true` when bad data was received. ¯\\_(ツ)_/¯

## `addr()`

```
uint8_t addr[8];
sensor.addr(addr);
```

Copies the 1-Wire ROM data / address of the last read device in the buffer. All zeros if no device was found or search is done.

See the datasheet for your device to decode this.

## `type()`

`DS18Type type = sensor.type();`

The type of the last read device. One of `WIRE_DS1820`, `WIRE_DS18B20`, `WIRE_DS1822`, `WIRE_DS2438` or `WIRE_UNKNOWN` if the device is not a temperature sensor, no device was found or the search is done.

## `raw()`

`int16_t value = sensor.raw();`

Integer value of the temperature without scaling. Useful if you want to do integer math on the temperature. The scaling between the raw value and physical value depends on the sensor.

## `data()`

```
uint8_t data[9];
sensor.data(data);
```

Copies the 1-Wire scratchpad RAM / data of the last read device in the buffer. All zeros if there was a CRC error in the address search, no device was fuond or search is done.

See the datasheet for your device to decode this.

## `setConversionTime`

`sensor.setConversionTime(milliseconds);`

This library pauses for 1000 milliseconds while waiting for the temperature conversion to take place. Check the datasheet before reducing this value.

## `OneWire`

`OneWire`, the 1-Wire protocol implementation used by the `DS18` object, is documented in [its header file.](src/OneWire.h)

## References

- [DS18B20 datasheet](http://datasheets.maximintegrated.com/en/ds/DS18B20.pdf)
- [How to Power 1-Wire Devices](https://www.maximintegrated.com/en/app-notes/index.mvp/id/4255): especially useful for devices using parasitic power

## License

Copyright 2016 Hotaman, Julien, Vanier, and many contributors (see individual files)

Licensed under the MIT license.
