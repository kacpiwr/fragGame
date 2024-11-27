#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>

int main() {
	for (int i = 0; i<9; i++){
		printf("%d %d %d %d /n", (rand()%5 + 1)*20000000, 1+4*i, 20, ((rand()%2)*(-2)+1)*(rand()%3+1));
	}
}