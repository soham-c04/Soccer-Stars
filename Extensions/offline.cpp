#include "Constants.h"
#include "offline.h"
#include <unistd.h>
#include <sys/time.h>
using namespace std;

int Timer_off=90;	       // Time (in sec) since the start of game
time_t ti_off, tf_off; 	   // ti represents the previously stored time of system when the time changed and tf is the current time of system.
POINT mouse_off; 		   // struct for declaring coordinates of mouse_off
bool run_game_off=true;    // breaks out of game or quits the game on selecting quit option;
int page_off=0;            // Double buffering

void Time_off(){ // time function which updates the time displayed in seconds in the screen.
	time(&tf_off);
	if(ti_off != tf_off){
		ti_off = tf_off;
		Timer_off--;
	}
}

int turn_off=0;  		// turn_off=1 => Player1 and turn_off=2 => Player2
double movement_off=0; 	// sum of speeds of ball_off + all strikers. If it is 0 that means all objects have stopped moving value of turn_off is swapped
int moves_off=0;	 	// Counts the number of moves_off played. Ensures that the first shot is not a goal_off

int collisions_off=0;
/*
"collisions_off" is a debuggging variale which stores the number of collisions_off occuring between any 2 objects after a
the ball_off has been shot. Which is reset to 0 after each time movement_off becomes 0. Collisions are used in debugging tangential
collisions_off bcecause the game gets stuck there due to infinitely many collisions_off occuring. If the collision count reaches>30
that means tangential collisions_off are occuring the objects causing tangential collisions_off are abruptly set to 0 to prevent
stoppage_off of game. This works because in almost all other cases collisions_off are less than MAX_COL in a single shot.
*/

void _print_off(int n=-1); // Prints everything
bool pause_off();  // Shows pause menu and screen

class Ball{
	private:
		const double loss=8.0;	// Reduction in speed of ball_off /sec
	public:
		const double mass=1.0;  // Mass of ball_off - used in collision physics
		const int radius=20;   	// Radius of ball_off
		int x,y; 			  	// Coords of ball_off
		double speed=0.0;      	// Speed of ball_off (slowed when goal_off occurs)
		double angle=90.0; 	   	// Angle at which the ball_off is currently moving w.r.t +ve x-axis anticlockwise

		void reset_off(){ 	   	// After scoring a goal_off reset back to center
			speed=0.0;		    // Reset ball_off speed to initial value
			x=(X2+X1)/2;  	    // reset ball_off X coordinate to middle of playable field
			y=(Y2+Y1)/2;        // reset ball_off Y coordinate to middle of playable field
			angle=90.0;         // moves_off ball_off vertically next round so both players get fair chance to score
		}

		void move_off(bool Goaling=false);  // Goaling check goal_off condition
		void boundary_off();				// Checks for boundary collisions_off and goal_offs
}ball_off;

class Striker{
	private:
		static const int number=5; 	 // # of strikers for each player
		const int timeout=5;         // Time (in sec) after which turn_off goes to other player.
		const double mass=6.0;       // Mass of striker - used in collision physics
		const double max_speed=15.0; // max. Speed of striker when shooting
		const double loss=6.0;       // Reduction in speed of striker /sec
		int x[number],y[number]; 	 // x and y coords of striker[i]
		double speed[number]; 		 // cur. speed of striker[i]
		double angle[number]; 		 // cur. angle striker[i] is travelling in
		double arc_angle=360.0; 	 // Arc angle is the constant timer around the striker, which is rorating and decreasing over time.
		// When it reaches 0 that means that the timer for the player is over and reaches a timeout! Turn is passed to next player.
	public:
		static const int radius=40; 		 // Radius of striker
		int color; 	  				 // Color of strikers

		void reset_off(int who){ 	 // After scoring a goal_off reset back to initial Formation
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

		void print_off(int who,int n){
			setlinestyle(0,0,1);
			setcolor(15);
			setfillstyle(SOLID_FILL,color);
			for(int i=0;i<number;i++){
				circle(x[i],y[i],radius);
				floodfill(x[i],y[i],15);
			}

			if(movement_off==0){ // Timer_off arc around the ball_off
		    	// Depending on whose turn_off it is, an arc around all the strikers of the current shooter are drawn with pink
				if(turn_off==who){
			    	setlinestyle(0,0,5); // 5 pixel thick line arc around the strikers
			    	setcolor(13);

			    	for(int i=0;i<number;i++) arc(x[i],y[i],0,arc_angle,radius+4);

			    	// The size of this arc decreases with time. The length of arc denotes the time the player has to shoot the ball_off.
					arc_angle-=(360.0*DELAY/(timeout*ms))*2; // Extra factor of 2 to account for processing delays

					// Mouse pullback
					if(n!=-1){ // Here n denotes the striker from whose center the mouse_off was pulled to shoot
						GetCursorPos(&mouse_off); // Gets the current location/position of the mouse_off.
						int releasepos_x=mouse_off.x,releasepos_y=mouse_off.y;
						double anglee=deg(atan2(releasepos_y-y[n],x[n]-releasepos_x)); // anglee is the angle which the pointer of mouse_off is making with the
						// center of striker from whom the left button is pressed. Slope formula is used
						double distance=round(sqrt(pow(releasepos_x-x[n],2)+pow(releasepos_y-y[n],2))); // distance of centre of striker to mouse_off position
						// this distance is also in ratio to the speed of the striker.
						distance=min(distance,max_speed*10.0); // Since the distance is in ratio to the speed of striker and the speed of striker
						// has a maximum limit hence the distance also has a maximum limit.

						anglee=rad(anglee); // converting angle into radian to be used in sin and cos functions
						setcolor(YELLOW);
						if(distance>radius){
							// YELLOW line is created if the mouse_off is at least outside the striker radius. It starts at the boundary of radius
							// YELLOW  line denoting the vector along which the striker will be projected, if the mouse_off is released form that position
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
							// mouse_off
							line(x[n]-round(radius*cos(anglee)),y[n]+round(radius*sin(anglee)),x[n]-round((radius+distance/3)*cos(anglee)),y[n]+round((distance/3+radius)*sin(anglee)));
						}
						else{ // if the mouse_off coordinates are inside the striker_radius
							line(x[n],y[n],x[n]+round(distance*cos(anglee)),y[n]-round(distance*sin(anglee)));
						}
					}
				}
			}
		}

		void control_off(){
			arc_angle=360.0; // this is the time left for a player to move teh striker which is refilled each time new turn_off comes
			int n=-1; 		 // n represents which striker was chosen while inside the VK_LBUTTON was pressed
			_print_off();

			while(n==-1 && arc_angle>0.0){ // User shoots the striker or runs out of time
				while(!GetAsyncKeyState(VK_LBUTTON)){ // this loops runs while user doesn't press leftbutton to select some object
					if(GetAsyncKeyState(VK_ESCAPE)){
						while(GetAsyncKeyState(VK_ESCAPE)) delay(DELAY);
						run_game_off=pause_off();
						if(!run_game_off) return;
					}

					if(Timer_off < 0) return;
					if(arc_angle<=0) break;
					Time_off(); // time has to be updated even inside loop
					_print_off();
					delay(DELAY);
				}

				GetCursorPos(&mouse_off);
				// Breaks out of loop if correct striker is left clicked at the proper turn_off. Otherwise loop continues running
				for(int i=0;i<number;i++){
					if((x[i]-mouse_off.x)*(x[i]-mouse_off.x)+(y[i]-mouse_off.y)*(y[i]-mouse_off.y)<=radius*radius){
						n=i;
						break;
					}
				}

				// If proper object is selected
				if(n!=-1){
					while(GetAsyncKeyState(VK_LBUTTON)){  // While the required object has been selected keep running the loop.
						if(Timer_off < 0) return;
						if(arc_angle<=0) break;
						if(GetAsyncKeyState(VK_RBUTTON)){ // After one striker is selected for shooting one can change the
														  // choice of choosing another striker by clicking the right button.
							n=-1;
							break;
						}
						Time_off();
						_print_off(n);
						delay(DELAY);
					}
				}
				else{
					while(GetAsyncKeyState(VK_LBUTTON)){ // to accomodate wrong selected position of object
						if(Timer_off < 0) return;
						if(arc_angle<=0) break;
						Time_off();
						_print_off();
						delay(DELAY);
					}
				}
				delay(DELAY);
			}

			if(n!=-1 && arc_angle>0){
				GetCursorPos(&mouse_off);

				double anglee=deg(atan2(mouse_off.y-y[n],x[n]-mouse_off.x)); // angle of striker projection to be determined from slope

				// distance of mouse_off release point from the centre of chosen striker
				double distance=(arc_angle>0)? ((sqrt(pow(mouse_off.x-x[n],2)+pow(mouse_off.y-y[n],2)))):0;
				distance = min(distance, max_speed*10);
				// if arc_angle is less than speed_striker will be 0 due to time runout
				angle[n]=anglee;
				speed[n]=distance/10.0; 				  // set striker speed accordingly
				if(speed[n]) movement_off=speed[n];       // Set to non-zero movement_off
			}
			turn_off=3-turn_off;
		}

		void boundary_off(){ // Striker rebound on field edges
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

		void move_off(){ // Checks is goal_off has occured
			for(int i=0;i<number;i++){ // same process is repeated for all strikers.
				int incx=round(speed[i]*cos(rad(angle[i])));
				int incy=round(speed[i]*sin(rad(angle[i])));
				x[i]+=incx;
				y[i]-=incy;

				speed[i]-=loss*DELAY/ms; // Frictional speed loss of striker per second
				speed[i]=max(speed[i],0.0);
				movement_off+=speed[i];

				boundary_off();
			}
		}

		bool Collision_off(){ // Collision of Striker with ball_off and Same side strikers
			bool collide=false; // Checks if at least one collision has happened

			for(int i=0;i<number;i++){
				// These are the distances travelled by the x and y component of ball_off and striker in the next iteration at that angle.
				int incxb=round(ball_off.speed*cos(rad(ball_off.angle))),incyb=round(ball_off.speed*sin(rad(ball_off.angle)));
				int incxs=round(speed[i]*cos(rad(angle[i]))),incys=round(speed[i]*sin(rad(angle[i])));
				if((pow((x[i]+incxs)-(ball_off.x+incxb),2)+pow((y[i]-incys)-(ball_off.y-incyb),2))<=(radius+ball_off.radius)*(radius+ball_off.radius)){
					collide=true; // Collision happens
					// theta is also the slope of line of approach.Calculating slope of line and converting that to degree from tan inverse
					double theta=deg(atan2(ball_off.y-y[i],x[i]-ball_off.x)); // Theta is the angle of line joining centres of striker and ball_off
					// with +X axis measured in anticlockwise direction. Also the direction of theta points from the centre of striker to ball_off
//
					double vbi_parallel=ball_off.speed*cos(rad(ball_off.angle-theta)),vb_perp=ball_off.speed*sin(rad(ball_off.angle-theta));
					// initial parallel  and perpendicular component of striker
					double vsi_parallel=speed[i]*cos(rad(angle[i]-theta)),vs_perp=speed[i]*sin(rad(angle[i]-theta));
					// Final parallel component of ball_off
					double vbf_parallel=((1+e)*mass*vsi_parallel+vbi_parallel*(ball_off.mass-e*mass))/(ball_off.mass+mass);
					// Final parallel component of striker
					double vsf_parallel=((1+e)*ball_off.mass*vbi_parallel+vsi_parallel*(mass-e*ball_off.mass))/(ball_off.mass+mass);
					// Perpendicular component of velocities from line of approach will not change.
					ball_off.angle=theta+deg(atan2(vb_perp,vbf_parallel)); // new ball_off angle after collision
					angle[i]=theta+deg(atan2(vs_perp,vsf_parallel)); // new striker angle after collision
					ball_off.speed=sqrt(pow(vbf_parallel,2)+pow(vb_perp,2)); // new ball_off.speed after collision
					speed[i]=sqrt(pow(vsf_parallel,2)+pow(vs_perp,2)); // new striker_speed after collision

					if(collisions_off>MAX_COL){
//						printf("%d, Ball\n",i);     // For debugging
						speed[(i+1)%number]+=ball_off.speed;
						speed[(i+2)%number]+=speed[i];
						ball_off.speed=speed[i]=0; // more than MAX_COL collision means tangential collisions_off and in such cases
											   // the velocities of elemnts causing it is to be set to 0
					// We are always taking the magnitude of ball_off and striker velocity. So if the parallel component changes sign to
					// negative instead of changing signs of velocities the angle of direction is reversed by 180.
					}
				}

				for(int j=i+1;j<number;j++){ // striker1_off collision with itself
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

						if(collisions_off>MAX_COL){
//							printf("%d %d, Same\n",i,j);    // For debugging
							if(j-i>2){
								speed[i+1]+=speed[i];
								speed[i+2]+=speed[j];
							}
							else{
								speed[(j+1)%number]+=speed[i];
								speed[(j+2)%number]+=speed[j];
							}
							speed[i]=speed[j]=0; // more than 10 collision means tangential collisions_off and in such cases
												   // the velocities of elemnts causing it is to be set to 0
						// We are always taking the magnitude of ball_off and striker velocity. So if the parallel component changes sign to
						// negative instead of changing signs of velocities the angle of direction is reversed by 180.
						}
					}
				}
			}
			return collide;
		}

		bool Collision_off(Striker &other){ // Collision of one striker with opponent striker
			bool collide=false;

			for(int i=0;i<number;i++){     // striker1_off
				int incxs=round(speed[i]*cos(rad(angle[i]))),incys=round(speed[i]*sin(rad(angle[i])));
				for(int j=0;j<other.number;j++){ // striker2_off
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

						if(collisions_off>MAX_COL){
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
		
}striker1_off,striker2_off;

void reset_off(){ // Reset everything to initial values after goal_off
	ball_off.reset_off();
	striker1_off.reset_off(1);
	striker2_off.reset_off(2);
	moves_off=0;
	collisions_off=0;
}

struct Goal{
	int width, height;
	int goal_off_1=0,goal_off_2=0;  // goal_off counter for first and second player respectively

	void goaling_off(int who){ // This is initiated whenever a goal_off happens. Here who defines whoever accepted the goal_off
		turn_off=who;
		ball_off.speed=3.0;
		if(who==1){
			if(moves_off>1) goal_off_2++; // Ensures no goal_off on first strike
			while(ball_off.x>X1-2*width/3 && (ball_off.y>=(Y2+Y1)/2-height+ball_off.radius) && (ball_off.y<=(Y2+Y1)/2+height-ball_off.radius)){
				/*
				THe 1st condition of while loop condition defines the amount of distnace ball_off with travel before stopping inside the goal_off
				The 2nd and 3rd define that the ball_off should not cross the upper and lower boundaries of goal_off while travelling to the
				required distance inside the goal_off i.e. 2/3 * width_goal_off
				*/
				_print_off(); // print the new position of ball_off inside goal_off
				ball_off.move_off(true);
				striker1_off.move_off();
				striker2_off.move_off();
				delay(DELAY);
			}
		}
		else{ // Same process as above but this time when player 2 accepts goal_off
			if(moves_off>1) goal_off_1++;
			while(ball_off.x<X2+2*width/3 && (ball_off.y>=(Y2+Y1)/2-height+ball_off.radius) && (ball_off.y<=(Y2+Y1)/2+height-ball_off.radius)){
				_print_off();
				ball_off.move_off(true);
				striker1_off.move_off();
				striker2_off.move_off();
				delay(DELAY);
			}
		}

		setvisualpage(1-page_off); // since in the above while loop in double buffering the visual and active page_offs were being constantly
		// swapped. To set visual page_off back to the newly printed one this is done.
		// GOAL !! is printed for 2 seconds in the middle of the ground for 2 seconds after all process of ball_off movement_off inside goal_off has
		//occured
		if(moves_off>1)
			readimagefile("resources/goal.jpg",(X1+X2)/2-150*Xratio,(Y1+Y2)/2-75*Yratio,(X1+X2)/2+150*Xratio,(Y1+Y2)/2+75*Yratio);
		else{
			setcolor(13);
			settextstyle(0,0,5);
			char c[]="FOUL !!"; // if goal_off is done in first move it will me considered a foul
			outtextxy((X1+X2)/2-120*Xratio,(Y1+Y2)/2-50*Yratio,c);
			settextstyle(4,HORIZ_DIR,3*Xratio);
		}

		reset_off();

		sleep(2); // play goal_off/foul text for 2 seconds
	}
}goal_off;

void Full_Reset_off(){ // Restarts whole game
	reset_off();
	run_game_off = true;
	goal_off.goal_off_1=0;
	goal_off.goal_off_2=0;
	Timer_off=90;
}

void Ball::move_off(bool Goaling){
	int incx=round(speed*cos(rad(angle))); // X component of speed of ball_off
	int incy=round(speed*sin(rad(angle))); // Y component of speed of ball_off
	x+=incx; // increment in X coordinate of ball_off
	y-=incy; // increment in Y coordinate of ball_off
	if(!Goaling){
		if(y<((Y2+Y1)/2-goal_off.height+radius) || y>((Y2+Y1)/2+goal_off.height-radius)){
			x=max(x,X1+radius);
			x=min(x,X2-radius);
		}
		y=max(y,Y1+radius);
		y=min(y,Y2-radius);

		speed-=loss*DELAY/ms; // Frictional speed loss of ball_off per frame
		speed=max(speed,0.0);
		movement_off=speed;

		boundary_off();
	}
}

void Ball::boundary_off(){
	if(x<=(radius+X1) && y>=((Y2+Y1)/2-goal_off.height+radius) && y<=((Y2+Y1)/2+goal_off.height-radius)){
		goal_off.goaling_off(1); // Ball in left goal_off
		return;
	}
	else if((x+radius)>=X2 && y>=((Y2+Y1)/2-goal_off.height+radius) && y<=((Y2+Y1)/2+goal_off.height-radius)){
		goal_off.goaling_off(2); // Ball in right goal_off
		return;
	}
	// Collision with left,right and top,down boundaries
	if(x<=(radius+X1) || (x+radius)>=X2) angle=fmod(fmod(180.0-angle,360.0)+360.0,360.0);
	if(y<=(radius+Y1) || (y+radius)>=Y2) angle=fmod(fmod(360.0-angle,360.0)+360.0,360.0);
}

/*
Check_Collision() boolean type function was defined instead of just a normal collisions_off because in normal collisions_off function
only one collision at a time can occur in the next iteration this can cause overlapping of objects.
This can be prevented using check_collision() integer type.
Check out more about it in "../resources/Soccer Stars Debug"
*/
bool check_collision_off(){ // this checks if moving the ball_off will cause a collison in the next iteration. If YES, chagne angle accordingly.
	bool s1 = striker1_off.Collision_off();
	bool s2 = striker2_off.Collision_off();
	bool s12= striker1_off.Collision_off(striker2_off);
	return s1 || s2 || s12;
}

void print_movable_off(int n){
	striker1_off.print_off(1,n);
	striker2_off.print_off(2,n);
	setlinestyle(0,0,1);
	setcolor(15);
	circle(ball_off.x,ball_off.y,ball_off.radius);
	setfillstyle(SOLID_FILL,14);
	floodfill(ball_off.x,ball_off.y,15);
}

void settings_off(){
	setactivepage(page_off);
 	setvisualpage(page_off);
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
	circle(width/4+75,height/2+25,striker1_off.radius*2.5);//prints first circle
	floodfill(width/4+75,height/2+25,WHITE);//fills circle 1 with first color choice
	circle(3*width/4-75,height/2+25,striker1_off.radius*2.5);//prints second circle
	setfillstyle(SOLID_FILL,3);//color choice 2
	floodfill(3*width/4-75,height/2+25,WHITE);//fills circle 2 with second color choice
	setvisualpage(page_off);
	setactivepage(1-page_off);
	page_off=1-page_off;

	while(true){
		if(GetAsyncKeyState('1')){
			striker1_off.color=1; // assigns color choice 1 to striker 1
			break;
		}
		else if(GetAsyncKeyState('2')){
			striker1_off.color=3; // assigns color choice 2 to striker 1
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
	circle(width/4+75,height/2+25,striker1_off.radius*2.5);//prints first circle
	floodfill(width/4+75,height/2+25,WHITE);//fills circle 1 with first color choice
	circle(3*width/4-75,height/2+25,striker1_off.radius*2.5);//prints second cirlce
	setfillstyle(SOLID_FILL,5);//color choice 2
	floodfill(3*width/4-75,height/2+25,WHITE);//fills circle 2 with second color choice
	setvisualpage(page_off);
	setactivepage(1-page_off);
	page_off=1-page_off;

	while(true){
		if(GetAsyncKeyState('1')){
			striker2_off.color=4; // assigns color choice 1 to striker 2
			break;
		}
		else if(GetAsyncKeyState('2')){
			striker2_off.color=5; // assigns color choice 2 to striker 2
			break;
		}
		delay(DELAY);
	}
	while(GetAsyncKeyState('1') || GetAsyncKeyState('2')) delay(DELAY); //to avoid misckicking of keys
	settextstyle(4,HORIZ_DIR,3*Xratio);
}

void text_off(){//prints texts while game is running
	char g1[3]="00";
	char g2[3]="00";
	char c1[]="TIME LEFT",c2[]="PLAYER 1",c3[]="PLAYER 2",c4[]="Press Escape",c5[]="for Menu",c6[]="Press Escape",c7[]="for Menu";
		g1[1]=(char)(goal_off.goal_off_1%10+'0');
		g1[0]=(char)(goal_off.goal_off_1/10+'0');
		g2[1]=(char)(goal_off.goal_off_2%10+'0');
		g2[0]=(char)(goal_off.goal_off_2/10+'0');
	outtextxy((X1+X2)/2-108*Xratio-40*Xratio,30*Yratio,g1);
	outtextxy((X1+X2)/2+70*Xratio+45*Xratio,30*Yratio,g2);
	char t[4];
	sprintf(t,"%d",Timer_off);
	int tme=Timer_off,sz=0;
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

void color_off(){
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

void print_field_off(){
	cleardevice();
	setcolor(BLACK);
	setlinestyle(0,0,3);
	//making stadium
		rectangle(X,Y,width,height);
		rectangle(X,(Y+height)/2-goal_off.height,X+goal_off.width,(Y+height)/2+goal_off.height);//making left goal_off
		rectangle(width-goal_off.width,(Y+height)/2-goal_off.height,width,(Y+height)/2+goal_off.height);//making right goal_off
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

void _print_off(int n){
	setactivepage(page_off);
	setvisualpage(1-page_off);
	print_field_off();
	color_off();
	text_off();
	print_movable_off(n);
	page_off=1-page_off;
}

void pause_screen_off(int option){
	setactivepage(page_off);
	setvisualpage(1-page_off);
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
	page_off=1-page_off;
}

bool pause_off(){
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
					Full_Reset_off();
					return true;
				case 3:
					page_off=1-page_off;
					settings_off();
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
		pause_screen_off(option);
		delay(100);
	}
}

void start_off(){
	srand(time(0)); // generates a random number to determine who will start the game
	turn_off=1+rand()%2;
	time(&ti_off);

	while(run_game_off && Timer_off >= 0){
		if(check_collision_off()){
			collisions_off++;
			continue;
		}
		collisions_off=0;
		Time_off();
		_print_off();
		if(movement_off==0){ // Only initiated when all are stagnant
			moves_off++;
			if(turn_off==1) striker1_off.control_off();
			else striker2_off.control_off();
		}
		ball_off.move_off();
		striker1_off.move_off();
		striker2_off.move_off();

		if(GetAsyncKeyState(VK_ESCAPE)){
			while(GetAsyncKeyState(VK_ESCAPE)) delay(DELAY);
			run_game_off=pause_off();
		}
		delay(DELAY);
	}

	setvisualpage(0);
	setactivepage(0);
	if(goal_off.goal_off_1 > goal_off.goal_off_2){
		readimagefile("resources/player1_wins.jpg",(X1+X2)/2-250*Xratio,(Y1+Y2)/2-250*Yratio,(X1+X2)/2+350*Xratio,(Y1+Y2)/2+150*Yratio);
	}
	else if(goal_off.goal_off_1 < goal_off.goal_off_2){
		readimagefile("resources/player2_wins.jpg",(X1+X2)/2-250*Xratio,(Y1+Y2)/2-250*Yratio,(X1+X2)/2+350*Xratio,(Y1+Y2)/2+150*Yratio);
	}
	else{
		readimagefile("resources/draw.jpg",(X1+X2)/2-250*Xratio,(Y1+Y2)/2-150*Yratio,(X1+X2)/2+350*Xratio,(Y1+Y2)/2+150*Yratio);
	}
	sleep(4);
}

void main_offline(){
	goal_off.width = 90*Xratio;
	goal_off.height= 125*Yratio;
	
	X1=X+goal_off.width; 	 Y1=Y;
	X2=width-goal_off.width; Y2=height;

	Full_Reset_off();
	settings_off();
	start_off();
}
