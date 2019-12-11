#ifndef PN7150Interface_h												// Header Guard
#define PN7150Interface_h

// #############################################################################
// ###                                                                       ###
// ### NXP PN7150 Driver                                                     ###
// ###                                                                       ###
// ### https://github.com/Strooom/PN7150                                     ###
// ### Author(s) : Pascal Roobrouck - @strooom                               ###
// ### License : https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode ###
// ###                                                                       ###
// #############################################################################


// Summary :
//   This library implements the hardware interface for the NXP PN7150 NFC device.
//   The PN7150 uses I2C plus 2 additional data signals : IRQ and VEN
//     IRQ :
//     VEN :
//
//   The library provides :
//   * init() : Initialization of the interface
//   * read()
//   * write()
//   * hasMessage() 

#include <Arduino.h>											// Gives us access to all typical Arduino types and functions
#include <i2c_t3.h>												// the HW interface between The PN7150 and the DeviceHost is I2C, so we need the I2C library. As I'm developing this on Teensy3.2, i use that I2C library

class PN7150Interface
    {
    public:
        PN7150Interface(uint8_t IRQpin, uint8_t VENpin);							// Constructor with default I2C address
        PN7150Interface(uint8_t IRQpin, uint8_t VENpin, uint8_t I2Caddress);		// Constructor with custom I2C address
        void initialize(void);														// Initialize the HW interface at the Device Host
        uint8_t write(uint8_t data[], uint32_t dataLength);							// write data from DeviceHost to PN7150. Returns success (0) or Fail (> 0)
        uint32_t read(uint8_t data[]);												// read data from PN7150
        bool hasMessage();															// does the PN7150 indicate it has data for the DeviceHost to be read

    private:
        uint8_t IRQpin;											// MCU pin to which IRQ is connected
        uint8_t VENpin;											// MCU pin to which VEN is connected
        uint8_t I2Caddress;										// I2C Address at which the PN7150 is found. Default is 0x28, but can be adjusted by setting to pins at the device

    public:
        void test001();											// testing VEN output on the HW
        void test002();											// testing IRQ input on the HW
        void test003();											// testing the sending of an I2C message
        void test004();											// testing the notification of data from the PN7150 by rising IRQ
        void test005();											// sending CORE_RESET_CMD and checking if the PN7150 responds with CORE_RESET_RSP
    };

#endif															// End Header Guard

