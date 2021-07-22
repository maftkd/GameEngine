#ifndef SCREEN_H
#define SCREEN_H

#include <d3d11.h>
#include <dxgi.h>
#include <assert.h>

HRESULT hr;
vec2 screenSize;
bool fullScreen=false;
bool initD3Buffers=false;

//swap chain
ID3D11Device* device= NULL;
ID3D11DeviceContext* deviceContext= NULL;
IDXGISwapChain* swapChain= NULL;
D3D_FEATURE_LEVEL featureLevel;
D3D11_VIEWPORT viewport = {0};

//textures
ID3D11Texture2D* frameBuffer;
ID3D11RenderTargetView* renderTarget = NULL;

void toggleFullScreen(HWND hwnd){
	fullScreen=!fullScreen;
	printf("Toggling fullScreen %d\n",fullScreen);
	if(fullScreen)
	{
		SetWindowLongPtr(hwnd,GWL_STYLE,WS_POPUP| WS_VISIBLE);
		SetWindowPos(hwnd,HWND_NOTOPMOST,0,0,1920,1080,0);
	}
	else
	{
		SetWindowLong(hwnd,GWL_STYLE,WS_OVERLAPPEDWINDOW | WS_VISIBLE);
	}
}

void initSwapChain(HWND hwnd){
	//set up swap chain
	DXGI_SWAP_CHAIN_DESC swap_chain_descr  = { 0 };
	swap_chain_descr.BufferDesc.RefreshRate.Numerator   = 0;
	swap_chain_descr.BufferDesc.RefreshRate.Denominator = 1; 
	swap_chain_descr.BufferDesc.Format  = DXGI_FORMAT_B8G8R8A8_UNORM; 
	swap_chain_descr.SampleDesc.Count   = 1;//one sample - multi sampling not supported with flip-sequential present
	swap_chain_descr.SampleDesc.Quality = 0;                               
	swap_chain_descr.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_descr.BufferCount        = 2;//double buffer                 
	swap_chain_descr.OutputWindow       = hwnd;                
	swap_chain_descr.Windowed           = true;
	swap_chain_descr.SwapEffect			= DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	swap_chain_descr.Flags				= 0;//DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	//UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED;
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
	  &featureLevel,
	  &deviceContext);
	assert( S_OK == hr && swapChain && device&& deviceContext);
	printf("initialized directx device and swap chain\n");
}

void initRenderBuffers(int width, int height){
	//create viewports
	viewport = {
	  0.0f,
	  0.0f,
	  ( FLOAT )( width ),
	  ( FLOAT )( height ),
	  0.0f,
	  1.0f };

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

void handleScreenSizeChange(HWND hwnd, int width, int height){
	printf("Handling screen size %d, %d\n",width,height);
	if(!initD3Buffers){
		initSwapChain(hwnd);
		initRenderBuffers(width,height);
		initD3Buffers=true;
	}

	//tmp code render
	deviceContext->OMSetRenderTargets( 1, &renderTarget, nullptr );
	deviceContext->RSSetViewports( 1, &viewport );
	deviceContext->ClearRenderTargetView(renderTarget, vec4(1,0,1,1).points );
	//#todo learn how to double buffer
	//swapChain->Present(1,1);
	//swapChain->Present(0,DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL);
	//swapChain->Present(0,DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL);
	//swapChain->Present(0,DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL);
	swapChain->Present(1,0);
	//swapChain->Present1(1,DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL,0);
}

#endif
