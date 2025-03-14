#include <graphics.h>
#include <conio.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>

const double pi=2*acos(0.0);
double rad(int degree){
	double ans=pi*degree/180.0;
	return ans;
}
double deg(double radi){
	double ans=180.0*radi/pi;
	return ans;
}

#define min(x,y) ((x)>(y))? (y):(x)
#define max(x,y) ((x)>(y))? (x):(y)
#define ms 1000
#define Xratio X_full/1366 // For Adjusting to size of objects in X direction according to resolution of Screen
#define Yratio Y_full/768 // For Adjusting to size of objects in Y direction according to resolution of Screen
#define X1ratio 1
#define Y1ratio 1
// For Adjusting to size of objects in circular direction according to resolution of Screen
#define number 5 // number is the number of strikers for each player
// it is set as define and not in const int or int because other while declaring x[2*number] array C can't store it
/*
Since this game was build in my PC. Hence I adjusted the X and Y resolution of Screen According to my PC so that the
relative sizes of all objects in any resolution are the same.
*/
int X; // Gets the X resolution of Screen
int Y; // Gets the Y resolution of Screen;
int X_full,Y_full; // Stores the X and Y orignial resolution of screen used in Xratio and Yratio
int X0,Y0; // Defines the starting corrdinates of playable field
int radius,radius_ball; // Radius of radius of striker and radius_ball is the radius of ball
int goal_height,goal_width; // Defines the size of goal
int goal1=0,goal2=0,goal=0,tme=0,page=0;
int run_game=1; // breaks out of game or quits the game on selecting quit option;
/*
goal1 and goal 2 and goals scored by left and right respectively.
goal is variable which is set to 1 when a goal occurs. This is done so as to prevent boundary collisions in x
when goal occurs. As a result ball can travel inside the goal
tme keeps track of  current time after the start of game
page refers to the visual and active pages used in double buffering
*/
double max_speed;
double angle[2*number]; // angle array stores the angle currently travelling in  of the all the 2*number strikers in the field
// Here the first $number of element represent the angles of the strikers of player 1(left side) and the next $number elements
// are the angles of player 2
double ball_angle=0.0; // This is angle of ball
// Angle is measured in anticlockwise diredction from the +x x-axis. Assuming the bottom left of our laptop to be origin.
// This is not the same as graph origin which is at the upper_left corner of laptop.

double speed_ball=0.0; // spped_ball is the speed of ball.
double striker_speed[2*number]; // striker_speed array stores the striker speeds of the all the 2*number strikers in the field
// Here the first $number of element represent the speeds of the strikers of player 1(left side) and the next $number elements
// are the striker_speeds of player 2
int x[2*number],y[2*number]; // same as angle and striker_speed array but this time stores coordinates of strikers in the array.
int ballx,bally; // Stores the x and y coordinates of ball respectively.
double arc_angle=360.0; // Arc angle the constant timer around the striker which is rorating and decreasing over time. When it 
// reaches 0 that means that the timer for the player is over and reaches a timeout! Turn is passed to next player.
double movement=0; // It is a variable signifying the sum of speeds of all the objects in the game. IT is the sum of all ball speeds
// and all the striker_speeds. If it is 0 that means all objects have stopped moving and the next player gets the chance to strike.
int moves=0; // Moves is the number of moves played in the game after a goal is done. If it is 1 that is, if the goal is scored in
// the first shot then it will be a foul.
double Mb=1.0,Ms=7.0; // This is the relative masses of balls and strikers to be used in collision.
double e=0.7; //Coefficient of restitution
int turn=-1; // turn means which player will shoot when movement becomes 0. turn=0 means player1's turn and turn=1 means player2's
int collisions=0; 
/*
Collisions is a debuggging variale which stores the number of collisions occuring between any 2 objects after a
the ball has been shot. Which is reset to 0 after each time movement becomes 0. Collisions are used in debugging tangential
collisions bcecause the game gets stuck there due to infinitely many collisions occuring. If the collision count reaches>30
that means tangential collisions are occuring the objects causing tangential collisions are abruptly set to 0 to prevent 
stoppage of game. This works because in almost all other cases collisions are less than 30 in a single shot.
*/
time_t ti,tf; // ti represents the previously stored time of system when the time changed and tf is the current time of system.
int color_1,color_2; // For the color of striker chosen in settings
int i; // to be used in for loops.
POINT mouse; // struct for declaring coordinates of mouse

void color()
{
	setfillstyle(SOLID_FILL,9);
		floodfill((X0+X)/2-100*Xratio,40*Yratio,BLACK); // left scorecard
	setfillstyle(SOLID_FILL,12);
		floodfill((X0+X)/2+100*Xratio,40*Yratio,BLACK); // right scorecard
	setbkcolor(7);
	setfillstyle(4,BLACK);//bleachers
		floodfill(X0-2,Y0+2,BLACK);
		floodfill(X0-2,Y-2,BLACK);
		floodfill(X+2,Y0+2,BLACK);
		floodfill(X+2,Y-2,BLACK);
	setbkcolor(0);
}
void text()//prints texts while game is running
{
	
	setbkcolor(7);
	setcolor(0);
	char g1[3]="00";
	char g2[3]="00";
	char c1[]="TIME",c2[]="PLAYER 1",c3[]="PLAYER 2",c4[]="Press Escape",c5[]="for Menu",c6[]="Press Escape",c7[]="for Menu";
		g1[1]=(char)(goal1%10+'0');
		g1[0]=(char)(goal1/10+'0');
		g2[1]=(char)(goal2%10+'0');
		g2[0]=(char)(goal2/10+'0');
	outtextxy((X0+X)/2-108*Xratio,30*Yratio,g1);
	outtextxy((X0+X)/2+70*Xratio,30*Yratio,g2);
	char t[4];
		sprintf(t,"%d",tme);
		int teme=tme,sz=0;
	    while(teme>0){
			teme/=10;
			sz++;
		}
		sz=max(sz,1);
		outtextxy((X0+X)/2-42*Xratio,5,c1);
		outtextxy((X0+X)/2-10*sz,38*Yratio,t);
		outtextxy((X0+X)/2-304*Xratio,28*Yratio,c2);
		outtextxy((X0+X)/2+135*Xratio,28*Yratio,c3);
	outtextxy(10,18,c4);
	outtextxy(10,42,c5);
	outtextxy(X+goal_width-230*Xratio,18*Yratio,c6);
	outtextxy(X+goal_width-150*Xratio,42*Yratio,c7);
	setbkcolor(0);
}
void settings()
{
	setbkcolor(7);
	cleardevice();
	setcolor(1);
	setlinestyle(0,0,5);
	rectangle(50*X1ratio,50*Y1ratio,X+goal_width-50*X1ratio,Y);
	setfillstyle(SOLID_FILL,8);
	floodfill((X+goal_width)/2,Y/2,1);
	char c1[]="Choose color for PLAYER 1:";
	settextstyle(3,HORIZ_DIR,7);
	outtextxy((X+goal_width)/4-100*X1ratio,100*Y1ratio,c1);//prints text for player 1 to choose color
	char c2[]="Press 1 to choose";
	settextstyle(3,HORIZ_DIR,4);
	outtextxy((X+goal_width)/4-75*X1ratio,Y-150*Y1ratio,c2);
	char c3[]="Press 2 to choose";
	outtextxy(3*(X+goal_width)/4-225*X1ratio,Y-150*Y1ratio,c3);
	setcolor(WHITE);
	setfillstyle(SOLID_FILL,9);//color choice 1
	circle((X+goal_width)/4+75*X1ratio,Y/2+25*Y1ratio,radius*2.5);//prints first circle
	floodfill((X+goal_width)/4+75*X1ratio,Y/2+25*Y1ratio,WHITE);//fills circle 1 with first color choice
	circle(3*(X+goal_width)/4-75*X1ratio,Y/2+25*Y1ratio,radius*2.5);//prints second circle
	setfillstyle(SOLID_FILL,11);//color choice 2
	floodfill(3*(X+goal_width)/4-75*X1ratio,Y/2+25*Y1ratio,WHITE);//fills circle 2 with second color choice
	while(1){
		if(GetAsyncKeyState('1'))
		{
			color_1=9;//assigns color choice 1 to striker 1
			break;
		}
		else if(GetAsyncKeyState('2'))
		{
			color_1=11;//assigns color choice 2 to striker 1
			break;
		}
		delay(10);
	}
	while(GetAsyncKeyState('1') || GetAsyncKeyState('2'))//to avoid misckicking of keys
	{
		delay(10);
	}
	cleardevice();
	setcolor(1);
	setlinestyle(0,0,5);
	rectangle(50*X1ratio,50*Y1ratio,(X+goal_width)-50*X1ratio,Y);//outer border
	setfillstyle(SOLID_FILL,8);
	floodfill((X+goal_width)/2,Y/2,1);
	char c4[]="Choose color for PLAYER 2:";//prints text for player 2 to chose color
	settextstyle(3,HORIZ_DIR,7);
	outtextxy((X+goal_width)/4-100*X1ratio,100*Y1ratio,c4);
	char c5[]="Press 1 to choose";
	settextstyle(3,HORIZ_DIR,4);
	outtextxy((X+goal_width)/4-75*X1ratio,Y-150*Y1ratio,c5);
	char c6[]="Press 2 to choose";
	outtextxy(3*(X+goal_width)/4-225*X1ratio,Y-150*Y1ratio,c6);
	setcolor(WHITE);
	setfillstyle(SOLID_FILL,2);//color choice 1
	circle((X+goal_width)/4+75*X1ratio,Y/2+25*Y1ratio,radius*2.5);//prints first circle
	floodfill((X+goal_width)/4+75*X1ratio,Y/2+25*Y1ratio,WHITE);//fills circle 1 with first color choice
	circle(3*(X+goal_width)/4-75*X1ratio,Y/2+25*Y1ratio,radius*2.5);//prints second cirlce
	setfillstyle(SOLID_FILL,10);//color choice 2
	floodfill(3*(X+goal_width)/4-75*X1ratio,Y/2+25*Y1ratio,WHITE);//fills circle 2 with second color choice
	while(1)
	{
		if(GetAsyncKeyState('1'))
		{
			color_2=2;//assigns color choice 1 to striker 2
			break;
		}
		else if(GetAsyncKeyState('2'))
		{
			color_2=10;//assigns color choice 2 to striker 2
			break;
		}
		delay(10);
	}
	settextstyle(4,HORIZ_DIR,3*Xratio);
}
void start(){
	readimagefile("resources/Soccer_stars_start.jpg",0,0,X_full,Y_full);
	// Press Enter to start the game
	while(!GetAsyncKeyState(VK_RETURN)) delay(10);
	settings();
}
void pause_screen(int option)
{
	setactivepage(page);
	setvisualpage(1-page);
	switch(option)
	{
		case 1:
			readimagefile("resources/Air_Hockey/pause_screen_resume.jpg",(X0+X)/2-200*Xratio,(Y0+Y)/2-200*Yratio,(X0+X)/2+200*Xratio,(Y0+Y)/2+200*Yratio);
			break;
		case 2:
			readimagefile("resources/Air_Hockey/pause_screen_reset.jpg",(X0+X)/2-200*Xratio,(Y0+Y)/2-200*Yratio,(X0+X)/2+200*Xratio,(Y0+Y)/2+200*Yratio);
			break;
		case 3:
			readimagefile("resources/Air_Hockey/pause_screen_settings.jpg",(X0+X)/2-200*Xratio,(Y0+Y)/2-200*Yratio,(X0+X)/2+200*Xratio,(Y0+Y)/2+200*Yratio);
			break;
		case 4:
			readimagefile("resources/Air_Hockey/pause_screen_quit.jpg",(X0+X)/2-200*Xratio,(Y0+Y)/2-200*Yratio,(X0+X)/2+200*Xratio,(Y0+Y)/2+200*Yratio);
			break;
		default:
			readimagefile("resources/Air_Hockey/pause_screen_resume.jpg",(X0+X)/2-200*Xratio,(Y0+Y)/2-200*Yratio,(X0+X)/2+200*Xratio,(Y0+Y)/2+200*Yratio);
	}
	page=1-page;
}
int pause()
{
	int option=1;
	while(1)
	{
		if(GetAsyncKeyState(VK_UP))
		{
			option--;
			if(option<1)
				option=4;
		}
		else if(GetAsyncKeyState(VK_DOWN))
		{
			option++;
			if(option>4)
				option=1;
		}
		else if(GetAsyncKeyState(VK_RETURN))
		{
			switch(option)
			{
				case 1:
					return 1;
					break;
				case 2:
					goal1=0,goal2=0,tme=0;
					speed_ball=0.0; // Restore the speed of ball to 0
					for(i=0;i<2*number;i++) striker_speed[i]=0; // Restore the all the striker_speeds to 0 
					// Restore all the initial positions of all strikers and ball
					x[0]=X0+radius+10*Xratio;
					y[0]=y[0+number]=(Y+Y0)/2;
					x[1]=x[2]=X0+(X-X0)/5;
					y[1]=y[1+number]=Y0+(Y-Y0)/4;
					y[2]=y[2+number]=Y0+(Y-Y0)*3/4;
					x[3]=x[4]=X0+2*(X-X0)/5;
					y[3]=y[3+number]=(Y+Y0)/2-2*goal_height/3;
					y[4]=y[4+number]=(Y+Y0)/2+2*goal_height/3;
					
					x[0+number]=X-radius-10*Xratio;
					x[1+number]=x[2+number]=X-(X-X0)/5;
					x[3+number]=x[4+number]=X-2*(X-X0)/5;
					
					ballx=(X+X0)/2;
					bally=(Y+Y0)/2;
					ball_angle=0.0;
					arc_angle=360.0;
					srand(time(0));
					turn=rand()%2;
					return 1;
					break;
				case 3:
					setvisualpage(1-page);
					settings();
					return 1;
					break;
				case 4:
					return 0;
					break;
				default:
					return 1;
			}
		}
		else if(GetAsyncKeyState(VK_ESCAPE))
		{	
			while(GetAsyncKeyState(VK_ESCAPE));
			return 1;
		}
		pause_screen(option);
		delay(100);
	}
}
void Time(){ // time function which updates the time displayed in seconds in the screen.
	time(&tf);
	if(ti!=tf){
		ti=tf;
		tme++;
	}
}
void move_all(); // used for move ment of all objects. predefined as it is called in gola() before definition of move_all();
void _print(int e){ // Used for printing the whole field and screen
	// _print(-1) means no striker has been chosen for shooting
	setactivepage(page); // double buffering
	setvisualpage(1-page);
	cleardevice();
	setcolor(WHITE);
	setlinestyle(0,0,3); // Set the line width to a 3 pixel line
	line(X0,Y0,X0,(Y+Y0)/2-goal_height); // Creating left X boundary above left goal
	line(X0,(Y+Y0)/2+goal_height,X0,Y); // Creating left X boundary below left goal
	line(X,Y0,X,(Y+Y0)/2-goal_height); // Creating right X boundary above right goal
	line(X,(Y0+Y)/2+goal_height,X,Y); // Creating right X boundary below right goal
	line(X0,Y0,X,Y0); // Upper Y-boundary
	line(X0,Y,X,Y); // Lower Y-boundary
	setlinestyle(0,0,1); // Set the line width to a 1 pixel line
	rectangle(X0,Y0,X,Y);
	setfillstyle(SOLID_FILL,7); // Colouring the external background to GREY keeping the field BLACK
	floodfill(X_full-1,Y_full-1,WHITE);
	line((X+X0)/2,Y0,(X+X0)/2,Y); // Draw central boundary line of strikers
	setcolor(1); // Set to dark bluish type colour
	line(X0-goal_width,(Y+Y0)/2-goal_height,X0,(Y+Y0)/2-goal_height); // Upper left goal boundary
	line(X0-goal_width,(Y+Y0)/2+goal_height,X0,(Y+Y0)/2+goal_height); // Lower left goal boundary
	line(X0-goal_width,(Y+Y0)/2-goal_height,X0-goal_width,(Y+Y0)/2+goal_height); // Left boundary of left goal
	line(X0-1,(Y+Y0)/2-goal_height,X0,(Y+Y0)/2+goal_height); // Left Goal entrance shifted 1 pixel to left for floodfilling
	setfillstyle(SOLID_FILL,9);
	floodfill(X0-3,(Y+Y0)/2,1); // BLUE coloring left goal
	setcolor(4);
	line(X+goal_width,(Y+Y0)/2-goal_height,X,(Y+Y0)/2-goal_height); // Upper boundary of right goal
	line(X+goal_width,(Y+Y0)/2+goal_height,X,(Y+Y0)/2+goal_height); // Lower boundary of right goal
	line(X+goal_width,(Y+Y0)/2-goal_height,X+goal_width,(Y+Y0)/2+goal_height); // Right boundary of right goal
	line(X+1,(Y+Y0)/2-goal_height,X,(Y+Y0)/2+goal_height); // Right goal entrance shifted 1 pixel to right.
	setfillstyle(SOLID_FILL,12);
	floodfill(X+3,(Y+Y0)/2,4); // RED coloring right goal
	// Making D and penalty box
	setcolor(WHITE);
	setfillstyle(SOLID_FILL,WHITE);
	line(X0,(Y+Y0)/2-goal_height,X0,(Y+Y0)/2+goal_height); // Entrance of left goal marked with white for coloring D.
	line(X0,(Y+Y0)/2-goal_height,X0+50*Xratio+2*radius,(Y+Y0)/2-goal_height); // Upper boundary of left penalty box
	line(X0,(Y+Y0)/2+goal_height,X0+50*Xratio+2*radius,(Y+Y0)/2+goal_height); // Lower boundary of left penalty box.
	line(X0+50*Xratio+2*radius,(Y+Y0)/2-goal_height,X0+50*Xratio+2*radius,(Y+Y0)/2+goal_height); // Right boundary/beginning of left penalty box
	arc(X0,(Y+Y0)/2,-90,90,3*goal_height/4); // Making D of left goal
	floodfill(X0+5,(Y+Y0)/2,WHITE); // Coloring left D
	line(X,(Y+Y0)/2-goal_height,X,(Y+Y0)/2+goal_height); // Entrance of right goal marked with white for coloring D.
	line(X,(Y+Y0)/2-goal_height,X-50*Xratio-2*radius,(Y+Y0)/2-goal_height); // Upper boundary of right penalty box
	line(X,(Y+Y0)/2+goal_height,X-50*Xratio-2*radius,(Y+Y0)/2+goal_height); // Lower boundary of right penalty box.
	line(X-50*Xratio-2*radius,(Y+Y0)/2-goal_height,X-50*Xratio-2*radius,(Y+Y0)/2+goal_height); // Left boundary/beginning of right penalty box.
	arc(X,(Y+Y0)/2,90,-90,3*goal_height/4); // Making D of right goal
	floodfill(X-5,(Y+Y0)/2,WHITE); // Coloring right D
	setcolor(YELLOW);
	circle(ballx,bally,radius_ball); // Making the ball with YELLOW boundary of 3 pixels and color WHITE.
	setfillstyle(SOLID_FILL,WHITE);
	floodfill(ballx,bally,YELLOW);
	setcolor(color_1);
	setfillstyle(SOLID_FILL,color_1);
	for(i=0;i<number;i++){ // Coloring all the left strikers
		circle(x[i],y[i],radius);
		floodfill(x[i],y[i],color_1);
	}
	setcolor(color_2);
	setfillstyle(SOLID_FILL,color_2);
	for(i=number;i<2*number;i++){ // Coloring all the right strikers
		circle(x[i],y[i],radius);
		floodfill(x[i],y[i],color_2);
	}
	setcolor(BLACK);
	//Bleachers
	rectangle(X0-goal_width,Y0,X0-1,(Y+Y0)/2-goal_height-1);
	rectangle(X0-goal_width,(Y+Y0)/2+goal_height+1,X0-1,Y);
	rectangle(X+1,Y0,X+goal_width,(Y+Y0)/2-goal_height-1);
	rectangle(X+1,(Y+Y0)/2+goal_height+1,X+goal_width+1,Y);
	setlinestyle(0,0,3);
		rectangle((X0+X)/2-50*Xratio,0,(X0+X)/2+50*Xratio,80*Yratio);//time square
		rectangle(((X0+X)/2-50*Xratio)-80*Xratio,10,(X0+X)/2-50*Xratio,70*Yratio);//score square 1
		rectangle(((X0+X)/2+50*Xratio)+80*Xratio,10,(X0+X)/2+50*Xratio,70*Yratio);//score square 2
		rectangle(((X0+X)/2-50*Xratio)-265*Xratio,20*Yratio,(X0+X)/2-130*Xratio,60*Yratio);//name square 1
		rectangle(((X0+X)/2+50*Xratio)+265*Xratio,20*Yratio,(X0+X)/2+130*Xratio,60*Yratio);//name square 2
	color();
	text();
    if(movement==0){ // This is printed when the player has the turn to shoot the ball. 
    	setlinestyle(0,0,5); // 5 pixel thick line arc around the strikers
    	setcolor(13);
    	// Depending on whose turn it is, an arc around all the strikers of the current shooter are drawn with pink
    	if(turn==0) for(i=0;i<number;i++) arc(x[i],y[i],0,arc_angle,radius+4);
		else if(turn==1) for(i=number;i<2*number;i++) arc(x[i],y[i],0,arc_angle,radius+4); 
		arc_angle-=1.0/3; // The size of this arc decreases with time. The length of arc denotes the time the player has to shoot the ball.
	}
    if(e!=-1){ // Here e denotes the striker from whose center the mouse was ulled to shoot
		GetCursorPos(&mouse); // Gets the current location/position of the mouse.
		int releasepos_x=mouse.x,releasepos_y=mouse.y;
		double anglee=deg(atan((releasepos_y-y[e])*1.0/(x[e]-releasepos_x))); // anglee is the angle which the pointer of mouse is making with the 
		// center of striker from whom the left button is pressed. Slope formula is used
		if(releasepos_y==y[e]) anglee=(x[e]<releasepos_x)*(180.0);
		else if(releasepos_x==x[e]) anglee=90.0+(releasepos_y<y[e])*(180.0);
		else if(releasepos_x>x[e]) anglee+=180.0;
		double distance=round(sqrt(pow(releasepos_x-x[e],2)+pow(releasepos_y-y[e],2))); // distance of centre of striker to mouse position
		// this distance is also in ratio to the speed of the striker.
		distance=min(distance,max_speed*20.0); // Since the distance is in ratio to the speed of striker and the speed of striker
		// has a maximum limit hence the distance also has a maximum limit.
		distance/=2; // random scaling factor for appropriate visuals of line 
		setlinestyle(0,0,5);
		anglee=rad(anglee); // converting angle into radian to be used in sin and cos functions/
		if(distance>radius){
			setcolor(YELLOW);
			// YEELOW line is created if the mouse is at least outside the striker radius. It stars at the boundary of radius
			//  YELLOW  line denoting the vector along which the striker will be projected, if the mouse is released form that position
			line(x[e]+round(radius*cos(anglee)),y[e]-round(radius*sin(anglee)),x[e]+round(distance*cos(anglee)),y[e]-round(distance*sin(anglee)));
			// Arrow head are the end of striker projected yellow line endpoint
			line(x[e]+round(distance*cos(anglee))-round(distance*cos(anglee+rad(30.0))/4),y[e]-round(distance*sin(anglee))+round(distance*sin(anglee+rad(30.0))/4),x[e]+round(distance*cos(anglee)),y[e]-round(distance*sin(anglee)));
			line(x[e]+round(distance*cos(anglee))-round(distance*cos(anglee-rad(30.0))/4),y[e]-round(distance*sin(anglee))+round(distance*sin(anglee-rad(30.0))/4),x[e]+round(distance*cos(anglee)),y[e]-round(distance*sin(anglee)));
			setlinestyle(1,0,2); // 2 pixel dashed line
			setcolor(WHITE);
			// Dashed WHITE line 1/3*distance extension from the end point of YELLOW line 
			line(x[e]+round(4*distance*cos(anglee)/3),y[e]-round(4*distance*sin(anglee)/3),x[e]+round(distance*cos(anglee)),y[e]-round(distance*sin(anglee)));
			setcolor(7);
			setlinestyle(1,0,3);
			// Dashed GREY line 1/3*distnace extension from the centre of striker along the line joining the current coordiantes of
			// mouse
			line(x[e]-round(radius*cos(anglee)),y[e]+round(radius*sin(anglee)),x[e]-round((radius+distance/3)*cos(anglee)),y[e]+round((distance/3+radius)*sin(anglee)));		
		}
		else{
			setcolor(RED);
			// if the mouse coordinates are inside the striker_radius then red line from centre to mouse coordinates are created.
			line(x[e],y[e],x[e]+round(distance*cos(anglee)),y[e]-round(distance*sin(anglee)));
		}
	}
    page=1-page; // double buffering page change
}
void gola(int who){ // This is initiated whenever a goal happens. Here who defines whoever accepted the goal
	goal=1; // Goal is set to 1 as defined above because the process of occurence of goal has started
	if(who==1){
		turn=0; // turn is set to 0. because if left player accepts the goal then. During kick-off next time it will be the turn of
		// left player.
		if(moves>1) goal2++;  // If player 1 accepts goal means number of goals of 2 increases by 1.
		// moves>1 ensures that it was a goal and not a FOUL.
		while(ballx>X0-2*goal_width/3 && (bally>=(Y+Y0)/2-goal_height+radius_ball) && (bally<=(Y+Y0)/2+goal_height-radius_ball)){
			/*
			THe 1st condition of while loop condition defines the amount of distnace ball with travel before stopping inside the goal
			The 2nd and 3rd define that the ball should not cross the upper and lower boundaries of goal while travelling to the 
			required distance inside the goal i.e. 2/3 * width_goal
			*/
			_print(-1); // print the new position of ball inside goal
			speed_ball=min(speed_ball,3); // This is to slow down ball movement or keep speed constant if less than 3 inside goal
			move_all(); // To keep moving the ball in the same direction with slower speed_ball when goal happens
			usleep(10*ms);
		}
	}
	else{ // Same process as above but this time when player 2 accepts goal
		turn=1;
		if(moves>1) goal1++;
		while(ballx<X+2*goal_width/3 && (bally>=(Y+Y0)/2-goal_height+radius_ball) && (bally<=(Y+Y0)/2+goal_height-radius_ball)){
			_print(-1);
			speed_ball=min(speed_ball,3);
			move_all();
			usleep(10*ms);
		}
	}
	speed_ball=0.0; // Restore the speed of ball to 0 
	for(i=0;i<2*number;i++) striker_speed[i]=0; // Restore the all the striker_speeds to 0 
	// Restore all the initial positions of all strikers and ball
	x[0]=X0+radius+10*Xratio;
	y[0]=y[0+number]=(Y+Y0)/2;
	x[1]=x[2]=X0+(X-X0)/5;
	y[1]=y[1+number]=Y0+(Y-Y0)/4;
	y[2]=y[2+number]=Y0+(Y-Y0)*3/4;
	x[3]=x[4]=X0+2*(X-X0)/5;
	y[3]=y[3+number]=(Y+Y0)/2-2*goal_height/3;
	y[4]=y[4+number]=(Y+Y0)/2+2*goal_height/3;
	
	x[0+number]=X-radius-10*Xratio;
	x[1+number]=x[2+number]=X-(X-X0)/5;
	x[3+number]=x[4+number]=X-2*(X-X0)/5;
	
	ballx=(X+X0)/2;
	bally=(Y+Y0)/2;
	ball_angle=0.0;
	goal=0; // End the process of goal i.e movement of ball inside goal
	setvisualpage(1-page); // since in the above while loop in double buffering the visual and active pages were being constantly 
	// swapped. To set visual page back to the newly printed one this is done.
	// GOAL !! is printed for 2 seconds in the middle of the ground for 2 seconds after all process of ball movement inside goal has 
	//occured 
	if(moves>1) readimagefile("resources/Air_Hockey/goal.jpg",(X0+X)/2-150*Xratio,(Y0+Y)/2-75*Yratio,(X0+X)/2+150*Xratio,(Y0+Y)/2+75*Yratio);
	else if(moves==1){
		setcolor(13);
		settextstyle(0,0,5);
		char c[]="FOUL !!"; // if goal is done in first move it will me considered a foul
		outtextxy((X0+X)/2-120*Xratio,(Y0+Y)/2-50*Yratio,c);
	}
	settextstyle(4,0,3*Xratio);
	moves=0; // After goal has occured again set the moves number to 0
	sleep(2); // play goal/foul text for 2 seconds
}
void boundary(){
	if(ballx<=(radius_ball+X0) && (bally>=(Y+Y0)/2-goal_height+radius_ball) && (bally<=(Y+Y0)/2+goal_height-radius_ball)){
		gola(1); // This if is to detect the left goal when ball goes inside it. Then gola function is initiated with parameter
		return; // who accepted the goal
	}
	else if((ballx+radius_ball)>=X && (bally>=(Y+Y0)/2-goal_height+radius_ball) && (bally<=(Y+Y0)/2+goal_height-radius_ball)){
		gola(2); // TO detect right goal when ball goes inside
		return;
	}
	if(ballx<=(radius_ball+X0) || ballx+radius_ball>=X ) ball_angle=fmod(fmod(180.0-ball_angle,360.0)+360.0,360.0); // Change the direction when collision with left and right boundaries
	if(bally<=(radius_ball+Y0) || bally+radius_ball>=Y) ball_angle=fmod(fmod(360.0-ball_angle,360.0)+360.0,360.0); // Change the direction when collision with upper and lower boundaries
	for(i=0;i<2*number;i++){
		if(x[i]<=(radius+X0) || x[i]+radius>=X){ 
			x[i]=min(x[i],X-radius); // Ensures that striker stays within the left and right boundaries
			x[i]=max(x[i],radius+X0);
			angle[i]=fmod(fmod(180.0-angle[i],360.0)+360.0,360.0); // Change the direction when collision with left and right boundaries
		}
		if(y[i]<=(radius+Y0) || y[i]+radius>=Y){ // Change the direction of striker when collision with upper and lower boundaries
			y[i]=min(y[i],Y-radius); // Ensures that striker stays within the upper and lower boundaries
			y[i]=max(y[i],radius+Y0);
			angle[i]=fmod(fmod(360.0-angle[i],360.0)+360.0,360.0); // Change the direction of striker when collision with upper and lower boundaries
		}
	}
}
/*
Check_Collision() integer type function was defined instead of just a normal collisions because in normal collisiosn function 
only one collision at a time can occur in the next iteration this can cause overlapping of objects.
This can be prevented using check_collision() integer type.
Learn more about it in
"C:\Users\SOHAM\Desktop\PDS Final Prohect\More about Soccer Stars.txt"
*/
int check_collision(){ // this checks if moving the ball will cause a collison in the next iteration. If YES, chagne angle accordingly.
	int ans=0;
	// ans stores if any collision will occur in the next iteration.1 if yes, 0 otherwise.
	for(i=0;i<2*number;i++){
		// These are the distances travelled by the x and y component of ball and striker in the next iteration at that angle.
		int incxb=round(speed_ball*cos(rad(ball_angle))),incyb=round(speed_ball*sin(rad(ball_angle)));
		int incxs=round(striker_speed[i]*cos(rad(angle[i]))),incys=round(striker_speed[i]*sin(rad(angle[i])));
		if((pow((x[i]+incxs)-(ballx+incxb),2)+pow((y[i]-incys)-(bally-incyb),2))<=(radius+radius_ball)*(radius+radius_ball)){
			ans=1;
			// theta is also the slope of line of approach.Calculating slope of line and converting that to degree from tan inverse
			double theta=deg(atan((bally-y[i])*1.0/(x[i]-ballx))); // Theta is the angle of line joining centres of striker and ball
			// with +X axis measured in anticlockwise direction. Also the direction of theta points from the centre of striker to ball
			if(bally==y[i]) theta=(x[i]>ballx)*(180.0); // When both on same horizontal line theta is 0 or 180
			else if(ballx==x[i]) theta=90.0+(bally>y[i])*(180.0); // When both on same vertical line theta is 90 or 270
			else if(ballx>x[i]) theta+=180.0; // If the ball x>x[i] change by 180. This is the principal values of arguments in 
			// 2 and 3 quadrants. becoz tan inverse will be same in both cases. 
			// initial parallel  and perpendicular component of ball
			double vbi_parallel=speed_ball*cos(rad(ball_angle-theta)),vb_perp=speed_ball*sin(rad(ball_angle-theta));
			// initial parallel  and perpendicular component of striker
			double vsi_parallel=striker_speed[i]*cos(rad(angle[i]-theta)),vs_perp=striker_speed[i]*sin(rad(angle[i]-theta));
			// Final parallel component of ball
			double vbf_parallel=((1+e)*Ms*vsi_parallel+vbi_parallel*(Mb-e*Ms))/(Mb+Ms);
			// Final parallel component of striker
			double vsf_parallel=((1+e)*Mb*vbi_parallel+vsi_parallel*(Ms-e*Mb))/(Mb+Ms);
			// Perpendicular component of velocities from line of approach will not change.
			ball_angle=theta+deg(atan(vb_perp/vbf_parallel)); // new ball angle after collision
			angle[i]=theta+deg(atan(vs_perp/vsf_parallel)); // new striker angle after collision
			speed_ball=sqrt(pow(vbf_parallel,2)+pow(vb_perp,2)); // new speed_ball after collision
			striker_speed[i]=sqrt(pow(vsf_parallel,2)+pow(vs_perp,2)); // new striker_speed after collision
			if(collisions>30) speed_ball=striker_speed[i]=0; // more than 30 collision means tangential collisions and in such cases
			// the velocities of elemnts causing it is to be set to 0
			// We are always taking the magnitude of ball and striker velocity. So if the parallel component changes sign to negative
			// instead of changing signs of velocities the angle of direction is reversed by 180.
			if(vbf_parallel<0) ball_angle+=180.0;
			if(vsf_parallel<0) angle[i]+=180.0;
		}
		int j;
		for(j=i+1;j<2*number;j++){ // same case for striker striker collisions
			int incxi=incxs,incyi=incys;
			int incxj=round(striker_speed[j]*cos(rad(angle[j]))),incyj=round(striker_speed[j]*sin(rad(angle[j])));
			if(pow((x[i]+incxi)-(x[j]+incxj),2)+pow((y[i]-incyi)-(y[j]-incyj),2)<=4*radius*radius){
				ans=1;
				double theta=deg(atan((y[j]-y[i])*1.0/(x[i]-x[j])));
				if(y[j]==y[i]) theta=(x[i]>x[j])*(180.0);
				else if(x[j]==x[i]) theta=90.0+(y[j]>y[i])*(180.0);
				else if(x[j]>x[i]) theta+=180.0;
				
				double vji_parallel=striker_speed[j]*cos(rad(angle[j]-theta)),vj_perp=striker_speed[j]*sin(rad(angle[j]-theta));
				double vii_parallel=striker_speed[i]*cos(rad(angle[i]-theta)),vi_perp=striker_speed[i]*sin(rad(angle[i]-theta));
				double vjf_parallel=((1+e)*vii_parallel+vji_parallel*(1-e))/2;
				double vif_parallel=((1+e)*vji_parallel+vii_parallel*(1-e))/2;
				angle[j]=theta+deg(atan(vj_perp/vjf_parallel));
				angle[i]=theta+deg(atan(vi_perp/vif_parallel));
				striker_speed[j]=sqrt(pow(vjf_parallel,2)+pow(vj_perp,2));
				striker_speed[i]=sqrt(pow(vif_parallel,2)+pow(vi_perp,2));
				if(collisions>30) striker_speed[i]=striker_speed[j]=0;
				if(vjf_parallel<0) angle[j]+=180.0;
				if(vif_parallel<0) angle[i]+=180.0;
			}
		}
	}
	return ans;
}
void move_all(){
	int incx=round(speed_ball*cos(rad(ball_angle))),incy=round(speed_ball*sin(rad(ball_angle))); // Move the ball by taking its 
	// vector components in x and y direction
	ballx+=incx;
	bally-=incy;
	if(goal==0){
		if(bally<((Y+Y0)/2-goal_height+radius) || bally>((Y+Y0)/2+goal_height-radius)) ballx=max(ballx,X0+radius_ball);
		 // Ball touches left boundary above and below goal.
		if(bally<((Y+Y0)/2-goal_height+radius) || bally>((Y+Y0)/2+goal_height-radius)) ballx=min(ballx,X-radius_ball);
		// Ball touches right boundary above and below goal
		bally=max(bally,Y0+radius_ball);  // Lower Y limits on ball
		bally=min(bally,Y-radius_ball); // Lower X limits on ball
	}
	for(i=0;i<2*number;i++){ // same process is repeated for all strikers.
		incx=round(striker_speed[i]*cos(rad(angle[i])));
		incy=round(striker_speed[i]*sin(rad(angle[i])));
		x[i]+=incx;
		y[i]-=incy;
		if(goal==1){
			x[i]=min(x[i],X-radius);
			x[i]=max(x[i],X0+radius);
			y[i]=min(y[i],Y-radius);
			y[i]=max(y[i],Y0+radius);
		}
	}
}
void control_striker(){
	collisions=0; // collisions are set to 0 each time all collisions in a single shot are over
	moves++; // moves represent the number of moves played after kick-off or goals scored
	arc_angle=360.0; // this is the time left for a player to move teh striker which is refilled each time new turn comes
	int incx=0,incy=0;
	int flag=-1; // flag represents which striker was chosen while inside the VK_LBUTTON was pressed
	_print(-1); // Field printed
	while(1){
		if(arc_angle<=0) break; // condition where user runs out of time
		while(!GetAsyncKeyState(VK_LBUTTON)){ // this loops runs while user doesn't press leftbutton to select some object
			if(GetAsyncKeyState(VK_ESCAPE))
			{
				while(GetAsyncKeyState(VK_ESCAPE)) delay(10);
				run_game=pause();
				if(run_game==0)
				{
					return;
				}
			}
			if(arc_angle<=0) break;
			Time(); // time has to be updated even inside loop
			_print(-1);
			usleep(10*ms);
		}
		GetCursorPos(&mouse);
		// Breaks out of loop if correct striker_is left clicked at the proper turn. Otherwise loop continues running
		if(turn==0){
			for(i=0;i<number;i++){
				if((x[i]-mouse.x)*(x[i]-mouse.x)+(y[i]-mouse.y)*(y[i]-mouse.y)<=radius*radius){
					flag=i;
					break;
				}
			}
		}
		else if(turn==1){
			for(i=number;i<2*number;i++){
				if((x[i]-mouse.x)*(x[i]-mouse.x)+(y[i]-mouse.y)*(y[i]-mouse.y)<=(radius+radius_ball)*(radius+radius_ball)){
					flag=i;
					break;
				}
			}
		}
		// if proper object is selected
		if(flag!=-1){
			while(GetAsyncKeyState(VK_LBUTTON)){ // While the required object has been selected keep running the loop.
				if(arc_angle<=0) break;
				if(GetAsyncKeyState(VK_RBUTTON)){ // This ensures that after one striker is selected for shooting one can change
				// the choice of choosing another striker by clicking the right button.
					flag=-1;
					break;
				}
				Time();
				_print(flag);
				delay(10);
			}
			if(flag!=-1) break;
		}
		else{
			while(GetAsyncKeyState(VK_LBUTTON)){ // to accomodate wrong selected position of object
				if(arc_angle<=0) break;
				Time();
				_print(-1);
				delay(10);
			}
		}
		usleep(10*ms);
	}
	
	i=flag;
	GetCursorPos(&mouse);
	int releasepos_x=mouse.x,releasepos_y=mouse.y; // Final position of mouse when released
	double anglee=deg(atan((releasepos_y-y[i])*1.0/(x[i]-releasepos_x))); // angle of striker projection to be determined from slope
	if(releasepos_y==y[i]) anglee=(x[i]<releasepos_x)*(180.0);
	else if(releasepos_x==x[i]) anglee=90.0+(releasepos_y<y[i])*(180.0);
	else if(releasepos_x>x[i]) anglee+=180.0;
	// distance is the distance of mouserelease point from the centre of chosen striker
	int distance=(arc_angle>0)? (round(sqrt(pow(releasepos_x-x[i],2)+pow(releasepos_y-y[i],2)))):0;
	// if arc_angle is less than speed_striker will be 0 due to time runout
	if(i!=-1) angle[i]=anglee;
	striker_speed[i]=distance/10.0; // set striker speed accordingly 
	striker_speed[i]=min(striker_speed[i],max_speed); // limit themax striker speed
	if(striker_speed[i]) movement=1; // set value of movement to 1 is striker speed is not 0
	turn=1-turn;
}
int main(){
	X=GetSystemMetrics(SM_CXSCREEN); 
	Y=GetSystemMetrics(SM_CYSCREEN); 
	X_full=X;
	Y_full=Y; 
	X0=100*Xratio;Y0=85*Yratio;
	goal_height=100*Yratio;
	goal_width=90*Xratio;
	radius=40;radius_ball=20;
	max_speed=15.0;
	initwindow( X , Y , "",-3,-3);
	//setbkcolor(1);
	X-=100*Xratio;
	Y-=50*Yratio;
	// Formation:-
	// Initial formation of both sides of game
	ballx=(X0+X)/2;bally=(Y0+Y)/2;
	x[0]=X0+radius+10*Xratio;
	y[0]=y[0+number]=(Y+Y0)/2;
	x[1]=x[2]=X0+(X-X0)/5;
	y[1]=y[1+number]=Y0+(Y-Y0)/4;
	y[2]=y[2+number]=Y0+(Y-Y0)*3/4;
	x[3]=x[4]=X0+2*(X-X0)/5;
	y[3]=y[3+number]=(Y+Y0)/2-2*goal_height/3;
	y[4]=y[4+number]=(Y+Y0)/2+2*goal_height/3;
	
	x[0+number]=X-radius-10*Xratio;
	x[1+number]=x[2+number]=X-(X-X0)/5;
	x[3+number]=x[4+number]=X-2*(X-X0)/5;
	
	srand(time(0)); // generates a random number to determine who will start the game
	turn=rand()%2;
	time(&ti);
	start();
	while(run_game){
		if(check_collision()){
			collisions++;
			continue;
		}
		time(&tf);
		if(ti!=tf){
			ti=tf;
			speed_ball-=1.5; // Frictional speed loss of ball per second
			tme++;
			speed_ball=max(speed_ball,0);
			movement=speed_ball; // movement is net speed of all objects
			double ans=speed_ball;
			for(i=0;i<2*number;i++){
				striker_speed[i]-=3.0; // Frictional speed loss of striker per second
				striker_speed[i]=max(striker_speed[i],0);
				movement+=striker_speed[i];
				ans+=striker_speed[i];
			}
		}
		_print(-1);
		move_all();
		boundary();
		if(movement==0) control_striker(); // Only initiated when all are stagnant
		if(GetAsyncKeyState(VK_ESCAPE))
		{
			while(GetAsyncKeyState(VK_ESCAPE)) delay(10);
			run_game=pause();
		}
		usleep(10*ms);
	}
	setvisualpage(1-page);
	setcolor(WHITE);
	char c[]="BYE !! ";
    settextstyle(EUROPEAN_FONT,HORIZ_DIR,2);
    outtextxy(600*Xratio,300*Yratio,c);
    char c1[]="BYE!!";
    settextstyle(SANS_SERIF_FONT,HORIZ_DIR,4);
    outtextxy(600*Xratio,400*Yratio,c1);
    sleep(2);
    closegraph();
	return 0;
}
