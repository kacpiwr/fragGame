#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>

WINDOW* Start()
{
	WINDOW* win;

	if ( (win = initscr()) == NULL ) {					// initialize ncurses
		fprintf(stderr, "Error initialising ncurses.\n");
		exit(EXIT_FAILURE);
    	}

	start_color();								// initialize colors

	noecho();								// Switch off echoing, turn off cursor
	curs_set(0);
	return win;
}

int main(){

}