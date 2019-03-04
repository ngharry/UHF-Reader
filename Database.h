#ifndef _DATABASE_H_
#define _DATABASE_H_

#include <Keyboard.h>
#include <stdint.h>

#include "CQueue.h"

/*
* @note
* - `timeFlag` is used to ensure that timestamps are set once only for each 
* inventoried card
* - Make sure every time calling inventoryCards(), a timeFlag is wrapped 
* around (see the example below) 
* @example: 
* ```
*     timeFlag = true;
*     Status status = inventoryCards(&tmp, rawData);
*     timeFlag = false;
* ```
* @todo Find a more elegant way to do this!
*/
extern bool timeFlag;

/*
* @note
* `_prefix` is set corresponding to the project of Tictag 
* (default: empty string)
*
* @todo 
* it's really dangerous to set `_prefix` as an extern variable. 
* Figure out how to limit the scope of `_prefix`.
*/
extern String _prefix;

class Database: public CQueue //< Inheritance from CQueue
{
public:
    Database(); //< Default constructor
    
    /**
    * @brief Initialise Database
    * @detail This function is REALLY IMPORTANT as it seeds value for random(),
    * used for hashing TIDs (algorithm used by Tictag JSC).
    *
	* @param none
    *
	* @return none
	*/
    void begin();

    /**
    * @brief Store inventoried cards to database
    * @detail Raw data from inventory command are processed and enqueue to queue.
    *
    * @param
    * - inventoryDB: a temporary database used for storing cards every inventory
    * session.
    * - rawData: array of bytes got from inventory command.
    *
	* @return
	* - STATUS_SUCCESS: inventory cards to database successfully.
	* - ERR_NUM_CARDS: number of cards read from inventory command are larger.
	*   than a pre-defined maximum number of cards can be read at once.
	* - For other values returned from this functions, see `doc/Protocols`.
	*/
    Status inventoryCards(CQueue* inventoryDB, byte* rawData);

    /**
    * @brief Store cards to the permanent database
    * @detail This func compares if the inventoried cards and cards existed in
    * the database are matched.
    * - In case of matching, keep checking which cards are expired by comparing
    * with `EXPIRE_TIME` (see `attribute.h`) and set their status to false 
    * (ready to be printed again - or ready to be re-connected).
    * - In case of not matching, the queue will be iterated to find which cards
    * are expired, then the expired cards will be overwritten by the new cards.
    * - In case of not matching, but there is no expired card, the new cards 
    * will be enqueued to the end of the queue.
    *
    * @param
    * - rawData: bytes of data from inventory command (new inventory session).
	*
    * @return
	* - STATUS_SUCCESS: add cards to the database successfully.
	* - ERR_NUM_CARDS: number of cards read from inventory command are larger
	* than a pre-defined maximum number of cards can be read at once.
	* - For other values returned from this functions, see `doc/Protocols`.
	*/
    Status updateDB(byte* rawData);
    
    /**
    * @brief Get cards' database
    * @param none
    * @return reference to `_database`.s
    */
    CQueue& getDB();

    /**
    * @brief Print hashed TIDs to keyboard (for web dev team)s
    * @param none
    * @return none
    */
    void printToKeyboard();

/*
* These functions are intended to be protected - uncomment `// protected: `
* to protect them
*/
// protected:
    /**
    * @brief Debug function - print the whole database
    * @param 
    * - database: reference to the database needs to be printed.
    * @return none
    */
    void _debugPrintDB(CQueue& database);

    /**
    * @brief Debug function - print welcome message
    * @detail used to test if `card.status` is working properly.
    * @param none
    * @return none
    */
    void _debugPrintDBMsg();

private:
    CQueue _database; //< database stored cards
};


/**
* @brief Set prefix for Tictag JSC projects.
* @param 
* - prefix: constant string of prefix (default: empty string).
* @return none
*/
void _setPrefix(const String prefix);


/**
* @brief Convert TID to string
* @param reference to TID
* @return String of converted TID
*/
String toString(TID& tid);

/**
* @brief Hash generation
*
* @param 
*    - tid: TID of card needs to be hashed.
*    - prefix: for Tictag JSC.
*
* @return constant string
*/
const String generateHash(TID tid, const String prefix);

/*
* These functions are used as arguments passed to `CQueue::_debugPrint()` for 
* queue debugging purpose 
*/

/* Print card information: TID, status, time. */
void prtCardInfo(Card* card);

/* Print welcome message - for testing card.status */
void prtCardMsg(Card* card);

/* Print hased TID to keyboard */
void prtCardKeyBoard(Card* card);

#endif
