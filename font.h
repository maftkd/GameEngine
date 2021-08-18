#ifndef FONT_H
#define FONT_H

typedef struct {
	unsigned int sfnt;
	unsigned short numTables;
	unsigned short searchRange;
	unsigned short entrySelector;
	unsigned short rangeShift;
} offsetSubtable;

typedef struct {
	union{
		char tag_c[5];
		unsigned int tag;
	};
	unsigned int checkSum;
	unsigned int offset;
	unsigned int length;
} tableDirectory;

typedef struct {
	offsetSubtable offSub;
	tableDirectory* tableDirs;
} fontDirectory;

typedef struct {
	unsigned short platformID;
	unsigned short encodingID;
	unsigned int offset;
} cmapEncodingSubtable;

typedef struct {
	unsigned short version;
	unsigned short numSubTables;
	cmapEncodingSubtable* subTables;
} cmap;

typedef struct {
	unsigned short  format;
	unsigned short  length;
	unsigned short  language;
	unsigned short  segCountX2;
	unsigned short  searchRange;
	unsigned short  entrySelector;
	unsigned short  rangeShift;
	unsigned short  *endCode;
	unsigned short  reservedPad;
	unsigned short  *startCode;
	unsigned short  *idDelta;
	unsigned short  *idRangeOffset;
	unsigned short  *glyphIdArray;
} format4;

typedef struct {
	unsigned short unitsPerEm;
	short xMin;
	short yMin;
	short xMax;
	short yMax;
	short directionHint;
	short indexToLocFormat;
} header;

typedef struct {
	short numberOfContours;
	short xMin;
	short yMin;
	short xMax;
	short yMax;
	unsigned short* endPointsOfContours;
	unsigned short instructionLength;
	unsigned char* instructions;
	unsigned char* flags;
	unsigned short numPoints;
	short* xCoords;
	short* yCoords;
	bool* onCurve;
} simpleGlyph;

void drawBezier(vec2* contour, int index, int steps, vec2 a, vec2 control, vec2 b){
	for(int i=0; i<steps; i++){
		float t01 = i/(float)(steps);
		vec2 p = lerp(lerp(a,control,t01),lerp(control,b,t01),t01);
		contour[index+i]=p;
	}
}

void sdfTest(simpleGlyph *glyph){
	//get size and bounds
	vec2 mainOffset = vec2(-glyph->xMin,-glyph->yMin);//offset based on ttf data
	vec2 size = vec2(glyph->xMax-glyph->xMin,glyph->yMax-glyph->yMin);
	float imageScale=0.5f;//overall scale from ttf data
	float secondaryScale=0.8f;//scale down glyph for padding
	//scale image down for faster testing
	size*=imageScale;
	printf("mainOffset (%f,%f), size (%f,%f)\n",mainOffset.x,mainOffset.y,size.x,size.y);

	//pad the data so it's centered with some space around the edges
	vec2 paddingOffset = size*(1-secondaryScale)*0.5f;
	printf("paddingOffset (%f,%f)\n",paddingOffset.x,paddingOffset.y);

	//allocate byte array for pixel data
	int numPixels=(int)size.x*(int)size.y;
	int numBytes=numPixels*4;
	unsigned char* pixels = new unsigned char[numBytes];

	//break contours into edge segments
	int bezierCurvePoints=3;
	int numContours = glyph->numberOfContours;
	vec2** contours = new vec2*[numContours];
	int* numContourEdgeSegments = new int[numContours];
	int curPoint=0;

	//go through each contour
	for(int i=0;i<numContours; i++){
		numContourEdgeSegments[i]=1;
		int start=curPoint;
		int end = glyph->endPointsOfContours[i];
		for(int j=start;j<=end; j++){
			if(glyph->onCurve[j])
				numContourEdgeSegments[i]++;
			else
				numContourEdgeSegments[i]+=bezierCurvePoints;
		}

		//allocate enough vec2 for each contour
		contours[i] = new vec2[numContourEdgeSegments[i]];
		int counter=0;
		contours[i][counter]=vec2(glyph->xCoords[start],glyph->yCoords[start]);
		counter++;

		//determine points on each contour - which will connect into edges
		for(int j=start+1;j<=end+1;j++){
			int prev = j-1;
			int cur = j>end?start : j;
			int next = cur+1;
			if(next>end)
				next-=(end-start)+1;
		
			//get glyph coordinates
			vec2 prevVec=vec2(glyph->xCoords[prev],glyph->yCoords[prev]);
			vec2 curVec=vec2(glyph->xCoords[cur],glyph->yCoords[cur]);
			vec2 nextVec=vec2(glyph->xCoords[next],glyph->yCoords[next]);

			//X-1-X
			if(glyph->onCurve[cur]){
				contours[i][counter]=curVec;
				counter++;
			}
			//1-0-0
			else if(glyph->onCurve[prev]&&!glyph->onCurve[cur]&&!glyph->onCurve[next]){
				drawBezier(contours[i],counter,bezierCurvePoints,prevVec,curVec,lerp(curVec,nextVec,0.5f));
				counter+=bezierCurvePoints;
			}
			//1-0-1
			else if(glyph->onCurve[prev]&&!glyph->onCurve[cur]&&glyph->onCurve[next]){
				drawBezier(contours[i],counter,bezierCurvePoints,prevVec,curVec,nextVec);
				counter+=bezierCurvePoints;
			}
			//0-0-1
			else if(!glyph->onCurve[prev]&&!glyph->onCurve[cur]&&glyph->onCurve[next]){
				drawBezier(contours[i],counter,bezierCurvePoints,lerp(prevVec,curVec,0.5f),curVec,nextVec);
				counter+=bezierCurvePoints;
			}
			//0-0-0
			else if(!glyph->onCurve[prev]&&!glyph->onCurve[cur]&&!glyph->onCurve[next]){
				drawBezier(contours[i],counter,bezierCurvePoints,lerp(prevVec,curVec,0.5f),curVec,lerp(curVec,nextVec,0.5f));
				counter+=bezierCurvePoints;
			}
		}
		curPoint=end+1;
	}


	//fill background
	for(int y=0;y<(int)size.y;y++){
		for(int x=0;x<(int)size.x;x++){
			int pixelIndex=y*size.x*4+x*4;
			pixels[pixelIndex]=0x00;
			pixels[pixelIndex+1]=255*(y/(float)(size.y-1));
			pixels[pixelIndex+2]=255*(x/(float)(size.x-1));
			pixels[pixelIndex+3]=255;

			//int nearPoint=0;
			//sdf stuff goes here
		}
	}
	for(int i=0;i<numContours;i++){
		for(int j=1; j<numContourEdgeSegments[i];j++){
			//get prev point
			vec2 prevPoint = contours[i][j-1]+mainOffset;
			prevPoint*=imageScale*secondaryScale;
			prevPoint+=paddingOffset;
			
			//get cur point
			vec2 point = contours[i][j]+mainOffset;
			point*=imageScale*secondaryScale;
			point+=paddingOffset;

			//draw line
			float dist=distance(point,prevPoint);
			int pixelDist=(int)dist;
			for(int k=0; k<pixelDist; k++){
				float t01=k/(float)pixelDist;
				vec2 p = lerp(prevPoint,point,t01);
				p.round();
				int pointIndex=p.y*size.x*4+p.x*4;
				pixels[pointIndex]=0xFF;
				pixels[pointIndex+1]=0x00;
				pixels[pointIndex+2]=0x00;
				pixels[pointIndex+3]=0xFF;
			}
		}
	}
	exportBitmapBgra("images/glyphA.bmp",size.x,size.y,pixels,numBytes);
}

void loadFont(char* fontName){
	printf("loading font %s\n", fontName);
	unsigned char* fontRaw = readAllBytes(fontName);
	fontDirectory mFont;
	//useful things
	unsigned int locaOffset;
	unsigned int locaLength;
	unsigned int glyphStart;
	unsigned int hheaStart;
	unsigned int hmtxStart;
	unsigned short * glyphIndices;
	unsigned short numHMetrics;
	header head;

	//load offset subtable
	memcpy(&mFont.offSub.sfnt, &fontRaw[0],4);
	copyBigEndian(&mFont.offSub.numTables,fontRaw,4);
	printf("num tables: %d\n",mFont.offSub.numTables);
	copyBigEndian(&mFont.offSub.searchRange,fontRaw,6);
	copyBigEndian(&mFont.offSub.entrySelector,fontRaw,8);
	copyBigEndian(&mFont.offSub.rangeShift,fontRaw,10);

	//load table directories
	mFont.tableDirs = new tableDirectory[mFont.offSub.numTables];
	for(int i=0; i<mFont.offSub.numTables; i++){
		memcpy(&mFont.tableDirs[i].tag,&fontRaw[12+i*16],4);
		mFont.tableDirs[i].tag_c[4]=0;
		copyBigEndian(&mFont.tableDirs[i].checkSum,fontRaw,12+i*16+4);
		copyBigEndian(&mFont.tableDirs[i].offset,fontRaw,12+i*16+8);
		copyBigEndian(&mFont.tableDirs[i].length,fontRaw,12+i*16+12);
	}

	//load table stuff
	for(int i=0; i<mFont.offSub.numTables; i++){
		if(strcmp(mFont.tableDirs[i].tag_c,"cmap")==0){
			printf("found cmap!\n");
			cmap foo;
			int cmapOffset = mFont.tableDirs[i].offset;
			copyBigEndian(&foo.version,fontRaw,cmapOffset);
			copyBigEndian(&foo.numSubTables,fontRaw,cmapOffset+2);
			foo.subTables = new cmapEncodingSubtable[foo.numSubTables];
			for(int j=0; j<foo.numSubTables; j++){
				copyBigEndian(&foo.subTables[j].platformID,fontRaw,cmapOffset+4+j*8);
				copyBigEndian(&foo.subTables[j].encodingID,fontRaw,cmapOffset+4+j*8+2);
				copyBigEndian(&foo.subTables[j].offset,fontRaw,cmapOffset+4+j*8+4);
			}
			//encoding sub-table 0 is our go-to for now
			//assuming format 4
			format4 format;
			copyBigEndian(&format.format, fontRaw,cmapOffset+foo.subTables[0].offset);
			copyBigEndian(&format.length, fontRaw,cmapOffset+foo.subTables[0].offset+2);
			copyBigEndian(&format.language, fontRaw,cmapOffset+foo.subTables[0].offset+4);
			copyBigEndian(&format.segCountX2, fontRaw,cmapOffset+foo.subTables[0].offset+6);
			copyBigEndian(&format.searchRange, fontRaw,cmapOffset+foo.subTables[0].offset+8);
			copyBigEndian(&format.entrySelector, fontRaw,cmapOffset+foo.subTables[0].offset+10);
			copyBigEndian(&format.rangeShift, fontRaw,cmapOffset+foo.subTables[0].offset+12);
			int segCount = format.segCountX2/2;
			format.endCode = new unsigned short[segCount];
			format.startCode = new unsigned short[segCount];
			format.idDelta = new unsigned short[segCount];
			format.idRangeOffset = new unsigned short[segCount];
			int segmentOffset = cmapOffset+foo.subTables[0].offset+14;
			for(int j=0; j<segCount; j++){
				copyBigEndian(&format.endCode[j],fontRaw,segmentOffset+j*2);
				copyBigEndian(&format.startCode[j],fontRaw,segmentOffset+2+segCount*2+j*2);
				copyBigEndian(&format.idDelta[j],fontRaw,segmentOffset+2+segCount*4+j*2);
				copyBigEndian(&format.idRangeOffset[j],fontRaw,segmentOffset+2+segCount*6+j*2);
			}
			unsigned int glyphIdStart = cmapOffset+foo.subTables[0].offset+14+segCount*8+2;
			unsigned int remainingBytes = format.length-(14+segCount*4+2);
			format.glyphIdArray = new unsigned short[remainingBytes/2];
			for(int j=0;j<remainingBytes/2;j++){
				copyBigEndian(&format.glyphIdArray[j],fontRaw,glyphIdStart+j*2);
			}

			int numMappings=0;
			for(int j=0;j<segCount; j++){
				//printf("start %d\tend %d\n",format.startCode[j],format.endCode[j]);
				numMappings+=format.endCode[j]-format.startCode[j]+1;
			}
			printf("num mappings %d\n",numMappings);

			glyphIndices = new unsigned short[126-32+1];//indices for chars 32 - 126
			for(int j=0; j<95; j++){
				unsigned short codePoint=j+32;
				bool glyphFound=false;
				for(int k=0; k<segCount&&!glyphFound; k++){
					if(format.endCode[k]>=codePoint){
						if(format.startCode[k]<=codePoint){
							glyphFound=true;
							//glyph found
							if(format.idRangeOffset[k]==0)
							{
								glyphIndices[j]=codePoint+format.idDelta[k];
							}
							else{
								int numElements=format.idRangeOffset[k]/2;
								int elementsRemainInIro=segCount-k;
								int indexInGlyphIdArray=numElements-elementsRemainInIro+(codePoint-format.startCode[k]);
								glyphIndices[j]=format.glyphIdArray[indexInGlyphIdArray]+format.idDelta[k];
							}
						}
						else{
							//no glyph found
							glyphFound=true;
							glyphIndices[j]=0;
						}
					}
				}
			}
		}
		if(strcmp(mFont.tableDirs[i].tag_c,"head")==0){
			printf("found head!\n");
			int headOffset = mFont.tableDirs[i].offset;
			copyBigEndian(&head.unitsPerEm,fontRaw,headOffset+18);
			printf("UPM %d\n",head.unitsPerEm);
			copyBigEndian(&head.xMin,fontRaw,headOffset+36);
			copyBigEndian(&head.yMin,fontRaw,headOffset+38);
			copyBigEndian(&head.xMax,fontRaw,headOffset+40);
			copyBigEndian(&head.yMax,fontRaw,headOffset+42);
			printf("bounding box min: (%d,%d), max: (%d,%d)\n",head.xMin,head.yMin,head.xMax,head.yMax);
			copyBigEndian(&head.directionHint,fontRaw,headOffset+48);
			printf("dir hint: %d\n",head.directionHint);
			copyBigEndian(&head.indexToLocFormat,fontRaw,headOffset+50);
			printf("index to loc format: %d\n",head.indexToLocFormat);
		}
		if(strcmp(mFont.tableDirs[i].tag_c,"loca")==0){
			locaOffset = mFont.tableDirs[i].offset;
			printf("found loca w/ offset %d\n",locaOffset);
			locaLength = mFont.tableDirs[i].length;
			printf("loca length: %d\n",locaLength);
		}
		if(strcmp(mFont.tableDirs[i].tag_c,"glyf")==0){
			glyphStart = mFont.tableDirs[i].offset;
			printf("found glyf w/ offset %d\n",glyphStart);
		}
		if(strcmp(mFont.tableDirs[i].tag_c,"hhea")==0){
			hheaStart = mFont.tableDirs[i].offset;
			printf("found hhea w/ offset %d\n",hheaStart);
			copyBigEndian(&numHMetrics,fontRaw,hheaStart+34);
			printf("Num h metrics: %d\n",numHMetrics);
			short ascent, descent;
			copyBigEndian(&ascent,fontRaw,hheaStart+4);
			copyBigEndian(&descent,fontRaw,hheaStart+6);
			printf("Acender: %d\n",ascent);
			printf("Descender: %d\n",descent);
		}
		if(strcmp(mFont.tableDirs[i].tag_c,"hmtx")==0){
			hmtxStart = mFont.tableDirs[i].offset;
			printf("found hmtx w/ offset %d\n",hmtxStart);
		}
	}
	/*
	unsigned short advanceWidth;
	short leftSideBearing;
	for(int i=0; i<numHMetrics; i++){
		copyBigEndian(&advanceWidth,fontRaw,hmtxStart+i*4);
		copyBigEndian(&leftSideBearing,fontRaw,hmtxStart+i*4+2);
		int charCode=-1;
		for(int j=0; j<95;j++){
			if(glyphIndices[j]==i)
				charCode=j+32;
		}
		if(charCode!=-1)
			printf("Char:%d\t\tAdv: %d\tLSB: %d\n",charCode,advanceWidth,leftSideBearing);
	}
	*/
	
	//glyph time
	unsigned int glyphOffset;
	unsigned short glyphOffsetShort;
	for(int i=0; i<95; i++){
		if(glyphIndices[i]*2>locaLength)
		{
			printf("glyph index too high...\n");
			continue;
		}
		//splt
		if(i+32!=65)
			continue;
		if(head.indexToLocFormat==0)
		{
			copyBigEndian(&glyphOffsetShort,fontRaw,locaOffset+glyphIndices[i]*2);
			glyphOffset=glyphOffsetShort*2;
		}
		else
			copyBigEndian(&glyphOffset,fontRaw,locaOffset+glyphIndices[i]*4);

		unsigned int gStart = glyphStart+glyphOffset;
		//assume simple glyph
		simpleGlyph sg;
		copyBigEndian(&sg.numberOfContours,fontRaw,gStart);
		printf("char code: %d\n",(32+i));
		printf("num contours: %d\n",sg.numberOfContours);
		copyBigEndian(&sg.xMin,fontRaw,gStart+2);
		copyBigEndian(&sg.yMin,fontRaw,gStart+4);
		copyBigEndian(&sg.xMax,fontRaw,gStart+6);
		copyBigEndian(&sg.yMax,fontRaw,gStart+8);
		printf("%d,%d,%d,%d\n",sg.xMin,sg.yMin,sg.xMax,sg.yMax);
		if(sg.numberOfContours<=0)
			continue;
		sg.endPointsOfContours = new unsigned short[sg.numberOfContours];
		for(int j=0; j<sg.numberOfContours; j++){
			copyBigEndian(&sg.endPointsOfContours[j],fontRaw,gStart+10+j*2);
			printf("ct end:%d\n",sg.endPointsOfContours[j]);
		}
		int numPoints = sg.endPointsOfContours[sg.numberOfContours-1]+1;
		sg.numPoints=numPoints;
		printf("num points %d\n",numPoints);
		//instructions
		unsigned int instStart = gStart+10+sg.numberOfContours*2;
		copyBigEndian(&sg.instructionLength,fontRaw,instStart);
		printf("num instructions %d\n",sg.instructionLength);
		//flags
		sg.flags = new unsigned char[numPoints];
		sg.onCurve = new bool [numPoints];
		unsigned int flagsStart = instStart+2+sg.instructionLength;
		for(int j=0; j<numPoints; j++){
			sg.flags[j]=fontRaw[flagsStart];
			sg.onCurve[j]=sg.flags[j]&0x01;
			//check repeat bit
			if(((sg.flags[j]>>3)&0x01)==1)
			{
				flagsStart++;
				int repeatCount = fontRaw[flagsStart];
				while(repeatCount>0)
				{
					j++;
					sg.flags[j]=sg.flags[j-1];
					sg.onCurve[j]=sg.flags[j]&0x01;
					repeatCount--;
				}
			}
			flagsStart++;
		}
		//coords
		sg.xCoords= new short[numPoints];
		sg.yCoords= new short[numPoints];
		unsigned int coordCounter = flagsStart;
		short prevCoord=0;
		short curCoord=0;
		//x coord
		for(int j=0; j<numPoints; j++){
			int flagCombined = ((sg.flags[j]>>1)&0x01) << 1 | ((sg.flags[j]>>4)&0x01);
			switch(flagCombined){
				case 0://signed 16 bit delta
					copyBigEndian(&curCoord,fontRaw,coordCounter);
					coordCounter+=2;
					break;
				case 1://same as prev
				default:
					curCoord=0;
					break;
				case 2://1 byte negative
					curCoord=fontRaw[coordCounter]*-1;
					coordCounter++;
					break;
				case 3://1 byte positive
					curCoord=fontRaw[coordCounter];
					coordCounter++;
					break;
			}
			sg.xCoords[j]=curCoord+prevCoord;
			prevCoord=sg.xCoords[j];
		}
		//y coord
		prevCoord=0;
		for(int j=0; j<numPoints; j++){
			int flagCombined = ((sg.flags[j]>>2)&0x01) << 1 | ((sg.flags[j]>>5)&0x01);
			switch(flagCombined){
				case 0://signed 16 bit delta
					copyBigEndian(&curCoord,fontRaw,coordCounter);
					coordCounter+=2;
					break;
				case 1://same as prev
				default:
					curCoord=0;
					break;
				case 2://1 byte negative
					curCoord=fontRaw[coordCounter]*-1;
					coordCounter++;
					break;
				case 3://1 byte positive
					curCoord=fontRaw[coordCounter];
					coordCounter++;
					break;
			}
			sg.yCoords[j]=curCoord+prevCoord;
			prevCoord=sg.yCoords[j];
			//printf("%d,%d,%d\n",sg.xCoords[j],sg.yCoords[j],sg.onCurve[j]);
		}
		sdfTest(&sg);
	}
}

//splt

void initFontEditor(){
	printf("initializing font editor yo\n");
	//loadFont("fonts/Moonrising.ttf");
	//loadFont("fonts/Gorehand.otf");
	//loadFont("fonts/Envy Code R.ttf");
	loadFont("fonts/Grethania Script Reguler.ttf");
}

void updateFontEditor(){
	setClearColor(vec4(0.1,0.1,0.1,1));

	if(keyInput[32]==1)
	{
		//do something on space
	}
	if(keyInput[82]==1){
		//readTestImage();
	}
}

#endif
