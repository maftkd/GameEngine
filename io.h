#ifndef IO_H
#define IO_H

unsigned char * readAllBytes(char* fn){
	unsigned char * buff = 0;
	long length;
	FILE * f = fopen (fn, "rb");
	if (f){
		fseek(f,0,SEEK_END);
		length = ftell(f);
		fseek(f,0,SEEK_SET);
		buff=(unsigned char*)malloc(length+1);
		if(buff){
			fread(buff,1,length,f);
		}
		fclose(f);
		buff[length]='\0';
	}
	unsigned char * bytes = new unsigned char[length];
	memcpy(&bytes[0],&buff[0],length);
	return bytes;
}

void writeAllBytes(char* path, unsigned char* data,int size){
	FILE * f = fopen (path, "wb");
	if (f){
		fwrite(data,1,size,f);
		fclose(f);
	}
	else
		printf("Error writing to file %s\n",path);
}

void copyBigEndian(unsigned short* val, unsigned char* data, int offset){
	memcpy(val,&data[offset],2);
	*val = (*val)<<8 | (*val)>>8;
}

void copyBigEndian(short* val, unsigned char* data, int offset){
	memcpy(val,&data[offset],2);
	*val = (*val)<<8 | ((*val)>>8 & 0xFF);
}

void copyBigEndian(unsigned int* val, unsigned char* data, int offset){
	memcpy(val,&data[offset],4);
	*val = ((*val>>24)&0xff) | // move byte 3 to byte 0
		((*val<<8)&0xff0000) | // move byte 1 to byte 2
		((*val>>8)&0xff00) | // move byte 2 to byte 1
		((*val<<24)&0xff000000); // byte 0 to byte 3
}

#endif
