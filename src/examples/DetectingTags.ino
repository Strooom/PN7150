// #############################################################################
// ###                                                                       ###
// ### NXP PN7150 Driver : example : detect tags and show their unique ID    ###
// ###                                                                       ###
// ### https://github.com/Strooom/PN7150                                     ###
// ### Author(s) : Pascal Roobrouck - @strooom                               ###
// ### License : https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode ###
// ###                                                                       ###
// #############################################################################


#include "PN7150Interface.h"									// NCI protocol runs over a hardware interface, in this case an I2C with 2 extra handshaking signals
#include "NCI.h"												// Talking to the NFC module is done in an NCI language
#include "NFCReaderWriter.h"									// Implementing a NFC Reader/Writer application


PN7150Interface	theInterface = PN7150Interface(32, 14);			// creates a global NFC device interface object, attached to pins 32 (IRQ) and 14 (VEN) and using the default I2C address 0x28
NCI theNCI(theInterface);										// creates a global NCI object, referring to the underlaying HW interface object. Application mode is set default to NciApplicationMode::CardReadWrite
NFCReaderWriter theReaderWriter(theNCI);						// creates a global NFC Reader/Writer application object, referring to the underlaying NCI object.

void setup()
    {
    delay(50);
    Serial.begin(115200);
    delay(50);
	Serial.print("\n\n\n\n");

    theReaderWriter.initialize();								// initialize the application object. It will will in its turn initialize the underlaying NCI object, and this one in its turn the HW interface object
    }

void loop()
    {
    theReaderWriter.run();										// give the application object some CPU time to do its job. This is a non-blocking function
    }
