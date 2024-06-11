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


void* threadDoctor(void* pData)
{
	Child* pChild = (Child*)pData;

	CircleHead* pCircleHead = pChild->pCircleHead;

	pthread_mutex_lock(&pCircleHead->mutex);									// mutex lock
	if (pChild->isExit == true)
	{
		printf("isExit == true !!!!!!!!!!!!! d-o-c\n");
		pthread_mutex_unlock(&pCircleHead->mutex);								// mutex unlock
		pthread_exit(NULL);
	}
	pChild->inProcess = true;  // prevent terminating by Exit Signal

	if (pChild->doctorInc == true)
	{
		circleQueueSizeInc(pCircleHead);

		printf("  circleQueue size is UP\t");
		circleQueueLogState(pCircleHead);

		printf(" ( %s )\n", pChild->nameProgram);
	}
	else
	{
		bool result = circleQueueSizeDec(pCircleHead);
		printf("  circleQueue size is DOWN\t");
		printf(" (%s)\t", result ? "true" : "false");
		circleQueueLogState(pCircleHead);
		printf(" ( %s )\n", pChild->nameProgram);
	}
	usleep(1000000);

	pChild->inProcess = false;  // prevent terminating by Exit Signal
	pthread_mutex_unlock(&pCircleHead->mutex);									// mutex unlock
	
	return NULL;
}
