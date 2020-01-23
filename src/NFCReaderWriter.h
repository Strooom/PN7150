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


class NCI;													// Forward declaration

enum class ReaderWriterState : uint8_t
    {
    initializing,
    noTagPresent,
    singleTagPresent,
    multipleTagsPresent
    };

class NFCReaderWriter
    {
    private:
        NCI &theNCI;										// reference to the NCI instance being used to talk to the NFC device
		ReaderWriterState theState;

    public:
        NFCReaderWriter(NCI &theNCI);						// constructor
        void initialize();									// initialize the Reader/Writer application. Will in its turn initialize the NCI layer and the HW interface on which the application relies
        void run();
		ReaderWriterState getState() const;					// returns the actual state to trigger events like a sound
    };

#endif

