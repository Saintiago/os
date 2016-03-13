// os_ver.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#pragma warning (disable : 4996)

using namespace std;

int main()
{
	OSVERSIONINFOEX osvi;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	GetVersionEx((OSVERSIONINFO*) &osvi);

	string osName = "";

	switch (osvi.dwMajorVersion)
	{
	case 10:
		osName = (osvi.wProductType == VER_NT_WORKSTATION) ? "Windows 10" : "Windows Server 2016 Technical Preview";
		break;

	case 6:

		switch (osvi.dwMinorVersion)
		{
		case 3:
			osName = (osvi.wProductType == VER_NT_WORKSTATION) ? "Windows 8.1" : "Windows Server 2012 R2";
			break;
		case 2:
			osName = (osvi.wProductType == VER_NT_WORKSTATION) ? "Windows 8" : "Windows Server 2012";
			break;
		case 1:
			osName = (osvi.wProductType == VER_NT_WORKSTATION) ? "Windows 7" : "Windows Server 2008 R2";
			break;
		case 0:
			osName = (osvi.wProductType == VER_NT_WORKSTATION) ? "Windows Vista" : "Windows Server 2008";
			break;
		}
		break;

	case 5:

		switch (osvi.dwMinorVersion)
		{
		case 2:
			osName = (GetSystemMetrics(SM_SERVERR2) == 0) ? "Windows Server 2003" : "Windows Server 2003 R2";
			break;
		case 1:
			osName = "Windows XP";
			break;
		case 0:
			osName = "Windows 2000";
			break;
		}
		break;
	}

	if (osName != "")
	{
		cout << osName << " " << osvi.szCSDVersion << endl;
	}
	else
	{
		cout << "Unknown OS." << endl;
	}
}
