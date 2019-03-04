#ifndef _ATTRIBUTE_H_
#define _ATTRIBUTE_H_

#include <Arduino.h>

/*
* User Configuration *
*/
#define MAX_CARDS                    15 //< Maximum number of cards can be 
                                        //  inventoried at once
#define EXPIRE_TIME                  5000
/* End of user config */

/* Developer Configuration */
#define DEFAULT_PROTOCOL             SERIAL_8N1 
#define DEFAULT_BAUD_RATE            57600 //< default baud rate of PK-UHF101
#define DEFAULT_RS485_CTL_PIN        4 //< RS485 control pin
#define ANALOG_PIN                   A0 //< Analog pin for seeding random number

// Command configuration (see `doc/Protocols`)
const byte READER_ADDRESS         =  0x00;
const byte TID_ARRESSS            =  0x03;
const byte LENGTH_TID             =  3;

/*
* End of Developer Configuration *
*/

// Maximum size of TID (bytes)
#define MAX_SIZE_TID                 6 //< Should not be changed.
                                       //  This MAX_SIZE_TID is chosen carefully
                                       //  to maximise number of cards can be 
                                       //  read at once. 

#define RS485_TRANSMIT               HIGH
#define RS485_RECEIVE                LOW

/* Data type represents TID */
typedef struct {
    byte size;
    byte tidByte[MAX_SIZE_TID];
} TID;

/* Data type represents UHF Card */
typedef struct {
    bool status;
    TID tid;
    uint32_t time;
} Card;

/* Response Frame Format
+------+------+-------+--------+--------+-----------+
| Len  | Adr  | reCmd | Status | Data[] |   CRC 16  |
+------+------+-------+--------+--------+-----------+
| 0xXX | 0xXX |  0x01 |  0xXX  | ...... | LSB | MSB |
+------+------+-------+--------+--------+-----+-----+
- Len (1 byte): Length of the response frame (excluding itself).
- Adr (1 byte): Reader address.
- reCmd (1 byte): indicates which command this frame is responding to. For unre-
  cognised command, reCmd should be 0x00.
- Status (1 byte): command execution status/result.
- Data[] (variable): Data field, reader may or may not provide parameters according
  to actual commands.
- LSB/MSB CRC16: Low byte and high byte respectively of CRC 16 checksum.   
*/
/* Constants relates to index of respond data frame */
enum RespondIndex: byte {
    RE_LENGTH_INDEX,
    RE_ADDRESS_INDEX,
    RE_COMMAND_INDEX,
    RE_STATUS_INDEX,
    RE_DATA_INDEX
};

/* Response Frame of Inventory Command Format
---------------------------------+   Data                    +------------
+------+------+-------+--------+------+----------------------+-----------+
| Len  | Adr  | reCmd | Status | Num  |       EPC/TID        |   CRC 16  |
+------+------+-------+--------+------+----------------------+-----+-----+
| 0xxx | 0xXX |  0x01 |  0xXX  | 0xXX | TID1, TID2, TID3,... | LSB | MSB |
+------+------+-------+--------+------+----------------------+-----+-----+
- Status (1 byte): See StatusCode.
- Num: the amount of RFID EPC/TID included in this response.
- EPC/TID: inquired EPC/TID data. For instance, TID-1 is the TID length + TID data
*/
enum InventoryInfo: byte {
    RE_INV_NUM_CARDS_INDEX     = 4,
    RE_INV_TID_SIZE_INDEX      = 5,
    RE_INV_TID_DATA_INDEX      = 6,

    INV_MAX_SIZE               = MAX_CARDS * (MAX_SIZE_TID + 1) + 7
};

enum Status: byte {
    STATUS_ERROR       = 0x00,
    STATUS_SUCCESS     = 0x01,

    // Inventory timeout, operation is aborted. Reader will report all the
    // already inquired tags to host.
    ERR_INV_TIMEOUT    = 0x02,

    // Reader is not able to response all the data in a single frame, further data
    // will be transmitted in the following frames.
    ERR_INV_FRAME_OUT  = 0x03,

    // Reader only completed parts of the inventory but run out of memory space
    // due to the amount of tags. Reader will report all the already inquired tags
    // as well as this status value to host.
    ERR_INV_MEM_OUT    = 0x04,

    // No operatable tag is detected in the effective range
    ERR_INV_NO_CARD    = 0xFB,

    // Size of respond TIDs is larger than the pre-defined TID size
    ERR_TID_SIZE       = 0xFA,

    // Number of inquired cards are larger than the pre-define maximum
    // number of cards.
    ERR_NUM_CARDS      = 0xFC,

    ERR_CRC            = 0xFE,

    ERR_QUEUE_FULL     = 0x0A,
    ERR_QUEUE_EMPTY    = 0x0B,

    ERR_READ_RS485     = 0x1A
};

#endif
