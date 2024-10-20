#pragma once

// SPDX-License-Identifier: CC-BY-NC-SA-4.0 OR GPL-3.0-or-later
// #############################################################################
// ###                                                                       ###
// ### NXP PN7150 Driver                                                     ###
// ###                                                                       ###
// ### https://github.com/Strooom/PN7150                                     ###
// ### Author(s) : Pascal Roobrouck - @strooom                               ###
// ### License : CC-BY-NC-SA-4.0 OR GPL-3.0-or-later                         ###
// ###                                                                       ###
// ### Credits : Thomas Buhot, for his PN7120 library, the predecessor of    ###
// ### the PN7150. Due to the NFC Specs not being available for free, I used ###
// ### his library to reverse-engineer some of the missing information links ###
// ###                                                                       ###
// #############################################################################

#include <stdint.h>        // Gives us access to uint8_t types etc

// Todo : upgrade everything to uint32 io uint8 as this is faster on 32-bit MCUs

class Tag {
  public:
    static constexpr uint32_t maxUniqueIdLength{10U};        //
    uint8_t uniqueIdLength{0};                               // How long is the NFCID1 of the tag. Can be 4, 7 or 10 bytes. Typically 4 or 7.
    uint8_t uniqueId[maxUniqueIdLength]{0};                  // array to store the NFCID1. Maximum length is 10 bytes at this time..
    unsigned long detectionTimestamp;                        // remembers the time at which the tag was detected

  public:
    // void print() const;                        // prints all properties of the tag to Serial
    uint8_t getLength() const;                 // returns the length of the UID
    uint8_t getID(uint8_t index) const;        // get the UID value at a certain index
    void clear();                              // resets the datafields to their defaults
    bool isSame(Tag *otherTag) const;          // compares this tag to another tag to see if they are identical
    //v void dump() const;                         // send tag data to logging
};
