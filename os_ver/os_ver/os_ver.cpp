// os_ver.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#pragma warning (disable : 4996)

int main()
{
	OSVERSIONINFO osvi;
	BOOL bIsWindowsXPorLater;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	GetVersionEx(&osvi);

	bIsWindowsXPorLater =
		((osvi.dwMajorVersion > 5) ||
			((osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion >= 1)));

	if (bIsWindowsXPorLater)
		printf("The system meets the requirements.\n");
	else printf("The system does not meet the requirements.\n");
}
