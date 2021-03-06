#ifndef FONT_H
#define FONT_H

const short numChars=95;
const short charOffset=32;

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
header head;

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
	vec2 size;
	vec2 offset;
	short leftSideBearing;
	unsigned short advanceWidth;
	float glyphScale;
} simpleGlyph;


void drawBezier(vec2* contour, int index, int steps, vec2 a, vec2 control, vec2 b){
	for(int i=0; i<steps; i++){
		float t01 = i/(float)(steps);
		vec2 p = lerp(lerp(a,control,t01),lerp(control,b,t01),t01);
		contour[index+i]=p;
	}
}

//temp - node class for texture packing data structure
//should probably go in image.h and should be decoupled from simpleGlyphs
struct node{
	node* children;
	//x,y = offset z,w = extents
	vec4 box;
	int boxIndex=0;

	node(){
		boxIndex=-2;
	}

	node(vec4 b){
		box=b;
		boxIndex=-1;
	}

	void init(vec4 b){
		box=b;
		boxIndex=-1;
		children = new node[2];
	}

	bool isOccupied(){
		return boxIndex!=-1;
	}

	bool hasRoom(vec2 extents){
		return extents.x<=box.z&&extents.y<=box.w;
	}

	bool justRight(vec2 extents){
		return (box.z-extents.x<1)&&(box.w-extents.y<1);
	}

	bool isLeaf(){
		return children[0].boxIndex==-2 && children[1].boxIndex==-2;
	}

	bool insert(simpleGlyph * glyphs, int i){
		vec2 b = glyphs[i].size;
		if(!isLeaf()){
			if(children[0].insert(glyphs,i))
				return true;
			return children[1].insert(glyphs,i);
		}
		if(isOccupied())
			return false;
		if(!hasRoom(b))
			return false;
		if(justRight(b))
		{
			boxIndex=i;
			glyphs[i].offset=vec2(box.x,box.y);
			return true;
		}
		//gotta split
		//decide which way to split
		float dw = box.z - b.x;
		float dh = box.w - b.y;
		if(dw>dh){//horizontal split
			children[0].init(vec4(box.x,box.y,b.x,box.w));
			children[1].init(vec4(box.x+b.x,box.y,box.z-b.x,box.w));
		}
		else{
			children[0].init(vec4(box.x,box.y,box.z,b.y));
			children[1].init(vec4(box.x,box.y+b.y,box.z,box.w-b.y));
		}
		return children[0].insert(glyphs,i);
	}
};

//simple func to pack the glyphs in a grid layout
//also generates accompanying .fdat file
void fontAtlasTest(simpleGlyph *glyphs,int texSize=512,float glyphSize=32,float secondaryScale=0.75){
	//get bounds
	for(int i=0; i<numChars;i++){
		vec2 size = vec2(glyphs[i].xMax-glyphs[i].xMin,glyphs[i].yMax-glyphs[i].yMin);
		//scale image down for faster testing
		//todo, try and make these things square and see if it fixes the l's
		float largeSide = max(size.x,size.y);
		float imageScale=glyphSize/largeSide;
		glyphs[i].glyphScale=imageScale;
		size*=imageScale;
		size.round();
		//printf("%d,%f,%f\n",(i+32),size.x,size.y);
		glyphs[i].size=size;
	}

	int * mapping = new int[numChars];
	for(int i=0;i<numChars;i++){
		mapping[i]=i;
	}
	//bubble sort these guys biggest to smallest
	for(int i=0; i<numChars-1; i++){
		for(int j=0; j<numChars-1-i;j++){
			//compare area
			float aCur=glyphs[j].size.x*glyphs[j].size.y;
			float aNext=glyphs[j+1].size.x*glyphs[j+1].size.y;
			if(aCur<aNext){
				simpleGlyph tmp = glyphs[j];
				glyphs[j]=glyphs[j+1];
				glyphs[j+1]=tmp;
				int tmpI = mapping[j];
				mapping[j]=mapping[j+1];
				mapping[j+1]=tmpI;
			}
		}
	}

	bool fit=false;
	bool first=true;
	while(!fit){
		if(!first)
			texSize+=100;
		first=false;
		fit=true;
		//insert the glyphs
		node root;
		root.init(vec4(0,0,texSize,texSize));
		for(int i=0; i<numChars; i++){
			if(!root.insert(glyphs,i))
				fit=false;
		}
	}

	//generate sdf data and copy to texture data
	int numPixels=texSize*texSize;
	int numBytes=numPixels*3;
	unsigned char* pixels= new unsigned char[numBytes];
	for(int i=0; i<numChars; i++){
		printf("rasterizing %c\n",(mapping[i]+32));
		vec2 mainOffset = vec2(-glyphs[i].xMin,-glyphs[i].yMin);//offset based on ttf data

		//pad the data so it's centered with some space around the edges
		vec2 paddingOffset = glyphs[i].size*(1-secondaryScale)*0.5f;
		paddingOffset.round();

		//break contours into edge segments
		int bezierCurvePoints=3;
		int numContours = glyphs[i].numberOfContours;

		//skip for 'space'
		if(numContours<=0 || mapping[i]+32==32)
			continue;
		vec2** contours = new vec2*[numContours];
		int* numContourEdgeSegments = new int[numContours];
		int curPoint=0;

		//go through each contour and gen points
		for(int j=0;j<numContours; j++){
			numContourEdgeSegments[j]=1;
			int start=curPoint;
			int end = glyphs[i].endPointsOfContours[j];
			for(int k=start;k<=end; k++){
				if(glyphs[i].onCurve[k])
					numContourEdgeSegments[j]++;
				else
					numContourEdgeSegments[j]+=bezierCurvePoints;
			}

			//allocate enough vec2 for each contour
			contours[j] = new vec2[numContourEdgeSegments[j]];
			int counter=0;
			contours[j][counter]=vec2(glyphs[i].xCoords[start],glyphs[i].yCoords[start]);
			counter++;

			//determine points on each contour - which will connect into edges
			for(int k=start+1;k<=end+1;k++){
				int prev = k-1;
				int cur = k>end?start : k;
				int next = cur+1;
				if(next>end)
					next-=(end-start)+1;
			
				//get glyphs[i] coordinates
				vec2 prevVec=vec2(glyphs[i].xCoords[prev],glyphs[i].yCoords[prev]);
				vec2 curVec=vec2(glyphs[i].xCoords[cur],glyphs[i].yCoords[cur]);
				vec2 nextVec=vec2(glyphs[i].xCoords[next],glyphs[i].yCoords[next]);

				//X-1-X
				if(glyphs[i].onCurve[cur]){
					contours[j][counter]=curVec;
					counter++;
				}
				//1-0-0
				else if(glyphs[i].onCurve[prev]&&!glyphs[i].onCurve[cur]&&!glyphs[i].onCurve[next]){
					drawBezier(contours[j],counter,bezierCurvePoints,prevVec,curVec,lerp(curVec,nextVec,0.5f));
					counter+=bezierCurvePoints;
				}
				//1-0-1
				else if(glyphs[i].onCurve[prev]&&!glyphs[i].onCurve[cur]&&glyphs[i].onCurve[next]){
					drawBezier(contours[j],counter,bezierCurvePoints,prevVec,curVec,nextVec);
					counter+=bezierCurvePoints;
				}
				//0-0-1
				else if(!glyphs[i].onCurve[prev]&&!glyphs[i].onCurve[cur]&&glyphs[i].onCurve[next]){
					drawBezier(contours[j],counter,bezierCurvePoints,lerp(prevVec,curVec,0.5f),curVec,nextVec);
					counter+=bezierCurvePoints;
				}
				//0-0-0
				else if(!glyphs[i].onCurve[prev]&&!glyphs[i].onCurve[cur]&&!glyphs[i].onCurve[next]){
					drawBezier(contours[j],counter,bezierCurvePoints,lerp(prevVec,curVec,0.5f),curVec,lerp(curVec,nextVec,0.5f));
					counter+=bezierCurvePoints;
				}
			}
			curPoint=end+1;
		}
		
		//scale and translate points to fit image
		for(int j=0;j<numContours;j++){
			for(int k=0; k<numContourEdgeSegments[j];k++){
				vec2 point = contours[j][k]+mainOffset;
				point*=glyphs[i].glyphScale*secondaryScale;
				point+=paddingOffset;
				contours[j][k]=point;
			}
		}

		float maxDist=max(1,min(paddingOffset.x,paddingOffset.y));
		//fill pixel array
		int pixelStart=(int)glyphs[i].offset.y*texSize*3+(int)glyphs[i].offset.x*3;
		//printf("size (%f,%f)\n",glyphs[i].size.x,glyphs[i].size.y);
		//printf("offset (%f,%f)\n",glyphs[i].offset.x,glyphs[i].offset.y);
		//printf("filling pixels starting at: %d\n",pixelStart);
		for(int y=0;y<(int)glyphs[i].size.y;y++){
			for(int x=0;x<(int)glyphs[i].size.x;x++){
				int pixelIndex=pixelStart+y*texSize*3+x*3;
				if(pixelIndex>=numBytes){
					printf("font data overflowing atlas size. Try a larger atlas\n");
					return;
				}
				
				//sdf stuff goes here
				float minDist=1000;
				float minSign=1;
				int minI, minJ;
				float samesies=0;
				vec2 p = vec2(x,y);
				for(int j=0;j<numContours;j++){
					for(int k=1; k<numContourEdgeSegments[j];k++){
						//get prev point
						vec2 prevPoint = contours[j][k-1];
						//get cur point
						vec2 point = contours[j][k];
						if(prevPoint==point)
							continue;
						float md = signedDistanceToEdge(prevPoint,point,p);
						if(fabsf(md)<minDist)
						{
							minDist=fabsf(md);
							minSign=sign(md);
							minI=j;
							minJ=k;
						}
						else if(fabsf(md)==minDist)
							samesies=minDist;
					}
				}
				if(minDist>maxDist)
					minDist=maxDist;
				float normDist=(minSign*minDist)/maxDist;
				normDist=(normDist+1)*0.5f;
				pixels[pixelIndex]=(int)(255*normDist);//b
				pixels[pixelIndex+1]=(int)(255*normDist);//g
				pixels[pixelIndex+2]=(int)(255*normDist);//r
			}
		}
	}
	exportBitmapBgr("fonts/fontAtlas.bmp",texSize,texSize,pixels,numBytes);

	printf("done exporting bitmap, time to write glyph data\n");
	const int glyphStride=4*4+6*2;
	const int headerSize=6;
	const int fontDataSize=numChars*(glyphStride)+headerSize;
	unsigned char fontDataTest[fontDataSize] = {0};
	float pad=1-secondaryScale;
	//first padding
	memcpy(&fontDataTest[0],&pad,4);
	//then units per em
	memcpy(&fontDataTest[4],&head.unitsPerEm,2);
	//then glyph data
	for(int i=0; i<numChars; i++){
		int gIndex = mapping[i];
		//normalize pixel cords to UV cords
		float uLeft=glyphs[i].offset.x/texSize;
		float vBot=glyphs[i].offset.y/texSize;
		float uRight=uLeft+glyphs[i].size.x/texSize;
		float vTop=vBot+glyphs[i].size.y/texSize;
		//copy uv cords
		memcpy(&fontDataTest[headerSize+gIndex*glyphStride],&uLeft,4);
		memcpy(&fontDataTest[headerSize+gIndex*glyphStride+4],&vBot,4);
		memcpy(&fontDataTest[headerSize+gIndex*glyphStride+8],&uRight,4);
		memcpy(&fontDataTest[headerSize+gIndex*glyphStride+12],&vTop,4);
		//copy bounds
		memcpy(&fontDataTest[headerSize+gIndex*glyphStride+16],&glyphs[i].xMin,2);
		memcpy(&fontDataTest[headerSize+gIndex*glyphStride+18],&glyphs[i].yMin,2);
		memcpy(&fontDataTest[headerSize+gIndex*glyphStride+20],&glyphs[i].xMax,2);
		memcpy(&fontDataTest[headerSize+gIndex*glyphStride+22],&glyphs[i].yMax,2);
		//left side bearing
		memcpy(&fontDataTest[headerSize+gIndex*glyphStride+24],&glyphs[i].leftSideBearing,2);
		//advance width
		memcpy(&fontDataTest[headerSize+gIndex*glyphStride+26],&glyphs[i].advanceWidth,2);
	}
	writeAllBytes("fonts/fontAtlas.fdat",fontDataTest,fontDataSize);
}

void importFont(char* fontName){
	printf("importing font %s\n", fontName);
	unsigned char* fontRaw = readAllBytes(fontName);
	fontDirectory mFont;
	//useful things
	unsigned int locaOffset;
	unsigned int locaLength;
	unsigned int glyphStart;
	unsigned int hheaStart;
	unsigned int hmtxStart;
	unsigned int kernStart;
	unsigned short * glyphIndices;
	unsigned short numHMetrics;

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
		//printf("table: %s\n",mFont.tableDirs[i].tag_c);
		copyBigEndian(&mFont.tableDirs[i].checkSum,fontRaw,12+i*16+4);
		copyBigEndian(&mFont.tableDirs[i].offset,fontRaw,12+i*16+8);
		copyBigEndian(&mFont.tableDirs[i].length,fontRaw,12+i*16+12);
	}

	//load table data - except glyph data
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
			for(int j=0; j<numChars; j++){
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
		if(strcmp(mFont.tableDirs[i].tag_c,"kern")==0){
			kernStart = mFont.tableDirs[i].offset;
			unsigned int version;
			unsigned int numTables;
			copyBigEndian(&version,fontRaw,kernStart);
			printf("found kern table @ %d, version: %d\n",kernStart,version);
		}
	}
	
	//glyph time
	unsigned int glyphOffset;
	unsigned short glyphOffsetShort;
	int numGlyphs=numChars;
	simpleGlyph* glyphs = new simpleGlyph[numGlyphs];
	for(int i=0; i<numGlyphs; i++){
		//printf("reading glyph data at index %d. glyph index %d\n",i,glyphIndices[i]);
		if(glyphIndices[i]*2>locaLength)
		{
			printf("glyph index too high...\n");
			continue;
		}
		//splt
		/*
		if(i+32!=46)
			continue;
			*/
		if(head.indexToLocFormat==0)
		{
			copyBigEndian(&glyphOffsetShort,fontRaw,locaOffset+glyphIndices[i]*2);
			glyphOffset=glyphOffsetShort*2;
		}
		else
			copyBigEndian(&glyphOffset,fontRaw,locaOffset+glyphIndices[i]*4);

		unsigned int gStart = glyphStart+glyphOffset;
		//assume simple glyph
		copyBigEndian(&glyphs[i].numberOfContours,fontRaw,gStart);
		copyBigEndian(&glyphs[i].xMin,fontRaw,gStart+2);
		copyBigEndian(&glyphs[i].yMin,fontRaw,gStart+4);
		copyBigEndian(&glyphs[i].xMax,fontRaw,gStart+6);
		copyBigEndian(&glyphs[i].yMax,fontRaw,gStart+8);
		//printf("%d,%d,%d,%d\n",glyphs[i].xMin,glyphs[i].yMin,glyphs[i].xMax,glyphs[i].yMax);
		if(glyphs[i].numberOfContours<=0)
			continue;
		glyphs[i].endPointsOfContours = new unsigned short[glyphs[i].numberOfContours];
		for(int j=0; j<glyphs[i].numberOfContours; j++){
			copyBigEndian(&glyphs[i].endPointsOfContours[j],fontRaw,gStart+10+j*2);
			//printf("ct end:%d\n",glyphs[i].endPointsOfContours[j]);
		}
		int numPoints = glyphs[i].endPointsOfContours[glyphs[i].numberOfContours-1]+1;
		glyphs[i].numPoints=numPoints;
		//printf("num points %d\n",numPoints);
		//instructions
		unsigned int instStart = gStart+10+glyphs[i].numberOfContours*2;
		copyBigEndian(&glyphs[i].instructionLength,fontRaw,instStart);
		//printf("num instructions %d\n",glyphs[i].instructionLength);
		//flags
		glyphs[i].flags = new unsigned char[numPoints];
		glyphs[i].onCurve = new bool [numPoints];
		unsigned int flagsStart = instStart+2+glyphs[i].instructionLength;
		for(int j=0; j<numPoints; j++){
			glyphs[i].flags[j]=fontRaw[flagsStart];
			glyphs[i].onCurve[j]=glyphs[i].flags[j]&0x01;
			//check repeat bit
			if(((glyphs[i].flags[j]>>3)&0x01)==1)
			{
				flagsStart++;
				int repeatCount = fontRaw[flagsStart];
				while(repeatCount>0)
				{
					j++;
					glyphs[i].flags[j]=glyphs[i].flags[j-1];
					glyphs[i].onCurve[j]=glyphs[i].flags[j]&0x01;
					repeatCount--;
				}
			}
			flagsStart++;
		}
		//coords
		glyphs[i].xCoords= new short[numPoints];
		glyphs[i].yCoords= new short[numPoints];
		unsigned int coordCounter = flagsStart;
		short prevCoord=0;
		short curCoord=0;
		//x coord
		for(int j=0; j<numPoints; j++){
			int flagCombined = ((glyphs[i].flags[j]>>1)&0x01) << 1 | ((glyphs[i].flags[j]>>4)&0x01);
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
			glyphs[i].xCoords[j]=curCoord+prevCoord;
			prevCoord=glyphs[i].xCoords[j];
		}
		//y coord
		prevCoord=0;
		for(int j=0; j<numPoints; j++){
			int flagCombined = ((glyphs[i].flags[j]>>2)&0x01) << 1 | ((glyphs[i].flags[j]>>5)&0x01);
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
			glyphs[i].yCoords[j]=curCoord+prevCoord;
			prevCoord=glyphs[i].yCoords[j];
			//printf("%d,%d,%d\n",glyphs[i].xCoords[j],glyphs[i].yCoords[j],glyphs[i].onCurve[j]);
		}
		//sdfTest(&sg);
	}

	//horizontal metrics
	unsigned short advanceWidth;
	short leftSideBearing;
	for(int i=0; i<numHMetrics; i++){
		copyBigEndian(&advanceWidth,fontRaw,hmtxStart+i*4);
		copyBigEndian(&leftSideBearing,fontRaw,hmtxStart+i*4+2);
		int charCode=-1;
		for(int j=0; j<numChars;j++){
			if(glyphIndices[j]==i)
				charCode=j+32;
		}
		if(charCode!=-1)
		{
			//printf("Char:%d\t\tAdv: %d\tLSB: %d\n",charCode,advanceWidth,leftSideBearing);
			//save advance width and lsb to the glyphs array
			glyphs[charCode-32].leftSideBearing=leftSideBearing;
			glyphs[charCode-32].advanceWidth=advanceWidth;

		}
	}

	fontAtlasTest(glyphs,512,48,0.6);
}

//hmmm maybe these structs should go at the top?
tex2D fontTex;
typedef struct {
	float uLeft;
	float vBot;
	float uRight;
	float vTop;
	short xMin;
	short yMin;
	short xMax;
	short yMax;
	short lsb;
	unsigned short advance;
} glyphData;
typedef struct {
	glyphData* gData;
	unsigned short upm;
	float padding;
} fontData;
fontData mFontData;
void loadFont(char* path){
	char * dataPath = new char[strlen(path)+5];
	char * texPath = new char[strlen(path)+4];
	strcpy(dataPath,path);
	strcpy(texPath,path);
	char * dataExt = ".fdat";
	char * texExt = ".bmp";
	strncat(dataPath,dataExt,5);
	strncat(texPath,texExt,4);
	printf("loading font data from %s\n",dataPath);
	unsigned char * fdBytes = readAllBytes(dataPath);
	//tmp just logs this stuff out
	//to do save these numbers in some font data struct
	float uLeft;
	float vBot;
	float uRight;
	float vTop;
	//fontDat = new fd[128];
	mFontData.gData = new glyphData[128];
	//read header
	memcpy(&mFontData.padding,&fdBytes[0],4);
	memcpy(&mFontData.upm,&fdBytes[4],2);
	int headerSize=6;
	int glyphStride=4*4+6*2;
	//read font data
	for(int i=0; i<numChars; i++){
		int charCode=i+32;
		//get uv coords
		memcpy(&mFontData.gData[charCode].uLeft,&fdBytes[headerSize+i*glyphStride],4);
		memcpy(&mFontData.gData[charCode].vBot,&fdBytes[headerSize+i*glyphStride+4],4);
		memcpy(&mFontData.gData[charCode].uRight,&fdBytes[headerSize+i*glyphStride+8],4);
		memcpy(&mFontData.gData[charCode].vTop,&fdBytes[headerSize+i*glyphStride+12],4);
		//get pixel bounds
		memcpy(&mFontData.gData[charCode].xMin,&fdBytes[headerSize+i*glyphStride+16],2);
		memcpy(&mFontData.gData[charCode].yMin,&fdBytes[headerSize+i*glyphStride+18],2);
		memcpy(&mFontData.gData[charCode].xMax,&fdBytes[headerSize+i*glyphStride+20],2);
		memcpy(&mFontData.gData[charCode].yMax,&fdBytes[headerSize+i*glyphStride+22],2);
		//get lsb
		memcpy(&mFontData.gData[charCode].lsb,&fdBytes[headerSize+i*glyphStride+24],2);
		//get advance
		memcpy(&mFontData.gData[charCode].advance,&fdBytes[headerSize+i*glyphStride+26],2);
	}
	//load font atlas texture into memory
	printf("loading font atlas from %s\n",texPath);
	//int width,height;
	//importBitmap(texPath,&width,&height,NULL);
	//printf("Image size (%d,%d)\n",width,height);
	fontTex = tex2D(texPath);
	fontRes=fontTex.res_ptr;
}

//splt

void drawString(char* text, float startX, float startY, float squareSize, int numChars=-1){
	//move cursor to start
	float x = startX;
	float y = startY;
	int count = numChars==-1? strlen(text) : numChars;
	for(int i=0; i<count; i++){
		//get data for glyph
		glyphData glyph = mFontData.gData[text[i]];

		//get dimensions in UPM
		short w = (glyph.xMax-glyph.xMin);
		short h = (glyph.yMax-glyph.yMin);

		//get percentage of M square UPMP
		float charWidthUPMP = (w/(float)mFontData.upm);
		float charHeightUPMP = (h/(float)mFontData.upm);
		float lsbUPMP = (glyph.lsb/(float)mFontData.upm);
		float yOffUPMP = (glyph.yMin/(float)mFontData.upm);
		float advanceUPMP = (glyph.advance/(float)mFontData.upm);

		//convert dimensions to NDC
		float squareHeightNDC = squareSize;
		float squareWidthNDC = squareSize/aspect;
		float charWidthNDC = squareWidthNDC*charWidthUPMP;
		float charHeightNDC = squareHeightNDC*charHeightUPMP;
		float quadHeightNDC = charHeightNDC/(1.0-mFontData.padding);
		float quadWidthNDC = charWidthNDC/(1.0-mFontData.padding);
		float verPaddingNDC = (quadHeightNDC-charHeightNDC)*0.5f;
		float horPaddingNDC = (quadWidthNDC-charWidthNDC)*0.5f;
		float lsbNDC = lsbUPMP*squareWidthNDC;
		float yOffNDC = yOffUPMP*squareHeightNDC;
		float advanceNDC = advanceUPMP*squareWidthNDC;

		//add char vertices - positions and uvs
		addCharVertData(x-horPaddingNDC+lsbNDC,y-verPaddingNDC+yOffNDC,quadWidthNDC,quadHeightNDC,
				glyph.uLeft,glyph.vBot,glyph.uRight,glyph.vTop);

		//move cursor
		x+=advanceNDC;
	}
}

void drawString(int num, float startX, float startY, float squareSize, int numChars=-1){
	char text[15];
	itoa(num,text,10);
	//move cursor to start
	float x = startX;
	float y = startY;
	int count = numChars==-1? strlen(text) : numChars;
	for(int i=0; i<count; i++){
		//get data for glyph
		glyphData glyph = mFontData.gData[text[i]];

		//get dimensions in UPM
		short w = (glyph.xMax-glyph.xMin);
		short h = (glyph.yMax-glyph.yMin);

		//get percentage of M square UPMP
		float charWidthUPMP = (w/(float)mFontData.upm);
		float charHeightUPMP = (h/(float)mFontData.upm);
		float lsbUPMP = (glyph.lsb/(float)mFontData.upm);
		float yOffUPMP = (glyph.yMin/(float)mFontData.upm);
		float advanceUPMP = (glyph.advance/(float)mFontData.upm);

		//convert dimensions to NDC
		float squareHeightNDC = squareSize;
		float squareWidthNDC = squareSize/aspect;
		float charWidthNDC = squareWidthNDC*charWidthUPMP;
		float charHeightNDC = squareHeightNDC*charHeightUPMP;
		float quadHeightNDC = charHeightNDC/(1.0-mFontData.padding);
		float quadWidthNDC = charWidthNDC/(1.0-mFontData.padding);
		float verPaddingNDC = (quadHeightNDC-charHeightNDC)*0.5f;
		float horPaddingNDC = (quadWidthNDC-charWidthNDC)*0.5f;
		float lsbNDC = lsbUPMP*squareWidthNDC;
		float yOffNDC = yOffUPMP*squareHeightNDC;
		float advanceNDC = advanceUPMP*squareWidthNDC;

		//add char vertices - positions and uvs
		addCharVertData(x-horPaddingNDC+lsbNDC,y-verPaddingNDC+yOffNDC,quadWidthNDC,quadHeightNDC,
				glyph.uLeft,glyph.vBot,glyph.uRight,glyph.vTop);

		//move cursor
		x+=advanceNDC;
	}
}

//vars
char fps[50]; //size of the number
int counter = 0;
char helloWorld[50];
float fpsTimer=0;

// Start
void initFontEditor(){
	printf("initializing font editor yo\n");
	//importFont("fonts/source/Moonrising.ttf");
	//importFont("fonts/source/Gorehand.otf");
	//importFont("fonts/source/Envy Code R.ttf");
	//importFont("fonts/source/Grethania Script Reguler.ttf");
	//importFont("fonts/source/arial.ttf");
	//importFont("fonts/source/times.ttf");
	loadFont("fonts/fontAtlas");
	strcpy(fps,"....");
}

void updateFontEditor(){
	setClearColor(vec4(0.3,0.3,0.3,1));

	strcpy(helloWorld,"Hello World: ");
	itoa(counter,helloWorld+13,10);
	drawString(helloWorld,-0.95f,0,0.3f);
	drawString("The quick brown fox jumped over the lazy dog",-0.95f,-0.2,0.14f);
	drawString("The quick brown fox jumped over the lazy dog",-0.95f,-0.4,0.05f);
	drawString("1234567890 !@#$%^&*()-=_+[]{}\\|;:'\",.<>/?",-0.95f,-0.6,0.05f);


	if(keyInput[32]==2)
	{
		//do something on space
		setClearColor(vec4(1.0,0.1,0.1,1));
	}
	if(keyInput[82]==1){
		//readTestImage();
	}
	if(keyInput[77]==1){//M
		//addTriangle(-1.0,0.5,-0.5,1.0,0.0,0.5);
		//addChar(-0.9,0.7,-0.7,0.9);
	}
	if(keyInput[78]==1){//N
		//addTriangle(0.0,0.5,0.5,1.0,1.0,0.5);
		//addChar(0.7,0.7,0.9,0.9);
	}
	if(keyInput[8]==1){
		printf("backspace\n");
		counter++;
		//testRemoveLastChar();
	}
	fpsTimer+=deltaTime;
	if(fpsTimer>1.0f){
		float frameRate=1/deltaTime; 
		sprintf(fps,"%f",frameRate);
		fpsTimer=0;
	}
	drawString(fps,0.9f,0.9f,0.05f,4);
	drawString(gpuClockFrequency,0.8f,0.8f,0.05f);
}

#endif
