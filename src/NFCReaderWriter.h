// #############################################################################
// ###                                                                       ###
// ### NXP PN7150 Driver                                                     ###
// ###                                                                       ###
// ### https://github.com/Strooom/PN7150                                     ###
// ### Author(s) : Pascal Roobrouck - @strooom                               ###
// ### License : https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode ###
// ###                                                                       ###
// #############################################################################

#ifndef NFCReaderWriter_h									// Header Guard
#define NFCReaderWriter_h

#include <Arduino.h>										// Gives us access to all typical Arduino types and functions

class NCI;
//#include "NCI.h"

// Class to hold the data of an NFC tag
class Tag
    {
    public:
        uint8_t uniqueIdLength = 0;							// How long is the NFCID1 of the tag. Can be 4, 7 or 10 bytes. Typically 4 or 7.
        uint8_t uniqueId[10];								// array to store the NFCID1. Maximum length is 10 bytes at this time..
        unsigned long detectionTimestamp;					// remembers the time at which the tag was detected
    };


class NFCReaderWriter
    {
    public:
        static constexpr unsigned long scanPeriod = 1000;					// lenght of a scan for tags cycle, in milliseconds. Note : setting this to very short times, eg. < 100 ms will not work, because the NFC discovery loop has a certain minumum constrained by the HW protocols
        static constexpr uint8_t maxNmbrTags = 4;							// maximum number of (simultaneously present) tags we can keep track of. PN7150 is limited to 3
        Tag theTags[maxNmbrTags];											// array to store the data of a number of currently present tags. When uniqueIdLenght == 0 it means invalid data in this position of the array
		uint8_t nmbrOfTags = 0;												// how many tags are actually in the array

    private:
        NCI &theNCI;														// reference to the NCI instance being used to talk to the NFC device

    public:
        NFCReaderWriter(NCI &theNCI);										// constructor
        void initialize();													// initialize the Reader/Writer application. Will in its turn initialize the NCI layer and the HW interface on which the application relies
        void run();
		void reportTagProperties(uint8_t msgBuffer[], uint8_t msgType);		// takes the notification message in the msgBuffer, interprete it, extracts the TAG info from it and store this info in theTags[] array.
    };


#endif

