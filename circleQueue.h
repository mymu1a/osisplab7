#include "global.h"

#include <pthread.h>


struct CircleHead
{
	short indexHead;
	short indexTail;
	short size;

	void* pBuffer;					// queue first element

	bool isEmpty;					// no info in the buffer
	pthread_mutex_t mutex;			// mutual exclusive access to the Queue

	short countWrite;				// счетчик добавленных сообщений
	short countRead;				// счетчик извлеченных сообщений
};

struct CircleElement
{
	struct Message* pMessage;
};

void circleQueueInit(CircleHead* pHead, short sizeQueue);
bool circleQueueNextRead(CircleHead* pHead, CircleElement** pElement);
bool circleQueueNextWrite(CircleHead* pHead, CircleElement** pElement);

void circleQueueLogState(CircleHead* pHead);
bool circleQueueSizeDec(CircleHead* pHead);
void circleQueueSizeInc(CircleHead* pHead);

