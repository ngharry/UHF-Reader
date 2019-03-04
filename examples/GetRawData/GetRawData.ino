#include "Database.h"
#include "UHFRecv.h"

#define RS485_CONTROL           4  //< Pin for RS485 Direction Control
#define LED                     17 //< LED Pin
#define BAUD_RATE               9600

#define DEBUG

void flash();

/* 
* If you change this initialisation to `Database database`, the program would
* be crashed. I still dont know the reasons yet. But better using pointer.
*/
Database* database; 

UHFRecv TictagUhf(Serial1, BAUD_RATE, RS485_CONTROL);

// UHFRecv can also be instantiated like this
// UHFRecv TictagUhf(Serial1, BAUD_RATE, SERIAL_8N1, RS485_CONTROL);

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
    TictagUhf.begin();

    /* Setting database */
    Serial.println("Initialise Database");
    flash();
    database = new Database;
    database->begin();

    /* Setting prefix */
    String prefix = "";
    Serial.print("Setting prefix to ");
    if (prefix == "")
        Serial.println("default prefix: empty string");
    else
        Serial.println(prefix);
    _setPrefix(prefix);
    
    delay(300);
    Serial.println("Scanning cards...");
}

void loop()
{
    Status status = 0x00;

    byte reData[INV_MAX_SIZE] = {0};
    
    byte* cmd = TictagUhf.setCommand(READER_ADDRESS, TID_ARRESSS, LENGTH_TID);
    size_t sizeCmd = TictagUhf.getSizeCommand();

    if (millis() > (now + period)) { 
        now = millis();
        TictagUhf.getRawData(reData, cmd, sizeCmd); //< Get raw data from 
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

        TictagUhf._debugPrintRawData(reData, sizeof(reData)); //< Print default (HEX)

        // Print with specified base
        // TictagUhf._debugPrintRawData(reData, sizeof(reData), DEC); 
        Serial.println();
    }
}

void flash()
{
    digitalWrite(LED, HIGH);
    delay(800);
    digitalWrite(LED, LOW);
}
