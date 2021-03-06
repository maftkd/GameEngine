#ifndef SCREEN_H
#define SCREEN_H

#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <assert.h>

HRESULT hr;//for random windows functions
vec2 screenSize;
const float aspect = 16.0f/9.0f;
bool fullScreen=false;
bool initD3Buffers=false;

//timing
int gpuClockFrequency;

//swap chain
ID3D11Device* device= NULL;
ID3D11DeviceContext* deviceContext= NULL;
IDXGISwapChain* swapChain= NULL;

//textures
ID3D11Texture2D* frameBuffer;
ID3D11RenderTargetView* renderTarget = NULL;

//viewport
D3D11_VIEWPORT viewport = {0};

//blending
ID3D11BlendState* default_blend_state;

//buffer
//temp
ID3D11Buffer* charVertices;
ID3D11Buffer* charIndices;
const int maxCharVerts=4096;
const int bytesPerCharVert=16;

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

void initBlendStates(){
	D3D11_BLEND_DESC bs = {0};
	bs.RenderTarget[0].BlendEnable = true;

	//srcAlpha*src.rgb + (1-srcAlpha)*dst.rgb
	bs.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	bs.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	bs.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

	//force 1 alpha in destination
	bs.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	bs.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	bs.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	bs.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = device->CreateBlendState(&bs, &default_blend_state);
	if(FAILED(hr))
		printf("Failed creating blend state  %x\n",hr);
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
	//for storing vertex data - copy is stored in cpu memory but only to pass between frames
	//data is only updated on the gpu on an as needed basis, hence the Changed flag
	float* charVerts;
	bool charVertsChanged;
	int numCharVerts=0;

	renderCommands(){}

	void init(){
		//create gpu profiling queries
		D3D11_QUERY_DESC qdes = {0};
		qdes.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
		device->CreateQuery(&qdes,&disjoint);
		qdes.Query = D3D11_QUERY_TIMESTAMP;
		device->CreateQuery(&qdes,&start);
		device->CreateQuery(&qdes,&end);

		charVerts = new float[(bytesPerCharVert/4)*maxCharVerts];
	}

	void reset(){
		charVertsChanged=false;

		//clear char array
		numCharVerts=0;
	}

	void copy(renderCommands rcq){
		clearColor=rcq.clearColor;

		//text vertices
		bool textChange=false;
		if(numCharVerts!=rcq.numCharVerts)
		{
			textChange=true;
		}
		else{
			for(int i=0;i<rcq.numCharVerts*(bytesPerCharVert/4); i++){
				if(fabsf(rcq.charVerts[i]-charVerts[i])>0.00001f){
					textChange=true;
					break;
				}
			}
		}
		charVertsChanged=textChange;
		if(textChange)
		{
			printf("updating char vertex buffer\n");
			for(int i=0;i<rcq.numCharVerts*(bytesPerCharVert/4); i++)
				charVerts[i]=rcq.charVerts[i];
			numCharVerts=rcq.numCharVerts;
		}
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
			gpuClockFrequency=tsDisjoint.Frequency;
			float denom = float(gpuClockFrequency);
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
	initBlendStates();
	prevFrameRenderCommands.init();
	curFrameRenderCommands.init();

	//init a vertex buffer for some triangles
	D3D11_BUFFER_DESC vertex_buff_descr     = {};
	vertex_buff_descr.ByteWidth             = maxCharVerts*bytesPerCharVert;
	vertex_buff_descr.Usage                 = D3D11_USAGE_DEFAULT;
	vertex_buff_descr.BindFlags             = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA sr_data          = { 0 };
	//prefill with zeros
	float zeros[maxCharVerts*(bytesPerCharVert/4)]={0}; 
	sr_data.pSysMem                         = zeros;
	hr = device->CreateBuffer(
		&vertex_buff_descr,
		&sr_data,
		&charVertices );
	if(FAILED(hr))
		printf("failed making vertex buffer %x\n",hr);
	
	//init an index buffer for some rects
	D3D11_BUFFER_DESC index_buff_descr     = {};
	index_buff_descr.ByteWidth             = (maxCharVerts/4)*6*4;
	index_buff_descr.Usage                 = D3D11_USAGE_IMMUTABLE;//this won't change at runtime
	index_buff_descr.BindFlags             = D3D11_BIND_INDEX_BUFFER;
	D3D11_SUBRESOURCE_DATA sr_data2		   = { 0 };
	//temp
	int * triIndices = new int[(maxCharVerts/4)*6]; 
	int triCount=0;
	for(int i=0; i<maxCharVerts;i+=4){//every 4th vertex requires 2 tris / 6 indices
		triIndices[triCount]=0+i;
		triIndices[triCount+1]=1+i;
		triIndices[triCount+2]=2+i;
		triIndices[triCount+3]=2+i;
		triIndices[triCount+4]=1+i;
		triIndices[triCount+5]=3+i;
		triCount+=6;
	}
	sr_data2.pSysMem						= triIndices;
	hr = device->CreateBuffer(
		&index_buff_descr,
		&sr_data2,
		&charIndices );
	if(FAILED(hr))
		printf("failed making index buffer %x\n",hr);
}

void setClearColor(vec4 col){
	curFrameRenderCommands.clearColor=col;
}

//todo think about this
shader fontShader;
void compileTestShader(){
	//temp - regardless of mode, just compile this test shader
	fontShader.compile(L"shaders/sdfFont.hlsl");
	D3D11_INPUT_ELEMENT_DESC inputElementLayout[] = {
		{ "POS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	fontShader.setLayout(inputElementLayout, 2);
}

//todo think about this
ID3D11ShaderResourceView* fontRes;

//todo move this
//temp code test stride and vert offset
UINT stride = 4*sizeof(float);
UINT vertOffset=0;

DWORD WINAPI renderThread( LPVOID lpParam ) 
{ 
	while(rendering){
		while(frameSync);

		deviceContext->Begin(prevFrameRenderCommands.disjoint);
		deviceContext->End(prevFrameRenderCommands.start);

		//check for a change in char vert buffer
		if(prevFrameRenderCommands.charVertsChanged){
			D3D11_BOX dstBox;
			dstBox.left=0;
			dstBox.right=prevFrameRenderCommands.numCharVerts*4*4;
			dstBox.top=0;
			dstBox.bottom=1;
			dstBox.front=0;
			dstBox.back=1;
			deviceContext->UpdateSubresource(charVertices,0,&dstBox,prevFrameRenderCommands.charVerts,0,0);
			prevFrameRenderCommands.charVertsChanged=false;
			//printf("Updating subresource %d\n",dstBox.right);
		}

		clearRenderTarget(prevFrameRenderCommands.clearColor);

		//set blend state
		deviceContext->OMSetBlendState(default_blend_state,NULL,0xffffffff);

		//draw chars
		fontShader.use();
		deviceContext->PSSetShaderResources(0,1,&fontRes);
		deviceContext->IASetIndexBuffer(charIndices,DXGI_FORMAT_R32_UINT,0);
		deviceContext->IASetVertexBuffers(0,1,&charVertices,&stride,&vertOffset );
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		deviceContext->RSSetViewports( 1, &viewport );
		//printf("Drawing %d verts\n",(prevFrameRenderCommands.numCharVerts/4)*6);
		deviceContext->DrawIndexed((prevFrameRenderCommands.numCharVerts/4)*6,0,0);
		//deviceContext->Draw(prevFrameRenderCommands.numCharVerts,0);

		swapChain->Present(1,0);

		deviceContext->End(prevFrameRenderCommands.end);
		deviceContext->End(prevFrameRenderCommands.disjoint);

		frameSync=true;
	}
    return 0; 
} 

#endif
