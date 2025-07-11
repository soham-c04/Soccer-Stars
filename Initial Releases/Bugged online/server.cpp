#include <graphics.h>
#include <winsock2.h>
#include <iostream>
#include <map>
#include <thread>
#include <chrono>
using namespace std;

const int MAX_LEN = 1024;
SOCKET serverSocket;
map<SOCKET,int> opponent;

void Clients(SOCKET clientSocket){
    char receive[MAX_LEN];
    while(opponent[clientSocket] == -1){
    	for(auto oppo: opponent){
			if(oppo.first != clientSocket && oppo.second == -1){
				opponent[clientSocket] = oppo.first;
				opponent[oppo.first] = clientSocket;
			}
		}
	}
	int opp = opponent[clientSocket];
	string turn = ((clientSocket < opp)? "1 ":"2 ");
	send(clientSocket, turn.c_str(), turn.size(), 0);
	
    while(true){
    	memset(receive, 0, MAX_LEN);
    	
	    if(recv(clientSocket, receive, MAX_LEN, 0) > 0){
	    	if(strcmp(receive, "-4 ") == 0){
	    		cout << endl << "Client:" << clientSocket << " disconnected" << endl;
	    		opponent.erase(clientSocket);
				break;
			}
			else if(strcmp(receive, "-3 ") == 0){
	    		cout << endl << "Client: " << clientSocket << " unpaired from Client: " << opp << endl;
	    		opponent[clientSocket] = opponent[opp] = -1;
			}
			else if(strcmp(receive, "-2 ") == 0){
				while(opponent[clientSocket] == -1){
			    	for(auto oppo: opponent){
						if(oppo.first != clientSocket && oppo.second == -1){
							opponent[clientSocket] = oppo.first;
							opponent[oppo.first] = clientSocket;
						}
					}
				}
				opp = opponent[clientSocket];

				string turn = ((clientSocket < opp)? "1":"2");
				send(clientSocket, turn.c_str(), turn.size(), 0);
			}
			else
				send(opp, receive, MAX_LEN, 0);
		}
	}
	
	closesocket(clientSocket);
}

int startServer(){
    WSADATA wsaData;
    if(WSAStartup(MAKEWORD(2,2), &wsaData) != 0){
        cout << "WSAStartup failed\n";
        return 1;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSocket == INVALID_SOCKET){
        cout << "ERROR opening socket\n";
        WSACleanup();
        return 2;
    }

	int port_no;
	cout<<"Enter the port no. for server: ";
	cin>>port_no;

	struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port_no);

    if(bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR){
        cout << "ERROR on binding\n";
        closesocket(serverSocket);
        WSACleanup();
        return 3;
    }
    
    return 0;
}

int main(){
	int err;
	if(err = startServer())
		return err;
	
	auto start_time=chrono::high_resolution_clock::now();
	auto end_time=chrono::high_resolution_clock::now();
	int timeout = 120; // 120 sec timeout for server
	
	while(chrono::duration_cast<chrono::seconds>(end_time - start_time).count() < timeout){
		end_time=chrono::high_resolution_clock::now();
		if(listen(serverSocket, 5) == SOCKET_ERROR){
	        cout << "ERROR on listen\n";
	        closesocket(serverSocket);
	        WSACleanup();
	        return 4;
	    }

		struct sockaddr_in clientAddr;
		int clilen = sizeof(clientAddr);
	    SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clilen);
	    if (clientSocket == INVALID_SOCKET){
	        cout << "ERROR on accept\n";
	        continue;
		}
	    
	    if(opponent.find(clientSocket) == opponent.end()){
	    	start_time=chrono::high_resolution_clock::now();    // Restart timer
			opponent[clientSocket] = -1;
			cout << endl << "Client connected!\nSocket: " << clientSocket << endl;
	        thread t(Clients, clientSocket);
	        t.detach();
		}
	}
	
	cout << endl << "Server closed due to Timeout: " << timeout << "seconds" << endl;

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}

