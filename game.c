#include "MAX7219.h"
#include <reg52.h>
#include "game.h"
#include <math.h>

/** Hardware feedback */
sbit player1 = P3^0;	// indicator for player 1's turn
sbit player2 = P3^1;	// indicator for player 2's turn

/** private data */
int game[3][3];
int moveCounter;		// number of moves
int nextMove;			// next move
int winner;			// winner
char win_player1[] = {0, 1, 1, 127, 33, 17, 0, 0};
char win_player2[] = {0, 1, 49, 73, 69, 51, 0, 0};
char draw[] = {255, 255, 255, 255, 255, 255, 255, 255};

/** private functions */
void delay(int ms); // delay
void play_at(int row, int col);	// play at a position
void updateNext(); // update player for next turn
void paint(); // paint the display
void updateWinner(); // update winner

/** function definitions */

void init(){
	int i, j;
	// port config
	P1 = 0x00;
	P2 = 0x00;
	// display
	MAX7219_Init();
	MAX7219_SetBrightness(4);
	MAX7219_Clear(); // clear the display
	// game
	nextMove = -1; // init
	moveCounter = 0; // moves total counter
	winner = -1; // no winner; 0 for draw and 1,2 for players
	for(i=0;i<3;i++)
		for(j=0;j<3;j++)
			game[i][j] = 0;	// init game
	// hardware init
	updateNext();
}

void delay(int ms){
	int i = 0;
	for( ; i < ms; i++){
		TMOD = 0x01;
		TH0 = 0xFC;
		TL0 = 0x66;
		TR0 = 1;
		while(TF0 == 0);
		TR0 = 0;
		TF0 = 0;
	}
}

void play_at(int row, int col){
	if( row<0 || row>2 || col<0 || col>2) // out of bounds
			return;
	if(game[row][col] != 0)	// already played
		return;
	game[row][col] = nextMove; // play
	moveCounter++; // increment counter
	// change next move
	updateNext();
}

void updateNext(){
	if(nextMove==1){
		nextMove = 2;
		player1 = 0;
		player2 = 1;
	}else{
		nextMove = 1;
		player1 = 1;
		player2 = 0;
	}
}

/** input is distributed over P2 {0-8} and P1.0 */
int read(){
	int p2;
	p2 = P2;
	// read P1.0
	if(P1^0 == 1)
		return 9;
	if(p2>0)
		return ((int)(((float)log(p2))/log(2))+1);
	return -1;
}

/** play one turn */
void play(){
	int pos = -1, row, col;
	while(1){
		pos = read();
		if(pos>0)
			break;
	}
	row = (pos-1)/3;
	col = (pos-1)%3;
	play_at(row, col);
	paint();
	updateWinner();
}

void paint(){
	int i, j;
	for(i=0;i<3;i++)
		for(j=0;j<3;j++){
			if(game[i][j]==1){		// player 1, paint '='
	     		setLed(i*3, j*3, 1);
				setLed(i*3, j*3+1, 1);
				setLed(i*3+1, j*3, 1);
				setLed(i*3+1, j*3+1, 1);
			}else if(game[i][j]==2){	// player 2, paint '\'
				setLed(i*3, j*3, 1);
				setLed(i*3, j*3+1, 0);
				setLed(i*3+1, j*3, 0);
				setLed(i*3+1, j*3+1, 1);
			}
		}
}

/**
* set winner = -1 for still playing,
* 0 for draw,
* 1,2 for the player who won
*/
void updateWinner(){
	int i;
	winner = -1;
	// check if game is over
	if(moveCounter<5) // minimum moves threshold
		return;
	// scan horizontal and vertical
	for(i=0;i<3;i++){
		if(((game[i][0] == game[i][1]) && (game[i][1] == game[i][2])) && (game[i][0] != 0)){
			winner = game[i][0];
			return;
		}
		if(((game[0][i] == game[1][i]) && (game[1][i] == game[2][i])) && (game[0][i] != 0)){
			winner = game[0][i];
			return;
		}
	}
	// scan diagonally
	if(((game[0][0] == game[1][1]) && (game[1][1] == game[2][2])) && (game[0][0] != 0)){
		winner = game[0][0];
		return;
	}
	if(((game[2][0] == game[1][1]) && (game[1][1] == game[0][2])) && (game[2][0] != 0)){
		winner = game[2][0];
		return;
	}
	if(moveCounter == 9) // match draw
		winner = 0;
	return;
}

int getWinner(){
	return winner;
}

void printWinner(){
	char* disp;
	int i;
	delay(500);
	if(winner == 0)
		disp = draw;
	else if(winner == 1)
		disp = win_player1;
	else if(winner == 2)
		disp = win_player2;
	player1 = 1;
	player2 = 1;
	// flash message
	MAX7219_Clear();
	for(i=0;i<5;i++){
		setScreen(disp);
		delay(500);
		MAX7219_Clear();
		delay(500);
	}
}
