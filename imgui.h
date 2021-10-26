#ifndef IMGUI_H
#define IMGUI_H

void addCharVertData(float left, float bottom, float width, float height,float uLeft,float vBot, float uRight, float vTop){
	int startIndex=curFrameRenderCommands.numCharVerts*4;
	//bottom left
	curFrameRenderCommands.charVerts[startIndex]=left;
	curFrameRenderCommands.charVerts[startIndex+1]=bottom;
	curFrameRenderCommands.charVerts[startIndex+2]=uLeft;
	curFrameRenderCommands.charVerts[startIndex+3]=vBot;
	//top left
	curFrameRenderCommands.charVerts[startIndex+4]=left;
	curFrameRenderCommands.charVerts[startIndex+5]=bottom+height;
	curFrameRenderCommands.charVerts[startIndex+6]=uLeft;
	curFrameRenderCommands.charVerts[startIndex+7]=vTop;
	//bottom right
	curFrameRenderCommands.charVerts[startIndex+8]=left+width;
	curFrameRenderCommands.charVerts[startIndex+9]=bottom;
	curFrameRenderCommands.charVerts[startIndex+10]=uRight;
	curFrameRenderCommands.charVerts[startIndex+11]=vBot;
	//top right
	curFrameRenderCommands.charVerts[startIndex+12]=left+width;
	curFrameRenderCommands.charVerts[startIndex+13]=bottom+height;
	curFrameRenderCommands.charVerts[startIndex+14]=uRight;
	curFrameRenderCommands.charVerts[startIndex+15]=vTop;
	curFrameRenderCommands.numCharVerts+=4;
	curFrameRenderCommands.charVertsChanged=true;
}

#endif
