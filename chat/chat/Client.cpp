#include "stdafx.h"
#include "Client.h"

using namespace std;

CClient::CClient(HANDLE mainPipe)
	: m_serverPipe(mainPipe)
{
}


CClient::~CClient()
{
}

void CClient::StartSpeakingToServer()
{
	for (;;)
	{
		if (!WriteClientMessage(GetClientMessage(), m_serverPipe))
		{
			throw exception("WriteClientMessage");
		}
	}
	CloseHandle(m_serverPipe);
}

string CClient::GetClientMessage()
{
	string message;
	std::getline(std::cin, message);
	return message;
}

bool CClient::WriteClientMessage(string message, HANDLE hPipe)
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
