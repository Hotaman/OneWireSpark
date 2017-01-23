#ifndef OneWire_h
#define OneWire_h

#include <inttypes.h>
#include "application.h"

// you can exclude onewire_search by defining that to 0
#ifndef ONEWIRE_SEARCH
#define ONEWIRE_SEARCH 1
#endif

// You can exclude CRC checks altogether by defining this to 0
#ifndef ONEWIRE_CRC
#define ONEWIRE_CRC 1
#endif



// You can allow 16-bit CRC checks by defining this to 1
// (Note that ONEWIRE_CRC must also be 1.)
#ifndef ONEWIRE_CRC16
#define ONEWIRE_CRC16 1
#endif

// TRUE and FALSE are defined by default on the Spark
// #define FALSE 0
// #define TRUE  1

class OneWire
{
private:
  uint16_t _pin;

/**************Conditional fast pin access for Core and Photon*****************/
  #if PLATFORM_ID == 0 // Core
    // Fast pin access for STM32F1xx microcontroller
    inline void digitalWriteFastLow() {
      PIN_MAP[_pin].gpio_peripheral->BRR = PIN_MAP[_pin].gpio_pin;
    }

    inline void digitalWriteFastHigh() {
      PIN_MAP[_pin].gpio_peripheral->BSRR = PIN_MAP[_pin].gpio_pin;
    }

    inline void pinModeFastOutput() {
      GPIO_TypeDef *gpio_port = PIN_MAP[_pin].gpio_peripheral;
      uint16_t gpio_pin = PIN_MAP[_pin].gpio_pin;

      GPIO_InitTypeDef GPIO_InitStructure;

      if (gpio_port == GPIOA )
      {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
      }
      else if (gpio_port == GPIOB )
      {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
      }

      GPIO_InitStructure.GPIO_Pin = gpio_pin;
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
      PIN_MAP[_pin].pin_mode = OUTPUT;
      GPIO_Init(gpio_port, &GPIO_InitStructure);
    }

    inline void pinModeFastInput() {
      GPIO_TypeDef *gpio_port = PIN_MAP[_pin].gpio_peripheral;
      uint16_t gpio_pin = PIN_MAP[_pin].gpio_pin;

      GPIO_InitTypeDef GPIO_InitStructure;

      if (gpio_port == GPIOA )
      {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
      }
      else if (gpio_port == GPIOB )
      {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
      }

      GPIO_InitStructure.GPIO_Pin = gpio_pin;
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
      PIN_MAP[_pin].pin_mode = INPUT;
      GPIO_Init(gpio_port, &GPIO_InitStructure);
    }

    inline uint8_t digitalReadFast() {
      return GPIO_ReadInputDataBit(PIN_MAP[_pin].gpio_peripheral, PIN_MAP[_pin].gpio_pin);
    }

  // Assume all other platforms are STM32F2xx until proven otherwise
  //#elif PLATFORM_ID == 6 || PLATFORM_ID == 8 || PLATFORM_ID == 10  // Photon(P0),P1,Electron
  #else
    // Fast pin access for STM32F2xx microcontroller
    STM32_Pin_Info* PIN_MAP = HAL_Pin_Map(); // Pointer required for highest access speed

    inline void digitalWriteFastLow() {
      PIN_MAP[_pin].gpio_peripheral->BSRRH = PIN_MAP[_pin].gpio_pin;
    }

    inline void digitalWriteFastHigh() {
      PIN_MAP[_pin].gpio_peripheral->BSRRL = PIN_MAP[_pin].gpio_pin;
    }

    inline void pinModeFastOutput(void){
      // This could probably be speed up by digging a little deeper past
      // the HAL_Pin_Mode function.
      HAL_Pin_Mode(_pin, OUTPUT);
    }

    inline void pinModeFastInput(void){
      // This could probably be speed up by digging a little deeper past
      // the HAL_Pin_Mode function.
      HAL_Pin_Mode(_pin, INPUT);
    }

    inline uint8_t digitalReadFast(void){
      // This could probably be speed up by digging a little deeper past
      // the HAL_GPIO_Read function.
      return HAL_GPIO_Read(_pin);
    }

  //#else  // no need for this right now
    //#error "*** PLATFORM_ID not supported by this library. PLATFORM should be Core, Photon, P1 or Electron ***"
  #endif
/**************End conditional fast pin access for Core and Photon*************/

#if ONEWIRE_SEARCH
    // global search state
    unsigned char ROM_NO[8];
    uint8_t LastDiscrepancy;
    uint8_t LastFamilyDiscrepancy;
    uint8_t LastDeviceFlag;
#endif

  public:
    OneWire( uint16_t pin);

    // Perform a 1-Wire reset cycle. Returns 1 if a device responds
    // with a presence pulse.  Returns 0 if there is no device or the
    // bus is shorted or otherwise held low for more than 250uS
    uint8_t reset(void);

    // Issue a 1-Wire rom select command, you do the reset first.
    void select(const uint8_t rom[8]);

    // Issue a 1-Wire rom skip command, to address all on bus.
    void skip(void);

    // Write a byte. If 'power' is one then the wire is held high at
    // the end for parasitically powered devices. You are responsible
    // for eventually depowering it by calling depower() or doing
    // another read or write.
    void write(uint8_t v, uint8_t power = 0);

    void write_bytes(const uint8_t *buf, uint16_t count, bool power = 0);

    // Read a byte.
    uint8_t read(void);

    void read_bytes(uint8_t *buf, uint16_t count);

    // Write a bit. The bus is always left powered at the end, see
    // note in write() about that.
    void write_bit(uint8_t v);

    // Read a bit.
    uint8_t read_bit(void);

    // Stop forcing power onto the bus. You only need to do this if
    // you used the 'power' flag to write() or used a write_bit() call
    // and aren't about to do another read or write. You would rather
    // not leave this powered if you don't have to, just in case
    // someone shorts your bus.
    void depower(void);

#if ONEWIRE_SEARCH
    // Clear the search state so that if will start from the beginning again.
    void reset_search();

    // Setup the search to find the device type 'family_code' on the next call
    // to search(*newAddr) if it is present.
    void target_search(uint8_t family_code);

    // Look for the next device. Returns 1 if a new address has been
    // returned. A zero might mean that the bus is shorted, there are
    // no devices, or you have already retrieved all of them.  It
    // might be a good idea to check the CRC to make sure you didn't
    // get garbage.  The order is deterministic. You will always get
    // the same devices in the same order.
    uint8_t search(uint8_t *newAddr);
#endif

#if ONEWIRE_CRC
    // Compute a Dallas Semiconductor 8 bit CRC, these are used in the
    // ROM and scratchpad registers.
    static uint8_t crc8(uint8_t *addr, uint8_t len);

#if ONEWIRE_CRC16
    // Compute the 1-Wire CRC16 and compare it against the received CRC.
    // Example usage (reading a DS2408):
    //    // Put everything in a buffer so we can compute the CRC easily.
    //    uint8_t buf[13];
    //    buf[0] = 0xF0;    // Read PIO Registers
    //    buf[1] = 0x88;    // LSB address
    //    buf[2] = 0x00;    // MSB address
    //    WriteBytes(net, buf, 3);    // Write 3 cmd bytes
    //    ReadBytes(net, buf+3, 10);  // Read 6 data bytes, 2 0xFF, 2 CRC16
    //    if (!CheckCRC16(buf, 11, &buf[11])) {
    //        // Handle error.
    //    }
    //
    // @param input - Array of bytes to checksum.
    // @param len - How many bytes to use.
    // @param inverted_crc - The two CRC16 bytes in the received data.
    //                       This should just point into the received data,
    //                       *not* at a 16-bit integer.
    // @param crc - The crc starting value (optional)
    // @return True, iff the CRC matches.
    static bool check_crc16(const uint8_t* input, uint16_t len, const uint8_t* inverted_crc, uint16_t crc = 0);

    // Compute a Dallas Semiconductor 16 bit CRC.  This is required to check
    // the integrity of data received from many 1-Wire devices.  Note that the
    // CRC computed here is *not* what you'll get from the 1-Wire network,
    // for two reasons:
    //   1) The CRC is transmitted bitwise inverted.
    //   2) Depending on the endian-ness of your processor, the binary
    //      representation of the two-byte return value may have a different
    //      byte order than the two bytes you get from 1-Wire.
    // @param input - Array of bytes to checksum.
    // @param len - How many bytes to use.
    // @param crc - The crc starting value (optional)
    // @return The CRC16, as defined by Dallas Semiconductor.
    static uint16_t crc16(const uint8_t* input, uint16_t len, uint16_t crc = 0);
#endif
#endif
};

#endif // OneWire_h
