#ifndef UNICODE
#define UNICODE
#endif
 
//globals
float deltaTime;
float targetDeltaTime=0.016667f;
bool frameSync=true;
bool rendering=true;
bool running=true;
char mode = 1;//0 = play mode, 1 = font editor
int targetScreenWidth=1920;
int targetScreenHeight=1080;

#include <stdio.h>
#include <windows.h>
#include "io.h"
#include "vector.h"
#include "image.h"
#include "input.h"
#include "font.h"
#include "game.h"
#include <assert.h>
#include <thread>
#include <chrono>

//cpu timers
std::chrono::duration<float> dt;//tmp
std::chrono::system_clock::time_point prevFrameStart;
std::chrono::system_clock::time_point frameStart;
std::chrono::system_clock::time_point tempTimePoint;

void init(){
	switch(mode){
		case 0:
		default:
			initGame();
			break;
		case 1:
			initFontEditor();
			break;
	}
}

void update(){
	switch(mode){
		case 0:
		default:
			gameLoop();
			break;
		case 1:
			updateFontEditor();
			break;
	}
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow){
	//#todo make a console for logging within the main window
	//get a console for logging stuff
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	printf("Hello ! And thank you for testing this software :)\n");

	//register window class
	const wchar_t CLASS_NAME[] = L"Classy Class";
	WNDCLASS wc = { };
	wc.lpfnWndProc		= WindowProc;
	wc.hInstance 		= hInstance;
	wc.lpszClassName	= CLASS_NAME;
	wc.hCursor 			= LoadCursor(NULL, IDC_ARROW);
	RegisterClass(&wc);

	//Create the window
	HWND hwnd = CreateWindowEx(
			0,								//Optional window styles.
			CLASS_NAME,						//Windows class
			L"idk man",	//Window text
			WS_OVERLAPPEDWINDOW,			//Window style
			CW_USEDEFAULT, CW_USEDEFAULT, targetScreenWidth, targetScreenHeight,
			NULL,		//Parent window
			NULL,		//Menu
			hInstance,	//Instance handle
			NULL		//Additional app data
			);
	if(hwnd==NULL)
		return 0;
	ShowWindow(hwnd, nCmdShow);

	initRenderer(hwnd,targetScreenWidth,targetScreenHeight);

	//start render thread
	DWORD renderThreadId;
	HANDLE renderThreadHandle = CreateThread(
			NULL,
			0,
			renderThread,
			0,
			0,
			&renderThreadId);

	if(renderThreadHandle==NULL){
		return 0;
	}

	//initialization
	init();

	MSG msg = { };
	while(running){
		//start frame
		frameStart = std::chrono::system_clock::now();
		dt=frameStart-prevFrameStart;
		deltaTime = dt.count();
		prevFrameStart=frameStart;

		//reset input
		updateKeys();
		//get input
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
			if(msg.message == WM_QUIT){
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//start rendering frame N-1 on "render thread"
		frameSync=false;//this allows the gpu to begin rendering frame N-1
		
		//do game stuff for frame N on "main thread"
		update();

		//cpu sleep while N-1 finish rendering
		tempTimePoint = std::chrono::system_clock::now();
		dt = tempTimePoint-frameStart;
		float cpuTimer = dt.count();
		bool profiledPrevFrame=false;
		while(cpuTimer<targetDeltaTime-0.002f && !frameSync){
			std::this_thread::sleep_for(std::chrono::microseconds(100));
			tempTimePoint = std::chrono::system_clock::now();
			dt = tempTimePoint-frameStart;
			cpuTimer = dt.count();
			//gpu profiler
			if(!profiledPrevFrame)
				profiledPrevFrame=getGpuTimeStamps(curFrameRenderCommands);//cur frame holds prev frames profile data
		}
		while(!frameSync);
		
		//swap prev frame timestamp queries into current
		curFrameRenderCommands.swapTimeStamps(&prevFrameRenderCommands);
		//copy current frame render commands to previous
		prevFrameRenderCommands.copy(curFrameRenderCommands);
	}

	//close render thread
	CloseHandle(renderThreadHandle);

	return 0;
}

