#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include <cerrno>
#include <cstdio>
#include <semaphore.h>
#include <unistd.h>

#include <sys/queue.h>
#include <sys/types.h>


#define CHILD_PRODUSER_PROGRAM "produser"
#define CHILD_CONSUMER_PROGRAM "consumer"
#define CHILD_DOCTOR_PROGRAM   "doctor"

#define SEM_PRODUSER_VALUE 1
#define SEM_CONSUMER_VALUE 2

enum TYPE_CHILD { TC_PRODUCER, TC_CONSUMER, TC_DOCTOR };


struct Message
{
	unsigned char type;
	short hash;
	unsigned char size;
	u_char* pData;
};

struct CircleHead;

struct Child
{
	unsigned	index;					// child index
	char		nameProgram[12];
	bool		isExit;					// sugest Porducer ( or Consumer ) to exit
	bool		inProcess;				// Porducer ( or Consumer ) works with Message
	pthread_t	idThread;
	CircleHead* pCircleHead;
	sem_t*		pSemaphore;
	bool		doctorInc;

	TAILQ_ENTRY(Child) allChildren;         /* Tail queue. */
};

#endif // __GLOBALS_H__
