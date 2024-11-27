#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>


#define CENTERX (LINES/2 -2)
#define CENTERY (COLS/2 -2)
#define ROWS 41
#define COLUMNS 100
#define FROGHEIGHT 3
#define FROGWIDTH 3
#define CARWIDTH 6
#define CARHEIGHT 3




void Welcome(WINDOW* win)							// Welcome screen (optional): press any key to continue
{
	mvwaddstr(win, 1, 1, "Do you want to play a game?");
	mvwaddstr(win, 2, 1, "Press any key to continue..");
	wgetch(win);								
	wclear(win);								
	wrefresh(win);
}
struct car
{
	int alive;
	struct timespec speed;
	int x;
	int y;
	int direction;
};
int drivingCar(struct car current){
	nanosleep(&current.speed, NULL);
	if (current.y+CARWIDTH+current.direction>=COLUMNS-3 || current.y+current.direction<=4){
		return 1;
	}
	delCar(current.x, current.y);
	drawCar(current.x, current.y+=current.direction);
	return 0;

}
void drawCar(int x, int y){
	move(x,y);
	addstr(" ____ "); move(x+1,y);
	addstr("(____)"); move(x+2,y);
	addstr(" O  O ");
}
struct car newCar(int i){
	struct car new={1, {0,(rand()%5 + 1)*20000000}, 1+4*i, 20,  ((rand()%2)*(-2)+1)*(rand()%3+1)};
	return new;
}
void drawFrog(int x, int y){
	move(x,y);
	addstr("0 0"); move(x+1,y);
	addstr("###"); move(x+2,y);
	addstr("! !");
}
void delFrog(int x, int y){
	move(x,y);
	addstr("   "); move(x+1,y);
	addstr("   "); move(x+2,y);
	addstr("   ");
}
void delCar(int x, int y){
	move(x,y);
	addstr("      "); move(x+1,y);
	addstr("      "); move(x+2,y);
	addstr("      ");
}
int isCollision(int x1, int x2, int y1, int y2){
	return (x1 == x2 && abs(y2-y1)<3);
	
}
void DrawBoard(){
	for(int i = 0; i<ROWS; i++){
		 move(i,0); 
		 addstr("|");
	}
	for(int i = 0; i<ROWS; i++){
		 move(i,COLUMNS-1); 
		 addstr("|");
	}
	for(int i = 0; i<COLUMNS; i++){
		 move(0,i); 
		 addstr("=");
	}
	for(int i = 0; i<COLUMNS; i++){
		 move(ROWS-1,i); 
		 addstr("=");
	}
	for(int j = 4; j<ROWS-4; j+=4){
		for(int i = 1; i<COLUMNS-1; i++){
			move(j,i); 
			addstr("-");
		}
	}
	
	
}
void mainLoop(int ch, int x, int y, struct timespec sleeptime, WINDOW *win){
	struct car cars [9];
	for(int i=0; i<9; i++) {
		cars[i] = newCar(i);
	}
	int end = 0;
	int alive = 1;
	int score = 0;
	do{
		ch =wgetch(win);

		if(ch == KEY_F(1)){
			end = 1;
		}
		if (alive){
				switch (ch)
			{
			case KEY_LEFT:
				if(y-FROGWIDTH>=1 && y-FROGWIDTH<=COLUMNS-1){
					delFrog(x, y);
					y-=FROGWIDTH;
					drawFrog(x, y);
					flushinp(); 
					nanosleep(&sleeptime, NULL);
				}
				break;
			case KEY_RIGHT:
				if(y+FROGWIDTH>=1 && y+FROGWIDTH<=COLUMNS-1){
					delFrog(x, y);
					y+=FROGWIDTH;
					drawFrog(x, y);
					flushinp();  
					nanosleep(&sleeptime, NULL);
				}
				break;
			case KEY_UP:
				if(x-FROGHEIGHT>=1 && x-FROGHEIGHT+1<=ROWS-1){
					delFrog(x, y);
					x-=FROGHEIGHT+1;
					drawFrog(x, y);
					flushinp();  
					nanosleep(&sleeptime, NULL);
				}
				break;
			case KEY_DOWN:
				if(x+FROGHEIGHT>=1 && x+FROGHEIGHT+1<=ROWS-1){
					delFrog(x, y);
					x+=FROGHEIGHT+1;
					drawFrog(x, y);
					flushinp();  
					nanosleep(&sleeptime, NULL);
				}
				break;
			
			}
			for(int i=0; i<9; i++) {
				if (drivingCar(cars[i]) == 1){
					cars[i].direction*=(-1);
				}else{
					cars[i].y+=cars[i].direction;
				}
				if(isCollision(x,cars[i].x, y, cars[i].y) ){
					alive=0;
					
					clear();
					move(ROWS/2,COLUMNS/2-4);
					addstr("GAME OVER");
					printw(
						score);
					getch();
					nanosleep(&sleeptime, NULL);
				}
			}
		}
		
		
		// flushinp();                     				 	 	

		refresh();
		
	}while (end == 0);
	


}

int main(){
	WINDOW *win = initscr();
    cbreak(); 
    keypad(stdscr, TRUE);
	curs_set(0);

	// WINDOW *win = newwin(ROWS, COLUMNS, 0, 0);
	nodelay(win, TRUE);
    Welcome(win);
	keypad(stdscr, TRUE);
	int ch;
	int x = ROWS-FROGHEIGHT - 1, y = COLUMNS/2;
	struct timespec sleeptime = {0,2000000};
	clear();
	DrawBoard();
	drawFrog(x, y);

	mainLoop(ch,  x,  y,   sleeptime, win);
	
	clear();
	endwin();
	return 0;
}