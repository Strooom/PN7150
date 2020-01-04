// #############################################################################
// ###                                                                       ###
// ### NXP PN7150 Driver                                                     ###
// ###                                                                       ###
// ### https://github.com/Strooom/PN7150                                     ###
// ### Author(s) : Pascal Roobrouck - @strooom                               ###
// ### License : https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode ###
// ###                                                                       ###
// #############################################################################

#include "NFCReaderWriter.h"
#include "NCI.h"

NFCReaderWriter::NFCReaderWriter(NCI &theNCI) : theNCI(theNCI)
    {
    }

void NFCReaderWriter::initialize()
    {
    theNCI.initialize();				// initialize the NCI stateMachine and other. Will in its turn initialize the HW interface
    theNCI.registrate(this);			// registrate this NFCReaderWriter instance, so the NCI can callback in case of events..

    // further initialize the reader/writer application if needed
    for (uint8_t index = 0; index < maxNmbrTags; index++)
        {
        theTags[index].uniqueIdLength = 0;
        }
    }

void NFCReaderWriter::run()
    {
    theNCI.run();
    }

void NFCReaderWriter::reportTagProperties(uint8_t msgBuffer[], uint8_t msgType)
    {
    // Store the properties of detected TAGs in the Tag array.
    // Tag info can come in two different NCI messages : RF_DISCOVER_NTF and RF_INTF_ACTIVATED_NTF

    if (nmbrOfTags < maxNmbrTags)
        {
        uint8_t offSet;	// Offset in the NCI message where we can find the UniqueID
        switch (msgType)
            {
            case RF_INTF_ACTIVATED_NTF:
                offSet = 12;
                break;

            case RF_DISCOVER_NTF:
                offSet = 9;
                break;

            default:
                return;		// unknown type of msg sent here ?? we just ignore it..
                break;
            }

        uint8_t NfcId1Length = msgBuffer[offSet];
        if (NfcId1Length > 10)
            {
            NfcId1Length = 10;														// limit the length to 10, so in case of whatever error we don't write beyond the boundaries of the array
            }
        uint8_t newTagIndex = nmbrOfTags;											// index to the array item where we will store the info

        theTags[newTagIndex].uniqueIdLength = NfcId1Length;							// copy the length of the unique ID, is 4, 7 or 10
        for (uint8_t index = 0; index < NfcId1Length; index++)						// copy all bytes of the unique ID
            {
            theTags[newTagIndex].uniqueId[index] = msgBuffer[offSet + 1 +index];
            }
        theTags[newTagIndex].detectionTimestamp = millis();

        nmbrOfTags++;																// one more tag in the array now
        }
    }
