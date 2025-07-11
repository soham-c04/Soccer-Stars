#include "../Constants.h"
#include "online.h"
#include "client.cpp"

#define F first
#define S second

int Timer;                // Time since the start of game (excluding Goals)
POINT mouse; 			  // struct for declaring coordinates of mouse
int page=0;         	  // Double buffering

int my_turn;              // my_turn (striker no.) w.r.t server
int turn;  				  // whose turn
double movement;		  // sum of speeds of ball + all strikers. If it is 0 that means all objects have stopped moving value of turn is swapped

void update_components(); // Updates everything before printing
void _print(int n=-1); 	  // Prints everything

struct Ball{
	const int radius=20;
	int x,y;
}ball;

struct Striker{
	static const int number=5; 	 // # of strikers for each player
	static const int radius=40;  // Radius of striker
	int x[number],y[number]; 	 // x and y coords of striker[i]
	int color; 	  				 // Color of strikers
	double arc_angle=360.0; 	 // Arc angle is the constant timer around the striker, which is rorating and decreasing over time.

	void print(int who,int n){
		setlinestyle(0,0,1);
		setcolor(15);
		setfillstyle(SOLID_FILL,color);
		for(int i=0;i<number;i++){
			circle(x[i],y[i],radius);
			floodfill(x[i],y[i],15);
		}

		if(movement==0){
			if(turn==who){
		    	setlinestyle(0,0,5); // 5 pixel thick line arc around the strikers
		    	setcolor(13);

		    	for(int i=0;i<number;i++) arc(x[i],y[i],0,arc_angle,radius+4);

		    	// The size of this arc decreases with time. The length of arc denotes the time the player has to shoot the ball.
				arc_angle-=(360.0*DELAY/(timeout*ms))*2; // Extra factor of 2 to account for processing delays

				// Mouse pullback
				int releasepos_x, releasepos_y;
				double anglee, distance;

				if(turn == 2){
					vector<double> receive = receiveData();
					n = receive[0];
					if(n == -1) return;

					anglee = receive[1], distance = receive[2];
					if(receive.size() == 4){    // Opponent turn over
						angle[n] = anglee;
						movement = speed[n] = receive[3];
						turn = 1;
					}
				}

				if(n!=-1){ // Here n denotes the striker from whose center the mouse was pulled to shoot
	    			if(turn == 1){
						GetCursorPos(&mouse); // Gets the current location/position of the mouse.
						releasepos_x=mouse.x,releasepos_y=mouse.y;

						anglee=deg(atan2(releasepos_y-y[n],x[n]-releasepos_x)); // anglee is the angle which the pointer of mouse is making with the
						// center of striker from whom the left button is pressed. Slope formula is used
						distance=round(sqrt(pow(releasepos_x-x[n],2)+pow(releasepos_y-y[n],2))); // distance of centre of striker to mouse position
						// this distance is also in ratio to the speed of the striker.
						distance=min(distance,max_speed*20.0); // Since the distance is in ratio to the speed of striker and the speed of striker
						// has a maximum limit hence the distance also has a maximum limit.
						distance/=2; // random scaling factor for appropriate visuals of line

						sendData({n, 180-anglee, distance});
					}

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
					else{ // if the mouse coordinates are inside the striker_radius
						line(x[n],y[n],x[n]+round(distance*cos(anglee)),y[n]-round(distance*sin(anglee)));
					}
				}
				else
					sendData({-1});
			}
		}
	}

	void control(){
		update_components();
		_print();

		vector<string> receive;

		int n = -1;
		while(n == -1){
			while(arc_angle > 0){
				update_components();
				_print();
				receive = receiveData().S;
				if(receive[0] == "VK_LBUTTON"){
					double vkl = GetAsyncKeyState(VK_LBUTTON)
					sendData({vkl});
					if(vkl == 1)
						break;
				}
			}

			if(arc_angle <= 0)
				break;

			receive = receiveData().S;
			if(receive[0] == "mouse"){
				GetCursorPos(&mouse);
				sendData({mouse.x,mouse.y});
			}

			for(int i=0;i<number;i++){
				if((x[i]-mouse.F)*(x[i]-mouse.S)+(y[i]-mouse.F)*(y[i]-mouse.S)<=radius*radius){
					n=i;
					break;
				}
			}

			if(n!=-1){
				while(arc_angle > 0){
					update_components();
					receive = receiveData().S;
					_print(n);
					if(receive[0] == "VK_LBUTTON" && receive[1] == "VK_RBUTTON"){
						double vkl = GetAsyncKeyState(VK_LBUTTON);
						double vkr = GetAsyncKeyState(VK_RBUTTON);
						sendData({vkl,vkr});
						if(vkl == 0)
							break;
						if(vkr == 1){
							n = -1;
							break;
						}
					}
				}
			}
			else{
				while(arc_angle > 0){ // to accomodate wrong selected position of object
					update_components();
					_print();
					receive = receiveData().S;
					if(receive[0] == "VK_LBUTTON"){
						double vkl = GetAsyncKeyState(VK_LBUTTON);
						sendData({vkl});
						if(vkl == 0)
							break;
					}
				}
			}
		}

		if(n!=-1 && arc_angle>0){
			GetCursorPos(&mouse);

			double anglee=deg(atan2(mouse.y-y[n],x[n]-mouse.x)); // angle of striker projection to be determined from slope

			// distance of mouse release point from the centre of chosen striker
			double distance=(arc_angle>0)? ((sqrt(pow(mouse.x-x[n],2)+pow(mouse.y-y[n],2)))):0;
			// if arc_angle is less than 0, speed_striker will be 0 due to time runout
			angle[n]=anglee;
			speed[n]=distance/10.0; 				  // set striker speed accordingly
			speed[n]=min(speed[n],max_speed); 		  // limit the max. striker speed
			if(speed[n]) movement=speed[n];           // Set to non-zero movement

			sendData({n, 180-angle[n], distance, speed[n]});
		}
		else
			sendData({0, 0, 0, 0});
	}

}striker1,striker2;

struct Goal{
	int width, height;
	int goal_1, goal_2;  // goal counter for first and second player respectively
}goal;

void update_components(){
	vector<double> receive = receiveData.F;
	int i=0;

	Timer = receive[i++];
	turn = receive[i++];
	movement = receive[i++];

	ball.x = receive[i++];
	ball.y = receive[i++];

	striker1.arc_angle = receive[i++];
	for(int j=0;j<striker1.number;j++,i++){
		striker1.x[j] = receive[i++];
		striker1.y[j] = receive[i++];
	}

	striker2.arc_angle = receive[i++];
	for(int j=0;j<striker2.number;j++,i++){
		striker2.x[j] = receive[i++];
		striker2.y[j] = receive[i++];
	}

	goal.goal_1 = receive[i++];
	goal.goal_2 = receive[i++];
}

void print_movable(int n){
	striker1.print(1,n);
	striker2.print(2,n);
	setlinestyle(0,0,1);
	setcolor(15);
	circle(ball.x,ball.y,ball.radius);
	setfillstyle(SOLID_FILL,14);
	floodfill(ball.x,ball.y,15);
}

void settings(){
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

	while(GetAsyncKeyState('1') || GetAsyncKeyState('2')) delay(DELAY);//to avoid misckicking of keys

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
	settextstyle(4,HORIZ_DIR,3*Xratio);
}

void text(){	//prints texts while game is running
	char g1[3]="00";
	char g2[3]="00";
	char c1[]="TIME",c2[]="PLAYER 1",c3[]="PLAYER 2",c4[]="Press Escape",c5[]="for Menu",c6[]="Press Escape",c7[]="for Menu";
		g1[1]=(char)(goal.goal_1%10+'0');
		g1[0]=(char)(goal.goal_1/10+'0');
		g2[1]=(char)(goal.goal_2%10+'0');
		g2[0]=(char)(goal.goal_2/10+'0');
	outtextxy((X1+X2)/2-108*Xratio,30*Yratio,g1);
	outtextxy((X1+X2)/2+70*Xratio,30*Yratio,g2);
	char t[4];
		sprintf(t,"%d",Timer);
		int tme=Timer,sz=0;
	    while(tme>0){
			tme/=10;
			sz++;
		}
		sz=max(sz,1);
		outtextxy((X1+X2)/2-42*Xratio,5*Yratio,c1);
		outtextxy((X1+X2)/2-10*sz*Xratio,38*Yratio,t);
		outtextxy((X1+X2)/2-304*Xratio,28*Yratio,c2);
		outtextxy((X1+X2)/2+135*Xratio,28*Yratio,c3);
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

	rectangle((X1+X2)/2-50*Xratio,0,(X1+X2)/2+50*Xratio,80*Yratio);//time square
	rectangle(((X1+X2)/2-50*Xratio)-80*Xratio,10,(X1+X2)/2-50*Xratio,70*Yratio);//score square 1
	rectangle(((X1+X2)/2+50*Xratio)+80*Xratio,10,(X1+X2)/2+50*Xratio,70*Yratio);//score square 2
	rectangle(((X1+X2)/2-50*Xratio)-265*Xratio,20,(X1+X2)/2-130*Xratio,60*Yratio);//name square 1
	rectangle(((X1+X2)/2+50*Xratio)+265*Xratio,20,(X1+X2)/2+130*Xratio,60*Yratio);//name square 2
}

void _print(int n){
	setactivepage(page);
	setvisualpage(1-page);
	print_field();
	color();
	text();
	print_movable(n);
	page=1-page;
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

	turn = receiveData()[0];
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
		outtextxy((X1+X2)/2-350*Xratio, (Y1+Y2)/2, port);

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
				outtextxy((X1+X2)/2-350*Xratio, (Y1+Y2)/2, port);

				len=max(0,len-1);
				serv[len] = 0;
				outtextxy((X1+X2)/2 - 90*Xratio, (Y1+Y2)/2 - 150*Yratio, serv);
			}

			delay(DELAY*15);
		}
		while(GetAsyncKeyState(VK_RETURN)) delay(DELAY*10);

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
		port_no = stoi(portno);

		memset(serv, 0, 21);
		memset(portno, 0, 11);
	}
	while(connect_to_server(server_ip,port_no));
}

void start(){
	my_turn = receiveData().F[0];
	turn = 1;

	time(&ti);

	while(true){
		update_components();
		_print();

		if(movement==0){
			if(turn == my_turn){
				if(turn == 1)
					striker1.control();
				else
					striker2.control();
			}
		}
	}
}

void main_online(){
	goal.width = 90*Xratio;
	goal.height= 125*Yratio;

	X1=X+goal.width; 	 Y1=Y;
	X2=width-goal.width; Y2=height;
	Full_Reset();

	server_details();
	find_match();
	settings();
	start();
	closeConnection();
}
