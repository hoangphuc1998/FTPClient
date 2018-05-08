#include "Function.h"
string receiveMessage(CSocket& sock) {
	char buffer;
	int byteReceived = 0;
	string res;
	do {
		byteReceived = sock.Receive(&buffer, 1, 0);
		if (byteReceived == -1) {
			//cout << "Error when receiving server data" << endl;
			return "Error";
		}
		else {
			res += buffer;
		}
	} while (buffer!='\n');
	return res;
}

int getMessageCode(string message) {
	stringstream is(message);
	string code;
	is >> code;
	return stoi(code);
}

void logIn(CSocket& sock) {
	string username, pass;
	int code = 0;
	do {
		cout << "-->Username: ";
		getline(cin, username);
		username = "user " + username + "\r\n";
		sock.Send((char*)username.c_str(), username.length());
		cout << receiveMessage(sock);
		cout << "-->Password: ";
		getline(cin, pass);
		pass = "pass " + pass + "\r\n";
		sock.Send((char*)pass.c_str(), pass.length());
		string message = receiveMessage(sock);
		cout << message;
		code = getMessageCode(message);
	} while (code != 230);
}

void getFile(CSocket& sock,CSocket& ClientData, string filename,bool passive) {
	string stringSend = "RETR " + filename + "\n";
	sock.Send((char*)stringSend.c_str(), stringSend.length());
	string command = receiveMessage(sock);
	cout << command;
	CSocket activeSock;
	if (getMessageCode(command) != 550) {
		if (!passive) {
			int res = ClientData.Accept(activeSock);
			cout << res << endl;
		}
		//Open file
		ofstream f;
		f.open(filename, ios::out | ios::binary);
		//Data tranfer
		char buffer[MAX_BUFFER];
		int receivedLen = 0;
		do {
			if (passive) {
				receivedLen = ClientData.Receive(buffer, MAX_BUFFER, 0);
			}
			else {
				receivedLen = activeSock.Receive(buffer, MAX_BUFFER, 0);
			}
			if (receivedLen == -1) {
				cout << "Error when receiving server data" << endl;
				break;
			}
			else {
				f.write(buffer, receivedLen);
			}
		} while (receivedLen > 0);
		f.close();
		cout << receiveMessage(sock);
	}
	if (!passive)activeSock.Close();
}

void changeServerPath(CSocket& sock, string path) {
	while (path == "") {
		cout << "Remote directory: ";
		getline(cin, path);
		while (path[path.length() - 1] == ' ')path.pop_back();
		while (path[0] == ' ')path.erase(path.begin(), path.begin() + 1);
	}
	string command = "cwd " + path + "\r\n";
	sock.Send((char*)command.c_str(), command.length());
	cout << receiveMessage(sock);
}

void changeClientPath(string path) {
	while (path == "") {
		cout << "Local directory: ";
		getline(cin, path);
		while (path[path.length() - 1] == ' ')path.pop_back();
		while (path[0] == ' ')path.erase(path.begin(), path.begin() + 1);
	}
	int res = SetCurrentDirectoryA(path.c_str());
	if (res != 0) {
		char buf[256];
		GetCurrentDirectoryA(256, buf);
		cout << "Successfully change directory" << endl;
		cout << "Current dir is " + string(buf) << endl;
	}
	else {
		cout << "The system cannot find the path specified" << endl;
	}
}

string getParameter(stringstream& is, string type) {
	string temp = is.str();
	string command;
	while (temp[temp.length() - 1] == ' ')temp.pop_back();
	while (temp[0] == ' ')temp.erase(temp.begin(), temp.begin() + 1);
	if (temp == "cd") command = "";
	else is >> command;
	return command;
}

//Connect to data port
bool connectDataPort(CSocket& sock,CSocket& ClientData, bool passive) {
	if (passive) {
		string stringSend;
		string PASV = "PASV\n";
		
		sock.Send((char*)PASV.c_str(), PASV.length());
		string message = receiveMessage(sock);
		//String return if succeed: "227 Entering Passive Mode (a1,a2,a3,a4,p1,p2)" 
		//N = p1 * 256 + p2 IP: a1.a2.a3.a4

		cout << message;
		string IP;
		int port;

		//Xử lý tách chuỗi a1 a2 ....
		int pos = message.find('(');
		string c = message.substr(pos);
		string b(c.begin() + 1, c.end() - 1);
		vector<string> tokens;
		stringstream k(b);

		string temp;

		while (getline(k, temp, ',')) {
			tokens.push_back(temp);
		}

		IP = tokens[0] + '.' + tokens[1] + '.' + tokens[2] + '.' + tokens[3];
		port = stoi(tokens[4]) * 256 + stoi(tokens[5]);
		

		const char* msg = (const char*)IP.c_str();
		wchar_t *wmsg = new wchar_t[strlen(msg) + 1];
		mbstowcs(wmsg, msg, strlen(msg) + 1);

		if (ClientData.Connect(_T("127.0.0.1"), port) == 0) {
			cout << "Can't connect to FTP server" << endl;
			return false;
		}
		else {
			cout << "Successfully connect to FTP server" << endl;
		}
	}
	else {
		sockaddr_in add;
		int addlen = sizeof(add);
		sock.GetSockName((sockaddr*)&add, &addlen);
		
		string portCommand;
		//Create ip address string
		int a = add.sin_addr.S_un.S_un_b.s_b1;
		portCommand += to_string(a) + ",";
		a = add.sin_addr.S_un.S_un_b.s_b2;
		portCommand += to_string(a) + ",";
		a = add.sin_addr.S_un.S_un_b.s_b3;
		portCommand += to_string(a) + ",";
		a = add.sin_addr.S_un.S_un_b.s_b4;
		portCommand += to_string(a) + ",";
		//Append port to command
		ClientData.GetSockName((sockaddr*)&add, &addlen);
		int port = ntohs(add.sin_port);
		portCommand += to_string(port / 256) + ",";
		portCommand += to_string(port % 256);
		portCommand = "port " + portCommand + "\r\n";
		sock.Send(portCommand.c_str(), portCommand.length());
		string res= receiveMessage(sock);
		cout << res;
	}
	return true;
}

void uploadFile(CSocket& sock, CSocket &data, string filename,bool passive) {
	string stringSend = "STOR " + filename + "\n";
	sock.Send((char*)stringSend.c_str(), stringSend.length());
	CSocket activeSock;
	if (!passive)data.Accept(activeSock);
	cout << receiveMessage(sock);
	//Open file
	ifstream is;
	is.open(filename, ios::in | ios::app | ios::binary);
	is.seekg(0, is.end);
	int lengthFile = is.tellg();
	is.seekg(0, is.beg);
	//Data tranfer
	char* buffer = new char[lengthFile];
	int receivedLen = 0;
	is.read(buffer, lengthFile);
	if (passive) {
		receivedLen = data.Send(buffer, lengthFile, 0);
	}
	else {
		receivedLen = activeSock.Send(buffer, lengthFile, 0);
	}
	if (receivedLen == -1) {
		cout << "Error when sending server data" << endl;
	}
	//close
	is.close();
	if (!passive)activeSock.Close();
	delete[]buffer;
	cout << receiveMessage(sock);
}

void deleteFile(CSocket& sock, string fileName) {
	string stringSend = "DELE " + fileName + "\n";
	sock.Send((char*)stringSend.c_str(), stringSend.length());
	cout << receiveMessage(sock);
}

void createFolder(CSocket& sock, string folderName) {
	string stringSend = "MKD " + folderName + "\n";
	sock.Send((char*)stringSend.c_str(), stringSend.length());
	cout << receiveMessage(sock);
}

void deleteEmptyFolder(CSocket& sock, string folderName) {
	string stringSend = "RMD " + folderName + "\n";
	sock.Send((char*)stringSend.c_str(), stringSend.length());
	cout << receiveMessage(sock);
}