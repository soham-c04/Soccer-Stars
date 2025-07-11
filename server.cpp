#include "Extensions/Constants.cpp"
#include <iostream>
#include <vector>
#include <map>
#include <unistd.h>
#include <thread>
using namespace std;

#define F first
#define S second
#define DELAY 15

const int MAX_LEN = 1e3;
SOCKET serverSocket;
map<SOCKET,int> opponent;

// --- online START ---

struct Components;
struct Goal;

void Time(Components &comp);
void sendData(int sock, string message){
	message += '$';
	send(sock, message.c_str(), message.size(), 0);
}

void sendData(Components &comp, vector<double> extra = {});
vector<double> requestData(SOCKET client, vector<string> req){
	string message = "req ";
	for(string &s: req)
		message += s + ' ';
	sendData(client, message);
	
	char receive[MAX_LEN] = {0};
	vector<double> Data;
	if(recv(client, receive, MAX_LEN, 0) > 0){
		string data;
		int n = strlen(receive);
			
    	for(int i=0;i<n;i++){
    		if(receive[i] == ' '){
				try{
    				Data.push_back(stod(data));
				}
				catch(const exception& e){
					cout << receive << "receive" << endl;
					cout << data << "Data" << endl;
				}
    			data.clear();
			}
    		else
				data.push_back(receive[i]);
		}
	}
	return Data;
}

class Ball{
	private:
		static constexpr double loss=8.0;
	public:
		static constexpr double mass=1.0;
		static const int radius=20;   	 
		int x,y; 			  	  
		double speed=0.0;      	  
		double angle=90.0; 	   	  
		
		void reset(){ 		   
			speed=0.0;		   
			x=(X2+X1)/2;
			y=(Y2+Y1)/2;       
			angle=90.0;        
		}
		
		string convert(){
			string s;
			s += to_string(x) + ' ';
			s += to_string(y) + ' ';
			return s;
		}

		void move(Components &comp, bool Goaling=false);
		void boundary(Components &comp);
};

class Striker{
	private:
		static const int number=5; 	 
		static const int timeout=5;  
		static constexpr double mass=6.0;       
		const double max_speed=15.0;
		static constexpr double loss=6.0;       
		int x[number],y[number]; 	
		double speed[number]; 		
		double angle[number];
		
	public:
		static const int radius=40; 		 
		double arc_angle=360.0;
		SOCKET client;

		void reset(int who){
			arc_angle=360.0;
			for(int i=0;i<number;i++) speed[i]=0.0;
			
			if(who==1){
				x[0]=X1+radius+10;
				x[1]=x[2]=X1+(X2-X1)/5;
				x[3]=x[4]=X1+2*(X2-X1)/5;
			}
			else{
				x[0]=X2-radius-10;
				x[1]=x[2]=X2-(X2-X1)/5;
				x[3]=x[4]=X2-2*(X2-X1)/5;
			}
			
			y[0]=(Y1+Y2)/2;
			y[1]=Y1+(Y2-Y1)/4;
			y[2]=Y1+(Y2-Y1)*3/4;
			y[3]=(Y2+Y1)/2-2*radius;
			y[4]=(Y2+Y1)/2+2*radius;
		}
		
		string convert(){
			string s = to_string(arc_angle) + ' ';
			for(int i=0;i<number;i++){
				s += to_string(x[i]) + ' ';
				s += to_string(y[i]) + ' ';
			}
			return s;
		}
		
		void control(Components &comp);

		void boundary(){ // Striker rebound on field edges
			for(int i=0;i<number;i++){
				if(x[i]<=(radius+X1) || x[i]+radius>=X2){
					x[i]=min(x[i],X2-radius); // Ensures that striker stays within the left and right boundaries
					x[i]=max(x[i],radius+X1);
					angle[i]=fmod(fmod(180.0-angle[i],360.0)+360.0,360.0); // Change the direction when collision with left and right boundaries
				}
				if(y[i]<=(radius+Y1) || y[i]+radius>=Y2){ // Change the direction of striker when collision with upper and lower boundaries
					y[i]=min(y[i],Y2-radius); // Ensures that striker stays within the upper and lower boundaries
					y[i]=max(y[i],radius+Y1);
					angle[i]=fmod(fmod(360.0-angle[i],360.0)+360.0,360.0); // Change the direction of striker when collision with upper and lower boundaries
				}
			}
		}
		
		void move(double &movement){
			for(int i=0;i<number;i++){
				int incx=round(speed[i]*cos(rad(angle[i])));
				int incy=round(speed[i]*sin(rad(angle[i])));
				x[i]+=incx;
				y[i]-=incy;

				speed[i]-=loss*DELAY/ms; // Frictional speed loss of striker per S
				speed[i]=max(speed[i],0.0);
				movement+=speed[i];

				boundary();
			}
		}
		
		bool Collision(Ball &ball, int collisions){ 
			bool collide=false; 

			for(int i=0;i<number;i++){
				int incxb=round(ball.speed*cos(rad(ball.angle))),incyb=round(ball.speed*sin(rad(ball.angle)));
				int incxs=round(speed[i]*cos(rad(angle[i]))),incys=round(speed[i]*sin(rad(angle[i])));
				if((pow((x[i]+incxs)-(ball.x+incxb),2)+pow((y[i]-incys)-(ball.y-incyb),2))<=(radius+ball.radius)*(radius+ball.radius)){
					collide=true;
					double theta=deg(atan2(ball.y-y[i],x[i]-ball.x));
					double vbi_parallel=ball.speed*cos(rad(ball.angle-theta)),vb_perp=ball.speed*sin(rad(ball.angle-theta));
					
					double vsi_parallel=speed[i]*cos(rad(angle[i]-theta)),vs_perp=speed[i]*sin(rad(angle[i]-theta));
					
					double vbf_parallel=((1+e)*mass*vsi_parallel+vbi_parallel*(ball.mass-e*mass))/(ball.mass+mass);
					
					double vsf_parallel=((1+e)*ball.mass*vbi_parallel+vsi_parallel*(mass-e*ball.mass))/(ball.mass+mass);
					
					ball.angle=theta+deg(atan2(vb_perp,vbf_parallel)); 
					angle[i]=theta+deg(atan2(vs_perp,vsf_parallel)); 
					ball.speed=sqrt(pow(vbf_parallel,2)+pow(vb_perp,2)); 
					speed[i]=sqrt(pow(vsf_parallel,2)+pow(vs_perp,2)); 

					if(collisions>MAX_COL){
						speed[(i+1)%number]+=ball.speed;
						speed[(i+2)%number]+=speed[i];
						ball.speed=speed[i]=0;
					}
				}

				for(int j=i+1;j<number;j++){
					int incxi=incxs,incyi=incys;
					int incxj=round(speed[j]*cos(rad(angle[j]))),incyj=round(speed[j]*sin(rad(angle[j])));
					if(pow((x[i]+incxi)-(x[j]+incxj),2)+pow((y[i]-incyi)-(y[j]-incyj),2)<=4*radius*radius){
						collide=true;

						double theta=deg(atan2(y[j]-y[i],x[i]-x[j]));

						double vji_parallel=speed[j]*cos(rad(angle[j]-theta)),vj_perp=speed[j]*sin(rad(angle[j]-theta));
						double vii_parallel=speed[i]*cos(rad(angle[i]-theta)),vi_perp=speed[i]*sin(rad(angle[i]-theta));
						double vjf_parallel=((1+e)*vii_parallel+vji_parallel*(1-e))/2;
						double vif_parallel=((1+e)*vji_parallel+vii_parallel*(1-e))/2;
						angle[j]=theta+deg(atan2(vj_perp,vjf_parallel));
						angle[i]=theta+deg(atan2(vi_perp,vif_parallel));
						speed[j]=sqrt(pow(vjf_parallel,2)+pow(vj_perp,2));
						speed[i]=sqrt(pow(vif_parallel,2)+pow(vi_perp,2));

						if(collisions>MAX_COL){
							if(j-i>2){
								speed[i+1]+=speed[i];
								speed[i+2]+=speed[j];
							}
							else{
								speed[(j+1)%number]+=speed[i];
								speed[(j+2)%number]+=speed[j];
							}
							speed[i]=speed[j]=0;
						}
					}
				}
			}
			return collide;
		}

		bool Collision(Striker &other, int collisions){
			bool collide=false;

			for(int i=0;i<number;i++){    
				int incxs=round(speed[i]*cos(rad(angle[i]))),incys=round(speed[i]*sin(rad(angle[i])));
				for(int j=0;j<other.number;j++){ 
					int incxi=incxs,incyi=incys;
					int incxj=round(other.speed[j]*cos(rad(other.angle[j]))),incyj=round(other.speed[j]*sin(rad(other.angle[j])));
					if(pow((x[i]+incxi)-(other.x[j]+incxj),2)+pow((y[i]-incyi)-(other.y[j]-incyj),2)<=4*radius*other.radius){
						collide=true;

						double theta=deg(atan2(other.y[j]-y[i],x[i]-other.x[j]));

						double vji_parallel=other.speed[j]*cos(rad(other.angle[j]-theta)),vj_perp=other.speed[j]*sin(rad(other.angle[j]-theta));
						double vii_parallel=speed[i]*cos(rad(angle[i]-theta)),vi_perp=speed[i]*sin(rad(angle[i]-theta));
						double vjf_parallel=((1+e)*vii_parallel+vji_parallel*(1-e))/2;
						double vif_parallel=((1+e)*vji_parallel+vii_parallel*(1-e))/2;
						other.angle[j]=theta+deg(atan2(vj_perp,vjf_parallel));
						angle[i]=theta+deg(atan2(vi_perp,vif_parallel));
						other.speed[j]=sqrt(pow(vjf_parallel,2)+pow(vj_perp,2));
						speed[i]=sqrt(pow(vif_parallel,2)+pow(vi_perp,2));

						if(collisions > MAX_COL){
							speed[(i+1)%number]+=speed[i];
							other.speed[(j+1)%number]+=other.speed[j];
							speed[i]=other.speed[j]=0;
						}
					}
				}
			}
			return collide;
		}
};

struct Goal{
	int width, height;
	int goal_1=0, goal_2=0;
	
	string convert(){
		string s = to_string(goal_1) + ' ';
		s += to_string(goal_2) + ' ';
		return s;
	}
	
	void goaling(Components &comp, int who);
};

struct Components{  // Easier to pass by reference
	int Timer = 0;
	time_t ti, tf;
	Ball ball;
	Striker striker1, striker2;
	Goal goal;
	int turn = 1;
	double movement = 0;
	int moves = 0;
	int collisions = 0;

	void reset(){
		ball.reset();
		striker1.reset(1);
		striker2.reset(2);
		moves=0;
		collisions=0;
	}

	void Full_Reset(){
		Timer=90;
		reset();
		turn = 1;
		goal.goal_1=0;
		goal.goal_2=0;
	}
};

void Time(Components &comp){
	time(&(comp.tf));
	if(comp.ti!=comp.tf){
		comp.ti=comp.tf;
		comp.Timer--;
	}
}

// --- Striker ---

void Striker::control(Components &comp){
	int n=-1;

	arc_angle=360.0;
	sendData(comp);

	vector<double> receive;
	pair<int,int> mouse;

	while(n == -1){
		while(arc_angle > 0){
			if(comp.Timer < 0) return;
			receive = requestData(client, {"VK_LBUTTON"});
			if(receive[0] == 1)
				break;
			Time(comp);
			arc_angle-=(360.0*DELAY/(timeout*ms));
			sendData(comp);
			delay(DELAY);
		}
		if(arc_angle <= 0.0)
			break;

		receive = requestData(client, {"mouse"});
		mouse = {receive[0],receive[1]};

		for(int i=0;i<number;i++){
			if((x[i]-mouse.F)*(x[i]-mouse.F)+(y[i]-mouse.S)*(y[i]-mouse.S)<=radius*radius){
				n=i;
				break;
			}
		}

		if(n!=-1){
			while(arc_angle > 0){
				if(comp.Timer < 0) return;
				receive = requestData(client, {"VK_LBUTTON", "VK_RBUTTON","mouse"});
				if(receive[0] == 0)
					break;
				if(receive[1] == 1){
					n = -1;
					break;
				}
				Time(comp);
				arc_angle-=(360.0*DELAY/(timeout*ms));
				mouse = {receive[2],receive[3]};

				double anglee=deg(atan2(mouse.S-y[n],x[n]-mouse.F));
				double distance=sqrt(pow(mouse.F-x[n],2)+pow(mouse.S-y[n],2));
				distance=min(distance, max_speed*10.0);

				sendData(comp,{n,anglee,distance});

				delay(DELAY);
			}
		}
		else{
			while(arc_angle > 0){
				if(comp.Timer < 0) return;
				Time(comp);
				arc_angle-=(360.0*DELAY/(timeout*ms));
				sendData(comp);
				receive = requestData(client, {"VK_LBUTTON"});
				if(receive[0] == 0)
					break;
				delay(DELAY);
			}
		}

		delay(DELAY);
	}

	if(n!=-1 && arc_angle>0){
		receive = requestData(client, {"mouse"});
		mouse = {receive[0],receive[1]};

		double anglee=deg(atan2(mouse.S-y[n],x[n]-mouse.F));

		double distance=(arc_angle>0)? ((sqrt(pow(mouse.F-x[n],2)+pow(mouse.S-y[n],2)))):0;
		distance=min(distance, max_speed*10.0);

		angle[n]=anglee;
		speed[n]=distance/10.0;
		if(speed[n]) comp.movement=speed[n];

		sendData(comp,{n,anglee,distance});
	}
	else
		sendData(comp);
}

// ---- Goal ----

void Goal::goaling(Components &comp, int who){
	comp.turn=who;
	comp.ball.speed=3.0;
	if(who==1){
		if(comp.moves>1) goal_2++;
		while(comp.ball.x>X1-2*width/3 && (comp.ball.y>=(Y2+Y1)/2-height+comp.ball.radius) && (comp.ball.y<=(Y2+Y1)/2+height-comp.ball.radius)){
			comp.ball.move(comp, true);
			comp.striker1.move(comp.movement);
			comp.striker2.move(comp.movement);
			sendData(comp);
			delay(DELAY);
		}
	}
	else{
		if(comp.moves>1) goal_1++;
		while(comp.ball.x<X2+2*width/3 && (comp.ball.y>=(Y2+Y1)/2-height+comp.ball.radius) && (comp.ball.y<=(Y2+Y1)/2+height-comp.ball.radius)){
			comp.ball.move(comp, true);
			comp.striker1.move(comp.movement);
			comp.striker2.move(comp.movement);
			sendData(comp);
			delay(DELAY);
		}
	}

	if(comp.moves>1){
		sendData(comp.striker1.client, "Goal ");
		sendData(comp.striker2.client, "Goal ");
	}
	else{
		sendData(comp.striker1.client, "Foul ");
		sendData(comp.striker2.client, "Foul ");
	}

	comp.reset();
	
	sleep(2);
}

// ---- Ball ----

void Ball::move(Components &comp, bool Goaling){
	int incx=round(speed*cos(rad(angle))); 
	int incy=round(speed*sin(rad(angle))); 
	x+=incx; 
	y-=incy; 
	if(!Goaling){
		if(y<((Y2+Y1)/2-comp.goal.height+radius) || y>((Y2+Y1)/2+comp.goal.height-radius)){
			x=max(x,X1+radius);
			x=min(x,X2-radius);
		}
		y=max(y,Y1+radius);
		y=min(y,Y2-radius);

		speed-=loss*DELAY/ms; // Frictional speed loss of ball per frame
		speed=max(speed,0.0);
		comp.movement=speed;

		boundary(comp);
	}
}

void Ball::boundary(Components &comp){
	if(x<=(radius+X1) && y>=((Y2+Y1)/2-comp.goal.height+radius) && y<=((Y2+Y1)/2+comp.goal.height-radius)){
		comp.goal.goaling(comp,1);
		return;
	}
	else if((x+radius)>=X2 && y>=((Y2+Y1)/2-comp.goal.height+radius) && y<=((Y2+Y1)/2+comp.goal.height-radius)){
		comp.goal.goaling(comp,2);
		return;
	}
	if(x<=(radius+X1) || (x+radius)>=X2) angle=fmod(fmod(180.0-angle,360.0)+360.0,360.0);
	if(y<=(radius+Y1) || (y+radius)>=Y2) angle=fmod(fmod(360.0-angle,360.0)+360.0,360.0);
}

// --- Ball END ---

bool check_collision(Components &comp){
	bool s1 = comp.striker1.Collision(comp.ball, comp.collisions);
	bool s2 = comp.striker2.Collision(comp.ball, comp.collisions);
	bool s12= comp.striker1.Collision(comp.striker2, comp.collisions);
	return s1 || s2 || s12;
}

void start(Components &comp){
	time(&(comp.ti));

	while(comp.Timer >= 0){
		if(check_collision(comp)){
			comp.collisions++;
			continue;
		}
		comp.collisions=0;
		Time(comp);
		sendData(comp);

		if(comp.movement==0){
			comp.moves++;
			if(comp.turn == 1)
				comp.striker1.control(comp);
			else
				comp.striker2.control(comp);
				
			comp.turn = 3 - comp.turn;
		}

		comp.ball.move(comp);
		comp.striker1.move(comp.movement);
		comp.striker2.move(comp.movement);

		delay(DELAY);
	}
	
	if(comp.goal.goal_1 > comp.goal.goal_2){
		sendData(comp.striker1.client, "Win ");
		sendData(comp.striker2.client, "Lose ");
	}
	else if(comp.goal.goal_1 < comp.goal.goal_2){
		sendData(comp.striker1.client, "Lose ");
		sendData(comp.striker2.client, "Win ");
	}
	else{
		sendData(comp.striker1.client, "Draw ");
		sendData(comp.striker2.client, "Draw ");
	}
}

void online(Components &comp){
	comp.goal.width = 90*Xratio;
	comp.goal.height= 125*Yratio;

	X1=X+comp.goal.width; 	 Y1=Y;
	X2=width-comp.goal.width; Y2=height;

	comp.Full_Reset();
	
	start(comp);
}

// ---- online END ----

void sendData(Components &comp, vector<double> extra){
	string message;
	message += to_string(comp.Timer) + ' ';
	message += to_string(comp.turn) + ' ';
	message += to_string(comp.movement) + ' ';
	message += comp.ball.convert();
	message += comp.striker1.convert();
	message += comp.striker2.convert();
	message += comp.goal.convert();
	
	for(double &f: extra)
		message += to_string(f) + ' ';

	sendData(comp.striker1.client, message);
	sendData(comp.striker2.client, message);
}

void Clients(SOCKET clientSocket){
    char receive[MAX_LEN];
    while(opponent[clientSocket] == -1){
    	for(auto oppo: opponent){
			if(oppo.F != clientSocket && oppo.S == -1){
				opponent[clientSocket] = oppo.F;
				opponent[oppo.F] = clientSocket;
			}
		}
	}
	
	int opp = opponent[clientSocket];
	
	Components comp;
	if(clientSocket < opp){
		requestData(clientSocket, {"done?"});
		requestData(opp, {"done?"});
		
		comp.striker1.client = clientSocket;
		comp.striker2.client = opp;
		online(comp);
		
		opponent.erase(clientSocket);
		opponent.erase(opp);

		closesocket(clientSocket);
		closesocket(opp);
	}
	
//	else{
//		while(opponent[clientSocket] != -1){
//			sleep(1);
//		}
//	}
//
//    while(true){
//    	memset(receive, 0, MAX_LEN);
//
//	    if(recv(clientSocket, receive, MAX_LEN, 0) > 0){
//	    	if(strcmp(receive, "-4 ") == 0){
//	    		cout << endl << "Client:" << clientSocket << " disconnected" << endl;
//	    		opponent.erase(clientSocket);
//				break;
//			}
//			else if(strcmp(receive, "-3 ") == 0){
//	    		cout << endl << "Client: " << clientSocket << " unpaired from Client: " << opp << endl;
//	    		opponent[clientSocket] = opponent[opp] = -1;
//			}
//			else if(strcmp(receive, "-2 ") == 0){
//				while(opponent[clientSocket] == -1){
//			    	for(auto oppo: opponent){
//						if(oppo.F != clientSocket && oppo.S == -1){
//							opponent[clientSocket] = oppo.F;
//							opponent[oppo.F] = clientSocket;
//						}
//					}
//				}
//				
//				opp = opponent[clientSocket];
//				
//				if(clientSocket < opp){
//					requestData(clientSocket, {"done?"});
//					requestData(opp, {"done?"});
//
//					comp.striker1.client = clientSocket;
//					comp.striker2.client = opp;
//					online(comp);
//				}
//				else{
//					while(opponent[clientSocket] != -1){
//						sleep(1);
//					}
//				}
//			}
//		}
//	}
//
//	closesocket(clientSocket);
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
	srand(time(0));
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

