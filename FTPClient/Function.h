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
//Receive a message from FTP server
string receiveMessage(CSocket& sock);
//Extract code from server message
int getMessageCode(string);
//Login to server
void logIn(CSocket& sock);
//Get file from server
bool getFile(CSocket&,CSocket&, string,bool);
//Upload a file to server
bool uploadFile(CSocket&,CSocket&, string,bool);
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
//Connect to data port
bool connectDataPort(CSocket& sock,CSocket& ClientData, bool passive);
//Create new folder
void createFolder(CSocket&, string);
//Delete a file in server
void deleteFile(CSocket&, string);
//Delete empty folder in server
void deleteEmptyFolder(CSocket&, string);
//Listing file and folder in server
bool getFileList(CSocket&, CSocket&,string, bool, vector<string>&);
//Listing file and folder detail in server
string getFileListDetail(CSocket&, CSocket&,string, bool);
//Analyze the ls command and return list of files
vector<string> analyzeFileList(string);
//Delete multiple files
void deleteMultipleFiles(CSocket&, CSocket&,CSocket&, stringstream&, bool);
//Delete space at begin and end of the command
void deleteSpaces(string&);
//Download multiple files
void getMultipleFiles(CSocket&, CSocket&,CSocket&, stringstream&, bool);
//Upload multiple files
void uploadMultipleFiles(CSocket&, CSocket&, CSocket&, stringstream&, bool);
//Get file list in a folder in client
vector<string> getFileListClient(string);