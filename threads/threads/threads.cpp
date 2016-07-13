// threads.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define BUF_SIZE 255

DWORD WINAPI ThreadGet(LPVOID lpParam);
DWORD WINAPI ThreadPut1(LPVOID lpParam);
DWORD WINAPI ThreadPut2(LPVOID lpParam);
DWORD WINAPI ThreadPut3(LPVOID lpParam);

int main()
{
	HANDLE handleThreadGet;
	HANDLE handleThreadPut1;
	HANDLE handleThreadPut2;
	HANDLE handleThreadPut3;

	handleThreadGet = CreateThread(NULL, 0, ThreadGet, nullptr, 0, NULL);
	if (handleThreadGet == NULL)
		return 1;

	handleThreadPut1 = CreateThread(NULL, 0, ThreadPut1, nullptr, 0, NULL);
	if (handleThreadPut1 == NULL)
		return 1;

	handleThreadPut2 = CreateThread(NULL, 0, ThreadPut2, nullptr, 0, NULL);
	if (handleThreadPut2 == NULL)
		return 1;

	handleThreadPut3 = CreateThread(NULL, 0, ThreadPut3, nullptr, 0, NULL);
	if (handleThreadPut3 == NULL)
		return 1;

	WaitForSingleObject(handleThreadGet, INFINITE);

	CloseHandle(handleThreadGet);
	CloseHandle(handleThreadPut1);
	CloseHandle(handleThreadPut2);
	CloseHandle(handleThreadPut3);

    return 0;
}

DWORD WINAPI ThreadGet(LPVOID lpParam)
{
	getch();
	return 0;
}

DWORD WINAPI ThreadPut1(LPVOID lpParam)
{
	for (;;)
	{
		Sleep(500);
		putchar('1');
	}
		

	return 0;
}

DWORD WINAPI ThreadPut2(LPVOID lpParam)
{
	for (;;)
	{
		Sleep(500);
		putchar('2');
	}

	return 0;
}

DWORD WINAPI ThreadPut3(LPVOID lpParam)
{
	for (;;)
	{
		Sleep(500);
		putchar('3');
	}

	return 0;
}
