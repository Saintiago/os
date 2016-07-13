// chat.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define PIPE_NAME "chat"

using namespace std;

string GetClientMessage()
{
	string message;
	cin >> message;
	return message;
}

bool WriteClientMessage(string message, HANDLE hPipe)
{
	DWORD dwWritten;

	string mess = message + "\n";
	size_t messSize = mess.size() + 1;

	return WriteFile(hPipe,
		mess.c_str(),
		messSize,   // = length of string + terminating '\0' !!!
		&dwWritten,
		NULL
		);
}

int main(void)
{
	HANDLE hPipe;
	
	hPipe = CreateFile(TEXT(PIPE_NAME),
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
	if (hPipe != INVALID_HANDLE_VALUE)
	{
		for (;;)
		{
			WriteClientMessage(GetClientMessage(), hPipe);
		}
		CloseHandle(hPipe);
	}
	else
	{
		HANDLE hPipe;
		char buffer[1024];
		DWORD dwRead;

		hPipe = CreateNamedPipe(TEXT(PIPE_NAME),
			PIPE_ACCESS_DUPLEX | PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,   // FILE_FLAG_FIRST_PIPE_INSTANCE is not needed but forces CreateNamedPipe(..) to fail if the pipe already exists...
			PIPE_WAIT,
			1,
			1024 * 16,
			1024 * 16,
			NMPWAIT_USE_DEFAULT_WAIT,
			NULL);
		while (hPipe != INVALID_HANDLE_VALUE)
		{
			if (ConnectNamedPipe(hPipe, NULL) != FALSE)   // wait for someone to connect to the pipe
			{
				while (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &dwRead, NULL) != FALSE)
				{
					/* add terminating zero */
					buffer[dwRead] = '\0';

					/* do something with data in buffer */
					printf("%s", buffer);
				}
			}

			DisconnectNamedPipe(hPipe);
		}
	}

	return (0);
}