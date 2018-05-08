#pragma once
#include <afxsock.h>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include "FTPClient.h"
#define MAX_BUFFER 1024
using namespace std;
//Analyze the command and send to server
void sendCommandToServer(CSocket&,string);
//Receive a message from FTP server
string receiveMessage(CSocket& sock);
//Extract code from server message
int getMessageCode(string);
//Login to server
void logIn(CSocket& sock);
//Get file from server
void getFile(CSocket& sock, CSocket& data, string filename);
//Upload a file to server
void uploadFile(CSocket& sock, CSocket& data, string filename);
//Create new folder
void createFolder(CSocket&, string);
//Delete a file in server
void deleteFile(CSocket&, string);
//Delete empty folder in server
void deleteEmptyFolder(CSocket&, string);
//Quit
void quit(CSocket&);
//Change server path
void changeServerPath(CSocket&, string);
//Change client path
void changeClientPath(string);
//Get parameter from a command
string getParameter(stringstream&, string);

