// SPDX-License-Identifier: CC-BY-NC-SA-4.0 OR GPL-3.0-or-later
// #############################################################################
// ###                                                                       ###
// ### NXP PN7150 Driver                                                     ###
// ###                                                                       ###
// ### https://github.com/Strooom/PN7150                                     ###
// ### Author(s) : Pascal Roobrouck - @strooom                               ###
// ### License : CC-BY-NC-SA-4.0 OR GPL-3.0-or-later                         ###
// ###                                                                       ###
// #############################################################################

#include "tag.h"


// void Tag::print() const {
//     Serial.print("UniqueID = ");
//     for (uint8_t index = 0; index < uniqueIdLength; index++) {
//         uint8_t theValue = uniqueId[index];
//         if (theValue < 16) {
//             Serial.print("0x0");
//         } else {
//             Serial.print("0x");
//         }
//         Serial.print(theValue, HEX);
//         Serial.print(" ");
//     }
// }

uint8_t Tag::getLength() const {
    uint8_t length = uniqueIdLength;
    return length;
}

uint8_t Tag::getID(uint8_t index) const {
    uint8_t value = uniqueId[index];
    return value;
}

void Tag::clear() {
    uniqueIdLength = 0;
    for (int i = 0; i < maxUniqueIdLength; i++) {
        uniqueId[i] = 0;
    }
}

bool Tag::isSame(Tag *otherTag) const {
    // First compare the length of both UIDs to see if they are the same length
    if (uniqueIdLength == otherTag->uniqueIdLength) {
        // Now check byte per byte if they are the same
        for (uint8_t i = 0; i < uniqueIdLength; i++) {
            if (uniqueId[i] != otherTag->uniqueId[i]) {
                return false;
            }
        }
        return true;
    } else {
        return false;
    }
}

// void Tag::dump() const {
//     char textLine[69] = "uniqueID =";
//     char textPart[8];
//     for (int i = 0; i < uniqueIdLength; i++) {
//         snprintf(textPart, 8, " 0x%02X", uniqueId[i]);
//         strcat(textLine, textPart);
//     }
// }