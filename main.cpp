#ifndef UNICODE
#define UNICODE
#endif
 
#include "input.h"
#include <assert.h>
#include <thread>
#include <chrono>

//frame timing
float deltaTime;
std::chrono::duration<float> dt;//tmp
std::chrono::system_clock::time_point prevFrameStart;
std::chrono::system_clock::time_point frameStart;
std::chrono::system_clock::time_point frameEnd;
float TICK=0.0167f;//60fps
//float TICK=0.0327f;//30fps
float TICK_FUDGE=0.009f;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow){

	//open up ze terminal
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
			//WS_POPUPWINDOW,
			//WS_POPUPWINDOW,
			//Size and position
			//CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			CW_USEDEFAULT, CW_USEDEFAULT, 1600, 900,
			//CW_USEDEFAULT, CW_USEDEFAULT, 1920, 1080,
			NULL,		//Parent window
			NULL,		//Menu
			hInstance,	//Instance handle
			NULL		//Additional app data
			);
	if(hwnd==NULL)
		return 0;
	ShowWindow(hwnd, nCmdShow);

	MSG msg = { };
	while(true){
		//start frame
		frameStart = std::chrono::system_clock::now();
		dt=frameStart-prevFrameStart;
		deltaTime = dt.count();
		prevFrameStart=frameStart;

		//get input
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
			if(msg.message == WM_QUIT){
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//do stuff

		//end frame
		frameEnd = std::chrono::system_clock::now();
		dt = frameEnd-frameStart;
		float frameTime = dt.count();

		//big sleeps
		while(frameTime<TICK-TICK_FUDGE){
			//sleep in increments because big sleeps are inaccurate
			std::this_thread::sleep_for(std::chrono::microseconds(100));
			frameEnd = std::chrono::system_clock::now();
			dt=frameEnd-frameStart;
			frameTime = dt.count();
		}
		
		//little sleeps
		while(frameTime<TICK){
			frameEnd = std::chrono::system_clock::now();
			dt=frameEnd-frameStart;
			frameTime = dt.count();
		}
	}

	return 0;
}

