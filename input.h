#ifndef INPUT_H
#define INPUT_H

#include <stdio.h>
#include <windows.h>
#include "vector.h"
#include "screen.h"

//input variables
vec2 mousePos;
short* keyInput;

//#temp input vars
//later lets put these in keyInput array
bool wDown=false;
//#temp more tmp input vars
bool lmbDown=false;

//window message callback
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	switch(uMsg){
		case WM_ACTIVATE:
			break;
		case WM_CREATE:
			break;
		case WM_SETFOCUS:
			break;
		case WM_KILLFOCUS:
			break;
		case WM_DESTROY:
			rendering=false;
			running=false;
			break;
		case WM_SIZE:
			screenSize.set(LOWORD(lParam),HIWORD(lParam));

			//size changed
			if(wParam == SIZE_MINIMIZED){
				//minimized
			}
			else if(wParam == SIZE_MAXIMIZED){
				//maximized
			}
			break;
		case WM_EXITSIZEMOVE:
			//done moving size?
			break;
		case WM_CHAR:
			printf("char typed %x\n",wParam);
			//char pressed
			break;
		case WM_SYSKEYDOWN: {
			if (wParam == VK_RETURN) {
				toggleFullScreen(hwnd);
			}
			if (wParam == VK_F4){
				rendering=false;
				running=false;
			}
			break;
		}
		case WM_KEYDOWN:
			printf("key down\n");
			if (wParam == 0x57){
				wDown=true;
			}
			//key down
			break;
		case WM_KEYUP:
			printf("key up\n");
			if (wParam == 0x57){
				wDown=false;
			}
			//key up
			break;
		case WM_MOUSEMOVE:
			mousePos.set(LOWORD(lParam),HIWORD(lParam));
			printf("mouse move (%d,%d)\n",(int)mousePos.x,(int)mousePos.y);
			//mouse move
			break;
		case WM_RBUTTONDOWN:
			printf("rmb down\n");
			//rmb down
			break;
		case WM_RBUTTONUP:
			printf("rmb up\n");
			//rmb up
			break;
		case WM_LBUTTONDOWN:
			//printf("lmb down\n");
			lmbDown=true;
			//lmb down
			break;
		case WM_LBUTTONUP:
			lmbDown=false;
			printf("lmb up\n");
			//lmb up
			break;
		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
			
	}
	return 0;
}

#endif
