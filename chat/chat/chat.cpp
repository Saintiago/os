// chat.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define BUFSIZE 512

using namespace std;

struct ClientInfo
{
	HANDLE pipe;
	int id;
};

map<HANDLE, ClientInfo> clientList;

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

DWORD WINAPI ServerThread(LPVOID hPipe)
{
	hPipe = (HANDLE)hPipe;
	bool isNewClient = true;
	TCHAR buffer[BUFSIZE];

	while (true)
	{
		DWORD bytesRead = 0;
		if (!ReadFile(
			hPipe,
			buffer,
			BUFSIZE*sizeof(TCHAR),
			&bytesRead,
			NULL
			) || bytesRead == 0)
		{
			if (GetLastError() != ERROR_BROKEN_PIPE)
			{
				ErrorExit(TEXT("ReadFile"));
			}
			break;
		}

		if (isNewClient)
		{
			cout << "Client connected." << endl;
			isNewClient = false;
			clientList[hPipe].id = static_cast<int>(*buffer);
		}
		else
		{
			cout << buffer << endl;
			for (auto pipe : clientList)
			{
				if (pipe.first != hPipe)
				{
					DWORD bytesWritten = 0;
					if (!WriteFile(
						pipe.second.pipe,
						buffer,
						BUFSIZE*sizeof(TCHAR) + 1,
						&bytesWritten,
						NULL
						)
						|| BUFSIZE*sizeof(TCHAR) + 1 != bytesWritten)
					{
						ErrorExit(TEXT("WriteFile"));
					}
				}
			}
		}
	}

	cout << "Client disconnected." << endl;
	DisconnectNamedPipe(clientList[hPipe].pipe);
	CloseHandle(clientList[hPipe].pipe);
	DisconnectNamedPipe(hPipe);
	CloseHandle(hPipe);
	clientList.erase(hPipe);

	return 0;
}

void StartServer()
{
	LPTSTR pipeOutName = TEXT("\\\\.\\pipe\\chatOut");
	LPTSTR pipeInName = TEXT("\\\\.\\pipe\\chatIn");

	cout << "Server successfully started." << endl;
	for (;;)
	{
		HANDLE pipeIn = CreateNamedPipe(
			pipeInName,
			PIPE_ACCESS_INBOUND,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES,
			BUFSIZE,
			BUFSIZE,
			0,
			NULL
			);

		HANDLE pipeOut = CreateNamedPipe(
			pipeOutName,
			PIPE_ACCESS_OUTBOUND,
			PIPE_TYPE_MESSAGE | PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES,
			BUFSIZE,
			BUFSIZE,
			0,
			NULL
			);

		if (pipeIn == INVALID_HANDLE_VALUE || pipeOut == INVALID_HANDLE_VALUE)
		{
			ErrorExit(TEXT("CreateNamedPipe"));
		}

		BOOL pipeInReady = ConnectNamedPipe(pipeIn, NULL)
			? true
			: (GetLastError() == ERROR_PIPE_CONNECTED);
		BOOL pipeOutReady = ConnectNamedPipe(pipeOut, NULL)
			? true
			: (GetLastError() == ERROR_PIPE_CONNECTED);

		if (pipeInReady && pipeOutReady)
		{
			HANDLE hThread = CreateThread(NULL, 0, ServerThread, (LPVOID)pipeIn, 0, NULL);

			if (hThread == NULL)
			{
				ErrorExit(TEXT("CreateThread"));
				return;
			}

			CloseHandle(hThread);
			ClientInfo dc;
			dc.pipe = pipeOut;
			clientList.insert({ pipeIn, dc });
		}
		else
		{
			ErrorExit(TEXT("ConnectNamedPipe"));
			CloseHandle(pipeIn);
			CloseHandle(pipeOut);
		}
	}
}

DWORD WINAPI ClientSpeaker(LPVOID hPipe)
{
	HANDLE pipeOut = (HANDLE)hPipe;
	string message;

	message = to_string(rand() % 10000 + 1);
	DWORD bytesWritten = message.size() + 1;
	DWORD bytesRead = 0;
	if (!WriteFile(
		pipeOut,
		message.c_str(),
		bytesWritten,
		&bytesRead,
		NULL
		) || bytesRead != bytesWritten)
	{
		ErrorExit(TEXT("WriteFile"));
		CloseHandle(pipeOut);
		return 1;
	}

	while (getline(std::cin, message))
	{
		DWORD bytesWritten = message.size() + 1;
		DWORD bytesRead = 0;
		if (!WriteFile(
			pipeOut,
			message.c_str(),
			bytesWritten,
			&bytesRead,
			NULL
			) || bytesRead != bytesWritten)
		{
			ErrorExit(TEXT("WriteFile"));
			break;
		}
	}

	CloseHandle(pipeOut);

	return 0;
}

DWORD WINAPI ClientListener(LPVOID hPipe)
{
	HANDLE pipeIn = (HANDLE)hPipe;
	TCHAR buffer[BUFSIZE];

	for (;;)
	{
		DWORD bytesRead = 0;
		if (!ReadFile(
			pipeIn,
			buffer,
			BUFSIZE*sizeof(TCHAR),
			&bytesRead,
			NULL) || bytesRead == 0)
		{
			if (GetLastError() == ERROR_BROKEN_PIPE)
			{
				cout << "Server disconnected." << endl;
			}
			else
			{
				ErrorExit(TEXT("ReadFile"));
			}
			break;
		}

		cout << buffer << endl;
	}
	CloseHandle(pipeIn);

	return 0;
}

void StartClient()
{
	HANDLE hThread[2];

	LPTSTR pipeOutName = TEXT("\\\\.\\pipe\\chatOut");
	LPTSTR pipeInName = TEXT("\\\\.\\pipe\\chatIn");

	HANDLE pipeOut = CreateFile(
		pipeOutName,   // pipe name 
		GENERIC_READ |  // read and write access 
		GENERIC_WRITE,
		0,              // no sharing 
		NULL,           // default security attributes
		OPEN_EXISTING,  // opens existing pipe 
		0,              // default attributes 
		NULL);          // no template file 

	if (pipeOut == INVALID_HANDLE_VALUE)
	{
		ErrorExit(TEXT("CreateFile"));
	}

	hThread[0] = CreateThread(
		NULL,
		0,
		ClientSpeaker,
		(LPVOID)pipeOut,
		0,
		NULL
		);

	if (hThread[0] == NULL)
	{
		ErrorExit(TEXT("CreateThread"));
	}

	HANDLE pipeIn = CreateFile(
		pipeInName,
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL
		);

	if (pipeIn == INVALID_HANDLE_VALUE)
	{
		ErrorExit(TEXT("CreateFile"));
	}

	hThread[1] = CreateThread(NULL, 0, ClientListener, (LPVOID)pipeIn, 0, NULL);

	if (hThread[1] == NULL)
	{
		ErrorExit(TEXT("CreateThread"));
	}

	WaitForMultipleObjects(2, hThread, FALSE, INFINITE);

	for (int i = 0; i < 2; ++i)
	{
		CloseHandle(hThread[i]);
	}

	CloseHandle(pipeOut);
	CloseHandle(pipeIn);

	return;
}

int main()
{
	srand(time(NULL));

	LPCTSTR mutexName = TEXT("Chat");
	HANDLE handleMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, mutexName);
	if (handleMutex)
	{
		StartClient();
		return 0;
	}
	else
	{
		handleMutex = CreateMutex(NULL, FALSE, mutexName);
		StartServer();
	}

	return 0;
}