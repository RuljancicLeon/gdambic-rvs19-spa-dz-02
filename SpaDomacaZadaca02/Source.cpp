#include <iostream>
#include "Game.h"

int main()
{
	Game game;
	

	while (game.getWindowIsOpen())
	{
		
		//Update
		game.Update();


		//Render
		game.Render();
	}

	return 0;
}