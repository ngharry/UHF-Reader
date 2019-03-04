#include "Database.h"

/*
* @note
* - `timeFlag` is used to ensure that timestamps are set once only for each 
* inventoried card. Initialise to `true`.
* - Make sure every time calling inventoryCards(), a timeFlag is wrapped 
* around.
* @example: 
* ```
*     timeFlag = true;
*     Status status = inventoryCards(&tmp, rawData);
*     timeFlag = false;
* ```
* @todo Find a more elegant way to do this!
*/
bool timeFlag = true;

/*
* @note
* `_prefix` is set corresponding to the project of Tictag 
* (default: empty string)
*
* @todo 
* it's really dangerous to set `_prefix` as an extern variable. 
* Figure out how to limit the scope of `_prefix`.
*/
String _prefix = "";

Database::Database(): CQueue() {} //< Constructor

/**
* @public
* @brief Initialise Database
*/
void Database::begin()
{
    // [WARNING] IMPORTANT
    // seed for random number, used in hashing TID.
    randomSeed(analogRead(ANALOG_PIN));
}
/**
* @public
* @brief Store inventoried cards to database
*/
Status Database::inventoryCards(CQueue* inventoryDB, byte* rawData)
{
    // Get number of cards in the effective field
    byte numCards = rawData[RE_INV_NUM_CARDS_INDEX];
    
    if ((rawData[RE_STATUS_INDEX] == STATUS_SUCCESS) 
        && (numCards > MAX_CARDS)) {
        return ERR_NUM_CARDS;
    }
        
    if (rawData[RE_STATUS_INDEX] != STATUS_SUCCESS)
        return rawData[RE_STATUS_INDEX];
    
    Card tmp[MAX_CARDS] = {0};

    byte sizeIndex = RE_INV_TID_SIZE_INDEX;
    for (byte i = 0; i < numCards; i++) {
        tmp[i].tid.size = rawData[sizeIndex]; //< get TID size

        tmp[i].status = false;
        
        // Get TID from inventory command respond frame 
        for (size_t j = 0; j < tmp[i].tid.size; j++) {
            tmp[i].tid.tidByte[j] = rawData[j + sizeIndex + 1];
        }

        // A timeFlag is added to make sure the timestamp is set once for 
        // each card.
        if (timeFlag) {  
            tmp[i].time = millis();
        }

        if (inventoryDB->isFull())
            inventoryDB->dequeue();

        inventoryDB->enqueue(tmp[i]);

        /**
        * Update new index of TID size
        * New index = previous TID size + 1
        */
        sizeIndex += rawData[sizeIndex] + 1;
    }
    return STATUS_SUCCESS;
}

/**
* @public
* @brief Store cards to the permanent database
*/
Status Database::updateDB(byte* rawData)
{
    CQueue tmp; //< temporary database to store inventoried cards in the 
                //  current session

    timeFlag = true;
    Status status = inventoryCards(&tmp, rawData);
    timeFlag = false;

    if (status != STATUS_SUCCESS)
        return status;

    // Compare the temporary database (current inventory session) with the 
    // permanent database.
    for (byte i = 0; i < tmp.getSize(); i++) { //< Temporary databse
        String newTid = toString(tmp.getQueueData()[i].tid);
        byte j = 0;
        
        for (j = 0; j < _database.getCapacity(); j++) { //< Permanent one
            String tmpTid = toString(_database.getQueueData()[j].tid);

            if (tmpTid == newTid) { //< if match
                // Disconnect expired cards - by setting `.status = false` 
                if ((millis() - _database.getQueueData()[j].time) >= EXPIRE_TIME) {
                    _database.getQueueData()[j].status = false;
                }

                _database.getQueueData()[j].time = millis(); //< Update time stamp
                break; 
            }
        }

        // if there is no card that match
        if (j == _database.getCapacity()) {
            byte q = 0;

            // Iterate to overwrite new cards to expired cards
            for (q = 0; q < _database.getSize(); q++) {
                if ((millis() - _database.getQueueData()[q].time) >= EXPIRE_TIME) {
                    _database.getQueueData()[q] = tmp.getQueueData()[i];
                    break;
                }
            }

            // If there is no expired cards, then enqueue card to the end of 
            // the queue
            if (q == _database.getSize()) {
                _database.enqueue(tmp.getQueueData()[i]);
            }
        }
    }    

    return STATUS_SUCCESS;
}

/**
* @public
* @brief Get cards' database
*/
CQueue& Database::getDB()
{
    return _database;
}

/**
* @public
* @brief Print hashed TIDs to keyboard (for web dev team).
*/
void Database::printToKeyboard()
{
    // Pass prtCardKeyboard() to CQueue::_debugPrint()`
    _database._debugPrint(prtCardKeyBoard);
}

/**
* @public or @protected
* @brief Debug function - print the whole database
*/
void Database::_debugPrintDB(CQueue& database)
{
   Serial.println("TID\t\t\tStatus\t\tTime");
   // Pass prtCardInfo() to CQueue::_debugPrint()`
   database._debugPrint(prtCardInfo);
}

/**
* @public or @protected
* @brief Debug function - print welcome message
*/
void Database::_debugPrintDBMsg()
{
    // Pass prtCardMsg() to CQueue::_debugPrint()`
    _database._debugPrint(prtCardMsg);
}

/* @brief Set prefix for Tictag JSC projects */
void _setPrefix(const String prefix)
{
    _prefix = prefix;
}

/* @brief Convert TID to string */
String toString(TID& tid)
{
    String strTid;

    for (size_t i = 0; i < tid.size; i++) {
        byte tmp = tid.tidByte[i];

        // Add leading 0s to make sure every byte has 3 digits 
        if (tmp < 100) strTid += "0";
        if (tmp < 10) strTid += "0";

        strTid += String(tmp, DEC);
    }
    return strTid;
}


/* @brief Hash generation */
const String generateHash(TID tid, const String prefix) 
{
    String strTid;
    byte n = 2;
    byte key[n];
    for (byte i = 0; i < n; i++) {
        key[i] = random(1, 255);
    }
    
    for (byte i = 0; i < tid.size; i++) {
        tid.tidByte[i] ^= key[i % n]; 
    }
    
    strTid = toString(tid);

    for (byte i = 0; i < n; i++) {
        if (key[i] < 100) strTid += "0";
        if (key[i] < 10) strTid += "0";
        strTid += String(key[i], DEC);
    }

    return prefix + strTid;
}

/*
* These functions are used for passing to CQueue::_debugPrint for queue
* debugging purpose 
*/

/* Print card information: TID, status, time. */
void prtCardInfo(Card* card)
{
    String tid = toString(card->tid);
    Serial.print(tid);
    Serial.print("\t");
    Serial.print(card->status);
    Serial.print("\t\t");
    Serial.println(card->time);
}

/* Print welcome message  - for testing card.status */
void prtCardMsg(Card* card)
{
    if (card->status == false) {
        Serial.print("Hello ");
        Serial.print(toString(card->tid));
        Serial.print(". You are in at ");
        Serial.print(card->time);
        Serial.println(". ");
        card->status = true;
    }
}

/* Print hased TID to keyboard */
void prtCardKeyBoard(Card* card)
{
    if (card->status == false) {
        Keyboard.println(generateHash(card->tid, _prefix));
        card->status = true;
        delay(800); //< Modify this delay() to change delay time between cards
                    //  if there are multiple cards are tapped at once 
    }
}
