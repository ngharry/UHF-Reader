#include "CQueue.h"

/* All methods below are @public */

/* Default constructor */
CQueue::CQueue()
{
    _size = 0;
    _head = 0;
    _tail = -1;

    _data[_CAPACITY] = {0};
}

bool CQueue::isEmpty()
{
    return (_size == 0);
}

bool CQueue::isFull()
{
    return (_size == _CAPACITY);
}

Status CQueue::enqueue(const QDataType data)
{
    if (isFull()) {
        return ERR_QUEUE_FULL; //< the queue is full, can not enqueue anymore
    }

    _tail = (_tail + 1) % _CAPACITY;
    _data[_tail] = data;
    _size++;

    return STATUS_SUCCESS;
}

Status CQueue::dequeue()
{
    if (isEmpty()) {
        return ERR_QUEUE_EMPTY; //< the queue is empty, can not dequeue
    }

    _head = (_head + 1) % _CAPACITY;
    _size--;

    return STATUS_SUCCESS;
}

const byte CQueue::getSize()
{
    return _size;
}

const byte CQueue::getCapacity()
{
    return _CAPACITY;
}

CQueue::QDataType* CQueue::getQueueData()
{
    return _data; //< pointer to the first element of the queue's array
}

/* Pass pointer to function to this function */
void CQueue::_debugPrint(CallBackFunc cbFunc) //< Pointer to function 
{
    if (_head > _tail) {
        /* 
        * Change `int8_t` to `byte` can crash the program as the initial value
        * of `_tail` is -1.
        */
        for (int8_t i = _head; i < _size; i++) {
            /* Execute `cbFunc` which is passed as an argument to _debugPrint */
            cbFunc(&(_data[i]));
        }
        for (int8_t i = 0; i <= _tail; i++) {
            cbFunc(&(_data[i]));
        }
    } else {
        for (int8_t i = _head; i <= _tail; i++) {
            cbFunc(&(_data[i]));
        }   
    }
    Serial.println();
}
