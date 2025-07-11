#include <winsock2.h>
#include <algorithm>
#include "client.h"

const int MAX_LEN = 1e7;
char receive[MAX_LEN] = {0};
SOCKET clientSocket;
vector<string> keywords = {"Goal","Foul","Win","Lose","Draw"};

void sendData(vector<double> send_){
	string message;
	for(double &f: send_)
		message += to_string(f) + ' ';
	
	send(clientSocket, message.c_str(), message.size(), 0);
}

pair<vector<double>,vector<string>> receiveData(){  // vector<string> is when server requests for something
	vector<double> Data;
	vector<string> req;
    if(recv(clientSocket, receive, MAX_LEN, 0) > 0){
		string data;
		int n = strlen(receive);
//		cout << "\nreceive: " << receive << endl;
		int i=0;
		while(i<n){
			data.clear();
	    	for(;receive[i] != ' ';i++)
				data.push_back(receive[i]);
			i++;

			if(data == "req"){
				data.clear();

				for(;receive[i] != '$';i++){
		    		if(receive[i] == ' '){
		    			req.push_back(data);
		    			data.clear();
					}
		    		else
						data.push_back(receive[i]);
				}
				i++;
			}
			else if(find(keywords.begin(), keywords.end(), data) != keywords.end()){
				i+=data.size() + 2;
				req.push_back(data);
			}
			else{
				Data.clear();
				Data.push_back(stod(data));
				data.clear();
				for(;receive[i] != '$';i++){
		    		if(receive[i] == ' '){
		    			try{
		    				Data.push_back(stod(data));
						}
						catch(const exception& e){
							cout << receive <<"receive" << endl;
							cout << data <<"Data" <<endl;
						}
		    			data.clear();
					}
		    		else
						data.push_back(receive[i]);
				}
				i++;
			}
		}
		
		memset(receive, 0, n);
	}
	return {Data,req};
}

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
	else if(server_ip == "home")
		server_ip = "192.168.29.253";

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

void closeConnection(){
//	sendData({-4});
    closesocket(clientSocket);
    WSACleanup();
}
