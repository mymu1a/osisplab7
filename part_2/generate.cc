#include "global.h"

#include <stdio.h>
#include <stdlib.h>

#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

#include "gengetopt/generate_cmdline.h"

struct gengetopt_args_info	config;

void generateRecord(record_s* pRecord, uint64_t index);
void writeRecord(record_s* pRecord, int fd);


int main(int argc, char** argv)
{
    if (cmdline_parser(argc, argv, &config) != 0)
    {
        exit(1);
    }
    int fd;

	fd = open(config.filename_arg, O_RDWR | O_CREAT, 0666);
    if (fd < 0)
    {
        printf("Error: cannot open file:\n");
        return 1;
    }
    record_s record;

    record = { "Smith", "London", 1 };  writeRecord(&record, fd);
    record = { "Jones", "London", 1 };  writeRecord(&record, fd);
    record = { "Williams", "London", 1 };  writeRecord(&record, fd);
    record = { "Taylor", "London", 1 };  writeRecord(&record, fd);

    record = { "Brown", "York", 2 };  writeRecord(&record, fd);
    record = { "Green", "York", 3 };  writeRecord(&record, fd);
    record = { "Davies", "York", 6 };  writeRecord(&record, fd);
    record = { "Wilson", "York", 8 };  writeRecord(&record, fd);
    
    close(fd);

    printf("config.filename: %s\n", config.filename_arg);

    return 0;
}


void writeRecord(record_s* pRecord, int fd)
{
    write(fd, (void*)&pRecord->name, sizeof(pRecord->name));
    write(fd, (void*)&pRecord->address, sizeof(pRecord->address));
    write(fd, (void*)&pRecord->semester, sizeof(uint8_t));
}
