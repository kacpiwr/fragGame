#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>


#define CENTERX (ROWS/2 -2)
#define CENTERY (COLUMNS/2 -2)
#define ROWS 41
#define COLUMNS 200
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
#define WATER_COLOR 11

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
struct frogMove{
	int x;
	int y;
	struct timespec lastmove_time;
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
	init_pair(FROG_COLOR, COLOR_WHITE, COLOR_GREEN);
	init_pair(CAR_COLOR_ONE, COLOR_MAGENTA, COLOR_WHITE);
	init_pair(CAR_COLOR_TAXI, COLOR_RED, COLOR_WHITE);
	init_pair(CAR_COLOR_FRIENDLY, COLOR_YELLOW, COLOR_WHITE);
	init_pair(LOG_COLOR, COLOR_GREEN, COLOR_WHITE);
	init_pair(TERMINAL_COLOR, COLOR_BLACK, COLOR_WHITE);
	init_pair(STORK_COLOR, COLOR_WHITE, COLOR_RED);
	init_pair(SCORE_COLOR, COLOR_WHITE, COLOR_BLUE);
	init_pair(WATER_COLOR, COLOR_BLACK, COLOR_CYAN);

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
	//drawing background 
	for(int j = 1; j<ROWS-1; j++){
		for(int i = 1; i<COLUMNS-1; i++){
			if((i<=j && j <=4) || (j<=ROWS-1 && j>=ROWS-4)){
				wattron(playwin->window, COLOR_PAIR(ROAD_COLOR));
			}else{
				wattron(playwin->window, COLOR_PAIR(ROAD_COLOR));
			}
			wrefresh(playwin->window);
			mvwprintw(playwin->window, j, i, " ");
		}
	}
	//drawing border 
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
	//drawing pavements 
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
void drawFrog(struct cords frog, WIN *playwin){
	wattron(playwin->window, COLOR_PAIR(FROG_COLOR));
	wrefresh(playwin->window);
	mvwprintw(playwin->window, frog.x, frog.y, "O O");
	mvwprintw(playwin->window, frog.x+1, frog.y, " # ");
	mvwprintw(playwin->window, frog.x+2, frog.y, "o o");
}
void delFrog(struct cords frog, WIN *playwin){
	wattron(playwin->window, COLOR_PAIR(ROAD_COLOR));
	wrefresh(playwin->window);
	mvwprintw(playwin->window, frog.x, frog.y, "   ");
	mvwprintw(playwin->window, frog.x+1, frog.y, "   ");
	mvwprintw(playwin->window, frog.x+2, frog.y, "   ");
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
	//car position drawing
	if (rand()%2){
		x=5;
	}else{
		x=COLUMNS-10;
	}
	//drawing random car parameters
	struct car new={1, {0,10000000/level}, 5+4*(1+rand()%7), x,  ((rand()%2)*(-2)+1)*(rand()%3+1), CAR_COLOR_ONE, rand()%5, 0, 0};
	//choosing correct car color
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
	//checking if car doesn't hit logs
	for(int i = 0; i<number_of_logs; i++){
		if (current.x == logs[i].y && current.y+CARWIDTH+current.direction>=logs[i].x && current.y+current.direction<=logs[i].x+5){
			return 1;
		}
	}
	//checking if car doesn't hit border
	if (current.y+CARWIDTH+current.direction>=COLUMNS-3 || current.y+current.direction<=4){
		return 1;
	}
	//draws moved car
	delCar(current.x, current.y, playwin);
	drawCar(current.x, current.y+=current.direction, current.color, playwin);
	return 0;

}
int drivingTaxi(struct car current, WIN *playwin, struct cords logs[],struct cords frog, int number_of_logs){
	nanosleep(&current.speed, NULL);
	//checking if taxi doesn't hit logs
	for(int i = 0; i<number_of_logs; i++){
		if (current.x == logs[i].y && current.y+CARWIDTH+current.direction>=logs[i].x && current.y+current.direction<=logs[i].x+5){
			return 1;
		}
	}
	//checking if taxi doesn't hit border
	if (current.y+CARWIDTH+current.direction>=COLUMNS-3 || current.y+current.direction<=4){
		return 1;
	}
	//draws moved car and frog
	delCar(current.x, current.y, playwin);
	drawCar(current.x, current.y+=current.direction, current.color, playwin);
	delFrog(frog, playwin);
	drawFrog((struct cords){frog.x, frog.y+current.direction}, playwin);
	return 0;
}

//functions for stork
void drawStork(int x, int y,  WIN *playwin){
	//drawing stork
	wattron(playwin->window, COLOR_PAIR(STORK_COLOR));
	wrefresh(playwin->window);
	mvwprintw(playwin->window, x, y, "<^>");
}
void delStork(int x, int y, WIN *playwin){
	//deleting stork depending on irs position
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
	//choosing random logs position
	int x = rand()%(COLUMNS-20)+10;
	int y = 5+4*(1+rand()%7); 
	wattron(playwin->window, COLOR_PAIR(LOG_COLOR));
	//drawing logs
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
	//reading current ranking
	FILE *readRanking  = fopen("ranking.txt", "r");
	fscanf(readRanking, "%d %d %d", &first, &second, &third);
	struct ranking current = {first,second,third};
		FILE *writeRanking  = fopen("ranking.txt", "w");
	if(score>first) {
		current = (struct ranking){score, first,second};
		//updating ranking
		fprintf(writeRanking, "%d %d %d", current.first_place, current.second_place, current.third_place);
	}else if(score>second) {
		current = (struct ranking){first,score,second};
		//updating ranking
		fprintf(writeRanking, "%d %d %d", current.first_place, current.second_place, current.third_place);
	}else if(score>third) {
		current = (struct ranking){first,second,score};
		//updating ranking
		fprintf(writeRanking, "%d %d %d", current.first_place, current.second_place, current.third_place);
	}else{
		current = (struct ranking){first,second,third};
		//updating ranking
		fprintf(writeRanking, "%d %d %d", current.first_place, current.second_place, current.third_place);
	}
	fclose(readRanking);
	fclose(writeRanking);
	return current;
}

//Functions controlling movement of elements

//function allowing the controll of Frog
struct frogMove moveFrog(WIN *playwin, int ch, struct cords input, struct timespec cooldown, struct timespec last_move_time) {
    struct cords frog = input;
    struct timespec current_time;
    //geting current time
    clock_gettime(CLOCK_MONOTONIC, &current_time);

	struct frogMove output = {frog.x, frog.y, last_move_time};
    
    long elapsed_time = (current_time.tv_sec - last_move_time.tv_sec) * 1000000000L + (current_time.tv_nsec - last_move_time.tv_nsec);
    
    if (elapsed_time < cooldown.tv_sec * 1000000000L + cooldown.tv_nsec) {
        return output;
    }
    delFrog(frog, playwin);

    switch (ch) {
		//moving left
        case KEY_LEFT:
            if (frog.y - FROGWIDTH >= 1 && frog.y - FROGWIDTH <= COLUMNS - 1) {
                frog.y -= FROGWIDTH;
			}
            break;
		//moving right
        case KEY_RIGHT:
            if (frog.y + FROGWIDTH >= 1 && frog.y + FROGWIDTH <= COLUMNS - 1) {
                frog.y += FROGWIDTH;
            }
            break;
		//moving up
        case KEY_UP:
            if (frog.x - FROGHEIGHT >= 1 && frog.x - FROGHEIGHT + 1 <= ROWS - 1) {
                frog.x -= FROGHEIGHT + 1;
            }
            break;
		//moving down
        case KEY_DOWN:
            if (frog.x + FROGHEIGHT >= 1 && frog.x + FROGHEIGHT + 1 <= ROWS - 1) {
                frog.x += FROGHEIGHT + 1;
            }
            break;
    }

	drawFrog(frog, playwin);
	flushinp();
	//updating time
    clock_gettime(CLOCK_MONOTONIC, &last_move_time);

	output.x = frog.x;
	output.y =  frog.y;
	output.lastmove_time = last_move_time;
    return output;
}


//function allowing the controll of car
struct car handleCar(WIN *playwin,int number_of_cars,int number_of_logs, struct cords logs[], struct cords frog, struct cords stork, struct car current, int level){
	struct car car = current;
	if(car.friendly!=0 || isClose(frog.x,car.x, frog.y, car.y)==0){
		if(car.friendly == 1 && isCollision(frog.x,car.x, frog.y, car.y)){
			drivingTaxi(car, playwin, logs, frog, number_of_logs);
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
	//moving stork accordingly to frog position
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
	//drawing updated position stork 
	delStork(stork.x, stork.y, playwin);
	drawStork(output.x, output.y, playwin);
	return output;
}

//function displaying current score
void scoreDisplay(int score, WIN* playwin){
	wattron(playwin->window, COLOR_PAIR(SCORE_COLOR));
	mvwprintw(playwin->window,ROWS-1, 20, "|-- Score: %d --| Kacper Wrobel 5A 203365", score);
	refresh();
}

//nextLevel function declaration
int nextLevel(int currentScore,int currentLevel, int ch, struct cords frog, WIN *playwin,int number_of_cars,int number_of_logs);

//function ending the game
void endGame(WIN *playwin, int score){
	//deleting board
	clear();
	wattron(playwin->window, COLOR_PAIR(TERMINAL_COLOR));
	move(ROWS/2,COLUMNS/2-4);
	//printing game over info, score and ranking
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
//function to hamdeling moved frog
void handleFrog(struct cords * frog, WIN *playwin, int ch, struct timespec cooldown, struct timespec * lTime, int *score, int level, int ncars, int nlogs, int *end){
	struct frogMove moved = moveFrog(playwin, ch, *frog, cooldown, *lTime);
	//updating frog cords and time
		*frog = (struct cords){moved.x, moved.y};
		*lTime = moved.lastmove_time;
		//scoring a point 
		if ((*frog).x < 4) {
			//moving frog to starting position
			delFrog(*frog, playwin);
			*frog = (struct cords){ROWS - FROGHEIGHT - 1, CENTERY};
			drawFrog(*frog, playwin);
			//checking if advance to next level
			if (nextLevel(*score, level, ch, *frog, playwin, ncars, nlogs)) *end = 1;
			//updating score
			else *score+=1;
		}
}
//Main loop function containing all variables
void mainLoop(int ch, int x, int y, WIN *playwin, int ncars, int nlogs, int newLevel, int newScore) {
	// creating game elements
    struct cords frog = {x, y}, stork = {2, ROWS / 2};
    struct car *cars = malloc(ncars * sizeof(struct car));
    struct cords *logs = malloc(nlogs * sizeof(struct cords));
    struct timespec lTime = {0, 0}, cooldown = {0, 150000000}; 
    int level = newLevel, end = 0, score = newScore;
	//getting random cars and logs
    for (int i = 0; i < ncars; i++) cars[i] = newCar(level);
    for (int i = 0; i < nlogs; i++) logs[i] = newlog(playwin);
    scoreDisplay(score, playwin);

    do {
        stork = flyStork(stork, frog, playwin);
        for (int i = 0; i < ncars; i++) {
			//handeling each car
            struct car output = handleCar(playwin, ncars, nlogs, logs, frog, stork, cars[i], level);
            if (output.kill_frog) { end = 1; endGame(playwin, score); }
            else if (output.driven_taxi) { frog.y += output.driven_taxi; output.driven_taxi = 0; }
            cars[i] = output;
        }
		//geting keyboard input
        ch = wgetch(playwin->window);
        if (!end) {
			//handeling frog movement
            handleFrog(&frog, playwin, ch, cooldown, &lTime, &score, level, ncars, nlogs, &end);
        }
		//updating score display
        scoreDisplay(score, playwin);
        refresh();
    } while (!end); //checking if program should end loop

    free(cars);
    free(logs);
}

//function promoting to next level
int nextLevel(int currentScore,int currentLevel, int ch, struct cords frog, WIN *playwin,int number_of_cars,int number_of_logs){
	int score =currentScore;
	int level =currentLevel;
	if (currentScore>0 && currentScore%(POINTS_IN_LEVEL+1) == 0 && currentLevel<NUMBER_OF_LEVELS){  //checking if player should advamce to next level
		//showing next level screen
		clear();
		wattron(playwin->window, COLOR_PAIR(TERMINAL_COLOR));
		mvwprintw(playwin->window, ROWS/2+2,COLUMNS/2-4, "Level %d", currentLevel+1);
		refresh();
		struct timespec timeafterdeath = {2,2};
		nanosleep(&timeafterdeath, NULL);
		DrawBoard(playwin);
		int x = ROWS-FROGHEIGHT - 1, y = COLUMNS/2;
		drawFrog(frog, playwin);
		//calling a function to play the next level
		mainLoop(ch,  x,  y,  playwin, number_of_cars+5, number_of_logs+2, level+1, score+1);
		return 1;
		clear();
		endwin();

	}
	return 0;
}

//Main function
int main(){
	//setting correct window settings
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
	DrawBoard(playwin); //rysowanie planszy
	drawFrog((struct cords){x, y}, playwin);
	//defaning and downloading correct parameters
	int number_of_logs, number_of_cars, level = 1;
	FILE *paremeters  = fopen("parameters.txt", "r");
	fscanf(paremeters, "%d %d", &number_of_cars, &number_of_logs); 
	fclose(paremeters);
	//running main loop
	mainLoop(ch,  x,  y, playwin, number_of_cars, number_of_logs, level, 1);
	
	clear();
	endwin();
	return 0;
}