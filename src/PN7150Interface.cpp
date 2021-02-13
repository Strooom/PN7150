// #############################################################################
// ###                                                                       ###
// ### NXP PN7150 Driver                                                     ###
// ###                                                                       ###
// ### https://github.com/Strooom/PN7150                                     ###
// ### Author(s) : Pascal Roobrouck - @strooom                               ###
// ### License : https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode ###
// ###                                                                       ###
// #############################################################################

#include "PN7150Interface.h"									// NCI protocol runs over a hardware interface, in this case an I2C with 2 extra handshaking signals


PN7150Interface::PN7150Interface(uint8_t IRQ, uint8_t VEN) : IRQ(IRQ), VEN(VEN), I2Caddress(0x28)
    {
    // Constructor, initializing IRQ and VEN and setting I2Caddress to a default value of 0x28
    }

PN7150Interface::PN7150Interface(uint8_t IRQ, uint8_t VEN, uint8_t I2Caddress) : IRQ(IRQ), VEN(VEN), I2Caddress(I2Caddress)
    {
    // Constructor, initializing IRQ and VEN and initializing I2Caddress to a custom value
    }

void PN7150Interface::initialize(void)
    {
    pinMode(IRQ, INPUT);												// IRQ goes from PN7150 to DeviceHost, so is an input
    pinMode(VEN, OUTPUT);											// VEN controls the PN7150's mode, so is an output

    // PN7150 Reset procedure : see PN7150 datasheet 12.6.1, 12.6.2.2, Fig 18 and 16.2.2
    digitalWrite(VEN, LOW);											// drive VEN LOW... 
    delay(1);															// ...for at least 10us
    digitalWrite(VEN, HIGH);											// then VEN HIGH again, and wait for 2.5 ms for the device to boot and allow communication
    delay(3);

    Wire.begin();														// Start I2C interface
    }

bool PN7150Interface::hasMessage() const
    {
    return (HIGH == digitalRead(IRQ));								// PN7150 indicates it has data by driving IRQ signal HIGH
    }

uint8_t PN7150Interface::write(uint8_t txBuffer[], uint32_t txBufferLevel) const
    {
    Wire.beginTransmission(I2Caddress);									// Setup I2C to transmit
    uint32_t nmbrBytesWritten = 0;
    nmbrBytesWritten = Wire.write(txBuffer, txBufferLevel);				// Copy the data into the I2C transmit buffer
    if (nmbrBytesWritten == txBufferLevel)								// If this worked..
        {
        uint8_t resultCode;
        resultCode = Wire.endTransmission();							// .. transmit the buffer, while checking for any errors
        return resultCode;
        }
    else
        {
        return 4;														// Could not properly copy data ti I2C buffer, so treat as other error, see i2c_t3
        }
    }

uint32_t PN7150Interface::read(uint8_t rxBuffer[]) const
    {
    uint32_t bytesReceived;												// keeps track of how many bytes we actually received
    if (hasMessage())													// only try to read something if the PN7150 indicates it has something
        {
        // using 'Split mode' I2C read. See UM10936 section 3.5
        bytesReceived = Wire.requestFrom((int)I2Caddress, 3);			// first reading the header, as this contains how long the payload will be

        rxBuffer[0] = Wire.read();
        rxBuffer[1] = Wire.read();
        rxBuffer[2] = Wire.read();
        uint8_t payloadLength = rxBuffer[2];
        if (payloadLength > 0)
            {
            bytesReceived += Wire.requestFrom(I2Caddress, payloadLength);		// then reading the payload, if any
            uint32_t index = 3;
            while (index < bytesReceived)
                {
                rxBuffer[index] = Wire.read();
                index++;
                }
            }
        }
    else
        {
        bytesReceived = 0;
        }
    return bytesReceived;
    }

void PN7150Interface::test001()
// is the VEN signal being properly controlled ? Measure with a multimeter and verify the 2 second high + second low square wave
    {
    Serial.println("Test 001 Cycle ---- Start");
    Serial.println("Driving VEN HIGH");
    digitalWrite(VEN, HIGH);
    delay(2000);
    Serial.println("Driving VEN LOW");
    digitalWrite(VEN, LOW);
    delay(2000);
    Serial.println("Test 001 Cycle ---- End");
    }

void PN7150Interface::test002()
// is the IRQ signal being properly read ? Remove the NFC module and put a resistor between IRQ and VEN to serve as a loopback observe result in Serial monitor
    {
    Serial.println("Test 002 Cycle ---- Start");
    Serial.println("Driving VEN HIGH");
    digitalWrite(VEN, HIGH);
    delay(10);
    if (hasMessage())
        {
        Serial.println("reading IRQ HIGH - ok");
        }
    else
        {
        Serial.println("reading IRQ LOW - error");
        }
    delay(500);
    Serial.println("Driving VEN LOW");
    digitalWrite(VEN, LOW);
    delay(10);
    if (hasMessage())
        {
        Serial.println("reading IRQ HIGH - error");
        }
    else
        {
        Serial.println("reading IRQ LOW - ok");
        }
    delay(500);
    Serial.println("Test 002 Cycle ---- End");
    }

void PN7150Interface::test003()
    {
    // This will write data to the I2C. Monitor with a scope the I2C signals on the bus...
    Serial.println("Test 003 Cycle ---- Start");
    uint8_t tmpBuffer[] = { 0x00, 0xFF, 0xAA, 0x55 };
    uint8_t resultCode;
    resultCode = write(tmpBuffer, 4);
    switch (resultCode)
        {
        case 0:
            Serial.println("I2C Write succesfull");
            break;

        case 1:
            Serial.println("I2C Write fail : data too long");
            break;

        case 2:
            Serial.println("I2C Write fail : address NACK");
            break;

        case 3:
            Serial.println("I2C Write fail : data NACK");
            break;

        default:
            Serial.println("I2C Other Error");
        }

    Serial.println("Test 003 Cycle ---- End");
    delay(500);
    }

void PN7150Interface::test004()
    {
	// This will write data to the I2C, and then Check if the PN7150 indicates it wants to answer..
	Serial.println("Test 004 Cycle ---- Start");

	// Reset the PN7150, otherwise you can only run this test once..
	digitalWrite(VEN, LOW);									// drive VEN LOW for at least 0.5 ms after power came up : datasheet table 16.2.3
	delay(100);
	digitalWrite(VEN, HIGH);									// then VEN HIGH again, and wait for 2.5 ms for the device to boot and allow communication
	delay(50);

    if (hasMessage())
        {
        Serial.println("IRQ was already HIGH before sending - error");
        }
    else
        {
        Serial.println("IRQ LOW before sending - ok");
        }
    uint8_t tmpBuffer[] = { 0x20, 0x00, 0x01, 0x01 };
    write(tmpBuffer, 4);

    delay(5); // How much delay do you need to check if there is an answer from the Device ? I checked this with a scope and the device responded 2.3ms after the end of the message

    if (hasMessage())
        {
        Serial.println("IRQ HIGH after sending - ok");
        }
    else
        {
        Serial.println("IRQ still LOW after sending - error");
        }

    Serial.println("Test 004 Cycle ---- End");
    }

void PN7150Interface::test005()
    {
    // This will write CORE_REST_CMD to the PN7150, and then Check if we receive CORE_RESET_RSP back.. See NCI specification V1.0 section 4.1
    // I am using the reset behaviour of the NCI to test send and response here, as it is otherwise difficult to trigger a read
    Serial.println("Test 005 Cycle ---- Start");
	// Reset the PN7150, otherwise you can only run this test once..
	digitalWrite(VEN, LOW);									// drive VEN LOW for at least 0.5 ms after power came up : datasheet table 16.2.3
	delay(100);
	digitalWrite(VEN, HIGH);									// then VEN HIGH again, and wait for 2.5 ms for the device to boot and allow communication
	delay(50);

	uint8_t tmpBuffer[] = { 0x20, 0x00, 0x01, 0x01 };
    write(tmpBuffer, 4);

    delay(5); // How much delay do you need to check if there is an answer from the Device ?

    uint8_t tmpRxBuffer[260];
    uint32_t nmbrBytesReceived;

    nmbrBytesReceived = read(tmpRxBuffer);

    if (6 == nmbrBytesReceived)
        {
        Serial.print(nmbrBytesReceived);
        Serial.println(" bytes received, 6 bytes expected - ok");
        if (0x40 == tmpRxBuffer[0])
            {
            Serial.println("byte[0] = 0x40 : MT = Control Packet Response, PBF = 0, GID = Core = 0 - ok");
            }
        else
            {
            Serial.print("byte[0] = ");
            Serial.print(tmpRxBuffer[0]);
            Serial.println(" - error");
            }

        if (0x00 == tmpRxBuffer[1])
            {
            Serial.println("byte[1] = 0x00 : OID = CORE_RESET_RSP - ok");
            }
        else
            {
            Serial.print("byte[1] = ");
            Serial.print(tmpRxBuffer[1]);
            Serial.println(" - error");
            }

        if (0x03 == tmpRxBuffer[2])
            {
            Serial.println("byte[2] = 0x03 : payload length = 3 bytes - ok");
            }
        else
            {
            Serial.print("byte[2] = ");
            Serial.print(tmpRxBuffer[2]);
            Serial.println(" - error");
            }

        Serial.print("byte[3] = Status = ");							// See NCI V1.0 Specification Table 94. 0x00 = Status_OK
        Serial.print(tmpRxBuffer[3]);
        Serial.println("");

        Serial.print("byte[4] = NCI Version = ");						// See NCI V1.0 Specification Table 6. 0x17 = V1.7 ?? Not sure about this as I don't have official specs from NCI as they are quite expensive
        Serial.print(tmpRxBuffer[4]);
        Serial.println("");

        Serial.print("byte[5] = Configuration Status = ");				// See NCI V1.0 Specification Table 7. 0x01 = NCI RF Configuration has been reset
        Serial.print(tmpRxBuffer[5]);
        Serial.println("");
        }
    else
        {
        Serial.print(nmbrBytesReceived);
        Serial.println(" bytes received, 6 bytes expected - error");
        }

    Serial.println("Test 005 Cycle ---- End");
    delay(1000);
    }