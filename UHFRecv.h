#ifndef _UHF_RECEIVER_H_
#define _UHF_RECEIVER_H_

#include <Arduino.h>
#include <Keyboard.h>

#include <stdint.h>

#include "attribute.h"

class UHFRecv
{
public:
	typedef uint16_t Crc; //< CRC-16 data type
    typedef uint32_t SerialProtocol; //< Data type for Serial Protocol

    /**
    * Default constructor:
    * - Baud Rate: 57600 bps
    * - Protocol: 8N1
    * - RS485 Control Pin: 4
    * - Hardware Serial: Serial1
    */
    UHFRecv();

    /**
    * @brief Constructor
    * @param
    * - serial: HardwareSerial used in the system (e.g. Serial1)
    * - baudRate: baud rate of the PK-UHF101 reader
    * - ctlPin: RS485 control pin
    */
    UHFRecv(HardwareSerial& serial, const long baudRate, const byte ctlPin);

    /**
    * @brief Constructor
    * @param
    * - serial: HardwareSerial used in the system (e.g. Serial1)
    * - baudRate: baud rate of the PK-UHF101 reader
    * - protocol: Serial protocols
    * - ctlPin: RS485 control pin
    */
    UHFRecv(HardwareSerial& serial, const long baudRate, 
            const SerialProtocol protocol, const byte ctlPin);

    /**
    * @brief Initialise PK-UHF101 Receiver
    * @param none
    * @return none
    */
    void begin();

    /**
    * @brief Set command for cards inventory
    *
    * @param 
    * - readerAddr: reader address (range from 0 - 255, default 0).
    * - addrTid: entry address of inventory TID memory.
    * - lenTid: data length for TID inventory operation (range: 0 - 15)
    * - more info see `docs/Protocols`
    *
    * @return Array of inventory command
    */
    byte* setCommand(const byte uhfAddr, const byte startAddr, const byte numWords);
    
    /* Get inventory command's size */
    const size_t getSizeCommand();

    /**
    * @brief Ensure the preservation of data.
    * @detail This function provides 2 levels of data preservation (between UHF 
    * card to reader - between UHF reader to RS485).
    *
    * @param
    * - receivedData: pointer to array of bytes of received data from RS485
    * - size: number of elements in array.
    *
    * @return true if the data are preserved, otherwise false.
    */
    bool isDataPreserved(byte* receivedData, size_t size);

    /**
    * @brief Get bytes of data from UHF reader
    *
    * @param[in]
    * - request: array of request to be sent to UHF reader
    * - size: size of request array
    * @param[out]
    * - reData: response array is stored in reData
    *
    * @return
    * - STATUS_SUCCESS: successfully get bytes of data from SDM120
    * - ERR_READ_RS485: no read data from RS485. 
    */
    Status getRawData(byte* reData, byte* request, size_t size);

    /**
    * @brief Debug function - print bytes of data read from RS485
    *
    * @param
    * - reData: respond data from UHF Reader
    * - size: size of respond data
    * - base: base of data printed (HEX, DEC, OCT, or BIN)
    *
    * return none
    */
    void _debugPrintRawData(byte* reData, size_t size, byte base);

    /**
    * @brief Debug function - print bytes of data (HEX) read from RS485
    *
    * @param
    * - reData: respond data from UHF Reader
    * - size: size of respond data

    *
    * return none
    */
    void _debugPrintRawData(byte* reData, size_t size);

private:
    /**
    * @brief CRC-16 Calculator (polynomial of 0x8408)
    *
    * @param
    * - data: array of bytes of data needed to calculate checksum.
    * - size: number of elements in array.
    *
    * @return 16 bits of CRC-16 checksum
    */
	Crc _calculateCrc(byte* data, size_t size);
    Crc _crc;

    // Config
    long _baudRate;
    SerialProtocol _protocol;
    byte _ctlPin;
    HardwareSerial& _uhfSerial;

    byte _inventoryCmd[7]; //< Size of inventory command is 7 bytes
};

/** 
* @brief Check the system endianess
* @param none
* @return true if the system is little endian, false otherwise.
*/
bool isLittleEndian();

#endif
