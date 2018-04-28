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