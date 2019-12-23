#include "NCI.h"

NCI::NCI(PN7150Interface &theHardwareInterface, NciApplicationMode theMode) : theHardwareInterface(theHardwareInterface), theMode(theMode),  theState(NciState::HwResetRfc)
    {
    }

NCI::NCI(PN7150Interface &theHardwareInterface) : theHardwareInterface(theHardwareInterface), theMode(NciMode::CardReadWrite), theState(NciState::HwResetRfc)
    {
    }

void NCI::initialize()
    {
    if (theState == NciState::Error)			// If we are in Error state, we should re-initialize the Hardware interface, just to be safe..
        {
        theHardwareInterface.initialize();
        }
    theState = NciState::HwResetRfc;
    }

void NCI::run()
    {
    switch (theState)
        {
        case NciState::HwResetRfc:															// after Hardware reset / powerOn
            {
            uint8_t payloadData[] = {ResetKeepConfig};										// CORE_RESET-CMD with Keep Configuration
            sendMessage(MsgTypeCommand, GroupIdCore, CORE_RESET_CMD, payloadData, 1);		//
            printMessage(txBuffer);
            setTimeOut(10);																	// we should get a RESPONSE within 10 ms (it typically takes 2.3ms)
            theState = NciState::HwResetWfr;												// move to next state, waiting for the matching Response
            }
        break;

        case NciState::HwResetWfr:
            if (isTimeOut())
                {
                theState = NciState::Error;													// time out waiting for response..
                }
            else
                {
                if (theHardwareInterface.hasMessage())
                    {
                    getMessage();
                    printMessage(rxBuffer);
                    bool isOk = (6 == rxMessageLength);												// Does the received Msg have the correct lenght ?
                    isOk = isOk && isMessageType(MsgTypeResponse, GroupIdCore, CORE_RESET_RSP);		// Is the received Msg the correct type ?
                    isOk = isOk && (STATUS_OK == rxBuffer[3]);										// Is the received Status code Status_OK ?

                    if (isOk)																		// if everything is OK...
                        {
                        theState = NciState::SwResetRfc;											// ..move to the next state
                        }
                    else																			// if not..
                        {
                        theState = NciState::Error;													// goto error state
                        }
                    }
                }
            break;

        case NciState::SwResetRfc:
            {
            sendMessage(MsgTypeCommand, GroupIdCore, CORE_INIT_CMD);						// CORE_INIT-CMD
            printMessage(txBuffer);
            setTimeOut(10);																	// we should get a RESPONSE within 10 ms, typically it takes 0.5ms
            theState = NciState::SwResetWfr;												// move to next state, waiting for response
            }
        break;

        case NciState::SwResetWfr:
            if (isTimeOut())
                {
                theState = NciState::Error;													// time out waiting for response..
                }
            else
                {
                if (theHardwareInterface.hasMessage())
                    {
                    getMessage();
                    printMessage(rxBuffer);
                    bool isOk = isMessageType(MsgTypeResponse, GroupIdCore, CORE_INIT_RSP);			// Is the received Msg the correct type ?

                    if (isOk)																		// if everything is OK...
                        {
                        theState = NciState::EnableCustomCommandsRfc;								// ...move to the next state
                        }
                    else																			// if not..
                        {
                        theState = NciState::Error;													// .. goto error state
                        }
                    }
                }
            break;

        case NciState::EnableCustomCommandsRfc:
            sendMessage(MsgTypeCommand, GroupIdProprietary, NCI_PROPRIETARY_ACT_CMD);				// Send NCI_PROPRIETARY_ACT_CMD to activate extra PN7150-NCI features
            printMessage(txBuffer);
            setTimeOut(10);																			// we should get a RESPONSE within 10 ms, typically it takes 0.5ms
            theState = NciState::EnableCustomCommandsWfr;											// move to next state, waiting for response
            break;

        case NciState::EnableCustomCommandsWfr:
            if (isTimeOut())
                {
                theState = NciState::Error;															// time out waiting for response..
                }
            else
                {
                if (theHardwareInterface.hasMessage())
                    {
                    getMessage();
                    printMessage(rxBuffer);
                    bool isOk = isMessageType(MsgTypeResponse, GroupIdProprietary, NCI_PROPRIETARY_ACT_RSP);			// Is the received Msg the correct type ?
					isOk = isOk && (STATUS_OK == rxBuffer[3]);															// Is the received Status code Status_OK ?

                    if (isOk)																		// if everything is OK...
                        {
                        theState = NciState::RfIdleCmd;												// ...move to the next state
                        }
                    else																			// if not..
                        {
                        theState = NciState::Error;													// .. goto error state
                        }
                    }
                }
            break;


        case NciState::RfIdleCmd:
            // After configuring, we instruct the NFCDevice to go into Poll mode..
            {
            uint8_t payloadData[] = { 3, NFC_A_PASSIVE_POLL_MODE, 0x01, NFC_B_PASSIVE_POLL_MODE, 0x01, NFC_F_PASSIVE_POLL_MODE, 0x01 };
            sendMessage(MsgTypeCommand, GroupIdRfManagement, RF_DISCOVER_CMD, payloadData, 7);		//
            printMessage(txBuffer);
            setTimeOut(10);																			// we should get a RESPONSE within 10 ms ms
            theState = NciState::RfIdleWfr;															// move to next state, waiting for Response
            }
        break;

        case NciState::RfIdleWfr:
            if (isTimeOut())
                {
                theState = NciState::Error;													// time out waiting for response..
                }
            else
                {
                if (theHardwareInterface.hasMessage())
                    {
                    getMessage();
                    printMessage(rxBuffer);
                    bool isOk = (4 == rxMessageLength);															// Does the received Msg have the correct lenght ?
                    isOk = isOk && isMessageType(MsgTypeResponse, GroupIdRfManagement, RF_DISCOVER_RSP);		// Is the received Msg the correct type ?
                    isOk = isOk && (STATUS_OK == rxBuffer[3]);													// Is the received Status code Status_OK ?
                    if (isOk)																		// if everything is OK...
                        {
                        theState = NciState::RfDiscovery;											// ...move to the next state
                        }
                    else																			// if not..
                        {
                        theState = NciState::Error;													// .. goto error state
                        }
                    }
                }
            break;


        case NciState::RfDiscovery:
            // Here we don't check timeouts.. we can wait forever for a TAG/CARD to be presented..

            // When a card is detected, we will get a notification
            if (theHardwareInterface.hasMessage())
                {
                getMessage();
				printMessage(rxBuffer, true);
                //theState = NciState::End;
                //bool isOk = (4 == rxMessageLength);																	// Does the received Msg have the correct lenght ?
                //isOk = isOk && isMessageType(MsgTypeNotification, GroupIdRfManagement, RfDiscoverNotification);		// Is the received Msg the correct type ?
                //isOk = isOk && (StatusOk == rxBuffer[3]);																// Is the received Status code Status_OK ?
                }
            break;

        case NciState::RfPollActive:
            // A card is present, so we can read/write data to it. We could also receive a notification that the card has been removed..
            break;

        case NciState::Error:
            break;

        default:
            break;
        }
    }

NciState NCI::getState()
    {
    return theState;
    }

void NCI::sendMessage(uint8_t messageType, uint8_t groupId, uint8_t opcodeId)
    {
    txBuffer[0] = (messageType | groupId) & 0xEF;				// put messageType and groupId in first byte, Packet Boundary Flag is always 0
    txBuffer[1] = opcodeId & 0x3F;								// put opcodeId in second byte, clear Reserved for Future Use (RFU) bits
    txBuffer[2] = 0x00;											// payloadLength goes in third byte
    theHardwareInterface.write(txBuffer, 3);
    }

void NCI::sendMessage(uint8_t messageType, uint8_t groupId, uint8_t opcodeId, uint8_t payloadData[], uint8_t payloadLength)
    {
    txBuffer[0] = (messageType | groupId) & 0xEF;				// put messageType and groupId in first byte, Packet Boundary Flag is always 0
    txBuffer[1] = opcodeId & 0x3F;								// put opcodeId in second byte, clear Reserved for Future Use (RFU) bits
    txBuffer[2] = payloadLength;								// payloadLength goes in third byte
    for (uint32_t index = 0; index < payloadLength; index++)	// copy the payload
        {
        txBuffer[index + 3] = payloadData[index];
        }
    theHardwareInterface.write(txBuffer, 3 + payloadLength);
    }

void NCI::getMessage()
    {
    rxMessageLength = theHardwareInterface.read(rxBuffer);
    }

bool NCI::isMessageType(uint8_t messageType, uint8_t groupId, uint8_t opcodeId)
    {
    return (((messageType | groupId) & 0xEF) == rxBuffer[0]) && ((opcodeId & 0x3F) == rxBuffer[1]);
    }

bool NCI::isTimeOut()
    {
    return false;
    //return ((millis() - timeOutStartTime) >= timeOut);
    }

void NCI::setTimeOut(unsigned long theTimeOut)
    {
    timeOutStartTime = millis();
    timeOut = theTimeOut;
    }

void NCI::test001()
    {
    Serial.println("Test 001 Cycle ---- Start");
    theHardwareInterface.initialize();
    initialize();
    delay(10);
    run();
    Serial.println((uint8_t)getState());
    delay(1);
    run();
    Serial.println((uint8_t)getState());
    delay(1);
    run();
    Serial.println((uint8_t)getState());
    delay(1);
    run();
    Serial.println((uint8_t)getState());
    delay(1);
    run();
    Serial.println((uint8_t)getState());
    delay(1);
    run();
    Serial.println((uint8_t)getState());
    delay(1);
    run();
    Serial.println((uint8_t)getState());
    delay(1);
    run();
    Serial.println((uint8_t)getState());
    delay(1);
    run();
    Serial.println((uint8_t)getState());
    delay(1);
    run();
    Serial.println((uint8_t)getState());
    Serial.println("Test 001 Cycle ---- End");

    delay(5000);
    }

void NCI::test002()
    {
    theLog.log("Test 002 Cycle ---- Start\n\n");
    theHardwareInterface.initialize();
    initialize();
    delay(10);

    for (uint8_t cycle = 0; cycle < 10; cycle++)
        {
        theLog.log(cycle);
        theLog.log(" : State was : ");
        printState(getState());

        run();

        theLog.log("State is : ");
        printState(getState());
        theLog.logNow("\n");

        delay(10);
        }

    while (NciState::End != getState())
        {
        run();
        delay(10);
        }

    theLog.logNow("Test 002 Cycle ---- End\n");
    delay(5000);
    }
