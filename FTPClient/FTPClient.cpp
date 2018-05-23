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

			CSocket client, ClientData, activeSock;
			//Socket is used to send command to server
			client.Create();
			
			string ftpServer;
			bool connected = false;
			bool passive = false;
			vector<string> vstr;
			do {
				cout << "-->FTP server: ";
				getline(cin, ftpServer);
				deleteSpaces(ftpServer);
				const char* msg = (const char*)ftpServer.c_str();
				wchar_t *rIP = new wchar_t[strlen(msg) + 1];
				mbstowcs(rIP, msg, strlen(msg) + 1);
				if (client.Connect(rIP, 21) == 0) {
					cout << "Can't connect to FTP server" << endl;
				}
				else {
					cout << "Successfully connect to FTP server" << endl;
					vector<char> buffer(MAX_BUFFER);
					int byteReceived = 0;
					string res;
					do {
						byteReceived = client.Receive(&buffer[0], MAX_BUFFER, 0);
						if (byteReceived == -1) {
							cout << "Error when receiving server data" << endl;
						}
						else {
							res.append(buffer.begin(), buffer.begin() + byteReceived);
						}
					} while (byteReceived == MAX_BUFFER);
					connected = true;
					cout << res;
				}
			} while (!connected);
			//Login to server
			logIn(client);

			while (1) {
				cout << "-->ftp: ";
				string command;
				getline(cin, command);
				deleteSpaces(command);
				stringstream is(command);
				is >> command;
				for (int i = 0; i < command.length(); i++) {
					command[i] = tolower(command[i]);
				}
				if (command == "pwd") {
					command += "\r\n";
					client.Send((char*)command.c_str(), command.length());
					cout << receiveMessage(client);
				}
				else if (command == "cd") {
					command = getParameter(is, "cd");
					changeServerPath(client, command);
				}
				else if (command == "lcd") {
					command = getParameter(is, "lcd");
					changeClientPath(command);
				}
				else if (command == "get") {
					command = getParameter(is, "get");
					if (passive) {
						connectDataPort(client, ClientData, passive);
						getFile(client, ClientData, command, passive);
					}
					else {
						connectDataPort(client, activeSock, passive);
						getFile(client, activeSock, command, passive);
					}
				}
				else if (command == "passive") {
					passive = true;
					cout << "Successfully change to passive mode" << endl;
				}
				else if (command == "delete") {
					command = getParameter(is, "delete");
					deleteFile(client, command);
				}
				else if (command == "mkdir") {
					command = getParameter(is, "mkdir");
					createFolder(client, command);
				}
				else if (command == "rmdir") {
					command = getParameter(is, "rmdir");
					deleteEmptyFolder(client, command);
				}
				else if (command == "quit" || command == "exit") {
					string stringSend = "QUIT\n";
					client.Send((char*)stringSend.c_str(), stringSend.length());
					cout << receiveMessage(client);
				}
				else if (command == "put") {
					command = getParameter(is, "put");
					if (passive) {
						connectDataPort(client, ClientData, passive);
						uploadFile(client, ClientData, command, passive);
					}
					else {
						connectDataPort(client, activeSock, passive);
						uploadFile(client, activeSock, command, passive);
					}
				}
				else if (command == "ls") {
					command = getParameter(is, "ls");
					if (passive) {
						connectDataPort(client, ClientData, passive);
						getFileList(client, ClientData, command, passive, vstr);
						printFileList(vstr);
					}
					else {
						connectDataPort(client, activeSock, passive);
						getFileList(client, activeSock, command, passive, vstr);
						printFileList(vstr);
					}
				}
				else if (command == "dir") {
					command = getParameter(is, "dir");
					if (passive) {
						connectDataPort(client, ClientData, passive);
						cout << getFileListDetail(client, ClientData, command, passive);
					}
					else {
						connectDataPort(client, activeSock, passive);
						cout << getFileListDetail(client, activeSock, command, passive);
						//printFileList(list);
					}
				}
				else if (command == "mdelete") {
					deleteMultipleFiles(client, ClientData, activeSock, is, passive);
				}
				else if (command == "mget") {
					getMultipleFiles(client, ClientData, activeSock, is, passive);
				}
				else if (command == "mput") {
					uploadMultipleFiles(client, ClientData, activeSock, is, passive);
				}
				else {
					cout << "Invalid command" << endl;
				}
				ClientData.Close();
				activeSock.Close();
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
