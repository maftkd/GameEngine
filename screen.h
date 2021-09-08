#ifndef SCREEN_H
#define SCREEN_H

#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <assert.h>

HRESULT hr;//for random windows functions
vec2 screenSize;
bool fullScreen=false;
bool initD3Buffers=false;

//swap chain
ID3D11Device* device= NULL;
ID3D11DeviceContext* deviceContext= NULL;
IDXGISwapChain* swapChain= NULL;

//textures
ID3D11Texture2D* frameBuffer;
ID3D11RenderTargetView* renderTarget = NULL;

//viewport
D3D11_VIEWPORT viewport = {0};

//buffer
//temp
ID3D11Buffer* testVertices;

void initRenderBuffers(int width, int height){

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

	viewport = {
	  0.0f,
	  0.0f,
	  ( FLOAT )( width ),
	  ( FLOAT )( height ),
	  0.0f,
	  1.0f };
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
	deviceContext->ClearRenderTargetView(renderTarget, col.points );
}

struct shader{
	ID3DBlob* vsBlob;
	ID3D11VertexShader* vert = NULL;
	ID3D11PixelShader* pix   = NULL;
	ID3D11InputLayout* layout   = NULL;

	shader(){}

	void compile(LPCWSTR path){
		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
		ID3DBlob *ps_blob_ptr = NULL, *error_blob = NULL;
		//vert
		hr = D3DCompileFromFile(path,nullptr,D3D_COMPILE_STANDARD_FILE_INCLUDE,
		  "vs_main","vs_5_0",flags,0,&vsBlob,&error_blob );
		if ( FAILED( hr ) ) {
			if ( error_blob ) {
				printf("error in vert shader %s\n", (char*)error_blob->GetBufferPointer());
				error_blob->Release();
			}
			if ( vsBlob ) { vsBlob->Release(); }
			assert( false );
		}
		else
			printf("compiled shader %s [vert]\n",(char*)path);

		//pixel
		hr = D3DCompileFromFile(path,nullptr,D3D_COMPILE_STANDARD_FILE_INCLUDE,
		  "ps_main","ps_5_0",flags,0,&ps_blob_ptr,&error_blob );
		if ( FAILED( hr ) ) {
			if ( error_blob ) {
				printf("error in frag shader %s\n", (char*)error_blob->GetBufferPointer());
				error_blob->Release();
			}
			if ( ps_blob_ptr ) { ps_blob_ptr->Release(); }
			assert( false );
		}	
		else
			printf("compiled shader %s [pixel]\n",(char*)path);

		//create shaders
		hr = device->CreateVertexShader(vsBlob->GetBufferPointer(),
				vsBlob->GetBufferSize(),NULL,&vert );
		assert( SUCCEEDED( hr ) );
		hr = device->CreatePixelShader(ps_blob_ptr->GetBufferPointer(),
				ps_blob_ptr->GetBufferSize(),NULL,&pix );
		assert( SUCCEEDED( hr ) );
	}

	void setLayout(D3D11_INPUT_ELEMENT_DESC inputElemDes[], int elems){
		hr = device->CreateInputLayout(
			inputElemDes,
			elems,
			vsBlob->GetBufferPointer(),
			vsBlob->GetBufferSize(),
			&layout );
		assert( SUCCEEDED( hr ) );
	}

	void use(){
		deviceContext->VSSetShader( vert, NULL, 0 );
		deviceContext->PSSetShader( pix, NULL, 0 );
		deviceContext->IASetInputLayout( layout );
	}
	void Release(){
		vsBlob->Release();
		vert->Release();
		pix->Release();
		layout->Release();
	}
};

struct renderCommands{
	vec4 clearColor;
	ID3D11Query* disjoint;
	ID3D11Query* start;
	ID3D11Query* end;
	//store triangle data
	//todo rename this
	//ID3D11Buffer* testVertices;
	//temp stuff
	float* verts;
	bool vertsChanged;
	int numVerts=0;
	//temp - drawing a single triangle
	int maxVerts=9;
	int bytesPerVert = 8;

	renderCommands(){}

	void init(){
		//create gpu profiling queries
		D3D11_QUERY_DESC qdes = {0};
		qdes.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
		device->CreateQuery(&qdes,&disjoint);
		qdes.Query = D3D11_QUERY_TIMESTAMP;
		device->CreateQuery(&qdes,&start);
		device->CreateQuery(&qdes,&end);

		verts = new float[maxVerts*2];

		//todo can we clean this up at all
		//temp - later we should encapsulate this into a helper method
		//create vertex buffer
		/*
		D3D11_BUFFER_DESC vertex_buff_descr     = {};
		vertex_buff_descr.ByteWidth             = maxVerts*bytesPerVert;
		vertex_buff_descr.Usage                 = D3D11_USAGE_DEFAULT;
		vertex_buff_descr.BindFlags             = D3D11_BIND_VERTEX_BUFFER;
		D3D11_SUBRESOURCE_DATA sr_data          = { 0 };
		//temp
		float zeros[6]={-0.5,-0.5,0.0,0.5,0.5,-0.5}; 
		sr_data.pSysMem                         = zeros;
		hr = device->CreateBuffer(
			&vertex_buff_descr,
			&sr_data,
			&testVertices );
		if(FAILED(hr))
			printf("failed making vertex buffer %x\n",hr);
			*/
	}

	void reset(){
		vertsChanged=false;
	}

	void copy(renderCommands rcq){
		clearColor=rcq.clearColor;
		//copy verts
		for(int i=0;i<maxVerts*2; i++)
			verts[i]=rcq.verts[i];
		numVerts=rcq.numVerts;
		vertsChanged=rcq.vertsChanged;
		//need to think about how to copy buffers in an efficient way
		//i.e. only when changes occur
		//just make sure we really need two vertex buffers for our pipelined renderer
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
			//printf("-GPU dt: %f\n",gpuTimer);
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

	//init a vertex buffer for some triangles
	D3D11_BUFFER_DESC vertex_buff_descr     = {};
	vertex_buff_descr.ByteWidth             = curFrameRenderCommands.maxVerts*curFrameRenderCommands.bytesPerVert;
	vertex_buff_descr.Usage                 = D3D11_USAGE_DEFAULT;
	vertex_buff_descr.BindFlags             = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA sr_data          = { 0 };
	//temp
	float zeros[18]={0}; 
	sr_data.pSysMem                         = zeros;
	hr = device->CreateBuffer(
		&vertex_buff_descr,
		&sr_data,
		&testVertices );
	if(FAILED(hr))
		printf("failed making vertex buffer %x\n",hr);
}

void setClearColor(vec4 col){
	curFrameRenderCommands.clearColor=col;
}

//temp - this method seems pretty useless for now save some testing
void addTriangle(float x1, float y1, float x2, float y2, float x3, float y3){
	int startIndex=curFrameRenderCommands.numVerts*2;
	curFrameRenderCommands.verts[startIndex]=x1;
	curFrameRenderCommands.verts[startIndex+1]=y1;
	curFrameRenderCommands.verts[startIndex+2]=x2;
	curFrameRenderCommands.verts[startIndex+3]=y2;
	curFrameRenderCommands.verts[startIndex+4]=x3;
	curFrameRenderCommands.verts[startIndex+5]=y3;
	curFrameRenderCommands.numVerts+=3;

	curFrameRenderCommands.vertsChanged=true;
	printf("verts changed= true!\n");
}

//todo think about this
shader testShader;
void compileTestShader(){
	//temp - regardless of mode, just compile this test shader
	testShader.compile(L"shaders/test.hlsl");
	D3D11_INPUT_ELEMENT_DESC inputElementLayout[] = {
		{ "POS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	testShader.setLayout(inputElementLayout, 1);
}

//todo move this
//temp code test stride and vert offset
UINT stride = 2*sizeof(float);
UINT vertOffset=0;

DWORD WINAPI renderThread( LPVOID lpParam ) 
{ 
	while(rendering){
		while(frameSync);

		deviceContext->Begin(prevFrameRenderCommands.disjoint);
		deviceContext->End(prevFrameRenderCommands.start);

		//check triangle test buffer
		//temp
		if(prevFrameRenderCommands.vertsChanged){
			//update subresource
			D3D11_BOX dstBox;
			dstBox.left=0;
			dstBox.right=prevFrameRenderCommands.numVerts*2*4;
			dstBox.top=0;
			dstBox.bottom=1;
			dstBox.front=0;
			dstBox.back=1;
			deviceContext->UpdateSubresource(testVertices,0,&dstBox,prevFrameRenderCommands.verts,0,0);
			prevFrameRenderCommands.vertsChanged=false;
			printf("Updating subresource\n");
		}

		clearRenderTarget(prevFrameRenderCommands.clearColor);

		//test draw triangle
		//draw triangle from previous frame render commands
		testShader.use();
		deviceContext->IASetVertexBuffers(0,1,&testVertices,&stride,&vertOffset );
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		deviceContext->RSSetViewports( 1, &viewport );
		//todo pass in the number of vertices
		deviceContext->Draw(prevFrameRenderCommands.numVerts,0);
		//printf("num verts %d\n",prevFrameRenderCommands.numVerts);

		swapChain->Present(1,0);

		deviceContext->End(prevFrameRenderCommands.end);
		deviceContext->End(prevFrameRenderCommands.disjoint);

		frameSync=true;
	}
    return 0; 
} 

#endif
