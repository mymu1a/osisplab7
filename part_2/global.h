#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include <cerrno>
#include <cstdio>
#include <unistd.h>

#include <sys/types.h>
#include <cstdint>

struct record_s
{
	char name[80];		// �.�.�. ��������
	char address[80];	// ����� ����������
	uint8_t semester;	// �������
};

enum STATE_ACTIVE { SA_NONE, SA_GET, SA_MODIFY };

struct record_active
{
	short			index;
	STATE_ACTIVE	state;
	record_s		record;
};

#endif // __GLOBALS_H__
