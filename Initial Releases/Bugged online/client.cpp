#include <winsock2.h>
#include <iostream>
#include <vector>
using namespace std;

const int MAX_LEN = 1024;
char receive[MAX_LEN];
SOCKET clientSocket;

int connect_to_server(string server_ip, int port_no){
    WSADATA wsaData;
    if(WSAStartup(MAKEWORD(2,2), &wsaData) != 0){
	    cout << "WSAStartup failed\n";
	    return 1;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(clientSocket == INVALID_SOCKET){
    	cout << "ERROR opening socket\n";
        WSACleanup();
        return 2;
    }
    
	if(server_ip == "localhost")
		server_ip = "127.0.0.1";
		
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(server_ip.c_str());
    serverAddr.sin_port = htons(port_no);

    if(connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR){
        cout << "ERROR connecting\n";
        closesocket(clientSocket);
        WSACleanup();
        return 3;
    }
    
    return 0;
}

void sendData(vector<double> send_){
	string message;
	for(double &f: send_)
		message += to_string(f) + ' ';
	
	send(clientSocket, message.c_str(), message.size(), 0);
}

vector<double> receiveData(){
	memset(receive, 0, MAX_LEN);
	vector<double> Data;
    if(recv(clientSocket, receive, MAX_LEN, 0) > 0){
		string data;
		int n = strlen(receive);
    	for(int i=0;i<n;i++){
    		if(receive[i] == ' '){
    			Data.push_back(stod(data));
    			data.clear();
			}
    		else
				data.push_back(receive[i]);
		}
	}
	return Data;
}

void closeConnection(){
	sendData({-4});
    closesocket(clientSocket);
    WSACleanup();
}
