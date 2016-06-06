// process.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

//  Forward declarations:
void PrintProcessList();
bool PrintProcessInfo(DWORD pid);
BOOL ListProcessModules(DWORD dwPID);

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
			cerr << "Invalid number " << argv[1] << '\n';
		if (!PrintProcessInfo(processId))
		{
			cout << "Process #" << processId << " not found" << endl;
		}
	}

	return 0;
}

void PrintProcessList()
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	
	if (hSnapshot) 
	{
		PROCESSENTRY32 pe32;
		pe32.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(hSnapshot, &pe32)) 
		{
			do 
			{
				cout << pe32.th32ProcessID << ": " << pe32.szExeFile << endl;
			} 
			while (Process32Next(hSnapshot, &pe32));
		}
		CloseHandle(hSnapshot);
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
					cout << "Process name " << pe32.szExeFile << endl;
					cout << "Process ID " << pe32.th32ProcessID << endl;
					cout << "Thread count " << pe32.cntThreads << endl;
					cout << "Parent process " << pe32.th32ParentProcessID << endl;
					cout << "Priority base " << pe32.pcPriClassBase << endl;
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

BOOL ListProcessModules(DWORD dwPID)
{
	HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
	MODULEENTRY32 me32;

	// Take a snapshot of all modules in the specified process.
	hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
	if (hModuleSnap == INVALID_HANDLE_VALUE)
	{
		return(FALSE);
	}

	// Set the size of the structure before using it.
	me32.dwSize = sizeof(MODULEENTRY32);

	// Retrieve information about the first module,
	// and exit if unsuccessful
	if (!Module32First(hModuleSnap, &me32))
	{
		CloseHandle(hModuleSnap);           // clean the snapshot object
		return(FALSE);
	}

	// Now walk the module list of the process,
	// and display information about each module
	do
	{
		_tprintf(TEXT("\n\n     MODULE NAME:     %s"), me32.szModule);
		_tprintf(TEXT("\n     Executable     = %s"), me32.szExePath);
		_tprintf(TEXT("\n     Process ID     = 0x%08X"), me32.th32ProcessID);
		_tprintf(TEXT("\n     Ref count (g)  = 0x%04X"), me32.GlblcntUsage);
		_tprintf(TEXT("\n     Ref count (p)  = 0x%04X"), me32.ProccntUsage);
		_tprintf(TEXT("\n     Base address   = 0x%08X"), (DWORD)me32.modBaseAddr);
		_tprintf(TEXT("\n     Base size      = %d"), me32.modBaseSize);

	} while (Module32Next(hModuleSnap, &me32));

	CloseHandle(hModuleSnap);
	return(TRUE);
}