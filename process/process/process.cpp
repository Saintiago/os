#include "stdafx.h"

void PrintProcessList();
bool PrintProcessInfo(DWORD pid);
int ListProcessModules(DWORD dwPID);
void PrintProcessNameAndID(DWORD processID);

using namespace std;

int main(int argc, char* argv[])
{
	if (argc == 1)
	{
		PrintProcessList();
	}
	else
	{
		DWORD processId;
		istringstream ss(argv[1]);
		if (!(ss >> processId))
			_tprintf(TEXT("Invalid number %hs \n"), argv[1]);
		if (!PrintProcessInfo(processId))
		{
			_tprintf(TEXT("\n  Process #0x%08X not found\n"), processId);
		}
	}

	return 0;
}

void PrintProcessList()
{
	DWORD aProcesses[1024], cbNeeded, cProcesses;
	unsigned int i;

	if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
	{
		return;
	}

	cProcesses = cbNeeded / sizeof(DWORD);

	for (i = 0; i < cProcesses; i++)
	{
		if (aProcesses[i] != 0)
		{
			PrintProcessNameAndID(aProcesses[i]);
		}
	}
}


bool PrintProcessInfo(DWORD pid)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	bool found = false;

	if (hSnapshot)
	{
		PROCESSENTRY32 pe32;
		pe32.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(hSnapshot, &pe32))
		{
			do
			{
				if (pe32.th32ProcessID == pid)
				{
					_tprintf(TEXT("\n  Process name:  %s"), pe32.szExeFile);
					_tprintf(TEXT("\n  Process ID        = 0x%08X"), pe32.th32ProcessID);
					_tprintf(TEXT("\n  Thread count      = %d"), pe32.cntThreads);
					_tprintf(TEXT("\n  Parent process ID = 0x%08X"), pe32.th32ParentProcessID);
					_tprintf(TEXT("\n  Priority base     = %d"), pe32.pcPriClassBase);
					_tprintf(TEXT("\n  Modules:\n"));
					ListProcessModules(pe32.th32ProcessID);
					found = true;
				}
			} 
			while (Process32Next(hSnapshot, &pe32));
		}
		CloseHandle(hSnapshot);
	}
	return found;
}

int ListProcessModules(DWORD processID)
{
	HMODULE hMods[1024];
	HANDLE hProcess;
	DWORD cbNeeded;
	unsigned int i;

	printf("\nProcess ID: %u\n", processID);

	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
		PROCESS_VM_READ,
		FALSE, processID);
	if (NULL == hProcess)
		return 1;

	if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
	{
		for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
		{
			TCHAR szModName[MAX_PATH];

			if (GetModuleFileNameEx(hProcess, hMods[i], szModName,
				sizeof(szModName) / sizeof(TCHAR)))
			{
				_tprintf(TEXT("\t%s (0x%08X)\n"), szModName, hMods[i]);
			}
		}
	}

	CloseHandle(hProcess);

	return 0;
}

void PrintProcessNameAndID(DWORD processID)
{
	TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
		PROCESS_VM_READ,
		FALSE, processID);

	if (NULL != hProcess)
	{
		HMODULE hMod;
		DWORD cbNeeded;

		if (EnumProcessModules(hProcess, &hMod, sizeof(hMod),
			&cbNeeded))
		{
			GetModuleBaseName(hProcess, hMod, szProcessName,
				sizeof(szProcessName) / sizeof(TCHAR));
		}
	}

	_tprintf(TEXT("%s  (PID: %u)\n"), szProcessName, processID);

	CloseHandle(hProcess);
}