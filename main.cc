#include "global.h"

#include "circleQueue.h"

#include <ctype.h>
#include <dirent.h>
#include <semaphore.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/mman.h>
#include <sys/stat.h>	/* For mode constants */
#include <fcntl.h>		/* For O_* constants */
#include <sys/wait.h>


#define SIZE_MESSAGE_QUEUE 8


//=== Global Variables ===
TAILQ_HEAD(HeadOfCollection, Child)
	colProducer = TAILQ_HEAD_INITIALIZER(colProducer),
	colConsumer = TAILQ_HEAD_INITIALIZER(colConsumer),
	colDoctor   = TAILQ_HEAD_INITIALIZER(colDoctor);

sem_t semaphoreProducer;
sem_t semaphoreConsumer;

//===== Functions =====
int createChild(unsigned indexChild, char* dirChild, TYPE_CHILD childType);
CircleHead* createColMessage();
void stopChildren();
// stop children threads and optionaly free Children in collection
void stopChildren(TYPE_CHILD type, bool freeMemory);
void* threadConsumer(void* pData);
void* threadDoctor(void* pData);
void* threadProducer(void* pData);



int main(int argc, char* argv[], char* envp[])
{
	printf("main ST\n");

	int result;

	result = sem_init(&semaphoreProducer, 0, SEM_PRODUSER_VALUE);
	if (result != 0)
	{
		printf("Error: cannot open Semaphore\n");
		return 1;
	}

	result = sem_init(&semaphoreConsumer, 0, SEM_CONSUMER_VALUE);
	if (result != 0)
	{
		printf("Error: cannot open Semaphore\n");
		return 1;
	}
	printf("Type: p c k q\n");

	//=== read user input ===
	int ch;
	unsigned	indexProduser = 0;
	unsigned	indexConsumer = 0;
	unsigned	indexDoctor	  = 0;
	CircleHead* pCircleHead;
	bool		isQuit = false;

	TAILQ_INIT(&colProducer);                   /* Initialize the queue. */
	TAILQ_INIT(&colConsumer);                   /* Initialize the queue. */
	TAILQ_INIT(&colDoctor);                     /* Initialize the queue. */

	pCircleHead = createColMessage();			// initialize circled collection for Messages

	while ((ch = getchar()) != EOF && isQuit == false)
	{
		//=== select command ===
		switch (ch)
		{
		case 'p':
		{
			struct Child* pChild;
			
			pChild = (Child*)malloc(sizeof(struct Child));

			pChild->index = indexProduser;
			pChild->pCircleHead = pCircleHead;
			pChild->isExit = false;
			pChild->inProcess = false;
			pChild->pSemaphore = &semaphoreProducer;
			sprintf(pChild->nameProgram, "%s_%02d", CHILD_PRODUSER_PROGRAM, indexProduser);

			pthread_create(&pChild->idThread, NULL, &threadProducer, (void*)pChild);
			pthread_detach(pChild->idThread);			// will use detached threads

			TAILQ_INSERT_TAIL(&colProducer, pChild, allChildren);

			printf("Start Producer: %d\n", indexProduser);
			indexProduser++;

			break;
		}
		case 'c':
		{
			struct Child* pChild;

			pChild = (Child*)malloc(sizeof(struct Child));

			pChild->index = indexConsumer;
			pChild->pCircleHead = pCircleHead;
			pChild->isExit = false;
			pChild->inProcess = false;
			pChild->pSemaphore = &semaphoreConsumer;
			sprintf(pChild->nameProgram, "%s_%02d", CHILD_CONSUMER_PROGRAM, indexConsumer);

			pthread_create(&pChild->idThread, NULL, &threadConsumer, (void*)pChild);
			pthread_detach(pChild->idThread);			// will use detached threads

			TAILQ_INSERT_TAIL(&colConsumer, pChild, allChildren);

			printf("Start Consumer: %d\n", indexConsumer);
			indexConsumer++;

			break;
		}
		case 'd':
		{
			struct Child* pChild;

			pChild = (Child*)malloc(sizeof(struct Child));

			pChild->index = indexDoctor;
			pChild->doctorInc = false;
			pChild->pCircleHead = pCircleHead;
			pChild->isExit = false;
			sprintf(pChild->nameProgram, "%s_%02d", CHILD_DOCTOR_PROGRAM, indexDoctor);

			pthread_create(&pChild->idThread, NULL, &threadDoctor, (void*)pChild);
			pthread_detach(pChild->idThread);			// will use detached threads

			TAILQ_INSERT_TAIL(&colDoctor, pChild, allChildren);

			printf("Start Doctor: %d\n", indexDoctor);
			indexDoctor++;

			break;
		}
		case 'i':
		{
			struct Child* pChild;

			pChild = (Child*)malloc(sizeof(struct Child));

			pChild->index = indexDoctor;
			pChild->doctorInc = true;
			pChild->pCircleHead = pCircleHead;
			pChild->isExit = false;
			sprintf(pChild->nameProgram, "%s_%02d", CHILD_DOCTOR_PROGRAM, indexDoctor);

			pthread_create(&pChild->idThread, NULL, &threadDoctor, (void*)pChild);
			pthread_detach(pChild->idThread);			// will use detached threads

			TAILQ_INSERT_TAIL(&colDoctor, pChild, allChildren);

			printf("Start Doctor: %d\n", indexDoctor);
			indexDoctor++;

			break;
		}
		case 'k':								// terminate all Child processes
		{
			stopChildren();						// clear resources
			TAILQ_INIT(&colProducer);           /* Initialize the queue. */
			TAILQ_INIT(&colConsumer);           /* Initialize the queue. */
			TAILQ_INIT(&colDoctor);             /* Initialize the queue. */
			break;
		}
		case 'q':
		{
			printf("main OK quit\n");
			isQuit = true;
			break;
		}

		}
	}
	printf("main OK main\n");
	stopChildren();								// clear resources
	pthread_exit(NULL);							// exit main thread

	return 0;
}
// clear resources
void stopChildren()
{
	stopChildren(TC_PRODUCER, false /* freeMemory */);			// terminate all Child processes
	stopChildren(TC_CONSUMER, false /* freeMemory */);			// terminate all Child processes
	stopChildren(TC_DOCTOR, false /* freeMemory */);			// terminate all Child processes
}

void stopChild(Child* pChild)
{
	pChild->isExit = true;
}

void stopChildren(TYPE_CHILD type, bool freeMemory)
{
	struct HeadOfCollection* pColl = NULL;
	const char* message;

	if (type == TC_PRODUCER)
	{
		pColl = &colProducer;
		message = "Terminate Producer: %d\n";
	}
	else if (type == TC_CONSUMER)
	{
		pColl = &colConsumer;
		message = "Terminate Consumer: %d\n";
	}
	else
	{
		pColl = &colDoctor;
		message = "Terminate Doctor: %d\n";
	}
	// delete from the collection
	for (struct Child* pChild = pColl->tqh_first; pChild != NULL; )
	{
		struct Child* pChildNext;

		pChildNext = pChild->allChildren.tqe_next;

		printf(message, pChild->index);
		stopChild(pChild);						// send signal to Child

		if (freeMemory == true)
		{
			free(pChild);
		}
		pChild = pChildNext;
	}
}

// initialize circled collection for Messages
CircleHead* createColMessage()
{
	size_t		sizeHead;
	void*		pHeapMemory;
	CircleHead* pHead;

	sizeHead = sizeof(CircleHead);
	pHeapMemory = malloc(sizeHead);
	pHead = (CircleHead*)pHeapMemory;

	circleQueueInit(pHead, SIZE_MESSAGE_QUEUE);

	return pHead;
}
