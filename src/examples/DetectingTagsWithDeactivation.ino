// #############################################################################
// ###                                                                       ###
// ### NXP PN7150 Driver : example : detect tags and show their unique ID    ###
// ###                                                                       ###
// ### https://github.com/Strooom/PN7150                                     ###
// ### Author(s) : Pascal Roobrouck - @strooom                               ###
// ### License : https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode ###
// ###                                                                       ###
// #############################################################################

//#include "logging.h"											// Toolkit to allow easy logging towards, eg. Serial UART
//#include "NciToolkit.h"

#include "PN7150Interface.h"									// NCI protocol runs over a hardware interface, in this case an I2C with 2 extra handshaking signals
#include "NCI.h"												// Talking to the NFC module is done in an NCI language
#include "NFCReaderWriter.h"									// Implementing a NFC Reader/Writer application

//Logger theLog;												// creates a global object to log events, data, etc

PN7150Interface	theInterface = PN7150Interface(14, 13);			// creates a global NFC device interface object, attached to pins 14 (IRQ) and 13 (VEN) and using the default I2C address 0x28
NCI theNCI(theInterface);										// creates a global NCI object, referring to the underlaying HW interface object. Application mode is set default to NciApplicationMode::CardReadWrite
NFCReaderWriter theReaderWriter(theNCI);						// creates a global NFC Reader/Writer application object, referring to the underlaying NCI object.

void setup()
    {
    delay(50);
    Serial.begin(155200);
    delay(50);

    theLog.outputIsAvailable = true;

    theReaderWriter.initialize();								// initialize the application object. It will will in its turn initialize the underlaying NCI object, and this one in its turn the HW interface object
    }

void loop()
    {
    theReaderWriter.run();										// give the application object some CPU time to do its job. This is a non-blocking function

    // When we get to a state where we detected one or more cards, print what we've found
    if ((theNCI.getState() == NciState::RfPollActive) || (theNCI.getState() == NciState::RfWaitForHostSelect))
        {
        for (uint8_t cardIndex = 0; cardIndex < theReaderWriter.nmbrOfTags; cardIndex++)
            {
            Serial.print("Tag[");
            Serial.print(cardIndex);
            Serial.print("] : UniqueID = ");
            for (uint8_t index = 0; index < theReaderWriter.theTags[cardIndex].uniqueIdLength; index++)
                {
                uint8_t theValue = theReaderWriter.theTags[cardIndex].uniqueId[index];
                if (theValue < 16)
                    {
                    Serial.print("0x0");
                    }
                else
                    {
                    Serial.print("0x");
                    }
                Serial.print(theValue, HEX);
                Serial.print(" ");
                }
            Serial.println("");
            }

        theReaderWriter.nmbrOfTags = 0;
		theNCI.deActivate(NciRfDeAcivationMode::IdleMode);
        }
    }
