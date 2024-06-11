#include "circleQueue.h"

#include <sys/types.h>
#include <stdlib.h>


// initialize Mutex for mutual exclusive access to the Queue
void initMutex(CircleHead* pHead);


void circleQueueInitInner(CircleHead* pHead, short sizeQueue)
{
	pHead->indexHead = -1;
	pHead->indexTail = -1;
	pHead->size = sizeQueue;
	pHead->isEmpty = true;

	// queue first element
	pHead->pBuffer = malloc(sizeof(CircleElement) * sizeQueue);

	pHead->countWrite = 0;
	pHead->countRead = 0;
}

void circleQueueInit(CircleHead* pHead, short sizeQueue)
{
	circleQueueInitInner(pHead, sizeQueue);

	initMutex(pHead);
}

void circleQueueLogState(CircleHead* pHead)
{
	printf("Head=%d\tTail=%d\tEmpty=%s\tSize=%d", pHead->indexHead, pHead->indexTail, pHead->isEmpty ? "true":"false", pHead->size);
}

void circleQueueHeadCopy(CircleHead* pHeadSource, CircleHead* pHeadDest)
{
	pHeadDest->indexHead  = pHeadSource->indexHead;
	pHeadDest->indexTail  = pHeadSource->indexTail;
	pHeadDest->size		  = pHeadSource->size;
	pHeadDest->isEmpty	  = pHeadSource->isEmpty;
	pHeadDest->pBuffer	  = pHeadSource->pBuffer;
	pHeadDest->countWrite = pHeadSource->countWrite;
	pHeadDest->countRead  = pHeadSource->countRead;
}

bool circleQueueSizeDec(CircleHead* pHead)
{
	if (pHead->size == 1)
	{
		return false;
	}
	CircleHead		headPrev;
	CircleElement*	pElement;
	short			sizeQueue;

	sizeQueue = pHead->size - 1;

	circleQueueHeadCopy(pHead, &headPrev);
	circleQueueInitInner(pHead, sizeQueue);

	pHead->pBuffer = malloc(sizeof(CircleElement) * sizeQueue);

	if (headPrev.indexHead == headPrev.indexTail && headPrev.isEmpty == false)
	{ // queue is full
		circleQueueNextRead(&headPrev, &pElement);
	}
	while (circleQueueNextRead(&headPrev, &pElement) == true)
	{
		CircleElement* pElementWrite;

		circleQueueNextWrite(pHead, &pElementWrite);
		pElementWrite->pMessage = pElement->pMessage;
	}
	return true;
}

void circleQueueSizeInc(CircleHead* pHead)
{
	CircleHead		headPrev;
	CircleElement*	pElement;
	short			sizeQueue;
	
	sizeQueue = pHead->size + 1;

	circleQueueHeadCopy(pHead, &headPrev);
	circleQueueInitInner(pHead, sizeQueue);

	pHead->pBuffer = malloc(sizeof(CircleElement) * sizeQueue);

	while (circleQueueNextRead(&headPrev, &pElement) == true)
	{
		CircleElement* pElementWrite;

		circleQueueNextWrite(pHead, &pElementWrite);
		pElementWrite->pMessage = pElement->pMessage;
	}
}

bool circleQueueNextWrite(CircleHead* pHead, CircleElement** pElement)
{
	if (pHead->indexHead == (pHead->size - 1) && pHead->indexTail == -1)
	{
		return false;
	}
	if (pHead->indexHead == pHead->indexTail && pHead->isEmpty == false)
	{
		return false;
	}
	pHead->isEmpty = false;

	if (++pHead->indexHead == pHead->size)
	{
		pHead->indexHead = 0;
	}
	*pElement = &((CircleElement*)pHead->pBuffer)[pHead->indexHead];

	return true;
}

bool circleQueueNextRead(CircleHead* pHead, CircleElement** pElement)
{
	if (pHead->isEmpty == true)
	{
		return false;
	}
	if (++pHead->indexTail == pHead->size)
	{
		pHead->indexTail = 0;
	}
	if (pHead->indexTail == pHead->indexHead)
	{
		pHead->isEmpty = true;
	}
	*pElement = &((CircleElement*)pHead->pBuffer)[pHead->indexTail];

	return true;
}

// initialize Mutex for mutual exclusive access to the Queue
void initMutex(CircleHead* pHead)
{
	pthread_mutexattr_t	attribute;

	pthread_mutexattr_init(&attribute);
	pthread_mutexattr_setpshared(&attribute, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&pHead->mutex, &attribute);
}
