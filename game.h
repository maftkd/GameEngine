#ifndef GAME_H
#define GAME_H

vec4 col;

void initGame(){
	printf("starting game");
}

void gameLoop(){
	//CPU stuff
	//printf("-CPU dt: %f\n",deltaTime);
	float intensity=mousePos.x/screenSize.x;
	if(keyInput[68]==2)
		col.set(1,0,0,1);
	else if(keyInput[65]==2)
		col.set(0,1,0,1);
	else
		col.set(0,0,1,1);
	setClearColor(col*intensity);

	//drawTriangle(-0.5,-0.5,0,0.5,0.5,-0.5,invertColor(col));
}

#endif
