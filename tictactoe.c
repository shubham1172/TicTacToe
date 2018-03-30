#include <reg52.h>
#include "game.h"

void game_logic(){
	init();
	while(getWinner()==-1){
		play();
	}
	printWinner();
}

void main(){
	while(1){
		game_logic();
	}
}
