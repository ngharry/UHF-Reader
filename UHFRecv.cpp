#include "UHFRecv.h"

/* Default constructor */
UHFRecv::UHFRecv(): _uhfSerial(Serial1)
{
    _baudRate = DEFAULT_BAUD_RATE;   //< 57600 bps
    _protocol = DEFAULT_PROTOCOL;    //< 8N1
    _ctlPin = DEFAULT_RS485_CTL_PIN; //< 4
}

UHFRecv::UHFRecv(HardwareSerial& serial, const long baudRate, const byte ctlPin): 
                 _uhfSerial(serial)
{
    _baudRate = baudRate;
    _ctlPin = ctlPin;
    _protocol = DEFAULT_PROTOCOL;
}

/* Full customisation for UHFRecv() */
UHFRecv::UHFRecv(HardwareSerial& serial, const long baudRate, 
                 const SerialProtocol protocol, const byte ctlPin): _uhfSerial(serial)
{
    _baudRate = baudRate;
    _protocol = protocol;
    _ctlPin = ctlPin;
}

/**
* @public
* @brief Initialise PK-UHF101 Receiver
*/
void UHFRecv::begin()
{
    pinMode(_ctlPin, OUTPUT);
    _uhfSerial.begin(_baudRate, _protocol);
    digitalWrite(_ctlPin, RS485_RECEIVE);
} 

/**
* @public
* @brief Set command for cards inventory
*/
byte* UHFRecv::setCommand(const byte readerAddr, 
                          const byte addrTid, const byte lenTid)
{
    // see `doc/Protocols` for more commands
    const byte command = 0x01; //< Inventory command

    // size of the command (exclude the size itself)
    _inventoryCmd[0] = sizeof(_inventoryCmd) - 1; 
    
    _inventoryCmd[1] = readerAddr;
    _inventoryCmd[2] = command; 
    _inventoryCmd[3] = addrTid;
    _inventoryCmd[4] = lenTid;

    UHFRecv::Crc crc = _calculateCrc(_inventoryCmd, sizeof(_inventoryCmd) - 2);
    _inventoryCmd[5] = highByte(crc);
    _inventoryCmd[6] = lowByte(crc);

    return _inventoryCmd;
}

/* Get inventory command's size */
const size_t UHFRecv::getSizeCommand()
{
    return sizeof(_inventoryCmd);
}

/**
* @public
* @brief Ensure the preservation of data.
*/
bool UHFRecv::isDataPreserved(byte* receivedData, size_t size)
{
    UHFRecv::Crc reCalCrc = UHFRecv::_calculateCrc(receivedData, size - 2);
    
    UHFRecv::Crc receivedCrc = 0x00;
    
    if (receivedData[RE_STATUS_INDEX] == ERR_CRC)
        return false;
    else {
        // Merge 2 checksum bytes into 16-bit CRC-16
        byte lowByte = receivedData[size - 2];
        byte highByte = receivedData[size - 1];
        if (isLittleEndian())
            receivedCrc = (lowByte << 8) | highByte;
        else 
            receivedCrc = (highByte << 8) | lowByte;

        /* 
        * If the calculated CRC-16 and the received CRC-16 are not the same, 
        * then the data from reader to RS485 are not preserved.
        */
        return (receivedCrc != reCalCrc) ? false : true; 
    }
}

/**
* @public
* @brief Get bytes of data from UHF reader
*/
Status UHFRecv::getRawData(byte* reData, byte* request, size_t size)
{
    digitalWrite(_ctlPin, RS485_TRANSMIT);
    _uhfSerial.write(request, size);
    delay(10); //< [WARNING] This delay is VERY IMPORTANT
    digitalWrite(_ctlPin, RS485_RECEIVE);

    byte i = 0;
    while (_uhfSerial.available() > 0) {
        reData[i] = _uhfSerial.read();
        if (reData[i] < 0) //< return -1 if no data is read
            return ERR_READ_RS485;
        i++;
    }
    
    delay(2); //< [WARNING] This delay is VERY IMPORTANT
    return STATUS_SUCCESS;
}

/**
* @public
* @brief Debug function - print bytes of data read from RS485 with base
*/
void UHFRecv::_debugPrintRawData(byte* reData, size_t size, byte base)
{
	for (byte i = 0; i < size; i++) {
		Serial.print(reData[i], base);
		Serial.print(" ");
	}
	Serial.println();
}

/**
* @public
* @brief Debug function - print bytes of data (HEX) read from RS485
*/
void UHFRecv::_debugPrintRawData(byte* reData, size_t size)
{
    for (byte i = 0; i < size; i++) {
        Serial.print(reData[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

/**
* @private
* @brief CRC-16 Calculator (polynomial of 0x8408)
*/
UHFRecv::Crc UHFRecv::_calculateCrc(byte* data, size_t size)
{
    _crc = 0xFFFF;
    for (size_t i = 0; i < size; i++) {
        _crc ^= data[i];
        for (byte j = 0; j < 8; j++) {
            if (_crc & 0x0001)
                _crc = (_crc >> 1) ^ 0x8408; //< Fixed polynomial
            else
                _crc = _crc >> 1;
        }
    }
    
    /* return the result based on the endianess of the system */
    if (isLittleEndian())
        return _crc >> 8 | _crc << 8;
    else
        return _crc;
}

/** 
* @brief Check the system endianess
*/
bool isLittleEndian()
{
    byte endianess = 0x0001;
    return (bool)(*(byte *)&(endianess)); //< True if the system is little endian
}
