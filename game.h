#ifndef GAME_H
#define GAME_H

vec4 col;

void mainThread(){
	//CPU stuff
	printf("-CPU dt: %f\n",deltaTime);
	//if(mousePos.x<screenSize.x*0.5f)
	//if(lmbDown)
	if(wDown)
		col.set(1,0,0,1);
	else
		col.set(0,0,1,1);
	setClearColor(col);

	//drawTriangle(-0.5,-0.5,0,0.5,0.5,-0.5,invertColor(col));
}

#endif
