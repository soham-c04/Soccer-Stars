#include "Constants.h"
#include "offline.h"
#include "Online/online.h"
using namespace std;

void gamemode_screen(int option){
	switch(option){
		case 1:
			readimagefile("resources/gamemode_offline.jpg",350*Xratio,50*Yratio,X_full-350*Xratio,Y_full-50*Yratio);
			break;
		case 2:
			readimagefile("resources/gamemode_online.jpg",350*Xratio,50*Yratio,X_full-350*Xratio,Y_full-50*Yratio);
			break;
		case 3:
			readimagefile("resources/gamemode_quit.jpg",350*Xratio,50*Yratio,X_full-350*Xratio,Y_full-50*Yratio);
			break;
		default:
			readimagefile("resources/gamemode_offline.jpg",350*Xratio,50*Yratio,X_full-350*Xratio,Y_full-50*Yratio);
	}
}

int gamemode(){
	while(GetAsyncKeyState(VK_RETURN)) delay(DELAY);
	
	setactivepage(0);
	setvisualpage(0);
	cleardevice();
	int option=1;
	while(true){
		if(GetAsyncKeyState(VK_UP)){
			option--;
			if(option<1) option=3;
		}
		else if(GetAsyncKeyState(VK_DOWN)){
			option++;
			if(option>3) option=1;
		}
		else if(GetAsyncKeyState(VK_RETURN)){
			while(GetAsyncKeyState(VK_RETURN)) delay(DELAY);
			break;
		}
		gamemode_screen(option);
		delay(100);
	}
	return option;
}

int main(){
	height-=20;
	width-=10;

	initwindow(X_full,Y_full,"",-3,-3);
	cleardevice();
	readimagefile("resources/Soccer_stars_start.jpg",0,0,X_full,Y_full);
	while(!GetAsyncKeyState(VK_RETURN)) delay(DELAY*10); // Press Enter to start the game
	while(GetAsyncKeyState(VK_RETURN)) delay(DELAY*10);; // Stop Enter press

	cleardevice();
	setbkcolor(7);
	
	while(true){
		int option = gamemode();
			
		if(option == 1)
			main_offline();
		else if(option == 2)
			main_online();
		else
			break;
	}

	getch();
    closegraph();
	return 0;
}
