#pragma once

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
//     IRQ : output of the PN7150, input for the MCU. Through this signal the PN7150 signals it has data to be read by the MCU
//     VEN : input of the PN7150, output for the MCU. Through this signal the MCU can RESET the PN7150
//
//   The library provides :
//   * init() : Initialization of the interface
//   * read() : Read message from PN7150 over I2C
//   * write() : Write message to PN7150 over I2C
//   * hasMessage() : Check if PN7150 has message waiting for MCU

#include <stdint.h>                                  // Gives us access to uint8_t types etc.
                                                     // The HW interface between The PN7150 and the DeviceHost is I2C, so we need the I2C library.library
#if defined(TEENSYDUINO) && defined(KINETISK)        // Teensy 3.0, 3.1, 3.2, 3.5, 3.6 :  Special, more optimized I2C library for Teensy boards
#include <i2c_t3.h>                                  // Credits Brian "nox771" : see https://forum.pjrc.com/threads/21680-New-I2C-library-for-Teensy3
#else
#include <Wire.h>        // Otherwise, just use the more standard Wire.h - For ESP32 this will link in a version dedicated for this MCU
                         // TODO :	i2c_t3.h ensures a maximum I2C message of 259, which is sufficient. Other I2C implementations have shorter buffers (32 bytes)
                         //			See : https://github.com/Strooom/PN7150/issues/7
#endif

class PN7150Interface {
  public:
    PN7150Interface(uint8_t IRQ, uint8_t VEN);                            // Constructor with default I2C address
    PN7150Interface(uint8_t IRQ, uint8_t VEN, uint8_t I2Caddress);        // Constructor with custom I2C address
    void initialize(void);                                                // Initialize the HW interface at the Device Host
    uint8_t write(uint8_t data[], uint32_t dataLength) const;             // write data from DeviceHost to PN7150. Returns success (0) or Fail (> 0)
    uint32_t read(uint8_t data[]) const;                                  // read data from PN7150, returns the amount of bytes read
    bool hasMessage() const;                                              // does the PN7150 indicate it has data for the DeviceHost to be read

  private:
    uint8_t IRQ;               // MCU pin to which IRQ is connected
    uint8_t VEN;               // MCU pin to which VEN is connected
    uint8_t I2Caddress;        // I2C Address at which the PN7150 is found. Default is 0x28, but can be adjusted by setting to pins at the device

    // public:
    //     void test001();											// testing VEN output on the HW
    //     void test002();											// testing IRQ input on the HW
    //     void test003();											// testing the sending of an I2C message
    //     void test004();											// testing the notification of data from the PN7150 by rising IRQ
    //     void test005();											// sending CORE_RESET_CMD and checking if the PN7150 responds with CORE_RESET_RSP
};
