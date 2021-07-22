#ifndef INPUT_H
#define INPUT_H

#include <stdio.h>
#include <windows.h>
#include "vector.h"
#include "screen.h"

//input variables
vec2 mousePos;

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
			PostQuitMessage(0);
			break;
		case WM_SIZE:
			screenSize.set(LOWORD(lParam),HIWORD(lParam));
			handleScreenSizeChange(hwnd,LOWORD(lParam),HIWORD(lParam));
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
				//PostQuitMessage(0);
				DestroyWindow(hwnd);
			}
			break;
		}
		case WM_KEYDOWN:
			printf("key down\n");
			//key down
			break;
		case WM_KEYUP:
			printf("key up\n");
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
			printf("lmb down\n");
			//lmb down
			break;
		case WM_LBUTTONUP:
			printf("lmb up\n");
			//lmb up
			break;
		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
			
	}
	return 0;
}

#endif
