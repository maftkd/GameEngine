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

#endif
