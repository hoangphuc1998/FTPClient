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

bool getFile(CSocket& sock,CSocket& ClientData, string filename,bool passive) {
	string stringSend = "RETR " + filename + "\n";
	sock.Send((char*)stringSend.c_str(), stringSend.length());
	string command = receiveMessage(sock);
	cout << command;
	CSocket activeSock;
	bool success = false;
	if (getMessageCode(command) == 150) {
		if (!passive) {
			ClientData.Accept(activeSock);
		}
		//Open file
		ofstream f;
		int pos = filename.find("/");
		filename = filename.substr(pos + 1);
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
		success = true;
	}
	if (!passive)activeSock.Close();
	ClientData.Close();
	return success;
}

void changeServerPath(CSocket& sock, string path) {
	while (path == "") {
		cout << "Remote directory: ";
		getline(cin, path);
		deleteSpaces(path);
	}
	string command = "cwd " + path + "\r\n";
	sock.Send((char*)command.c_str(), command.length());
	cout << receiveMessage(sock);
}

void changeClientPath(string path) {
	while (path == "") {
		cout << "Local directory: ";
		getline(cin, path);
		deleteSpaces(path);
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
	deleteSpaces(temp);
	if (temp == type) command = "";
	else is >> command;
	return command;
}

//Connect to data port
bool connectDataPort(CSocket& sock,CSocket& ClientData, bool passive) {
	if (passive) {
		ClientData.Create();
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

		if (ClientData.Connect(wmsg, port) == 0) {
			cout << "Can't connect to FTP server" << endl;
			return false;
		}
		//else {
		//	cout << "Successfully connect to FTP server" << endl;
		//}
	}
	else {
		ClientData.Create();
		ClientData.Listen();
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

bool uploadFile(CSocket& sock, CSocket &data, string filename,bool passive) {
	while (filename == "") {
		cout << "Upload file: ";
		getline(cin, filename);
		deleteSpaces(filename);
	}
	//Open file
	ifstream is;
	is.open(filename,ios::binary | ios::in);
	bool success = false;
	if (is.good()) {
		is.seekg(0, is.end);
		int lengthFile = is.tellg();
		is.seekg(0, is.beg);
		string stringSend = "STOR " + filename + "\n";
		sock.Send((char*)stringSend.c_str(), stringSend.length());
		CSocket activeSock;
		if (!passive)data.Accept(activeSock);
		cout << receiveMessage(sock);

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
		data.Close();
		delete[]buffer;
		cout << receiveMessage(sock);
		success = true;
	}
	else {
		cout<<"There is no file name "<<filename<<endl;
	}
	return success;
}

void deleteFile(CSocket& sock, string filename) {
	while (filename == "") {
		cout << "Upload file: ";
		getline(cin, filename);
		deleteSpaces(filename);
	}
	string stringSend = "DELE " + filename + "\n";
	sock.Send((char*)stringSend.c_str(), stringSend.length());
	cout << receiveMessage(sock);
}

void createFolder(CSocket& sock, string folderName) {
	while (folderName == "") {
		cout << "New folder: ";
		getline(cin, folderName);
		deleteSpaces(folderName);
	}
	string stringSend = "MKD " + folderName + "\n";
	sock.Send((char*)stringSend.c_str(), stringSend.length());
	cout << receiveMessage(sock);
}

void deleteEmptyFolder(CSocket& sock, string folderName) {
	while (folderName == "") {
		cout << "Delete empty folder: ";
		getline(cin, folderName);
		deleteSpaces(folderName);
	}
	string stringSend = "RMD " + folderName + "\n";
	sock.Send((char*)stringSend.c_str(), stringSend.length());
	cout << receiveMessage(sock);
}

string getFileListDetail(CSocket& sock, CSocket& ClientData,string folderName, bool passive) {
	string stringSend = "list "+ folderName +"\r\n";
	sock.Send((char*)stringSend.c_str(), stringSend.length());
	string command = receiveMessage(sock);
	cout << command;
	int code = getMessageCode(command);
	string res = "";
	if (code == 150) {
		CSocket activeSock;
		//Get data from server
		if (!passive) {
			ClientData.Accept(activeSock);
		}
		
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
				res.append(buffer);
			}
		} while (receivedLen > 0);
		res = res.substr(res.length() / 2);
		while (res.length()>0 && res[res.length() - 1] != '\n') res.pop_back();
		cout << receiveMessage(sock);
		if (!passive)activeSock.Close();
		ClientData.Close();
	}
	if (res == "0\r")res = "";
	return res;
}

bool getFileList(CSocket& sock, CSocket& ClientData, string folderName, bool passive, vector<string>& result) {
	string stringSend = "nlst " + folderName + "\r\n";
	sock.Send((char*)stringSend.c_str(), stringSend.length());
	string command = receiveMessage(sock);
	//cout << command;
	int code = getMessageCode(command);
	string res = "";
	if (code == 150) {
		CSocket activeSock;
		//Get data from server
		if (!passive) {
			ClientData.Accept(activeSock);
		}

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
				res.append(buffer);
			}
		} while (receivedLen > 0);
		res = res.substr(res.length() / 2);
		while (res.length()>0 && res[res.length() - 1] != '\n') res.pop_back();
		cout << receiveMessage(sock);
		if (!passive)activeSock.Close();
		ClientData.Close();
	}
	if (res == "0")res = "";
	string temp = "";
	for (int i = 0; i < res.length(); i++) {
		if (res[i] == '\r') {
			i++;
			result.push_back(temp);
			temp = "";
		}
		else {
			temp += res[i];
		}
	}
	if (result.size() > 0) {
		if (result[0] == folderName) {
			return false;
		}
	}
	return true;;
}

void deleteMultipleFiles(CSocket& sock, CSocket& ClientData, CSocket& activeSock, stringstream& is, bool passive) {
	string fullCommand = is.str();
	if (fullCommand == "mdelete") {
		string temp = "";
		while (temp == "") {
			cout << "Files and folders to delete: " << endl;
			getline(cin, temp);
			is = stringstream(temp);
		}
	}
	string name;
	while (!is.eof()) {
		is >> name;
		cout << "Type y to delete file" << endl;
		if (name != "") {
			bool isFolder;
			vector<string>fileList;
			if (passive) {
				connectDataPort(sock, ClientData, passive);
				isFolder = getFileList(sock, ClientData, name, passive,fileList);
			}
			else {
				connectDataPort(sock, activeSock, passive);
				isFolder = getFileList(sock, activeSock, name, passive,fileList);
			}
			if (isFolder) {
				for (int i = 0; i < fileList.size(); i++) {
					cout << "Delete " << fileList[i] << " Y/N?";
					string answer;
					getline(cin, answer);
					deleteSpaces(answer);
					if (answer != "n" && answer != "N") {
						deleteFile(sock, fileList[i]);
					}
				}
			}
			else {
				cout << "Delete " << name << " Y/N?";
				string answer;
				getline(cin, answer);
				deleteSpaces(answer);
				if (answer != "n" && answer != "N") {
					deleteFile(sock, name);
				}
			}
		}
	}
}

void deleteSpaces(string& s) {
	while (s[s.length() - 1] == ' ')s.pop_back();
	while (s[0] == ' ')s.erase(s.begin(), s.begin() + 1);
}

void getMultipleFiles(CSocket& sock, CSocket& ClientData,CSocket& activeSock, stringstream& is, bool passive) {
	string fullCommand = is.str();
	if (fullCommand == "mget") {
		string temp = "";
		while (temp == "") {
			cout << "Files and folders to download: " << endl;
			getline(cin, temp);
			is = stringstream(temp);
		}
	}
	string name;
	while (!is.eof()) {
		is >> name;
		cout << "Type y to download file" << endl;
		if (name != "") {
			bool isFolder;
			vector<string>fileList;
			if (passive) {
				connectDataPort(sock, ClientData, passive);
				isFolder = getFileList(sock, ClientData, name, passive,fileList);
			}
			else {
				connectDataPort(sock, activeSock, passive);
				isFolder = getFileList(sock, activeSock, name, passive,fileList);
			}
			if (isFolder) {
				for (int i = 0; i < fileList.size(); i++) {
					if (fileList[i] != name) {
						cout << "Download " << fileList[i] << " Y/N?";
						string answer;
						getline(cin, answer);
						deleteSpaces(answer);
						if (answer != "n" && answer != "N") {
							if (passive) {
								connectDataPort(sock, ClientData, passive);
								getFile(sock, ClientData, fileList[i], passive);
							}
							else {
								connectDataPort(sock, activeSock, passive);
								getFile(sock, activeSock, fileList[i], passive);
							}
						}
					}
				}
			}
			else{
				cout << "Download " << name << " Y/N?";
				string answer;
				getline(cin, answer);
				deleteSpaces(answer);
				if (answer != "n" && answer != "N") {
					if (passive) {
						connectDataPort(sock, ClientData, passive);
						getFile(sock, ClientData, name, passive);
					}
					else {
						connectDataPort(sock, activeSock, passive);
						getFile(sock, activeSock, name, passive);
					}
				}
			}
		}
	}
}

void uploadMultipleFiles(CSocket& sock, CSocket& ClientData, CSocket& activeSock, stringstream& is, bool passive) {
	string fullCommand = is.str();
	if (fullCommand == "mput") {
		string temp = "";
		while (temp == "") {
			cout << "Files and folders to upload: " << endl;
			getline(cin, temp);
			is = stringstream(temp);
		}
	}
	string name;
	while (!is.eof()) {
		is >> name;
		cout << "Type y to upload file" << endl;
		if (name != "") {
			bool isFolder;
			vector<string>fileList;
			isFolder = getFileListClient(name, fileList);
			if (isFolder) {
				SetCurrentDirectoryA(name.c_str());
				for (int i = 0; i < fileList.size(); i++) {
					cout << "Upload " <<name<<"\\"<< fileList[i] << " Y/N?";
					string answer;
					getline(cin, answer);
					deleteSpaces(answer);
					if (answer != "n" && answer != "N") {
						if (passive) {
							connectDataPort(sock, ClientData, passive);
							uploadFile(sock, ClientData, fileList[i], passive);
						}
						else {
							connectDataPort(sock, activeSock, passive);
							uploadFile(sock, activeSock, fileList[i], passive);
						}
					}
				}
				SetCurrentDirectoryA("..");
			}
			else {
				cout << "Upload " << name << " Y/N?";
				string answer;
				getline(cin, answer);
				deleteSpaces(answer);
				if (answer != "n" && answer != "N") {
					if (passive) {
						connectDataPort(sock, ClientData, passive);
						uploadFile(sock, ClientData, name, passive);
					}
					else {
						connectDataPort(sock, activeSock, passive);
						uploadFile(sock, activeSock, name, passive);
					}
				}
			}
		}
	}
}

bool getFileListClient(string folderName,vector<string>& res) {
	bool isFolder = true;
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(folderName.c_str())) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL) {
			res.push_back(string(ent->d_name));
		}
		closedir(dir);
		if (res.size() > 0) {
			res.erase(res.begin());
			res.erase(res.begin());
		}
	}
	else {
		isFolder = false;
	}
	return isFolder;
}