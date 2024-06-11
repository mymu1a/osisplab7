#include "global.h"

#include "circleQueue.h"
#include "child.h"

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */


struct Message* createMessage();
static void startNanoSleep(char* nameProgram, Child* pChild);


void* threadProducer(void* pData)
{
	Child* pChild = (Child*)pData;

	startNanoSleep(pChild->nameProgram, pChild);

	return NULL;
}

static void startNanoSleep(char* nameProgram, Child* pChild)
{
	struct timespec		time, time2;
	int					count = 5;
	int					result;

	CircleHead* pCircleHead = pChild->pCircleHead;
	sem_t* pSemaphore = pChild->pSemaphore;

	time.tv_sec = 1;
	time.tv_nsec = 0.5 * 1000000000;

	while (count-- > 0)
	{
		result = nanosleep(&time, &time2);
		if (result == 0)
		{
			CircleElement* pElement;

			sem_wait(pSemaphore);
			pChild->inProcess = true;  // prevent terminating by Exit Signal

			pthread_mutex_lock(&pCircleHead->mutex);									// mutex lock
			usleep(1000000);

			if (circleQueueNextWrite(pCircleHead, &pElement) == false)
			{
				printf("  circleQueue is full --------\t");
///				circleQueueLogState(pCircleHead);
			}
			else
			{
				struct Message* pMessage;

				pMessage = createMessage();
				pElement->pMessage = pMessage;

				pCircleHead->countWrite++;												// increment `счетчик добавленных сообщений`
				printf("  %d ( count write )\t", pCircleHead->countWrite);
				
///				circleQueueLogState(pCircleHead);
			}
			printf(" ( %s )\n", nameProgram);


			pChild->inProcess = false;  // prevent terminating by Exit Signal
			pthread_mutex_unlock(&pCircleHead->mutex);									// mutex unlock
			
			sem_post(pSemaphore);
			if (pChild->isExit == true)
			{
				printf("isExit == true !!!!!!!!!!!!!\n");
				pthread_exit(NULL);
			}
			usleep(2);
		}
		if (result == -1)
		{
			if (errno != 4)
			{
				printf("Error in nanosleep():\n");
				printf("  errno: %d\n", errno);
				printf("  errno: %s\n", strerror(errno));

				return;
			}
		}
	}
}

struct Message* createMessage()
{
	int random;
	struct Message* pMessage;

	pMessage = (Message*)malloc(sizeof(struct Message));
	random = 0;

	while (random == 0)
	{
		random = rand();
		random = random % 257;
	}
	pMessage->size = random;
	pMessage->pData = (u_char*)malloc(pMessage->size);

	printf("pMessage->size=%d\n", pMessage->size);

	u_char byte;

	for (int i = 0; i < pMessage->size; i++)
	{
		byte = 0;
		while (byte == 0)
		{
			byte = (u_char)rand();
		}
		pMessage->pData[i] = byte;
		printf("%02x ", byte);
	}
	printf("\n");
	pMessage->hash = 0;
	pMessage->type = 1;

	return pMessage;
}
