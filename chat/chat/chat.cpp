// chat.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define BUFSIZE 512

using namespace std;

string GetClientMessage()
{
	string message;
	std::getline(std::cin, message);
	return message;
}

bool WriteClientMessage(string message, HANDLE hPipe)
{
	BOOL   fSuccess = FALSE;
	DWORD  cbToWrite, cbWritten;

	std::wstring stemp = std::wstring(message.begin(), message.end());
	LPCWSTR lpvMessage = stemp.c_str();

	cbToWrite = (lstrlen(lpvMessage) + 1)*sizeof(TCHAR);

	fSuccess = WriteFile(
		hPipe,                  // pipe handle 
		lpvMessage,             // message 
		cbToWrite,              // message length 
		&cbWritten,             // bytes written 
		NULL);

	return fSuccess;
}

void ReadClientMessage(HANDLE hPipe)
{
	TCHAR  chBuf[BUFSIZE];
	BOOL   fSuccess = FALSE;
	DWORD  cbRead;
	do
	{
		// Read from the pipe. 

		fSuccess = ReadFile(
			hPipe,    // pipe handle 
			chBuf,    // buffer to receive reply 
			BUFSIZE*sizeof(TCHAR),  // size of buffer 
			&cbRead,  // number of bytes read 
			NULL);    // not overlapped 

		if (!fSuccess && GetLastError() != ERROR_MORE_DATA)
			break;

		_tprintf(TEXT("\"%s\"\n"), chBuf);
	} while (!fSuccess);  // repeat loop if ERROR_MORE_DATA 
}

void ErrorExit(LPTSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	ExitProcess(dw);
}

DWORD WINAPI ClientConnection(LPVOID lpvParam)
{
	HANDLE hHeap = GetProcessHeap();
	TCHAR* pchRequest = (TCHAR*)HeapAlloc(hHeap, 0, BUFSIZE*sizeof(TCHAR));

	DWORD cbBytesRead = 0, cbReplyBytes = 0, cbWritten = 0;
	BOOL fSuccess = FALSE;
	HANDLE hPipe = NULL;

	if (lpvParam == NULL)
	{
		printf("\nERROR - Pipe Server Failure:\n");
		printf("   InstanceThread got an unexpected NULL value in lpvParam.\n");
		printf("   InstanceThread exitting.\n");
		if (pchRequest != NULL) HeapFree(hHeap, 0, pchRequest);
		return (DWORD)-1;
	}

	if (pchRequest == NULL)
	{
		printf("\nERROR - Pipe Server Failure:\n");
		printf("   InstanceThread got an unexpected NULL heap allocation.\n");
		printf("   InstanceThread exitting.\n");
		return (DWORD)-1;
	}

	// Print verbose messages. In production code, this should be for debugging only.
	printf("InstanceThread created, receiving and processing messages.\n");

	// The thread's parameter is a handle to a pipe object instance. 

	hPipe = (HANDLE)lpvParam;

	// Loop until done reading
	while (1)
	{
		// Read client requests from the pipe. This simplistic code only allows messages
		// up to BUFSIZE characters in length.
		fSuccess = ReadFile(
			hPipe,        // handle to pipe 
			pchRequest,    // buffer to receive data 
			BUFSIZE*sizeof(TCHAR), // size of buffer 
			&cbBytesRead, // number of bytes read 
			NULL);        // not overlapped I/O 

		if (!fSuccess || cbBytesRead == 0)
		{
			if (GetLastError() == ERROR_BROKEN_PIPE)
			{
				_tprintf(TEXT("InstanceThread: client disconnected.\n"), GetLastError());
			}
			else
			{
				_tprintf(TEXT("InstanceThread ReadFile failed, GLE=%d.\n"), GetLastError());
			}
			break;
		}

		printf("%ls\n", pchRequest);

		cbReplyBytes = (lstrlen(pchRequest) + 1)*sizeof(TCHAR);
		fSuccess = WriteFile(
			hPipe,        // handle to pipe 
			pchRequest,     // buffer to write from 
			cbReplyBytes, // number of bytes to write 
			&cbWritten,   // number of bytes written 
			NULL);        // not overlapped I/O 

		if (!fSuccess || cbReplyBytes != cbWritten)
		{
			_tprintf(TEXT("InstanceThread WriteFile failed, GLE=%d.\n"), GetLastError());
			break;
		}
	}

	// Flush the pipe to allow the client to read the pipe's contents 
	// before disconnecting. Then disconnect the pipe, and close the 
	// handle to this pipe instance. 

	FlushFileBuffers(hPipe);
	DisconnectNamedPipe(hPipe);
	CloseHandle(hPipe);

	HeapFree(hHeap, 0, pchRequest);

	printf("InstanceThread exitting.\n");
	return 1;
}


int main(void)
{
	HANDLE hPipe = INVALID_HANDLE_VALUE, hThread = NULL;
	LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\chat");
	
	hPipe = CreateFile(
		lpszPipename,   // pipe name 
		GENERIC_READ |  // read and write access 
		GENERIC_WRITE,
		0,              // no sharing 
		NULL,           // default security attributes
		OPEN_EXISTING,  // opens existing pipe 
		0,              // default attributes 
		NULL);          // no template file 

	if (hPipe == INVALID_HANDLE_VALUE)
	{
		//ErrorExit(TEXT("CreateFile"));
	}

	if (hPipe != INVALID_HANDLE_VALUE)
	{
		for (;;)
		{
			if (!WriteClientMessage(GetClientMessage(), hPipe))
			{
				ErrorExit(TEXT("WriteClientMessage"));
			}
			ReadClientMessage(hPipe);
		}
		CloseHandle(hPipe);
	}
	else
	{
		BOOL   fConnected = FALSE;
		DWORD  dwThreadId = 0;
		cout << "Server started successfully." << endl;
		for (;;)
		{
			cout << "Waiting for connection." << endl;
			hPipe = CreateNamedPipe(
				lpszPipename,
				PIPE_ACCESS_DUPLEX,
				PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
				PIPE_UNLIMITED_INSTANCES,
				BUFSIZE,
				BUFSIZE,
				0,      
				NULL
			);

			if (hPipe == INVALID_HANDLE_VALUE)
			{
				ErrorExit(TEXT("CreateNamedPipe"));
				return -1;
			}

			fConnected = ConnectNamedPipe(hPipe, NULL) 
				? TRUE 
				: (GetLastError() == ERROR_PIPE_CONNECTED);

			if (fConnected)
			{
				printf("Client connected, creating a processing thread.\n");

				// Create a thread for this client. 
				hThread = CreateThread(
					NULL,              // no security attribute 
					0,                 // default stack size 
					ClientConnection,    // thread proc
					(LPVOID)hPipe,    // thread parameter 
					0,                 // not suspended 
					&dwThreadId);      // returns thread ID 

				if (hThread == NULL)
				{
					ErrorExit(TEXT("CreateThread"));
					return -1;
				}
				else CloseHandle(hThread);
			}
			else
				// The client could not connect, so close the pipe. 
				CloseHandle(hPipe);

		}
		
	}

	return (0);
}