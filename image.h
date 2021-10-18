#ifndef IMAGE_H
#define IMAGE_H

char* testPath = "images/testImage.bmp";

void exportBitmapBgra(char* path, int w, int h, unsigned char* data, int size){
	//header data
	unsigned char header[14] = {0};
	unsigned int fileSize = 122+size;
	unsigned int pixelStart=122;
	header[0]=0x42;
	header[1]=0x4D;
	memcpy(&header[2],&fileSize,4);
	//ignore bytes 6 thru 9
	memcpy(&header[10],&pixelStart,4);
	
	//dib header -V4
	unsigned int dibHeaderSize=108;
	unsigned char dibHeader[108] = {0};
	memcpy(&dibHeader[0],&dibHeaderSize,4);
	memcpy(&dibHeader[4],&w,4);
	memcpy(&dibHeader[8],&h,4);
	unsigned short colorPlanes=1;
	memcpy(&dibHeader[12],&colorPlanes,2);
	unsigned short bbp=32;
	memcpy(&dibHeader[14],&bbp,2);
	unsigned int compressionType=3;
	memcpy(&dibHeader[16],&compressionType,4);
	unsigned int bitmapDataSize=size;
	memcpy(&dibHeader[20],&bitmapDataSize,4);
	int pixelsPerM = 3780;//random default from paint.net
	memcpy(&dibHeader[24],&pixelsPerM,4);
	memcpy(&dibHeader[28],&pixelsPerM,4);
	//leave next 4 for color palette size
	//leave next 4 for important colors
	unsigned int redMask= 0x00FF0000;
	memcpy(&dibHeader[40],&redMask,4);
	unsigned int greenMask= 0x0000FF00;
	memcpy(&dibHeader[44],&greenMask,4);
	unsigned int blueMask= 0x000000FF;
	memcpy(&dibHeader[48],&blueMask,4);
	unsigned int alphaMask= 0xFF000000;
	memcpy(&dibHeader[52],&alphaMask,4);
	unsigned int colorSpace = 0x57696E20;//windows color space
	memcpy(&dibHeader[56],&colorSpace,4);
	
	//write to file
	FILE * f = fopen (path, "wb");
	if (f){
		fwrite(header,1,14,f);
		fwrite(dibHeader,1,108,f);
		fwrite(data,1,size,f);
		fclose(f);
	}
	else
		printf("Error writing to file %s\n",path);
}

void exportBitmapBgr(char* path, int w, int h, unsigned char* data, int size){
	//header data
	unsigned char header[14] = {0};
	unsigned int fileSize = 54+size;
	unsigned int pixelStart=54;
	header[0]=0x42;
	header[1]=0x4D;
	memcpy(&header[2],&fileSize,4);
	//ignore bytes 6 thru 9
	memcpy(&header[10],&pixelStart,4);
	
	//dib header - bitmapinfoheader
	unsigned int dibHeaderSize=40;
	unsigned char dibHeader[40] = {0};
	memcpy(&dibHeader[0],&dibHeaderSize,4);
	memcpy(&dibHeader[4],&w,4);
	memcpy(&dibHeader[8],&h,4);
	unsigned short colorPlanes=1;
	memcpy(&dibHeader[12],&colorPlanes,2);
	unsigned short bbp=24;
	memcpy(&dibHeader[14],&bbp,2);
	unsigned int compressionType=0;
	memcpy(&dibHeader[16],&compressionType,4);
	unsigned int bitmapDataSize=size;
	memcpy(&dibHeader[20],&bitmapDataSize,4);
	int pixelsPerM = 3780;//random default from paint.net
	memcpy(&dibHeader[24],&pixelsPerM,4);
	memcpy(&dibHeader[28],&pixelsPerM,4);
	//leave next 4 for color palette size
	//leave next 4 for important colors
	
	//write to file
	FILE * f = fopen (path, "wb");
	if (f){
		fwrite(header,1,14,f);
		fwrite(dibHeader,1,40,f);
		fwrite(data,1,size,f);
		fclose(f);
	}
	else
		printf("Error writing to file %s\n",path);
}

void genTestImage(){
	//pixel data - BGRA
	int width=256;
	int height=256;
	int numPixels=width*height;
	int numBytes=numPixels*4;
	unsigned char* pixels = new unsigned char[numBytes];
	for(int y=0;y<height;y++){
		for(int x=0;x<width;x++){
			int pixelIndex=y*width*4+x*4;
			pixels[pixelIndex]=0x00;
			pixels[pixelIndex+1]=y;
			pixels[pixelIndex+2]=x;
			pixels[pixelIndex+3]=255;
		}
	}
	exportBitmapBgra(testPath,width,height,pixels,numBytes);
}

unsigned int* importBitmap(char* path,int* width, int* height){
	unsigned char * buff = readAllBytes(path);
	int fsize;
	memcpy(&fsize,&buff[2],4);
	unsigned int dibHeaderSize;
	memcpy(&dibHeaderSize,&buff[14],4);
	printf("Dib header size: %d\n",dibHeaderSize);
	//check if no alpha
	if(dibHeaderSize==40){
		memcpy(width,&buff[18],4);
		memcpy(height,&buff[22],4);
		unsigned int bitmapDataSize;
		memcpy(&bitmapDataSize,&buff[34],4);
		int numPixels = (bitmapDataSize/3);
		printf("data size %d\n",bitmapDataSize);
		printf("num pixels %d\n",numPixels);
		unsigned int * pix = new unsigned int[numPixels];
		int pixIndex=0;
		for(int i=54;i<54+bitmapDataSize;i+=3){
			pix[pixIndex]=(unsigned int)buff[i+2] |
				(unsigned int)buff[i+1] << 8 |
				(unsigned int)buff[i] << 16 |
				0xFF << 24;
			pixIndex++;
		}
		return pix;
	}
	else if(dibHeaderSize==124){
		memcpy(width,&buff[18],4);
		memcpy(height,&buff[22],4);
		unsigned int bitmapDataSize=(*width* *height)*4;
		int numPixels = (bitmapDataSize/4);
		printf("this is unreliable with a dibheader of 124. Usually these use some compression\n");
		printf("and we currently have no idea how that works\n");
		unsigned short bbp;
		memcpy(&bbp,&buff[28],2);
		unsigned int * pix = new unsigned int[numPixels];
		int pixIndex=0;
		for(int i=142;i<142+bitmapDataSize;i+=4){
			pix[pixIndex]=(unsigned int)buff[i+2] |
				(unsigned int)buff[i+1] << 8 |
				(unsigned int)buff[i] << 16 |
				0xFF << 24;
			pixIndex++;
		}
		return pix;

	}
	return NULL;
}

struct tex2D{
	ID3D11Texture2D* tex_ptr;
	ID3D11ShaderResourceView* res_ptr;

	tex2D(){}

	tex2D(char* path){//maybe add DXGI_FORMAT and D3D11_USAGE as params
		int width;
		int height;
		unsigned int* data=importBitmap(path,&width,&height);
		D3D11_SUBRESOURCE_DATA srDes = {0};
		srDes.pSysMem = data;
		srDes.SysMemPitch=width*4;//4 rgba
		srDes.SysMemSlicePitch = 0;
		D3D11_TEXTURE2D_DESC tDes;
		tDes.Width = width;
		tDes.Height = height;
		tDes.MipLevels = 1;
		tDes.ArraySize=1;
		tDes.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		//tDes.Format = DXGI_FORMAT_R8G8B8A8_UINT;
		tDes.SampleDesc.Count = 1;
		tDes.SampleDesc.Quality = 0;
		tDes.Usage = D3D11_USAGE_DEFAULT;
		tDes.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		tDes.CPUAccessFlags = 0;
		tDes.MiscFlags = 0;//D3D11_RESOURCE_MISC_GENERATE_MIPS;
		hr = device->CreateTexture2D( &tDes, &srDes, &tex_ptr);
		if(FAILED(hr))
			printf("Failed creating texture2d tex%x\n",hr);

		//shader resource view
		hr = device->CreateShaderResourceView(tex_ptr, NULL, &res_ptr);
		if(FAILED(hr))
			printf("Failed creating shader resource %d\n",hr);
		//device_context_ptr->GenerateMips(res_ptr);
	}
};

#endif
