#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>


#define CENTERX (ROWS/2 -2)
#define CENTERY (COLUMNS/2 -2)
#define ROWS 41
#define COLUMNS 100
#define FROGHEIGHT 3
#define FROGWIDTH 3
#define CARWIDTH 6
#define CARHEIGHT 3
#define NUMBER_OF_CARS 10
#define NUMBER_OF_LOGS 3
#define NUMBER_OF_LEVELS 3
#define POINTS_IN_LEVEL 3


#define ROAD_COLOR      1
#define PAVEMENT_COLOR     2
#define FROG_COLOR       3
#define CAR_COLOR_ONE	4
#define CAR_COLOR_TAXI	5
#define CAR_COLOR_FRIENDLY	6
#define LOG_COLOR	7
#define TERMINAL_COLOR 8
#define STORK_COLOR 9
#define SCORE_COLOR 10

#define BORDER		1
#define DELAY_ON	1
#define DELAY_OFF	0
// STRUCTURES =============
// structure for cordints foe eg. frog stork logs
struct cords 
{
	int x;
	int y;
};
// structer for cars definition
struct car
{
	int alive;
	struct timespec speed;
	int x;
	int y;
	int direction;
	int color;
	int friendly;
	int driven_taxi; // 0-isn't driven else is driven
	int kill_frog; //0-didn't 1-did
};
// structure for rankings of best scores
struct ranking{
	int first_place;
	int second_place;
	int third_place;
};
//playable window structure
typedef struct{
	WINDOW* window;								// ncurses window
	int x, y;
	int rows, cols;
	int color;
}  WIN;	

// FUNCTIONS ==================
//Window functions
void Welcome(WINDOW* win)							// Welcome screen (optional): press any key to continue
{
	mvwaddstr(win, 1, 1, "Do you want to play a game?");
	mvwaddstr(win, 2, 1, "Press any key to continue..");
	wgetch(win);								
	wclear(win);								
	wrefresh(win);
}
WINDOW* Start()
{
	WINDOW* win;

	if ( (win = initscr()) == NULL ) {					// initialize ncurses
		fprintf(stderr, "Error initialising ncurses.\n");
		exit(EXIT_FAILURE);
    	}

	start_color();								// initialize colors
	init_pair(ROAD_COLOR, COLOR_BLACK, COLOR_WHITE);
	init_pair(PAVEMENT_COLOR, COLOR_WHITE, COLOR_YELLOW);
	init_pair(FROG_COLOR, COLOR_GREEN, COLOR_WHITE);
	init_pair(CAR_COLOR_ONE, COLOR_MAGENTA, COLOR_WHITE);
	init_pair(CAR_COLOR_TAXI, COLOR_RED, COLOR_WHITE);
	init_pair(CAR_COLOR_FRIENDLY, COLOR_YELLOW, COLOR_WHITE);
	init_pair(LOG_COLOR, COLOR_GREEN, COLOR_WHITE);
	init_pair(TERMINAL_COLOR, COLOR_BLACK, COLOR_WHITE);
	init_pair(STORK_COLOR, COLOR_WHITE, COLOR_RED);
	init_pair(SCORE_COLOR, COLOR_WHITE, COLOR_BLUE);

	noecho();								// Switch off echoing, turn off cursor
	curs_set(0);
	return win;
}
WIN* Init(WINDOW* parent, int rows, int cols, int y, int x, int color, int bo, int delay)
{
	WIN* W = (WIN*)malloc(sizeof(WIN));					// C version; compile with gcc
	W->x = x; W->y = y; W->rows = rows; W->cols = cols; W->color = color;
	W->window = subwin(parent, rows, cols, y, x);
	if (delay == DELAY_OFF)  nodelay(W->window,TRUE);							// non-blocking reading of characters (for real-time game)
	wrefresh(W->window);
	return W;
}

//function drawing board
void DrawBoard(WIN *playwin){
	for(int j = 1; j<ROWS-1; j++){
		for(int i = 1; i<COLUMNS-1; i++){
			wattron(playwin->window, COLOR_PAIR(ROAD_COLOR));
	wrefresh(playwin->window);
			mvwprintw(playwin->window, j, i, " ");
		}
	}
	for(int i = 0; i<ROWS; i++){
		wattron(playwin->window, COLOR_PAIR(PAVEMENT_COLOR));
	wrefresh(playwin->window);
		mvwprintw(playwin->window, i, 0, "|");
	}
	for(int i = 0; i<ROWS; i++){
		wattron(playwin->window, COLOR_PAIR(PAVEMENT_COLOR));
	wrefresh(playwin->window);
		mvwprintw(playwin->window, i, COLUMNS-1, "|");
	}
	for(int i = 0; i<COLUMNS; i++){
		wattron(playwin->window, COLOR_PAIR(PAVEMENT_COLOR));
	wrefresh(playwin->window);
		mvwprintw(playwin->window, 0, i, "=");
	}
	for(int i = 0; i<COLUMNS; i++){
		wattron(playwin->window, COLOR_PAIR(PAVEMENT_COLOR));
	wrefresh(playwin->window);
		mvwprintw(playwin->window,ROWS-1, i,  "=");
	}
	for(int j = 4; j<ROWS-4; j+=4){
		for(int i = 1; i<COLUMNS-1; i++){
			wattron(playwin->window, COLOR_PAIR(PAVEMENT_COLOR));
	wrefresh(playwin->window);
			mvwprintw(playwin->window, j, i, "-");
		}
	}
	
	
}

//Functions for menaging elements

//functions for frog
void drawFrog(int x, int y, WIN *playwin){
	wattron(playwin->window, COLOR_PAIR(FROG_COLOR));
	wrefresh(playwin->window);
	mvwprintw(playwin->window, x, y, "O O");
	mvwprintw(playwin->window, x+1, y, " # ");
	mvwprintw(playwin->window, x+2, y, "o o");
}
void delFrog(int x, int y, WIN *playwin){
	wattron(playwin->window, COLOR_PAIR(ROAD_COLOR));
	wrefresh(playwin->window);
	mvwprintw(playwin->window, x, y, "   ");
	mvwprintw(playwin->window, x+1, y, "   ");
	mvwprintw(playwin->window, x+2, y, "   ");
}

//functions for cars
void drawCar(int x, int y, int color,  WIN *playwin){

	wattron(playwin->window, COLOR_PAIR(color));
	wrefresh(playwin->window);
	mvwprintw(playwin->window, x, y, " ____ ");
	mvwprintw(playwin->window, x+1, y,"(____)");
	mvwprintw(playwin->window, x+2, y, " O  O ");
}
void delCar(int x, int y,  WIN *playwin){
	wattron(playwin->window, COLOR_PAIR(ROAD_COLOR));
	wrefresh(playwin->window);
	mvwprintw(playwin->window, x, y, "      ");
	mvwprintw(playwin->window, x+1, y, "      ");
	mvwprintw(playwin->window, x+2, y, "      ");
}
struct car newCar( int level){
	int x;
	if (rand()%2){
		x=5;
	}else{
		x=COLUMNS-10;
	}
	struct car new={1, {0,20000000/level}, 5+4*(1+rand()%7), x,  ((rand()%2)*(-2)+1)*(rand()%3+1), CAR_COLOR_ONE, rand()%5, 0, 0};
	if (new.friendly == 0){
		new.color = CAR_COLOR_FRIENDLY;
	}
	if (new.friendly == 1){
		new.color = CAR_COLOR_TAXI;
	}
	return new;
}
int drivingCar(struct car current, WIN *playwin, struct cords logs[], int number_of_logs){
	nanosleep(&current.speed, NULL);
	for(int i = 0; i<number_of_logs; i++){
		if (current.x == logs[i].y && current.y+CARWIDTH+current.direction>=logs[i].x && current.y+current.direction<=logs[i].x+5){
			return 1;
		}
	}
	if (current.y+CARWIDTH+current.direction>=COLUMNS-3 || current.y+current.direction<=4){
		return 1;
	}
	delCar(current.x, current.y, playwin);
	drawCar(current.x, current.y+=current.direction, current.color, playwin);
	return 0;

}
int drivingTaxi(struct car current, WIN *playwin, struct cords logs[],int x,int y,int number_of_logs){
	nanosleep(&current.speed, NULL);
	for(int i = 0; i<number_of_logs; i++){
		if (current.x == logs[i].y && current.y+CARWIDTH+current.direction>=logs[i].x && current.y+current.direction<=logs[i].x+5){
			return 1;
		}
	}
	if (current.y+CARWIDTH+current.direction>=COLUMNS-3 || current.y+current.direction<=4){
		return 1;
	}
	delCar(current.x, current.y, playwin);
	drawCar(current.x, current.y+=current.direction, current.color, playwin);
	delFrog(x, y, playwin);
	drawFrog(x, y+=current.direction, playwin);
	return 0;
}

//functions for stork
void drawStork(int x, int y,  WIN *playwin){

	wattron(playwin->window, COLOR_PAIR(STORK_COLOR));
	wrefresh(playwin->window);
	mvwprintw(playwin->window, x, y, "<^>");
}
void delStork(int x, int y, WIN *playwin){
	if((x)%4 == 0){
		wattron(playwin->window, COLOR_PAIR(PAVEMENT_COLOR));
		wrefresh(playwin->window);
		mvwprintw(playwin->window, x, y, "---");
	}else{
		wattron(playwin->window, COLOR_PAIR(ROAD_COLOR));
		wrefresh(playwin->window);
		mvwprintw(playwin->window, x, y, "   ");
	}
}

//logs function
struct cords newlog(WIN *playwin){
	int x = rand()%(COLUMNS-20)+10;
	int y = 5+4*(1+rand()%7); 
	wattron(playwin->window, COLOR_PAIR(LOG_COLOR));
	
	mvwprintw(playwin->window, y, x, "  0  ");
	mvwprintw(playwin->window, y+1, x," 0 0 ");
	mvwprintw(playwin->window, y+2, x, "0 0 0");
	struct cords new = {x, y};
	return new;
}

//Functions controlling distanses

//function checking if there is collision between two elements
int isCollision(int x1, int x2, int y1, int y2){
	return (x1 == x2 && abs(y2-y1)<4);
}

//function checking if two elements are close to eachother
int isClose(int x1, int x2, int y1, int y2){
	return (x1 == x2 && abs(y2-y1)>4 && abs(y2-y1)<9);
}

//function downloading and updating ranking of best scores
struct ranking getRanking(int score){
	int first=0, second=0, third=0;
	FILE *readRanking  = fopen("ranking.txt", "r");
	fscanf(readRanking, "%d %d %d", &first, &second, &third);
	if(score>first) {
		struct ranking current = {score, first,second};
		FILE *writeRanking  = fopen("ranking.txt", "w");
		fprintf(writeRanking, "%d %d %d", current.first_place, current.second_place, current.third_place);
		fclose(readRanking);
		fclose(writeRanking);
		return current;
	}else if(score>second) {
		struct ranking current = {first,score,second};
		FILE *writeRanking  = fopen("ranking.txt", "w");
		fprintf(writeRanking, "%d %d %d", current.first_place, current.second_place, current.third_place);
		fclose(readRanking);
		fclose(writeRanking);
		return current;
	}else if(score>third) {
		struct ranking current = {first,second,score};
		FILE *writeRanking  = fopen("ranking.txt", "w");
		fprintf(writeRanking, "%d %d %d", current.first_place, current.second_place, current.third_place);
		fclose(readRanking);
		fclose(writeRanking);
		return current;
	}else{
		struct ranking current = {first,second,third};
		FILE *writeRanking  = fopen("ranking.txt", "w");
		fprintf(writeRanking, "%d %d %d", current.first_place, current.second_place, current.third_place);
		fclose(readRanking);
		fclose(writeRanking);
		return current;
	}
}

//Functions controlling movement of elements

//function allowing the controll of Frog
struct cords moveFrog(WIN *playwin, int ch, struct timespec sleeptime, struct cords input){
	struct cords frog = input;
	switch (ch)
			{
			case KEY_LEFT:
				if(frog.y-FROGWIDTH>=1 && frog.y-FROGWIDTH<=COLUMNS-1){
					delFrog(frog.x, frog.y, playwin);
					frog.y-=FROGWIDTH;
					drawFrog(frog.x, frog.y, playwin);
					flushinp(); 
					nanosleep(&sleeptime, NULL);
				}
				break;
			case KEY_RIGHT:
				if(frog.y+FROGWIDTH>=1 && frog.y+FROGWIDTH<=COLUMNS-1){
					delFrog(frog.x, frog.y, playwin);
					frog.y+=FROGWIDTH;
					drawFrog(frog.x, frog.y, playwin);
					flushinp();  
					nanosleep(&sleeptime, NULL);
				}
				break;
			case KEY_UP:
				if(frog.x-FROGHEIGHT>=1 && frog.x-FROGHEIGHT+1<=ROWS-1){
					delFrog(frog.x, frog.y, playwin);
					frog.x-=FROGHEIGHT+1;
					drawFrog(frog.x, frog.y, playwin);
					flushinp();  
					nanosleep(&sleeptime, NULL);
				}
				break;
			case KEY_DOWN:
				if(frog.x+FROGHEIGHT>=1 && frog.x+FROGHEIGHT+1<=ROWS-1){
					delFrog(frog.x, frog.y, playwin);
					frog.x+=FROGHEIGHT+1;
					drawFrog(frog.x, frog.y, playwin);
					flushinp();  
					nanosleep(&sleeptime, NULL);
				}
				break;
			
			}
	return frog;
}

//function allowing the controll of car
struct car handleCar(WIN *playwin,int number_of_cars,int number_of_logs, struct cords logs[], struct cords frog, struct cords stork, struct car current, int level){
	struct car car = current;
	if(car.friendly!=0 || isClose(frog.x,car.x, frog.y, car.y)==0){
		if(car.friendly == 1 && isCollision(frog.x,car.x, frog.y, car.y)){
			drivingTaxi(car, playwin, logs, frog.x, frog.y, number_of_logs);
			car.driven_taxi = car.direction;
		}
		else{
			if (drivingCar(car, playwin, logs,number_of_logs) == 1){
				delCar(car.x, car.y, playwin);
				car=newCar(level);
			}else{
				car.y+=car.direction;
			}
		}
	}
	for (int j = 0; j<number_of_logs; j++){
		if(isCollision(frog.x,logs[j].y, frog.y, logs[j].x)){
			car.kill_frog = 1; 
		}
	}
	if(((isCollision(frog.x,car.x, frog.y, car.y)) && car.friendly != 1) || isCollision(stork.x, frog.x, stork.y, frog.y)){
		car.kill_frog = 1; 
	}
	return car;
}

//function allowing the controll of stork
struct cords flyStork(struct cords stork, struct cords frog, WIN *playwin){
	struct cords output = {stork.x, stork.y};
	if(stork.x > frog.x){
		output.x -= 1;
	}else if(stork.x <frog. x){
		output.x += 1;
	}
	if(stork.y < frog.y){
		output.y += 1;
	}else if(stork.y > frog.y){
		output.y -= 1;
	}
	delStork(stork.x, stork.y, playwin);
	drawStork(output.x, output.y, playwin);
	return output;
}

//function displaying current score
void scoreDisplay(int score, WIN* playwin){
	wattron(playwin->window, COLOR_PAIR(SCORE_COLOR));
	mvwprintw(playwin->window,ROWS-1, 20, "|-- Score: %d --|", score);
	refresh();
}

//nextLevel function declaration
int nextLevel(int currentScore,int currentLevel, int ch, int x, int y, struct timespec sleeptime, WIN *playwin,int number_of_cars,int number_of_logs);

//function ending the game
void endGame(WIN *playwin, int score){
	clear();
	wattron(playwin->window, COLOR_PAIR(TERMINAL_COLOR));
	move(ROWS/2,COLUMNS/2-4);
	addstr("GAME OVER");
	mvwprintw(playwin->window, ROWS/2+2,COLUMNS/2-4, "SCORE: %d", score);
	struct ranking current_ranking = getRanking(score);
	mvwprintw(playwin->window, ROWS/2+3,COLUMNS/2-4, "1: %d", current_ranking.first_place);
	mvwprintw(playwin->window, ROWS/2+4,COLUMNS/2-4, "2: %d", current_ranking.second_place);
	mvwprintw(playwin->window, ROWS/2+5,COLUMNS/2-4, "3: %d", current_ranking.third_place);
	getch();
	struct timespec timeafterdeath = {4,2};
	nanosleep(&timeafterdeath, NULL);
}

//Main loop function containing all variables
void mainLoop(int ch, int x, int y, struct timespec sleeptime, WIN *playwin,int number_of_cars,int number_of_logs,int newLevel, int newScore){
	struct cords frog = {x, y};
	struct cords stork = {2, ROWS/2};
	struct car* cars = malloc(number_of_cars * sizeof(struct car));
	struct cords* logs = malloc(number_of_logs * sizeof(struct cords));
	int level = newLevel;
	for(int i=0; i<number_of_cars; i++) {
		cars[i] = newCar(level);
	}
	for(int i=0; i<number_of_logs; i++) {
		logs[i] = newlog(playwin);
	}
	int end = 0;
	int alive = 1;
	int score = newScore;
	scoreDisplay(score, playwin);
	do{
		struct cords new =flyStork(stork, frog, playwin);
		stork = new;
		for(int i=0; i<number_of_cars; i++) {
			struct car output = handleCar(playwin, number_of_cars, number_of_logs, logs, frog, stork, cars[i], level);
			
			if( output.kill_frog == 1){
				alive=0;
				end=1;
				endGame(playwin, score);
				free(cars);
				free(logs);
			}else if(output.driven_taxi != 0){
				frog.y+=output.driven_taxi;
				output.driven_taxi = 0;
			}
			cars[i]=output;
		}
		ch =wgetch(playwin->window);

		if(ch == KEY_F(1)){
			end = 1;
		}
		if (alive){
			
			frog = moveFrog(playwin, ch, sleeptime, frog);
			if(frog.x < 4){
				delFrog(frog.x, frog.y, playwin);
				frog.x=ROWS-FROGHEIGHT - 1;
				frog.y=CENTERY;
				drawFrog(frog.x, frog.y, playwin);
				if(nextLevel(score, level, ch,  x,  y,   sleeptime,  playwin, number_of_cars, number_of_logs) == 1){
					alive=0;
					end=1;
					free(cars);
					free(logs);
				}else{

					score++;
				}
			}
		}
		scoreDisplay(score, playwin);
		refresh();
	}while (end == 0);
}

//recursive function promoting to next level
int nextLevel(int currentScore,int currentLevel, int ch, int x, int y, struct timespec sleeptime, WIN *playwin,int number_of_cars,int number_of_logs){
	int score =currentScore;
	int level =currentLevel;
	if (currentScore>0 && currentScore%(POINTS_IN_LEVEL+1) == 0 && currentLevel<=NUMBER_OF_LEVELS){
		clear();
		wattron(playwin->window, COLOR_PAIR(TERMINAL_COLOR));
		mvwprintw(playwin->window, ROWS/2+2,COLUMNS/2-4, "Level %d", currentLevel+1);
		refresh();
		struct timespec timeafterdeath = {2,2};
		nanosleep(&timeafterdeath, NULL);
		DrawBoard(playwin);
		int x = ROWS-FROGHEIGHT - 1, y = COLUMNS/2;
		drawFrog(x, y, playwin);
		mainLoop(ch,  x,  y,   sleeptime,  playwin, number_of_cars+5, number_of_logs+2, level+1, score+1);
		return 1;
		clear();
		endwin();

	}
	return 0;
}

//Main function
int main(){
	WINDOW *win = Start();
    cbreak(); 
    keypad(stdscr, TRUE);
	curs_set(0);
	nodelay(win, TRUE);
    Welcome(win);
	keypad(stdscr, TRUE);
	WIN* playwin = Init(win, ROWS, COLS, 0, 0, ROAD_COLOR, 1, DELAY_ON);	
	int ch;
	int x = ROWS-FROGHEIGHT - 1, y = COLUMNS/2;
	struct timespec sleeptime = {0,100000000};
	clear();
	mvwprintw(playwin->window, ROWS/2+2,COLUMNS/2-4, "Level 1");
	getch();
	struct timespec timeafterdeath = {4,2};
	nanosleep(&timeafterdeath, NULL);
	clear();

	nodelay(playwin->window, TRUE);
	keypad(playwin->window, TRUE);
	DrawBoard(playwin);
	drawFrog(x, y, playwin);
	
	int number_of_logs, number_of_cars, level = 1;
	FILE *paremeters  = fopen("parameters.txt", "r");
	fscanf(paremeters, "%d %d", &number_of_cars, &number_of_logs); 
	fclose(paremeters);
	mainLoop(ch,  x,  y,   sleeptime, playwin, number_of_cars, number_of_logs, level, 1);
	
	clear();
	endwin();
	return 0;
}