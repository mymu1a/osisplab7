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


void deleteMessage(struct Message*);
static void startNanoSleep(char* nameProgram, Child* pChild);


void* threadConsumer(void* pData)
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


			if (circleQueueNextRead(pCircleHead, &pElement) == false)
			{
				printf("  circleQueue is empty -- xxxx --\t");

			}
			else
			{
				struct Message* pMessage;

				pMessage = pElement->pMessage;
				printf("pMessage->type = %d\t", pMessage->type);
				printf("pMessage->size = %d\t", pMessage->size);
				deleteMessage(pMessage);

				pCircleHead->countRead++;												// increment `счетчик извлеченных сообщений`
				printf("  %d ( count read )\t", pCircleHead->countRead);
			}
			printf(" ( %s )\n", nameProgram);

			usleep(500000);
			pChild->inProcess = false;  // prevent terminating by Exit Signal
			pthread_mutex_unlock(&pCircleHead->mutex);									// mutex unlock

			sem_post(pSemaphore);
			if (pChild->isExit == true)
			{
				printf("isExit == true !!!!!!!!!!!!!\n");
				pthread_exit(NULL);
			}

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

void deleteMessage(struct Message* pMessage)
{
	free(pMessage->pData);
	free(pMessage);
}
