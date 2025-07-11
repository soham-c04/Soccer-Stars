#include <iostream>
#include <vector>
using namespace std;

int connect_to_server(string server_ip, int port_no);
void sendData(vector<double> send_);
pair<vector<double>,vector<string>> receiveData();
void closeConnection();
