#ifndef GAME_H
#define GAME_H

vec4 col;

void mainThread(){
	//CPU stuff
	printf("-CPU dt: %f\n",deltaTime);
	//if(mousePos.x<screenSize.x*0.5f)
	float intensity=mousePos.x/screenSize.x;
	//if(lmbDown)
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
