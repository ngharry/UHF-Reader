#ifndef _CQUEUE_H
#define _CQUEUE_H

#include <Arduino.h>
#include <Keyboard.h>

#include <stdint.h>

#include "attribute.h"

class CQueue
{
public:
    // Data type of elements in the queue
    typedef Card QDataType; /* Change `Card` to any data type you want
                               (e.g. byte, char, int, etc.) */   

    // Pass pointer to function to another function (call back function)
    typedef void (*CallBackFunc) (QDataType*); //< [WARNING] POINTER HERE IS IMPORTANT

    CQueue(); //< Default constructor
    
    bool isEmpty();
    bool isFull();

    /**
    * @brief Enqueue data
    * @param 
    *    - data: data (const) need to be enqueued
    * @return 
    *    - STATUS_SUCCESS: Enqueue data successfully.
    *    - ERR_QUEUE_FULL: the queue is full, can not enqueue anymore.
    */
    Status enqueue(const QDataType data);

    /**
    * @brief Dequeue the top element of the queue
    * @param none
    * @return 
    *    - STATUS_SUCCESS: Enqueue data successfully.
    *    - ERR_QUEUE_EMPTY: the queue is empty, can not dequeue anymore.
    */
    Status dequeue();

    const byte getSize(); //< Get queue's size at the moment this func is called
    const byte getCapacity(); //< Get capacity of the queue

    QDataType* getQueueData(); //< return pointer to the first element of 
                               //  the queue array

    /**
    * @public
    * @brief Debug function
    * @param pointer to the data-displaying function 
    * @return none
    */
    void _debugPrint(CallBackFunc func);

private:
    enum QueueInfo: byte {
        _CAPACITY = 25 //< Maximum capacity of the queue
    };

    byte _size; //< Keep track of queue's size
    
    int8_t _head; //< Pay attention to data type of `_head` and `_tail` is 
    int8_t _tail; //  `int8_t` not `byte`.
    
    QDataType _data[_CAPACITY]; //< Queue's data
};

#endif
