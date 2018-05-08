// FTPClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Function.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

CWinApp theApp;



int main()
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // initialize MFC and print and error on failure
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: change error code to suit your needs
            wprintf(L"Fatal Error: MFC initialization failed\n");
            nRetCode = 1;
        }
        else
        {
            // TODO: code your application's behavior here.
			if (AfxSocketInit() == false) {
				cout << "Socket initialization failed" << endl;
				return 1;
			}
			CSocket client;
			client.Create();
			string ftpServer;
			bool connected = false;
			do {
				//cout << "-->FTP server: ";
				//getline(cin, ftpServer);
				if (client.Connect(_T("127.0.0.1"),21) == 0) {
					cout << "Can't connect to FTP server" << endl;
				}
				else {
					cout << "Successfully connect to FTP server" << endl;
					cout << receiveMessage(client);
					connected = true;
				}
			} while (!connected);
			//Login to server
			logIn(client);
			
			while (1) {
				cout << "-->ftp: ";
				string command;
				getline(cin, command);
				sendCommandToServer(client,command);
			};
			client.Close();
        }
    }
    else
    {
        // TODO: change error code to suit your needs
        wprintf(L"Fatal Error: GetModuleHandle failed\n");
        nRetCode = 1;
    }

    return nRetCode;
}
