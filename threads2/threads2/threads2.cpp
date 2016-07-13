#include "stdafx.h"

#define BUF_SIZE 255

DWORD WINAPI Thread1(LPVOID lpParam);
DWORD WINAPI Thread2(LPVOID lpParam);
DWORD WINAPI Thread3(LPVOID lpParam);

unsigned long cycles, counterTotal, counterThread1, counterThread2, counterThread3;

int main()
{
	cycles = 100000;
	counterTotal = 0;
	counterThread1 = 0;
	counterThread2 = 0;
	counterThread3 = 0;

	HANDLE handleThread1;
	HANDLE handleThread2;
	HANDLE handleThread3;
	HANDLE arrayOfThreadHandles[3];

	handleThread1 = CreateThread(NULL, 0, Thread1, &cycles, 0, NULL);
	if (handleThread1 == NULL)
		return 1;

	handleThread2 = CreateThread(NULL, 0, Thread2, &cycles, 0, NULL);
	if (handleThread2 == NULL)
		return 1;

	handleThread3 = CreateThread(NULL, 0, Thread3, &cycles, 0, NULL);
	if (handleThread3 == NULL)
		return 1;

	arrayOfThreadHandles[0] = handleThread1;
	arrayOfThreadHandles[1] = handleThread2;
	arrayOfThreadHandles[2] = handleThread3;

	WaitForMultipleObjects(3, arrayOfThreadHandles, TRUE, INFINITE);

	CloseHandle(handleThread1);
	CloseHandle(handleThread2);
	CloseHandle(handleThread3);

	printf("\n counterTotal: %lu", counterTotal);
	printf("\n counterThread1 + counterThread2 + counterThread3: %lu", counterThread1 + counterThread2 + counterThread3);

	return 0;
}

DWORD WINAPI Thread1(LPVOID param)
{
	for (unsigned long i = 0; i < cycles; i++)
	{
		counterTotal++;
		counterThread1++;
	}
	return 0;
}

DWORD WINAPI Thread2(LPVOID param)
{
	for (unsigned long i = 0; i < cycles; i++)
	{
		counterTotal++;
		counterThread2++;
	}
	return 0;
}

DWORD WINAPI Thread3(LPVOID param)
{
	for (unsigned long i = 0; i < cycles; i++)
	{
		counterTotal++;
		counterThread2++;
	}
	return 0;
}
