#pragma once
#include <afxsock.h>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include "FTPClient.h"
#define MAX_BUFFER 1024
using namespace std;
//Receive a message from FTP server
string receiveMessage(CSocket& sock);
//Extract code from server message
int getMessageCode(string);
//Login to server
void logIn(CSocket& sock);