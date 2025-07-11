#include "../Constants.h"
#include "online.h"
#include "client.h"
#include <unistd.h>
#include <thread>

#define F first
#define S second

int Timer;                // Time since the start of game (excluding Goals)
POINT mouse; 			  // struct for declaring coordinates of mouse
int page=0;         	  // Double buffering
int result;

int turn;  				  // whose turn
double movement;		  // sum of speeds of ball + all strikers. If it is 0 that means all objects have stopped moving value of turn is swapped

void update_components(); // Updates everything before printing
void _print(int n=-1, double anglee = 0, double distance = 0); 	// Prints everything

struct Ball{
	static const int radius=20;
	int x,y;
}ball;

struct Striker{
	static const int number=5; 	 // # of strikers for each player
	static const int radius=40;  // Radius of striker
	int x[number],y[number]; 	 // x and y coords of striker[i]
	int color; 	  				 // Color of strikers
	double arc_angle=360.0; 	 // Arc angle is the constant timer around the striker, which is rorating and decreasing over time.

	void print(int who, int n, double anglee, double distance){
		setlinestyle(0,0,1);
		setcolor(15);
		setfillstyle(SOLID_FILL,color);
		for(int i=0;i<number;i++){
//			char num[2];
//			sprintf(num, "%d", i);
//			outtextxy(x[i],y[i],num);
			circle(x[i],y[i],radius);
			floodfill(x[i],y[i],15);
		}

		if(movement==0){
			if(turn==who){
		    	setlinestyle(0,0,5);
		    	setcolor(13);

		    	for(int i=0;i<number;i++)
					arc(x[i],y[i],0,arc_angle,radius+4);

				if(n != -1){
					anglee=rad(anglee); // converting angle into radian to be used in sin and cos functions
					setcolor(YELLOW);
					if(distance>radius){
						// YELLOW line is created if the mouse is at least outside the striker radius. It starts at the boundary of radius
						// YELLOW  line denoting the vector along which the striker will be projected, if the mouse is released form that position
						line(x[n]+round(radius*cos(anglee)),y[n]-round(radius*sin(anglee)),x[n]+round(distance*cos(anglee)),y[n]-round(distance*sin(anglee)));
						// Arrow head are the end of striker projected yellow line endpoint
						line(x[n]+round(distance*cos(anglee))-round(distance*cos(anglee+rad(30.0))/4),y[n]-round(distance*sin(anglee))+round(distance*sin(anglee+rad(30.0))/4),x[n]+round(distance*cos(anglee)),y[n]-round(distance*sin(anglee)));
						line(x[n]+round(distance*cos(anglee))-round(distance*cos(anglee-rad(30.0))/4),y[n]-round(distance*sin(anglee))+round(distance*sin(anglee-rad(30.0))/4),x[n]+round(distance*cos(anglee)),y[n]-round(distance*sin(anglee)));
						setlinestyle(1,0,2); // 2 pixel dashed line
						setcolor(WHITE);
						// Dashed WHITE line 1/3*distance extension from the end point of YELLOW line
						line(x[n]+round(4*distance*cos(anglee)/3),y[n]-round(4*distance*sin(anglee)/3),x[n]+round(distance*cos(anglee)),y[n]-round(distance*sin(anglee)));
						setcolor(7);
						setlinestyle(1,0,3);
						// Dashed GREY line 1/3*distnace extension from the centre of striker along the line joining the current coordiantes of
						// mouse
						line(x[n]-round(radius*cos(anglee)),y[n]+round(radius*sin(anglee)),x[n]-round((radius+distance/3)*cos(anglee)),y[n]+round((distance/3+radius)*sin(anglee)));
					}
					else // if the mouse coordinates are inside the striker_radius
						line(x[n],y[n],x[n]+round(distance*cos(anglee)),y[n]-round(distance*sin(anglee)));
				}
			}
		}
	}
	
}striker1,striker2;

struct Goal{
	int width, height;
	int goal_1, goal_2;  // goal counter for first and second player respectively
}goal;

void print_movable(int n, double anglee, double distance){
	striker1.print(1, n, anglee, distance);
	striker2.print(2, n, anglee, distance);
	setlinestyle(0,0,1);
	setcolor(15);
	circle(ball.x,ball.y,ball.radius);
	setfillstyle(SOLID_FILL,14);
	floodfill(ball.x,ball.y,15);
}

void text(){	//prints texts while game is running
	char g1[3]="00";
	char g2[3]="00";
	char c1[]="TIME LEFT",c2[]="PLAYER 1",c3[]="PLAYER 2",c4[]="Press Escape",c5[]="for Menu",c6[]="Press Escape",c7[]="for Menu";
		g1[1]=(char)(goal.goal_1%10+'0');
		g1[0]=(char)(goal.goal_1/10+'0');
		g2[1]=(char)(goal.goal_2%10+'0');
		g2[0]=(char)(goal.goal_2/10+'0');
	outtextxy((X1+X2)/2-108*Xratio-40*Xratio,30*Yratio,g1);
	outtextxy((X1+X2)/2+70*Xratio+45*Xratio,30*Yratio,g2);
	char t[4];
	sprintf(t,"%d",Timer);
	int tme=Timer,sz=0;
    while(tme>0){
		tme/=10;
		sz++;
	}
	sz=max(sz,1);
	outtextxy((X1+X2)/2-42*Xratio-42*Xratio,7*Yratio,c1);
	outtextxy((X1+X2)/2-10*sz*Xratio,38*Yratio,t);
	outtextxy((X1+X2)/2-304*Xratio-40*Xratio,28*Yratio,c2);
	outtextxy((X1+X2)/2+135*Xratio+45*Xratio,28*Yratio,c3);
	outtextxy(X,18*Yratio,c4);
	outtextxy(X,42*Yratio,c5);
	outtextxy(width-230*Xratio,18*Yratio,c6);
	outtextxy(width-150*Xratio,42*Yratio,c7);
}

void color(){
	setfillstyle(SOLID_FILL,2);
		floodfill(X1+2,Y1+2,BLACK);
		floodfill(X2-2,Y2-2,BLACK);
	setfillstyle(SOLID_FILL,9);
		floodfill(X1-10,(Y+height)/2,BLACK);
		floodfill((X1+X2)/2-10,(Y+height)/2,BLACK);
		floodfill((X1+X2)/2-100*Xratio,40*Yratio,BLACK);
	setfillstyle(SOLID_FILL,12);
		floodfill(X2+10,(Y+height)/2,BLACK);
		floodfill((X1+X2)/2+10,(Y+height)/2,BLACK);
		floodfill((X1+X2)/2+100*Xratio,40*Yratio,BLACK);
	setfillstyle(4,BLACK);	// bleachers
		floodfill(X1-2,Y1+2,BLACK);
		floodfill(X1-2,Y2-2,BLACK);
		floodfill(X2+2,Y1+2,BLACK);
		floodfill(X2+2,Y2-2,BLACK);
}

void print_field(){
	cleardevice();
	setcolor(BLACK);
	setlinestyle(0,0,3);
	//making stadium
		rectangle(X,Y,width,height);
		rectangle(X,(Y+height)/2-goal.height,X+goal.width,(Y+height)/2+goal.height);//making left goal
		rectangle(width-goal.width,(Y+height)/2-goal.height,width,(Y+height)/2+goal.height);//making right goal
		rectangle(X1,Y1,X2,Y2);//making playable field

	line((X1+X2)/2,Y1,(X1+X2)/2,Y2);//central line
	circle((X1+X2)/2,(Y1+Y2)/2,100);//making central circle

	int c = (X1+X2)/2;
	int x1 = c-355*Xratio, x2 = c-170*Xratio;
	int x3 = c-90*Xratio, x4 = c+90*Xratio;
	int x5 = c+170*Xratio, x6 = c+355*Xratio;
	rectangle(x1, 20*Yratio, x2, 60*Yratio);	// name square 1
	rectangle(x2, 10*Yratio, x3, 70*Yratio);	// score square 1
	rectangle(x3, 00*Yratio, x4, 80*Yratio);	// Time Left square
	rectangle(x4, 10*Yratio, x5, 70*Yratio);	// score square 2
	rectangle(x5, 20*Yratio, x6, 60*Yratio);	// name square 2
}

void _print(int n, double anglee, double distance){
	setactivepage(page);
	setvisualpage(1-page);
	print_field();
	color();
	text();
	print_movable(n, anglee, distance);
	page=1-page;
}

void settings(){
	setactivepage(page);
 	setvisualpage(page);
	cleardevice();
	setcolor(1);
	setlinestyle(0,0,5);
	rectangle(50,50,width-50,height-50);
	setfillstyle(SOLID_FILL,8);
	floodfill(width/2,height/2,1);
	char c1[]="Choose color for PLAYER 1:";
	settextstyle(3,HORIZ_DIR,7);
	outtextxy(width/4-100,100,c1); //prints text for player 1 to choose color
	char c2[]="Press 1 to choose";
	settextstyle(3,HORIZ_DIR,4);
	outtextxy(width/4-75,height-150,c2);
	char c3[]="Press 2 to choose";
	outtextxy(3*width/4-225,height-150,c3);
	setcolor(WHITE);
	setfillstyle(SOLID_FILL,1);//color choice 1
	circle(width/4+75,height/2+25,striker1.radius*2.5);//prints first circle
	floodfill(width/4+75,height/2+25,WHITE);//fills circle 1 with first color choice
	circle(3*width/4-75,height/2+25,striker1.radius*2.5);//prints second circle
	setfillstyle(SOLID_FILL,3);//color choice 2
	floodfill(3*width/4-75,height/2+25,WHITE);//fills circle 2 with second color choice
	setvisualpage(page);
	setactivepage(1-page);
	page=1-page;

	while(true){
		if(GetAsyncKeyState('1')){
			striker1.color=1; // assigns color choice 1 to striker 1
			break;
		}
		else if(GetAsyncKeyState('2')){
			striker1.color=3; // assigns color choice 2 to striker 1
			break;
		}
		delay(DELAY);
	}

	while(GetAsyncKeyState('1') || GetAsyncKeyState('2')) delay(DELAY); //to avoid misckicking of keys

	cleardevice();
	setcolor(1);
	setlinestyle(0,0,5);
	rectangle(50,50,width-50,height-50);//outer border
	setfillstyle(SOLID_FILL,8);
	floodfill(width/2,height/2,1);
	char c4[]="Choose color for PLAYER 2:";//prints text for player 2 to chose color
	settextstyle(3,HORIZ_DIR,7);
	outtextxy(width/4-100,100,c4);
	char c5[]="Press 1 to choose";
	settextstyle(3,HORIZ_DIR,4);
	outtextxy(width/4-75,height-150,c5);
	char c6[]="Press 2 to choose";
	outtextxy(3*width/4-225,height-150,c6);
	setcolor(WHITE);
	setfillstyle(SOLID_FILL,4);//color choice 1
	circle(width/4+75,height/2+25,striker1.radius*2.5);//prints first circle
	floodfill(width/4+75,height/2+25,WHITE);//fills circle 1 with first color choice
	circle(3*width/4-75,height/2+25,striker1.radius*2.5);//prints second cirlce
	setfillstyle(SOLID_FILL,5);//color choice 2
	floodfill(3*width/4-75,height/2+25,WHITE);//fills circle 2 with second color choice
	setvisualpage(page);
	setactivepage(1-page);
	page=1-page;

	while(true){
		if(GetAsyncKeyState('1')){
			striker2.color=4; // assigns color choice 1 to striker 2
			break;
		}
		else if(GetAsyncKeyState('2')){
			striker2.color=5; // assigns color choice 2 to striker 2
			break;
		}
		delay(DELAY);
	}
	while(GetAsyncKeyState('1') || GetAsyncKeyState('2')) delay(DELAY); //to avoid misckicking of keys
	
	settextstyle(4,HORIZ_DIR,3*Xratio);
	
	sendData({1});
}

void find_match(){
	cleardevice();
	char connected[] = "Connected to server!";
	settextstyle(3,HORIZ_DIR,5);
	outtextxy((X1+X2)/2-350*Xratio, (Y1+Y2)/2 - 200*Yratio, connected);
	settextstyle(3,HORIZ_DIR,7);
	char waiting[] = "Waiting for opponent...";
	outtextxy((X1+X2)/2-350*Xratio, (Y1+Y2)/2 - 50*Yratio, waiting);
	settextstyle(4,HORIZ_DIR,3*Xratio);

	if(receiveData().S[0] == "done?")
		cout << "Working" << endl;
}

void server_details(){
	setvisualpage(page);
	setactivepage(page);
	
	char ip[] = "Server ip: ";
	char port[] = "Port no:";
	char retry[] = "Unable to connect. Please retry";
	settextstyle(3,HORIZ_DIR,6);
	
	string server_ip;
	int port_no=-1;
	do{
		cleardevice();
		outtextxy((X1+X2)/2-350*Xratio, (Y1+Y2)/2 - 150*Yratio, ip);

		if(port_no != -1)
			outtextxy((X1+X2)/2-350*Xratio, (Y1+Y2)/2 + 150*Yratio, retry);

		int len = 0;
		char serv[21] = {0};
		while(!GetAsyncKeyState(VK_RETURN)){
			if(len <= 20){
				for(char c='0';c<='9';c++){
					if(GetAsyncKeyState(c)){
						serv[len++] = c;
						outtextxy((X1+X2)/2 - 90*Xratio, (Y1+Y2)/2 - 150*Yratio, serv);
					}
				}
				for(char c='A';c<='Z';c++){
					if(GetAsyncKeyState(c)){
						serv[len++] = c|32;
						outtextxy((X1+X2)/2 - 90*Xratio, (Y1+Y2)/2 - 150*Yratio, serv);
					}
				}
				if(GetAsyncKeyState(VK_OEM_PERIOD)){
					serv[len++] = '.';
					outtextxy((X1+X2)/2 - 90*Xratio, (Y1+Y2)/2 - 150*Yratio, serv);
				}
			}
			if(GetAsyncKeyState(VK_BACK)){
				cleardevice();
				port_no = -1;
				outtextxy((X1+X2)/2-350*Xratio, (Y1+Y2)/2 - 150*Yratio, ip);

				len=max(0,len-1);
				serv[len] = 0;
				outtextxy((X1+X2)/2 - 90*Xratio, (Y1+Y2)/2 - 150*Yratio, serv);
			}

			delay(DELAY*15);
		}
		while(GetAsyncKeyState(VK_RETURN)) delay(DELAY*10);

		outtextxy((X1+X2)/2-350*Xratio, (Y1+Y2)/2, port);
		
		len = 0;
		char portno[11] = {0};
		
		while(!GetAsyncKeyState(VK_RETURN)){
			if(len <= 10){
				for(char c='0';c<='9';c++){
					if(GetAsyncKeyState(c)){
						portno[len++] = c;
						outtextxy((X1+X2)/2 - 90*Xratio, (Y1+Y2)/2, portno);
					}
				}
			}
			if(GetAsyncKeyState(VK_BACK)){
				cleardevice();
				port_no = -1;
				outtextxy((X1+X2)/2-350*Xratio, (Y1+Y2)/2 - 150*Yratio, ip);
				outtextxy((X1+X2)/2-350*Xratio, (Y1+Y2)/2, port);

				len=max(0,len-1);
				portno[len] = 0;
				outtextxy((X1+X2)/2 - 90*Xratio, (Y1+Y2)/2 - 150*Yratio, serv);
				outtextxy((X1+X2)/2 - 90*Xratio, (Y1+Y2)/2, portno);
			}

			delay(DELAY*15);
		}
		while(GetAsyncKeyState(VK_RETURN)) delay(DELAY*10);

		server_ip = serv;
		try{
			port_no = stoi(portno);
		}
		catch(const exception& e){
			port_no = -2;
		}
		
		memset(serv, 0, 21);
		memset(portno, 0, 11);
	}
	while(connect_to_server(server_ip,port_no));
}

pair<vector<double>, vector<string>> extra;
bool goaling = false, fouling = false;
void update_components(){
	while(result == -1){
		extra = receiveData();

		vector<double> send_;
		for(string &req: extra.S){
			if(req == "mouse"){
				GetCursorPos(&mouse);
				send_.push_back(mouse.x);
				send_.push_back(mouse.y);
			}
			else if(req == "VK_LBUTTON")
				send_.push_back(GetAsyncKeyState(VK_LBUTTON) != 0);
			else if(req == "VK_RBUTTON")
				send_.push_back(GetAsyncKeyState(VK_RBUTTON) != 0);
			else if(req == "Goal")
				goaling = true;
			else if(req == "Foul")
				fouling = true;
			else if(req == "Win")
				result = 1;
			else if(req == "Lose")
				result = 2;
			else if(req == "Draw")
				result = 0;
		}

		if(!send_.empty())
			sendData(send_);

		if(extra.F.empty())
			continue;

		int i=0;
		Timer = extra.F[i++];
		turn = extra.F[i++];
		movement = extra.F[i++];

		ball.x = extra.F[i++];
		ball.y = extra.F[i++];

		striker1.arc_angle = extra.F[i++];
		for(int j=0;j<striker1.number;j++){
			striker1.x[j] = extra.F[i++];
			striker1.y[j] = extra.F[i++];
		}

		striker2.arc_angle = extra.F[i++];
		for(int j=0;j<striker2.number;j++){
			striker2.x[j] = extra.F[i++];
			striker2.y[j] = extra.F[i++];
		}

		goal.goal_1 = extra.F[i++];
		goal.goal_2 = extra.F[i++];

		extra.F.erase(extra.F.begin(), extra.F.begin() + i);
	}
}

void start(){
	result = -1;

	thread t(update_components);
	t.detach();
	while(result == -1){
		if(goaling){
			setvisualpage(1-page);
			readimagefile("resources/goal.jpg",(X1+X2)/2-150*Xratio,(Y1+Y2)/2-75*Yratio,(X1+X2)/2+150*Xratio,(Y1+Y2)/2+75*Yratio);
			sleep(2);
			goaling = false;
		}
		else if(fouling){
			setvisualpage(1-page);
			setcolor(13);
			settextstyle(0,0,5);
			char c[]="FOUL !!";
			outtextxy((X1+X2)/2-120*Xratio,(Y1+Y2)/2-50*Yratio,c);
			settextstyle(4,HORIZ_DIR,3*Xratio);
			sleep(2);
			fouling = false;
		}
		else if(extra.F.size() == 3){
			_print(extra.F[0], extra.F[1], extra.F[2]);
		}
		else
			_print();
			
//		delay(DELAY);
	}
	
	setvisualpage(0);
	setactivepage(0);
	if(result == 1){
		readimagefile("resources/you_win.jpg",(X1+X2)/2-300*Xratio,(Y1+Y2)/2-125*Yratio,(X1+X2)/2+300*Xratio,(Y1+Y2)/2+100*Yratio);
	}
	else if(result == 2){
		readimagefile("resources/you_lose.jpg",(X1+X2)/2-300*Xratio,(Y1+Y2)/2-125*Yratio,(X1+X2)/2+300*Xratio,(Y1+Y2)/2+100*Yratio);
	}
	else{
		readimagefile("resources/draw.jpg",(X1+X2)/2-250*Xratio,(Y1+Y2)/2-150*Yratio,(X1+X2)/2+350*Xratio,(Y1+Y2)/2+150*Yratio);
	}
	sleep(4);
}

void main_online(){
	goal.width = 90*Xratio;
	goal.height= 125*Yratio;
	
	X1=X+goal.width; 	 Y1=Y;
	X2=width-goal.width; Y2=height;
	
	server_details();
	find_match();
	settings();
	start();
	closeConnection();
}
