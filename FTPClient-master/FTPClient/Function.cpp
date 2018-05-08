#include "Function.h"
string receiveMessage(CSocket& sock) {
	vector<char> buffer(MAX_BUFFER);
	int byteReceived = 0;
	string res;
	do {
		byteReceived = sock.Receive(&buffer[0], MAX_BUFFER, 0);
		if (byteReceived == -1) {
			cout << "Error when receiving server data" << endl;
			return "Error";
		}
		else {
			res.append(buffer.begin(), buffer.begin() + byteReceived);
		}
	} while (byteReceived == MAX_BUFFER);
	return res;
}

int getMessageCode(string message) {
	stringstream is(message);
	string code;
	is >> code;
	return stoi(code);
}

void sendCommandToServer(CSocket& sock, string s) {
	stringstream is(s);
	string command;
	is >> command;
	for (int i = 0; i < command.length(); i++) {
		command[i] = tolower(command[i]);
	}
	if (command == "pwd") {
		command += "\r\n";
		sock.Send((char*)command.c_str(), command.length());
		cout << receiveMessage(sock);
	}
	else if (command == "cd") {
		command = getParameter(is, "cd");
		changeServerPath(sock, command);
	}
	else if (command == "lcd") {
		command = getParameter(is, "lcd");
		changeClientPath(command);
	}
	else if (command == "get") {
		command = getParameter(is, "get");
		uploadFile(sock, command);
	}
	else if (command == "delete") {
		command = getParameter(is, "delete");
		deleteFile(sock, command);
	}
	else if (command == "mkdir") {
		command = getParameter(is, "mkdir");
		createFolder(sock, command);
	}
	else if (command == "rmdir") {
		command = getParameter(is, "rmdir");
		deleteEmptyFolder(sock, command);
	}
	else if (command == "quit" || command == "exit") {
		string stringSend = "QUIT\n";
		sock.Send((char*)stringSend.c_str(), stringSend.length());
		cout << receiveMessage(sock);
	}

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


void getFile(CSocket& sock, CSocket& data, string filename) {
	/*string PASV = "PASV\n";
	
	CSocket data;
	data.Create();

	int code = 0;
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
	bool connected = false;

	if (data.Connect(_T("127.0.0.1"), port) == 0) {
		cout << "Can't connect to FTP server" << endl;
	}
	else {
		cout << "Successfully connect to FTP server" << endl;
		connected = true;
	}*/
	string stringSend = "RETR " + filename + "\n";
	sock.Send((char*)stringSend.c_str(), stringSend.length());

	//Open file
	ofstream f;
	f.open(filename,ios::out|ios::app|ios::binary);
	//Data tranfer
	char buffer[MAX_BUFFER];
	int receivedLen = 0;
	do {
		receivedLen = data.Receive(buffer, MAX_BUFFER, 0);
		if (receivedLen == -1) {
			cout << "Error when receiving server data" << endl;
			break;
		}
		else {
			f.write(buffer, receivedLen);
		}
	} while (receivedLen>0);
	//f.close();
	data.Close();
	cout << receiveMessage(sock);
}

void uploadFile(CSocket& sock, CSocket& data, string filename) {
	string stringSend = "STOR " + filename + "\n";
	sock.Send((char*)stringSend.c_str(), stringSend.length());
	//Open file
	ifstream is;
	is.open(filename, ios::in | ios::app | ios::binary);
	is.seekg(0, is.end);
	int lengthFile = is.tellg();
	is.seekg(0, is.beg);
	//Data tranfer
	char* buffer = new char[lengthFile];
	int sendLen = 0;
	is.read(buffer, lengthFile);
	sendLen = data.Send(buffer, lengthFile, 0);
	if (sendLen == -1) {
		cout << "Error when sending server data" << endl;
	}
	cout << receiveMessage(sock);
	//close
	is.close();
	data.Close();
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