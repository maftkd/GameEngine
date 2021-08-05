#ifndef SCREEN_H
#define SCREEN_H

#include <d3d11.h>
#include <dxgi.h>
#include <assert.h>

HRESULT hr;//for random windows functions
vec2 screenSize;
bool fullScreen=false;
bool initD3Buffers=false;

//swap chain
ID3D11Device* device= NULL;
ID3D11DeviceContext* deviceContext= NULL;
IDXGISwapChain* swapChain= NULL;
//D3D11_VIEWPORT viewport = {0};

//textures
ID3D11Texture2D* frameBuffer;
ID3D11RenderTargetView* renderTarget = NULL;

void initRenderBuffers(int width, int height){
	//create viewports
	/*
	viewport = {
	  0.0f,
	  0.0f,
	  ( FLOAT )( width ),
	  ( FLOAT )( height ),
	  0.0f,
	  1.0f };
	  */

	//create a frame buffer
	HRESULT hr = swapChain->GetBuffer(
	  0,
	  __uuidof( ID3D11Texture2D ),
	  (void**)&frameBuffer );
	if(FAILED(hr))
		printf("Failed getting texture2d %x\n",hr);
	
	//create render target view
	hr = device->CreateRenderTargetView(
	  frameBuffer, 0, &renderTarget );
	if(FAILED(hr))
		printf("1-failed creating first pass target view %x\n", hr);
}

void toggleFullScreen(HWND hwnd){
	fullScreen=!fullScreen;
	printf("Toggling fullScreen %d\n",fullScreen);
	if(fullScreen)
	{
		SetWindowLongPtr(hwnd,GWL_STYLE,WS_POPUP| WS_VISIBLE);
		SetWindowPos(hwnd,HWND_NOTOPMOST,0,0,targetScreenWidth,targetScreenHeight,0);
	}
	else
	{
		SetWindowLong(hwnd,GWL_STYLE,WS_OVERLAPPEDWINDOW | WS_VISIBLE);
		SetWindowPos(hwnd,HWND_NOTOPMOST,0,0,targetScreenWidth,targetScreenHeight,0);
	}
}

void initSwapChain(HWND hwnd){
	DXGI_SWAP_CHAIN_DESC swap_chain_descr  = { 0 };
	swap_chain_descr.BufferDesc.RefreshRate.Numerator   = 0;
	swap_chain_descr.BufferDesc.RefreshRate.Denominator = 1; 
	swap_chain_descr.BufferDesc.Format  = DXGI_FORMAT_B8G8R8A8_UNORM;//unsigned normalized ints - read as floats from 0 to 1 in shader
	swap_chain_descr.SampleDesc.Count   = 1;
	swap_chain_descr.SampleDesc.Quality = 0;                               
	swap_chain_descr.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_descr.BufferCount        = 2;
	swap_chain_descr.OutputWindow       = hwnd;                
	swap_chain_descr.Windowed           = true;
	swap_chain_descr.Flags				= 0;

	HRESULT hr = D3D11CreateDeviceAndSwapChain(
	  NULL,
	  D3D_DRIVER_TYPE_HARDWARE,
	  NULL,
	  0,
	  NULL,
	  0,
	  D3D11_SDK_VERSION,
	  &swap_chain_descr,
	  &swapChain,
	  &device,
	  NULL,//feature level
	  &deviceContext);
	assert( S_OK == hr && swapChain && device&& deviceContext);
	printf("initialized directx device and swap chain\n");

	//this funky stuff catches alt+enter
	IDXGIFactory1* factory;
	swapChain->GetParent(__uuidof (IDXGIFactory1), (void **) &factory);
    factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_WINDOW_CHANGES);
}


void clearRenderTarget(vec4 col){
	deviceContext->OMSetRenderTargets( 1, &renderTarget, nullptr );
	//deviceContext->RSSetViewports( 1, &viewport );
	deviceContext->ClearRenderTargetView(renderTarget, col.points );
}

struct renderCommands{
	vec4 clearColor;
	ID3D11Query* disjoint;
	ID3D11Query* start;
	ID3D11Query* end;

	renderCommands(){}

	void init(){
		//create gpu profiling queries
		D3D11_QUERY_DESC qdes = {0};
		qdes.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
		device->CreateQuery(&qdes,&disjoint);
		qdes.Query = D3D11_QUERY_TIMESTAMP;
		device->CreateQuery(&qdes,&start);
		device->CreateQuery(&qdes,&end);
	}

	void copy(renderCommands rcq){
		clearColor=rcq.clearColor;
	}

	void swapTimeStamps(renderCommands* rcq){
		ID3D11Query* tmpDisjoint;
		ID3D11Query* tmpStart;
		ID3D11Query* tmpEnd;
		tmpDisjoint=rcq->disjoint;
		tmpStart=rcq->start;
		tmpEnd=rcq->end;
		rcq->disjoint=disjoint;
		rcq->start=start;
		rcq->end=end;
		disjoint=tmpDisjoint;
		start=tmpStart;
		end=tmpEnd;
	}
};
//prev is for gpu, cur is for cpu
renderCommands prevFrameRenderCommands, curFrameRenderCommands;

//gpu profiler
bool getGpuTimeStamps(renderCommands rc){
	int res = deviceContext->GetData(rc.disjoint,NULL,0,0);
	if(res==S_OK){
		D3D11_QUERY_DATA_TIMESTAMP_DISJOINT tsDisjoint;
		deviceContext->GetData(rc.disjoint, &tsDisjoint, sizeof(tsDisjoint), 0);
		if(!tsDisjoint.Disjoint){
			UINT64 frameStart, frameEnd;
			deviceContext->GetData(rc.start, &frameStart, sizeof(UINT64), 0);
			deviceContext->GetData(rc.end, &frameEnd, sizeof(UINT64), 0);
			float denom = float(tsDisjoint.Frequency);
			float gpuTimer = float(frameEnd-frameStart)/denom;
			printf("-GPU dt: %f\n",gpuTimer);
		}
		return true;
	}
	//printf("query not ready\n");
	return false;
}

void initRenderer(HWND hwnd, int width, int height){
	initSwapChain(hwnd);
	initRenderBuffers(width,height);
	prevFrameRenderCommands.init();
	curFrameRenderCommands.init();
}

void setClearColor(vec4 col){
	curFrameRenderCommands.clearColor=col;
}

DWORD WINAPI renderThread( LPVOID lpParam ) 
{ 
	while(rendering){
		while(frameSync);

		deviceContext->Begin(prevFrameRenderCommands.disjoint);
		deviceContext->End(prevFrameRenderCommands.start);

		clearRenderTarget(prevFrameRenderCommands.clearColor);
		swapChain->Present(1,0);

		deviceContext->End(prevFrameRenderCommands.end);
		deviceContext->End(prevFrameRenderCommands.disjoint);

		frameSync=true;
	}
    return 0; 
} 

#endif
