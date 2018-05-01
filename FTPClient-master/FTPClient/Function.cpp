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
	istringstream is(message);
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
		username = "user " + username + "\n";
		sock.Send((char*)username.c_str(), username.length());
		cout << receiveMessage(sock);
		cout << "-->Password: ";
		getline(cin, pass);
		pass = "pass " + pass + "\n";
		sock.Send((char*)pass.c_str(), pass.length());
		string message = receiveMessage(sock);
		cout << message;
		code = getMessageCode(message);
	} while (code != 230);
}
void uploadFile(CSocket &sock) {
	string stringSend;
	string PASV = "PASV\n";
	if (AfxSocketInit() == false) {
		cout << "Socket initialization failed" << endl;
		return;
	}
	CSocket ClientData;
	ClientData.Create();

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

	const char* msg = (const char*)IP.c_str();
	wchar_t *wmsg = new wchar_t[strlen(msg) + 1];
	mbstowcs(wmsg, msg, strlen(msg) + 1);

	cout << port;
	do {
		//cout << "-->FTP server: ";
		//getline(cin, ftpServer);
		if (ClientData.Connect(_T("127.0.0.1"), port) == 0) {
			cout << "Can't connect to FTP server" << endl;
		}
		else {
			cout << "Successfully connect to FTP server" << endl;
			cout << receiveMessage(ClientData);
			connected = true;
		}
	} while (!connected);
	do {
		string fileName;
		cin >> fileName;
		stringSend = "RETR " + fileName + "\n";
		ClientData.Send((char*)stringSend.c_str(), stringSend.length());
		code = getMessageCode(message);
		cout << code;
		cout << receiveMessage(ClientData);
	} while (code == 503);
}