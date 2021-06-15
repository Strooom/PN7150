#include "NCI.h"
#include "logging.h"

extern uLog theLog;

NCI::NCI(PN7150Interface& aHardwareInterface) : theHardwareInterface(aHardwareInterface), theState(NciState::HwResetRfc), theTagsStatus(TagsPresentStatus::unknown) {
}

void NCI::initialize() {
    theHardwareInterface.initialize();
    theState      = NciState::HwResetRfc;        // re-initializing the state, so we can re-initialize at anytime
    theTagsStatus = TagsPresentStatus::unknown;
    nmbrOfTags    = 0;
}

void NCI::run() {
    switch (theState) {
        case NciState::HwResetRfc:        // after Hardware reset / powerOn
        {
            uint8_t payloadData[] = {ResetKeepConfig};                                       // CORE_RESET-CMD with Keep Configuration
            sendMessage(MsgTypeCommand, GroupIdCore, CORE_RESET_CMD, payloadData, 1);        //
            setTimeOut(20);                                                                  // we should get a RESPONSE within 20 ms (it typically takes 2.3ms)
            theState = NciState::HwResetWfr;                                                 // move to next state, waiting for the matching Response
        } break;

        case NciState::HwResetWfr:
            if (theHardwareInterface.hasMessage()) {
                getMessage();
                bool isOk = (6 == rxMessageLength);                                                     // Does the received Msg have the correct lenght ?
                isOk      = isOk && isMessageType(MsgTypeResponse, GroupIdCore, CORE_RESET_RSP);        // Is the received Msg the correct type ?
                isOk      = isOk && (STATUS_OK == rxBuffer[3]);                                         // Is the received Status code Status_OK ?

                if (isOk)        // if everything is OK...
                {
                    theState = NciState::SwResetRfc;        // ..move to the next state
                } else                                      // if not..
                {
                    theLog.snprintf(loggingLevel::Error, "NCI invalid response at line %d", __LINE__);
                    theState = NciState::Error;        // goto error state
                }
            } else if (isTimeOut()) {
                theLog.snprintf(loggingLevel::Error, "NCI no response at line %d", __LINE__);
                theState = NciState::Error;        // time out waiting for response..
            }
            break;

        case NciState::SwResetRfc: {
            sendMessage(MsgTypeCommand, GroupIdCore, CORE_INIT_CMD);        // CORE_INIT-CMD
            setTimeOut(20);                         // we should get a RESPONSE within 20 ms, typically it takes 0.5ms
            theState = NciState::SwResetWfr;        // move to next state, waiting for response
        } break;

        case NciState::SwResetWfr:
            if (theHardwareInterface.hasMessage()) {
                getMessage();
                bool isOk = isMessageType(MsgTypeResponse, GroupIdCore, CORE_INIT_RSP);        // Is the received Msg the correct type ?

                if (isOk)        // if everything is OK...
                {
                    theState = NciState::EnableCustomCommandsRfc;        // ...move to the next state
                } else                                                   // if not..
                {
                    theLog.snprintf(loggingLevel::Error, "NCI invalid response at line %d", __LINE__);
                    theState = NciState::Error;        // .. goto error state
                }
            } else if (isTimeOut()) {
                theLog.snprintf(loggingLevel::Error, "NCI no response at line %d", __LINE__);
                theState = NciState::Error;        // time out waiting for response..
            }
            break;

        case NciState::EnableCustomCommandsRfc:
            sendMessage(MsgTypeCommand, GroupIdProprietary, NCI_PROPRIETARY_ACT_CMD);        // Send NCI_PROPRIETARY_ACT_CMD to activate extra PN7150-NCI features
            setTimeOut(10);                                                                  // we should get a RESPONSE within 10 ms, typically it takes 0.5ms
            theState = NciState::EnableCustomCommandsWfr;                                    // move to next state, waiting for response
            break;

        case NciState::EnableCustomCommandsWfr:
            if (theHardwareInterface.hasMessage()) {
                getMessage();
                bool isOk = isMessageType(MsgTypeResponse, GroupIdProprietary, NCI_PROPRIETARY_ACT_RSP);        // Is the received Msg the correct type ?
                isOk      = isOk && (STATUS_OK == rxBuffer[3]);                                                 // Is the received Status code Status_OK ?

                if (isOk) {                                // if everything is OK...
                    theState = NciState::RfIdleCmd;        // ...move to the next state
                } else {                                   // if not..
                    theLog.snprintf(loggingLevel::Error, "NCI invalid response at line %d", __LINE__);
                    theState = NciState::Error;        // .. goto error state
                }
            } else if (isTimeOut()) {
                theLog.snprintf(loggingLevel::Error, "NCI no response at line %d", __LINE__);
                theState = NciState::Error;        // time out waiting for response..
            }
            break;

        case NciState::RfIdleCmd: {
            // After configuring, we are ready to go into Discovery, but we wait for the readerWriter application to give us this trigger
            // Or we can proceed into polling right away
            activate();
            // uint8_t payloadData[] = {4, NFC_A_PASSIVE_POLL_MODE, 0x01, NFC_B_PASSIVE_POLL_MODE, 0x01, NFC_F_PASSIVE_POLL_MODE, 0x01, NFC_15693_PASSIVE_POLL_MODE, 0x01};
            // sendMessage(MsgTypeCommand, GroupIdRfManagement, RF_DISCOVER_CMD, payloadData, 9);        //
            // setTimeOut(10);                                                                           // we should get a RESPONSE within 10 ms
            // theState = NciState::RfIdleWfr;                                                           // move to next state, waiting for Response
        } break;

        case NciState::RfIdleWfr:
            if (theHardwareInterface.hasMessage()) {
                getMessage();
                bool isOk = (4 == rxMessageLength);                                                              // Does the received Msg have the correct lenght ?
                isOk      = isOk && isMessageType(MsgTypeResponse, GroupIdRfManagement, RF_DISCOVER_RSP);        // Is the received Msg the correct type ?
                isOk      = isOk && (STATUS_OK == rxBuffer[3]);                                                  // Is the received Status code Status_OK ?
                if (isOk)                                                                                        // if everything is OK...
                {
                    theState = NciState::RfDiscovery;        // ...move to the next state
                    setTimeOut(500);                         // set a timeout of 1 second. If it times out, it means no cards are present..
                } else                                       // if not..
                {
                    theState = NciState::Error;        // .. goto error state
                }
            } else if (isTimeOut()) {
                theLog.snprintf(loggingLevel::Error, "NCI no response at line %d", __LINE__);
                theState = NciState::Error;        // time out waiting for response..
            }
            break;

        case NciState::RfDiscovery:
            // TODO : if we have no NTF here, it means no cards are present and we can delete them from the list...
            // Here we don't check timeouts.. we can wait forever for a TAG/CARD to be presented..
            if (theHardwareInterface.hasMessage()) {
                getMessage();
                if (isMessageType(MsgTypeNotification, GroupIdRfManagement, RF_INTF_ACTIVATED_NTF)) {
                    // When a single tag/card is detected, the PN7150 will immediately activate it and send you this type of notification
                    saveTag(RF_INTF_ACTIVATED_NTF);        // save properties of this Tag in the Tags array
                    if (TagsPresentStatus::noTagsPresent == theTagsStatus) {
                        theTagsStatus = TagsPresentStatus::newTagPresent;
                    }
                    theState = NciState::RfPollActive;        // move to PollActive, and wait there for further commands..
                } else if (isMessageType(MsgTypeNotification, GroupIdRfManagement, RF_DISCOVER_NTF)) {
                    // When multiple tags/cards are detected, the PN7150 will notify them all and wait for the DH to select one
                    // The first card will have NotificationType == 2 and move the stateMachine to WaitForAllDiscoveries.
                    // More notifications will come in that state
                    saveTag(RF_DISCOVER_NTF);        // save properties of this Tag in the Tags array
                    setTimeOut(25);                  // we should get more Notifications ubt set a timeout so we don't wait forever
                    theTagsStatus = TagsPresentStatus::multipleTagsPresent;
                    theState      = NciState::RfWaitForAllDiscoveries;
                }
            } else if (isTimeOut()) {
                theTagsStatus = TagsPresentStatus::noTagsPresent;        // this means no card has been detected for xxx millisecond, so we can conclude that no cards are present
            }

            break;

        case NciState::RfWaitForAllDiscoveries:
            if (theHardwareInterface.hasMessage()) {
                getMessage();
                if (isMessageType(MsgTypeNotification, GroupIdRfManagement, RF_DISCOVER_NTF)) {
                    notificationType theNotificationType = (notificationType)rxBuffer[rxBuffer[6] + 7];        // notificationType comes in rxBuffer at the end, = 7 bytes + length of RF Technology Specific parameters which are in rxBuffer[6]
                    switch (theNotificationType) {
                        case notificationType::lastNotification:
                        case notificationType::lastNotificationNfccLimit:
                            saveTag(RF_DISCOVER_NTF);        // save properties of this Tag in the Tags array
                            theState = NciState::RfWaitForHostSelect;
                            break;

                        case notificationType::moreNotification:
                            setTimeOut(25);                  // we should get more Notifications, so set a timeout so we don't wait forever
                            saveTag(RF_DISCOVER_NTF);        // save properties of this Tag in the Tags array
                            break;

                        default:
                            break;
                    }
                }
            } else if (isTimeOut()) {
                theLog.snprintf(loggingLevel::Error, "NCI no response at line %d", __LINE__);
                theState = NciState::Error;        // We need a timeout here, in case the final RF_DISCOVER_NTF with Notification Type == 0 or 1 never comes...
            }
            break;

        case NciState::RfWaitForHostSelect:
            // Multiple cards are present. We could select one and move into RfPollActive
            deActivate(NciRfDeAcivationMode::IdleMode);        //
            break;

        case NciState::RfPollActive:
            // A card is present, so we can read/write data to it. We could also receive a notification that the card has been removed..
            deActivate(NciRfDeAcivationMode::IdleMode);        //
            break;

        case NciState::RfDeActivate1Wfr:
            if (theHardwareInterface.hasMessage()) {
                getMessage();
                if (isMessageType(MsgTypeResponse, GroupIdRfManagement, RF_DEACTIVATE_RSP)) {
                    theState = NciState::RfIdleCmd;
                } else {
                }
            } else if (isTimeOut()) {
                theLog.snprintf(loggingLevel::Error, "NCI no response at line %d", __LINE__);
                theState = NciState::Error;        // We need a timeout here, in case the RF_DEACTIVATE_RSP never comes...
            }
            break;

        case NciState::RfDeActivate2Wfr:
            if (theHardwareInterface.hasMessage()) {
                getMessage();
                if (isMessageType(MsgTypeResponse, GroupIdRfManagement, RF_DEACTIVATE_RSP)) {
                    setTimeOut(10);
                    theState = NciState::RfDeActivate2Wfn;
                } else {
                }
            } else if (isTimeOut()) {
                theLog.snprintf(loggingLevel::Error, "NCI no response at line %d", __LINE__);
                theState = NciState::Error;        // We need a timeout here, in case the RF_DEACTIVATE_RSP never comes...
            }
            break;

        case NciState::RfDeActivate2Wfn:
            if (theHardwareInterface.hasMessage()) {
                getMessage();
                if (isMessageType(MsgTypeNotification, GroupIdRfManagement, RF_DEACTIVATE_NTF)) {
                    theState = NciState::RfIdleCmd;
                } else {
                }
            } else if (isTimeOut()) {
                theLog.snprintf(loggingLevel::Error, "NCI no response at line %d", __LINE__);
                theState = NciState::Error;        // We need a timeout here, in case the RF_DEACTIVATE_RSP never comes...
            }
            break;

        case NciState::Error:
            // Something went wrong, and we made an emergency landing by moving to this state...
            // To get out of it, the parent application can call initialize(), or we could decide to do that ourselves, maybe after some time-out
            initialize();
            break;

        default:
            break;
    }
}

void NCI::activate() {
    NciState tmpState = getState();
    if (tmpState == NciState::RfIdleCmd) {
        uint8_t payloadData[] = {4, NFC_A_PASSIVE_POLL_MODE, 0x01, NFC_B_PASSIVE_POLL_MODE, 0x01, NFC_F_PASSIVE_POLL_MODE, 0x01, NFC_15693_PASSIVE_POLL_MODE, 0x01};
        // TODO : instead of setting a fixed scanning for these 4 types, we should pass the to be scanned for types as parameters from the ReaderWriter... https://github.com/Strooom/PN7150/issues/1
        sendMessage(MsgTypeCommand, GroupIdRfManagement, RF_DISCOVER_CMD, payloadData, 9);        //
        setTimeOut(10);                                                                           // we should get a RESPONSE within 10 ms
        theState = NciState::RfIdleWfr;                                                           // move to next state, waiting for Response
    } else {
        // Error : we can only activate polling when in Idle...
    }
}

void NCI::deActivate(NciRfDeAcivationMode theMode) {
    nmbrOfTags        = 0;
    NciState tmpState = getState();
    switch (tmpState) {
        case NciState::RfWaitForHostSelect: {
            uint8_t payloadData[] = {(uint8_t)NciRfDeAcivationMode::IdleMode};                          // in RfWaitForHostSelect, the deactivation type is ignored by the NFCC
            sendMessage(MsgTypeCommand, GroupIdRfManagement, RF_DEACTIVATE_CMD, payloadData, 1);        //
            setTimeOut(10);                                                                             // we should get a RESPONSE within 10 ms
            theState = NciState::RfDeActivate1Wfr;                                                      // move to next state, waiting for response
        } break;

        case NciState::RfPollActive: {
            uint8_t payloadData[] = {(uint8_t)theMode};
            sendMessage(MsgTypeCommand, GroupIdRfManagement, RF_DEACTIVATE_CMD, payloadData, 1);        //
            setTimeOut(10);                                                                             // we should get a RESPONSE within 10 ms
            theState = NciState::RfDeActivate2Wfr;                                                      // move to next state, waiting for response
        } break;

        default:
            break;
    }
}

NciState NCI::getState() const {
    return theState;
}

TagsPresentStatus NCI::getTagsPresentStatus() const {
    return theTagsStatus;
}

uint8_t NCI::getNmbrOfTags() const {
    return nmbrOfTags;
}

Tag* NCI::getTag(uint8_t index) {
    theTagsStatus = TagsPresentStatus::oldTagPresent;        // after reading the Tag data, we consider it no longer a new tag
    return &theTags[index];
}

void NCI::sendMessage(uint8_t messageType, uint8_t groupId, uint8_t opcodeId) {
    txBuffer[0] = (messageType | groupId) & 0xEF;         // put messageType and groupId in first byte, Packet Boundary Flag is always 0
    txBuffer[1] = opcodeId & 0x3F;                        // put opcodeId in second byte, clear Reserved for Future Use (RFU) bits
    txBuffer[2] = 0x00;                                   // payloadLength goes in third byte
    (void)theHardwareInterface.write(txBuffer, 3);        // TODO :  could make this more robust by checking the return value and go into error is write did not succees
}

void NCI::sendMessage(uint8_t messageType, uint8_t groupId, uint8_t opcodeId, uint8_t payloadData[], uint8_t payloadLength) {
    txBuffer[0] = (messageType | groupId) & 0xEF;                   // put messageType and groupId in first byte, Packet Boundary Flag is always 0
    txBuffer[1] = opcodeId & 0x3F;                                  // put opcodeId in second byte, clear Reserved for Future Use (RFU) bits
    txBuffer[2] = payloadLength;                                    // payloadLength goes in third byte
    for (uint32_t index = 0; index < payloadLength; index++)        // copy the payload
    {
        txBuffer[index + 3] = payloadData[index];
    }
    (void)theHardwareInterface.write(txBuffer, 3 + payloadLength);        // TODO :  could make this more robust by checking the return value and go into error is write did not succees
}

void NCI::getMessage() {
    rxMessageLength = theHardwareInterface.read(rxBuffer);
}

bool NCI::isMessageType(uint8_t messageType, uint8_t groupId, uint8_t opcodeId) const {
    return (((messageType | groupId) & 0xEF) == rxBuffer[0]) && ((opcodeId & 0x3F) == rxBuffer[1]);
}

bool NCI::isTimeOut() const {
    return ((millis() - timeOutStartTime) >= timeOut);
}

void NCI::setTimeOut(unsigned long theTimeOut) {
    timeOutStartTime = millis();
    timeOut          = theTimeOut;
}

void NCI::saveTag(uint8_t msgType) {
    // Store the properties of detected TAGs in the Tag array.
    // Tag info can come in two different NCI messages : RF_DISCOVER_NTF and RF_INTF_ACTIVATED_NTF and the Tag properties are in slightly different location inside these messages

    if (nmbrOfTags < maxNmbrTags) {
        uint8_t offSet;        // Offset in the NCI message where we can find the UniqueID
        switch (msgType) {
            case RF_INTF_ACTIVATED_NTF:
                offSet = 12;
                break;

            case RF_DISCOVER_NTF:
                offSet = 9;
                break;

            default:
                return;        // unknown type of msg sent here ?? we just ignore it..
                break;
        }

        uint8_t NfcId1Length = rxBuffer[offSet];
        if (NfcId1Length > 10) {
            NfcId1Length = 10;        // limit the length to 10, so in case of whatever error we don't write beyond the boundaries of the array
        }
        uint8_t newTagIndex = nmbrOfTags;        // index to the array item where we will store the info

        theTags[newTagIndex].uniqueIdLength = NfcId1Length;           // copy the length of the unique ID, is 4, 7 or 10
        for (uint8_t index = 0; index < NfcId1Length; index++)        // copy all bytes of the unique ID
        {
            theTags[newTagIndex].uniqueId[index] = rxBuffer[offSet + 1 + index];
        }
        theTags[newTagIndex].detectionTimestamp = millis();

        nmbrOfTags++;        // one more tag in the array now
    }
}

bool NCI::newTagPresent() const {        // returns true only if a new tag is present
    return (TagsPresentStatus::newTagPresent == theTagsStatus);
}