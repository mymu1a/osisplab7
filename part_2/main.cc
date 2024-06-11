#include "global.h"

#include <ctype.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/queue.h>
#include <sys/wait.h>

#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

#include "gengetopt/cmdline.h"


u_short countRecord = 8;
struct gengetopt_args_info	config;
record_active recordActive;

bool lock(int fd, record_active& recordActive);
void printAll(int fd);
// read Record on the current position
void read(int fd, record_s& record);
bool read(int fd, record_active& recordActive, short index);
bool seekToIndex(int fd, short index);
bool unlock(int fd, record_active& recordActive);




int main(int argc, char* argv[], char* envp[])
{
	printf("main ST\n");

	if (cmdline_parser(argc, argv, &config) != 0)
	{
		exit(1);
	}
	//=== open file ===
	int fd;
	char* pathFile = config.filename_arg;

	fd = open(pathFile, O_RDWR, 0666);
	if (fd < 0)
	{
		printf("Error: cannot open Record file:\n");
		printf("main OK\n");
		return 1;
	}

	printf("Type: l k s g s<num> g<num> p<num>\n");

	//=== read user input ===
	int ch;
	int indexRecord = 0;

//	while ((ch = getchar()) != EOF)
	while (true)
	{
		ch = getchar();
		if (ch == EOF)
		{
			printf("EOF");
			continue;
		}
		int charNext;
		struct Child* pChild = NULL;

		if (ch == 'g')
		{
			charNext = getchar();
			if (isdigit(charNext))
			{
				indexRecord = charNext - 0x30;

				if (indexRecord < 0 || countRecord <= indexRecord)
				{
					ungetc(charNext, stdin);		// return 'char' back to the stream
					continue;
				}
			}
		}
		//=== select command ===
		switch (ch)
		{
		case 'g':
		{
			read(fd, recordActive, indexRecord);
			break;
		}
		case 'l':
		{
			printf("Current state:\n");
			printAll(fd);

			break;
		}
		case 'm':
		{
			if (recordActive.state != SA_GET)
			{
				printf("GET object for modification\n");
				continue;
			}
			// MOVE here Just For Test => increase operation time
			lock(fd, recordActive);									// LOCK

			record_s recordMod;
			record_s recordInFile;

			printf("Enter Name: ");
			scanf("%s", recordMod.name);

			printf("Enter Address: ");
			scanf("%s", recordMod.address);

//			lock(fd, recordActive);									// LOCK
			seekToIndex(fd, recordActive.index);
			read(fd, recordInFile);

			// nobody have changed this Record
			if (strcmp(recordActive.record.name, recordInFile.name) == 0 && strcmp(recordActive.record.address, recordInFile.address) == 0)
			{
				seekToIndex(fd, recordActive.index);

				write(fd, recordMod.name, sizeof(recordMod.name));
				write(fd, recordMod.address, sizeof(recordMod.address));

				unlock(fd, recordActive);
				continue;
			}
			// this Record was changed
			printf("\nRecord was changed, sorry\n");
			unlock(fd, recordActive);

			// 
			strcpy(recordActive.record.name, recordInFile.name/*, sizeof(recordInFile.name)*/);
			strcpy(recordActive.record.address, recordInFile.address/*, sizeof(recordInFile.address)*/);

			printf("Current state:\n\n");
			printf("RECORD #%d:\n", recordActive.index);
			printf("%10s\t", recordActive.record.name);
			printf("%10s\t", recordActive.record.address);
			printf("%d\n", recordActive.record.semester);

			recordActive.state = SA_GET;
break;
		}
		case 'q':
		{

			printf("main OK quit\n");
			close(fd);
			return 0;
		}

		}
	}
	close(fd);
	printf("main OK main");

	return 0;
}

void printAll(int fd)
{
	if (lseek(fd, 0, SEEK_SET) == -1)
	{
		printf("Error: printAll->lseek (%d)\n", errno);
		printf("  errno: %s\n", strerror(errno));
		return;
	}
	record_s record;

	for (short i = 0; i < countRecord; i++)
	{
		read(fd, record);

		printf("%10s\t", record.name);
		printf("%10s\t", record.address);
		printf("%d\n", record.semester);
	}
}

// read Record on the current position
void read(int fd, record_s& record)
{
	read(fd, (void*)&record.name, sizeof(record.name));
	read(fd, (void*)&record.address, sizeof(record.address));
	read(fd, (void*)&record.semester, sizeof(record.semester));
}

// lock the specific area of the file
bool lock(int fd, record_active& recordActive)
{
	struct flock lock;

	lock.l_type = F_RDLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = sizeof(record_s) * recordActive.index;
	lock.l_len = sizeof(record_s);

	if (fcntl(fd, F_SETLK, &lock) == -1)
	{
		printf("Error: lock->fcntl (%d)\n", errno);
		printf("  errno: %s\n", strerror(errno));
		return false;
	}

	return true;
}

// unlock the specific area of the file
bool unlock(int fd, record_active& recordActive)
{
	struct flock lock;

	lock.l_type = F_UNLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = sizeof(record_s) * recordActive.index;
	lock.l_len = sizeof(record_s);

	if (fcntl(fd, F_SETLK, &lock) == -1)
	{
		printf("Error: unlock->fcntl (%d)\n", errno);
		printf("  errno: %s\n", strerror(errno));
		return false;
	}

	return true;
}

bool read(int fd, record_active& recordActive, short index)
{
	//=== seek to record ===
	seekToIndex(fd, index);

	//=== read Record and print ===
	read(fd, recordActive.record);

	printf("RECORD #%d:\n", index);
	printf("%10s\t", recordActive.record.name);
	printf("%10s\t", recordActive.record.address);
	printf("%d\n", recordActive.record.semester);

	//=== change state ===
	recordActive.index = index;
	recordActive.state = SA_GET;

	return true;
}

bool seekToIndex(int fd, short index)
{
	off_t offset;

	offset = sizeof(record_s) * index;
	if (lseek(fd, offset, SEEK_SET) == -1)
	{
		printf("Error: read->lseek (%d)\n", errno);
		printf("  errno: %s\n", strerror(errno));
		return false;
	}
	return true;
}