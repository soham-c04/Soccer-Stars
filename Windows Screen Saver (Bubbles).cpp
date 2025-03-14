#include <graphics.h>
#include <conio.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
using namespace std;

const double pi=2*acos(0.0);
double rad(int degree){
	double ans=pi*degree/180.0;
	return ans;
}
double deg(double radi){
	double ans=180.0*radi/pi;
	return ans;
}

#define ms 1000
#define Xratio X_full/1366  // For Adjusting to size of objects in X direction according to resolution of Screen
#define Yratio Y_full/768 	// For Adjusting to size of objects in Y direction according to resolution of Screen
#define X1ratio 1 			// For Adjusting to size of objects in circular direction according to resolution of Screen
#define Y1ratio 1 			// Same as above
// Initial build resolution (of my PC) - 768 * 1366

int width=GetSystemMetrics(SM_CXSCREEN);  // Gets the X resolution of Screen
int height=GetSystemMetrics(SM_CYSCREEN); // Gets the Y resolution of Screen;
const int X_full=width,Y_full=height; 	  // Stores the X and Y orignial resolution of screen used in Xratio and Yratio
int X1=0,Y1=0; 		   // coordinates of top left corner of playable field
int X2=0,Y2=0; 		   // coordinates of bottom right corner of playable field
int X=10,Y=100*Yratio; // Non-playable part of field
int Timer=0;		   // Time (in sec) since the start of game
time_t ti,tf; 		   // ti represents the previously stored time of system when the time changed and tf is the current time of system.
POINT mouse; 		   // struct for declaring coordinates of mouse
bool run_game=true;    // breaks out of game or quits the game on selecting quit option;
int page=0;            // Double buffering
const double e=1.0;    // Coefficient of restitution
const int DELAY=10;    // Time difference between 2 frames (in ms)

const int MAX_COL=30;
int collisions=0;
/*
"collisions" is a debuggging variale which stores the number of collisions occuring between any 2 objects after a
the ball has been shot. Which is reset to 0 after each time movement becomes 0. Collisions are used in debugging tangential
collisions bcecause the game gets stuck there due to infinitely many collisions occuring. If the collision count reaches>30
that means tangential collisions are occuring the objects causing tangential collisions are abruptly set to 0 to prevent
stoppage of game. This works because in almost all other cases collisions are less than MAX_COL in a single shot.
*/

void Time(){ // time function which updates the time displayed in seconds in the screen.
	time(&tf);
	if(ti!=tf){
		ti=tf;
		Timer++;
	}
}

int turn=0;  		// turn=1 => Player1 and turn=2 => Player2
double movement=0; 	// sum of speeds of ball + all strikers. If it is 0 that means all objects have stopped moving value of turn is swapped

void _print(int n=-1); // Prints everything
bool pause();  // Shows pause menu and screen

class Ball{
	private:
		const double loss=0.0; 	  // Reduction in speed of ball /sec
	public:
		const double mass=1.0;    // Mass of ball - used in collision physics
		const int radius=20;   	  // Radius of ball
		int x,y; 			  	  // Coords of ball
		double speed=0.0;      	  // Speed of ball (slowed when goal occurs)
		double angle=90.0; 	   	  // Angle at which the ball is currently moving w.r.t +ve x-axis anticlockwise

		void reset(){ 		   // After scoring a goal reset back to center
			speed=0.0;		   // Reset ball speed to initial value
			x=(X2+X1)/2;  	   // reset ball X coordinate to middle of playable field
			y=(Y2+Y1)/2;       // reset ball Y coordinate to middle of playable field
			angle=90.0;        // moves ball vertically next round so both players get fair chance to score
		}

		void move();     // Moves ball
		void boundary(); // Checks for boundary collisions and goals
}ball;

class Striker{
	private:
		static const int number=5; 	 // # of strikers for each player
		const int timeout=5;         // Time (in sec) after which turn goes to other player.
		const double mass=6.0;       // Mass of striker - used in collision physics
		const double max_speed=20.0; // max. Speed of striker when shooting
		const double loss=0.0;       // Reduction in speed of striker /sec
		int x[number],y[number]; 	 // x and y coords of striker[i]
		double speed[number]; 		 // cur. speed of striker[i]
		double angle[number]; 		 // cur. angle striker[i] is travelling in
		double arc_angle=360.0; 	 // Arc angle is the constant timer around the striker, which is rorating and decreasing over time.
		// When it reaches 0 that means that the timer for the player is over and reaches a timeout! Turn is passed to next player.
	public:
		const int radius=40; 		 // Radius of striker
		int color; 	  				 // Color of strikers

		void reset(int who){ // After scoring a goal reset back to initial Formation
			arc_angle=360.0;
			for(int i=0;i<number;i++) speed[i]=0.0;
			// Formation:-
			// Reset x coords
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
			// Reset y coords
			y[0]=(Y1+Y2)/2;
			y[1]=Y1+(Y2-Y1)/4;
			y[2]=Y1+(Y2-Y1)*3/4;
			y[3]=(Y2+Y1)/2-2*radius;
			y[4]=(Y2+Y1)/2+2*radius;
		}
		
		void print(int who,int n){
			setlinestyle(0,0,1);
			setcolor(15);
			setfillstyle(SOLID_FILL,color);
			for(int i=0;i<number;i++){
				circle(x[i],y[i],radius);
				floodfill(x[i],y[i],15);
			}
				
			if(movement==0){ // Timer arc around the ball
		    	// Depending on whose turn it is, an arc around all the strikers of the current shooter are drawn with pink
				if(turn==who){
			    	setlinestyle(0,0,5); // 5 pixel thick line arc around the strikers
			    	setcolor(13);

			    	for(int i=0;i<number;i++) arc(x[i],y[i],0,arc_angle,radius+4);

			    	// The size of this arc decreases with time. The length of arc denotes the time the player has to shoot the ball.
					arc_angle-=(360.0*DELAY/(timeout*ms))*2; // Extra factor of 2 to account for processing delays

					// Mouse pullback
					if(n!=-1){ // Here n denotes the striker from whose center the mouse was pulled to shoot
						GetCursorPos(&mouse); // Gets the current location/position of the mouse.
						int releasepos_x=mouse.x,releasepos_y=mouse.y;
						double anglee=deg(atan2(releasepos_y-y[n],x[n]-releasepos_x)); // anglee is the angle which the pointer of mouse is making with the
						// center of striker from whom the left button is pressed. Slope formula is used
						
						double distance=round(sqrt(pow(releasepos_x-x[n],2)+pow(releasepos_y-y[n],2))); // distance of centre of striker to mouse position
						// this distance is also in ratio to the speed of the striker.
						distance=min(distance,max_speed*20.0); // Since the distance is in ratio to the speed of striker and the speed of striker
						// has a maximum limit hence the distance also has a maximum limit.
						distance/=2; // random scaling factor for appropriate visuals of line

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
				}
			}
		}

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
		
		void move(){
			for(int i=0;i<number;i++){ // same process is repeated for all strikers.
				int incx=round(speed[i]*cos(rad(angle[i])));
				int incy=round(speed[i]*sin(rad(angle[i])));
				x[i]+=incx;
				y[i]-=incy;

				speed[i]-=loss*DELAY/ms; // Frictional speed loss of striker per second
				speed[i]=max(speed[i],0.0);
				movement+=speed[i];
					
				boundary();
			}
		}
		
		bool Collision(){ // Collision of Striker with ball and Same side strikers
			bool collide=false; // Checks if at least one collision has happened
			
			for(int i=0;i<number;i++){
				// These are the distances travelled by the x and y component of ball and striker in the next iteration at that angle.
				int incxb=round(ball.speed*cos(rad(ball.angle))),incyb=round(ball.speed*sin(rad(ball.angle)));
				int incxs=round(speed[i]*cos(rad(angle[i]))),incys=round(speed[i]*sin(rad(angle[i])));
				if((pow((x[i]+incxs)-(ball.x+incxb),2)+pow((y[i]-incys)-(ball.y-incyb),2))<=(radius+ball.radius)*(radius+ball.radius)){
					collide=true; // Collision happens
					// theta is also the slope of line of approach.Calculating slope of line and converting that to degree from tan inverse
					double theta=deg(atan2(ball.y-y[i],x[i]-ball.x)); // Theta is the angle of line joining centres of striker and ball
					// with +X axis measured in anticlockwise direction. Also the direction of theta points from the centre of striker to ball
//					
					double vbi_parallel=ball.speed*cos(rad(ball.angle-theta)),vb_perp=ball.speed*sin(rad(ball.angle-theta));
					// initial parallel  and perpendicular component of striker
					double vsi_parallel=speed[i]*cos(rad(angle[i]-theta)),vs_perp=speed[i]*sin(rad(angle[i]-theta));
					// Final parallel component of ball
					double vbf_parallel=((1+e)*mass*vsi_parallel+vbi_parallel*(ball.mass-e*mass))/(ball.mass+mass);
					// Final parallel component of striker
					double vsf_parallel=((1+e)*ball.mass*vbi_parallel+vsi_parallel*(mass-e*ball.mass))/(ball.mass+mass);
					// Perpendicular component of velocities from line of approach will not change.
					ball.angle=theta+deg(atan2(vb_perp,vbf_parallel)); // new ball angle after collision
					angle[i]=theta+deg(atan2(vs_perp,vsf_parallel)); // new striker angle after collision
					ball.speed=sqrt(pow(vbf_parallel,2)+pow(vb_perp,2)); // new ball.speed after collision
					speed[i]=sqrt(pow(vsf_parallel,2)+pow(vs_perp,2)); // new striker_speed after collision
					
					if(collisions>MAX_COL){
//						printf("%d, Ball\n",i);     // For debugging
						speed[(i+1)%number]+=ball.speed;
						speed[(i+2)%number]+=speed[i];
						ball.speed=speed[i]=0; // more than MAX_COL collision means tangential collisions and in such cases
											   // the velocities of elemnts causing it is to be set to 0
					// We are always taking the magnitude of ball and striker velocity. So if the parallel component changes sign to
					// negative instead of changing signs of velocities the angle of direction is reversed by 180.
					}
				}
				
				for(int j=i+1;j<number;j++){ // striker1 collision with itself
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
//							printf("%d %d, Same\n",i,j);    // For debugging
							if(j-i>2){
								speed[i+1]+=speed[i];
								speed[i+2]+=speed[j];
							}
							else{
								speed[(j+1)%number]+=speed[i];
								speed[(j+2)%number]+=speed[j];
							}
							speed[i]=speed[j]=0; // more than 10 collision means tangential collisions and in such cases
												 // the velocities of elemnts causing it is to be set to 0
						// We are always taking the magnitude of ball and striker velocity. So if the parallel component changes sign to
						// negative instead of changing signs of velocities the angle of direction is reversed by 180.
						}
					}
				}
			}
			return collide;
		}
		
		bool Collision(Striker &other){ // Collision of one striker with opponent striker
			bool collide=false;
			
			for(int i=0;i<number;i++){     // striker1
				int incxs=round(speed[i]*cos(rad(angle[i]))),incys=round(speed[i]*sin(rad(angle[i])));
				for(int j=0;j<other.number;j++){ // striker2
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
						
						if(collisions>MAX_COL){
//							printf("%d %d, Diff\n",i,j);    // For debugging
							speed[(i+1)%number]+=speed[i];
							other.speed[(j+1)%number]+=other.speed[j];
							speed[i]=other.speed[j]=0;
						}
					}
				}
			}
			return collide;
		}
		
		void control(){
			arc_angle=360.0; // this is the time left for a player to move teh striker which is refilled each time new turn comes
			int n=-1; 		 // n represents which striker was chosen while inside the VK_LBUTTON was pressed
			_print();
			
			while(n==-1 && arc_angle>0.0){ // User shoots the striker or runs out of time
				while(!GetAsyncKeyState(VK_LBUTTON)){ // this loops runs while user doesn't press leftbutton to select some object
					if(GetAsyncKeyState(VK_ESCAPE)){
						while(GetAsyncKeyState(VK_ESCAPE)) delay(DELAY);
						run_game=pause();
						if(!run_game) return;
					}
					
					if(arc_angle<=0) break;
					Time(); // time has to be updated even inside loop
					_print();
					delay(DELAY);
				}
				
				GetCursorPos(&mouse);
				// Breaks out of loop if correct striker is left clicked at the proper turn. Otherwise loop continues running
				for(int i=0;i<number;i++){
					if((x[i]-mouse.x)*(x[i]-mouse.x)+(y[i]-mouse.y)*(y[i]-mouse.y)<=radius*radius){
						n=i;
						break;
					}
				}
				
				// If proper object is selected
				if(n!=-1){
					while(GetAsyncKeyState(VK_LBUTTON)){ // While the required object has been selected keep running the loop.
						if(arc_angle<=0) break;
						if(GetAsyncKeyState(VK_RBUTTON)){ // After one striker is selected for shooting one can change the
														  // choice of choosing another striker by clicking the right button.
							n=-1;
							break;
						}
						Time();
						_print(n);
						delay(DELAY);
					}
				}
				else{
					while(GetAsyncKeyState(VK_LBUTTON)){ // to accomodate wrong selected position of object
						if(arc_angle<=0) break;
						Time();
						_print();
						delay(DELAY);
					}
				}
				delay(DELAY);
			}

			if(n!=-1 && arc_angle>0){
				GetCursorPos(&mouse);

				double anglee=deg(atan2(mouse.y-y[n],x[n]-mouse.x)); // angle of striker projection to be determined from slope

				// distance of mouse release point from the centre of chosen striker
				double distance=(arc_angle>0)? ((sqrt(pow(mouse.x-x[n],2)+pow(mouse.y-y[n],2)))):0;
				// if arc_angle is less than speed_striker will be 0 due to time runout
				angle[n]=anglee;
				speed[n]=distance/10.0; 				  // set striker speed accordingly
				speed[n]=min(speed[n],max_speed); // limit themax striker speed
				if(speed[n]) movement=speed[n];           // Set to non-zero movement
			}
			turn=3-turn;
		}
}striker1,striker2;

void reset(){ // Reset everything to initial values after goal
	ball.reset();
	striker1.reset(1);
	striker2.reset(2);
}

class Goal{
	public:
		const int width=90*Xratio;
		const int height=125*Yratio;

}goal;

void Full_Reset(){ // Restarts whole game
	reset();
	Timer=0;
}

void Ball::move(){
	int incx=round(speed*cos(rad(angle))); // X component of speed of ball
	int incy=round(speed*sin(rad(angle))); // Y component of speed of ball
	x+=incx; // increment in X coordinate of ball
	y-=incy; // increment in Y coordinate of ball
	
	x=max(x,X1+radius);
	x=min(x,X2-radius);
	y=max(y,Y1+radius);
	y=min(y,Y2-radius);

	speed-=loss*DELAY/ms; // Frictional speed loss of ball per frame
	speed=max(speed,0.0);
	movement=speed;

	boundary();
}

void Ball::boundary(){
	// Collision with left,right and top,down boundaries
	if(x<=(radius+X1) || (x+radius)>=X2) angle=fmod(fmod(180.0-angle,360.0)+360.0,360.0);
	if(y<=(radius+Y1) || (y+radius)>=Y2) angle=fmod(fmod(360.0-angle,360.0)+360.0,360.0);
}

bool check_collision(){ // this checks if moving the ball will cause a collison in the next iteration. If YES, chagne angle accordingly.
	bool s1 = striker1.Collision();
	bool s2 = striker2.Collision();
	bool s12= striker1.Collision(striker2);
	return s1 || s2 || s12;
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
	outtextxy(width/4-100,100,c1);//prints text for player 1 to choose color
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

void text(){//prints texts while game is running
	char g1[3]="00";
	char g2[3]="00";
	char c1[]="TIME",c2[]="PLAYER 1",c3[]="PLAYER 2",c4[]="Press Escape",c5[]="for Menu",c6[]="Press Escape",c7[]="for Menu";
		g1[1]='0';
		g1[0]='0';
		g2[1]='0';
		g2[0]='0';
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
	setfillstyle(4,BLACK);//bleachers
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

void pause_screen(int option){
	setactivepage(page);
	setvisualpage(1-page);
	switch(option){
		case 1:
			readimagefile("resources/pause_screen_resume.jpg",(X1+X2)/2-200*Xratio,(Y1+Y2)/2-200*Yratio,(X1+X2)/2+200*Xratio,(Y1+Y2)/2+200*Yratio);
			break;
		case 2:
			readimagefile("resources/pause_screen_reset.jpg",(X1+X2)/2-200*Xratio,(Y1+Y2)/2-200*Yratio,(X1+X2)/2+200*Xratio,(Y1+Y2)/2+200*Yratio);
			break;
		case 3:
			readimagefile("resources/pause_screen_settings.jpg",(X1+X2)/2-200*Xratio,(Y1+Y2)/2-200*Yratio,(X1+X2)/2+200*Xratio,(Y1+Y2)/2+200*Yratio);
			break;
		case 4:
			readimagefile("resources/pause_screen_quit.jpg",(X1+X2)/2-200*Xratio,(Y1+Y2)/2-200*Yratio,(X1+X2)/2+200*Xratio,(Y1+Y2)/2+200*Yratio);
			break;
		default:
			readimagefile("resources/pause_screen_resume.jpg",(X1+X2)/2-200*Xratio,(Y1+Y2)/2-200*Yratio,(X1+X2)/2+200*Xratio,(Y1+Y2)/2+200*Yratio);
	}
	page=1-page;
}

bool pause(){
	int option=1;
	while(true){
		if(GetAsyncKeyState(VK_UP)){
			option--;
			if(option<1) option=4;
		}
		else if(GetAsyncKeyState(VK_DOWN)){
			option++;
			if(option>4) option=1;
		}
		else if(GetAsyncKeyState(VK_RETURN)){
			switch(option){
				case 1:
					return true;
				case 2:
					Full_Reset();
					return true;
				case 3:
					page=1-page;
					settings();
					return true;
				case 4:
					return false;
				default:
					return true;
			}
		}
		else if(GetAsyncKeyState(VK_ESCAPE)){
			while(GetAsyncKeyState(VK_ESCAPE));
			return true;
		}
		pause_screen(option);
		delay(100);
	}
}

void start(){
	srand(time(0)); // generates a random number to determine who will start the game
	turn=1+rand()%2;
	time(&ti);

	while(run_game){
		if(check_collision()){
			collisions++;
			continue;
		}
		collisions=0;
		
		time(&tf);
		if(ti!=tf){
			ti=tf;
			Timer++;
		}
		_print();
		if(movement==0){ // Only initiated when all are stagnant
			if(turn==1) striker1.control();
			else striker2.control();
		}
		ball.move();
		striker1.move();
		striker2.move();
		
		if(GetAsyncKeyState(VK_ESCAPE)){
			while(GetAsyncKeyState(VK_ESCAPE)) delay(DELAY);
			run_game=pause();
		}
		delay(DELAY);
	}
}

int main(){
	height-=20;
	width-=10;

	X1=X+goal.width; 	 Y1=Y;
	X2=width-goal.width; Y2=height;
	Full_Reset();
	
	initwindow(X_full,Y_full,"",-3,-3);
	cleardevice();
	readimagefile("resources/Soccer_stars_start.jpg",0,0,X_full,Y_full);
	while(!GetAsyncKeyState(VK_RETURN)) delay(DELAY); // Press Enter to start the game
	
	cleardevice();
	setbkcolor(7);
	settings();
	start();
	
	getch();
    closegraph();
	return 0;
}
