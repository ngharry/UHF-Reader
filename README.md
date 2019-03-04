# PK-UHF101U Library #
An Arduino Library for the PK-UHF101U reader.

## Index
- [Pre-requisite](#pre-requisite)
- [Installation](#installation)
  * [Windows](#windows)
  * [Linux](#)
- [How-to](#how-to)
  * [Initalise the Library](#initialise-the-library)
  * [Check Data Preservation](#check-data-preservation)
  * [Get Raw Data from UHF Reader](#get-raw-data-from-uhf-reader)
  * [Print the whole Database](#print-the-whole-database)
  * [Print Card-Holder Welcome Message](#print-card-holder-welcome-message)
  * [Print Encoded TIDs to Keyboard](#print-encoded-tids-to-keyboard)
- [For Developer](#for-developer)
- [Error Code](#error-code)
- [Bugs Reporting](#bugs-reporting)
- [TODO](#todo)

## Pre-requisite
- Installed [the lastest version of Arduino](#https://www.arduino.cc/en/Main/Software "Download Arduino").
- git

## Installation
## Windows
- Open `cmd`, enter

```
cd %userprofile%\Documents\Arduino\libraries &&
git clone https://github.com/ngharry/uhf-reader
```
   to download UHF-reader library.
- Wait until the downloading process finishes.

## How to
### Initialise the Library
Objects instantiated from the classes in this library must be global.

### Database
- Objects must be instantiated in form of pointer to `class Database`. Otherwise, the whole system will be **crashed**.
- For example:
```cpp
Database* database;

void setup()
{
    database = new Database;
	database->begin();
}
```

### UHFRecv
- For example:

```cpp
UHFRecv UHF101(Serial1, 9600, 4);

void setup()
{
    UHF101.begin();
}
```

### Check Data Preservation
```cpp
#include "UHFRecv.h"

#define RS485_CONTROL           4  //< Pin for RS485 Direction Control
#define LED                     17 //< LED Pin
#define BAUD_RATE               9600

#define DEBUG

void flash();

UHFRecv UHF101(Serial1, BAUD_RATE, RS485_CONTROL);

// UHFRecv can also be instantiated like this
// UHFRecv UHF101(Serial1, BAUD_RATE, SERIAL_8N1, RS485_CONTROL);

uint32_t now = 0;
uint32_t period = 1000; //< Change this value to modify time between inventory
                       //  sessions (600 means 600 ms).

void setup()
{
    /* Setting Serial */
    Serial.begin(BAUD_RATE);
    flash();

    /* Setting  UHF Receiver */
    Serial.println("Initialise UHF Receiver.");
    flash();
    UHF101.begin();
    
    delay(300);
    Serial.println("Scanning cards...");
}

void loop()
{
    Status status = 0x00;

    byte reData[INV_MAX_SIZE] = {0};
    
    byte* cmd = UHF101.setCommand(READER_ADDRESS, TID_ARRESSS, LENGTH_TID);
    size_t sizeCmd = UHF101.getSizeCommand();

    if (millis() > (now + period)) { 
        now = millis();
        UHF101.getRawData(reData, cmd, sizeCmd); //< Get raw data from 
                                                    //  inventory command
        if (reData[RE_STATUS_INDEX] == ERR_INV_NO_CARD) {
            Serial.println("[ERROR 0xFB] No cards in the effective field.");
        } else if (reData[RE_STATUS_INDEX] == ERR_CRC) {
            Serial.println("[ERROR 0xFE] Failed to checksum.");
        } else if (reData[RE_STATUS_INDEX] == STATUS_SUCCESS) {
            Serial.println("Card(s) detected!");
        } 
        else {
            Serial.println("[ERROR N/A] Refer to doc/Protocols for more info.");
        }

        if (UHF101.isDataPreserved(reData, sizeof(reData))) 
            Serial.println("Data are preserved.");
        else
            Serial.println("Some bytes of data are lost!");

        Serial.println();
    }
}

void flash()
{
    digitalWrite(LED, HIGH);
    delay(800);
    digitalWrite(LED, LOW);
}
```

### Get Raw Data from UHF Reader
See [examples/GetRawData](examples/GetRawData/GetRawData.ino "Get Raw Data").

### Print the whole Database
See [examples/PrintDatabase](examples/PrintDatabase/PrintDatabase.ino "Print The Whole Database").

### Print Card-Holder Welcome Message
See[examples/PrintWelcomeMessage](examples/PrintWelcomeMessage/PrintWelcomeMessage.ino "Print Welcome Message").

### Print Encoded TIDs to Keyboard
See [examples/PrintToKeyboard](examples/PrintToKeyboard/PrintToKeyboard.ino "Print TIDs to Keyboard").

## For Developer
- Because the buffer memory for serial communication of Arduino just can hold up to 64 bytes, the maximum number of cards that the system can read at once (without data loss) is **8 cards**. To satisfied the requirements of the system, I change `UHF_MAX_CARDS = 15` in `attribute.h` (to read 15 cards at once), with the acceptance that, **rarely**, a card with incorrect encoded TID will be inserted to the database. The system that encodes the TID can just ignore this value.

-	I do not recommend changing the size of TID returned from UHF reader. **6 bytes** is a reasonable value.

- The maximum number of cards that can exist in the database (25 cards) should stay unchanged to make sure the stable working status of the system.

- I adjust the default baudrate of UHF reader (from the default value of 57600 bps to 9600 bps) to make sure `void loop()` of Arduino runs fast enough to preserve the data 
transfered from UHF reader to Arduino (via RS485 communication). In case you want to change the baudrate to its default, you would have to fiddle a bit with `delay()` values in the system to preserve the data. However, I do not recommend doing that way, as 9600 bps is a reasonable value (Fix me if I am wrong).

- Every time calling `Database::inventoryCards()`, make sure a `timeFlag` is wrapped around the function (example below):
```cpp
timeFlag = true;
Status status = Database::inventoryCards(&database, newData);
timeFlag = false;
```
   to ensure that, cards' timestamps are updated correctly. 

## Error Code

| HEX | DEC | Name | Description |
|:----------:|:------------:|:--------|:-------|
|0x00 | 0 | STATUS_ERROR | General errors |
|0x01 | 1 | STATUS_SUCCESS| Function runs successfully, no errors |
|0x02 | 2 | ERR_INV_TIMEOUT | Inventory command is time-out |
|0x03 | 3 | ERR_INV_FRAME_OUT | Can not return all data in a single frame, all lost data wull be transfered in the next frames |
|0x04 | 4 | ERR_INV_MEM_OUT | Out of TIDs bank memory |
|0xFA |250| ERR_TID_SIZE | Returned TIDs size are greater than that specified in `attribute.h`|
|0xFB | 251 | ERR_INV_NO_CARD | No cards in the effective field |
|0xFC | 252 | ERR_NUM_CARDS | Returned number of cards are greater than that specifed in `attribute.h`|
|0xFE | 254 | ERR_CRC | Data are not preserved (fail to pass checksum test) |
|0x0A | 10 | ERR_QUEUE_FULL | Full queue, can not enqueue anymore |
|0x0B | 11 | ERR_QUEUE_EMPTY | Empty queue, can not dequeue anymore |
|0x1A | 26 | ERR_READ_RS485 | No data read from RS485 communication | 

## Bugs Reporting 

## TODO
- [ ] Add terminology table.
- [ ] Add more error codes (as specified in `doc/Protocols.pdf`).
- [ ] Add intalling process for Linux (I love Linux).



