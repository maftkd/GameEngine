#ifndef VECTOR_H_
#define VECTOR_H_

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>

const UINT uzero = 0;

//fast inverse square root from Quake III Arena and Id Software
float Q_rsqrt(float number){
	long i;
	float x2, y;
	const float threehalfs = 1.5F;

	x2 = number * 0.5F;
	y  = number;
	i  = * ( long * ) &y;                       // evil floating point bit level hacking
	i  = 0x5f3759df - ( i >> 1 );               // what the fuck? 
	y  = * ( float * ) &i;
	y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
//	y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

	return y;
}

struct vec3{
	float points[3];
	float x;
	float y;
	float z;
	vec3(){
		points[0]=0.0;
		points[1]=0.0;
		points[2]=0.0;
		copyVals();
	}
	vec3(float a, float b, float c){
		points[0]=a;
		points[1]=b;
		points[2]=c;
		copyVals();
	}
	vec3(float a){
		points[0]=a;
		points[1]=a;
		points[2]=a;
		copyVals();
	}
	void copyVals(){
		x=points[0];
		y=points[1];
		z=points[2];
	}

	void zero(){
		points[0]=0;
		points[1]=0;
		points[2]=0;
		copyVals();
	}

	bool isZero(){
		return points[0]==0&&points[1]==0&&points[2]==0;
	}

	void set(float a, float b, float c){
		points[0]=a;
		points[1]=b;
		points[2]=c;
		copyVals();
	}

	void setx(float x1){
		x=x1;
		points[0]=x;
	}

	void sety(float y1){
		y=y1;
		points[1]=y;
	}

	void setz(float z1){
		z=z1;
		points[2]=z;
	}

	//scale
	vec3 operator*(float scalar){
		vec3 scaled(points[0]*scalar,
				points[1]*scalar,
				points[2]*scalar);
		return scaled;
	}
	vec3 operator/(float scalar){
		vec3 scaled(points[0]/scalar,
				points[1]/scalar,
				points[2]/scalar);
		return scaled;
	}

	void operator*=(float scalar){
		points[0]*=scalar;
		points[1]*=scalar;
		points[2]*=scalar;
		copyVals();
	}

	void operator+=(vec3 a){
		points[0]+=a.points[0];
		points[1]+=a.points[1];
		points[2]+=a.points[2];
		copyVals();
	}

	void operator-=(vec3 a){
		points[0]-=a.points[0];
		points[1]-=a.points[1];
		points[2]-=a.points[2];
		copyVals();
	}

	//dot
	float dot(vec3 a, vec3 b){
		return a.x*b.x+a.y*b.y+a.z*b.z;
	}

	void normalize(){
		float dt = x*x+y*y+z*z;
		float qrsqrt = Q_rsqrt(dt);
		points[0]*=qrsqrt;
		points[1]*=qrsqrt;
		points[2]*=qrsqrt;
		copyVals();
	}

	void operator=(vec3 a){
		points[0]=a.points[0];
		points[1]=a.points[1];
		points[2]=a.points[2];
		copyVals();
	}

	void log(){
		printf("%f,%f,%f\n",x,y,z);
	}

};

//vector operator overloads
vec3 operator-(vec3 a, vec3 b){
	vec3 diff(a.x-b.x,a.y-b.y,a.z-b.z);
	return diff;
}

vec3 operator+(vec3 a, vec3 b){
	vec3 sum(a.x+b.x,a.y+b.y,a.z+b.z);
	return sum;
}

vec3 operator*(float scalar,vec3 v){
	vec3 scaled(v.x*scalar,
			v.y*scalar,
			v.z*scalar);
	return scaled;
}

float dot(vec3 a, vec3 b){
	return a.x*b.x+a.y*b.y+a.z*b.z;
}

vec3 cross(vec3 a, vec3 b){
	vec3 cr(a.y*b.z-a.z*b.y,
			a.z*b.x-a.x*b.z,
			a.x*b.y-a.y*b.x);
	return cr;
}

vec3 normal(vec3 v){
	vec3 norm = v*Q_rsqrt(dot(v,v));
	return norm;
}

struct vec4{
	float points[4];
	float x;
	float y;
	float z;
	float w;
	vec4(){
		points[0]=0.0;
		points[1]=0.0;
		points[2]=0.0;
		points[3]=0.0;
		copyVals();
	}
	vec4(float a, float b,float c, float d){
		points[0]=a;
		points[1]=b;
		points[2]=c;
		points[3]=d;
		copyVals();
	}
	void operator=(vec4 a){
		points[0]=a.points[0];
		points[1]=a.points[1];
		points[2]=a.points[2];
		points[3]=a.points[3];
		copyVals();
	}
	vec4(float a){
		points[0]=a;
		points[1]=a;
		points[2]=a;
		points[3]=a;
		copyVals();
	}
	void copyVals(){
		x=points[0];
		y=points[1];
		z=points[2];
		w=points[3];
	}

	void zero(){
		points[0]=0;
		points[1]=0;
		points[2]=0;
		points[3]=0;
		copyVals();
	}

	bool isZero(){
		return points[0]==0&&points[1]==0&&points[2]==0&&points[3]==0;
	}

	void set(float a, float b,float c, float d){
		points[0]=a;
		points[1]=b;
		points[2]=c;
		points[3]=d;
		copyVals();
	}
	void setx(float x1){
		x=x1;
		points[0]=x;
	}

	void sety(float y1){
		y=y1;
		points[1]=y;
	}

	void setz(float z1){
		z=z1;
		points[2]=z;
	}

	void setw(float w1){
		w=w1;
		points[3]=w;
	}
	void log(){
		printf("%f,%f,%f,%f\n",x,y,z,w);
	}
	void operator=(vec3 a){
		points[0]=a.points[0];
		points[1]=a.points[1];
		points[2]=a.points[2];
		points[3]=1;
		copyVals();
	}

	void operator/=(float scalar){
		points[0]/=scalar;
		points[1]/=scalar;
		points[2]/=scalar;
		points[3]/=scalar;
		copyVals();
	}

	vec4 operator*(float scalar){
		vec4 scaled(points[0]*scalar,
				points[1]*scalar,
				points[2]*scalar,
				points[3]*scalar);
		return scaled;
	}

	//scale
	/*
	void operator*=(float scalar){
		points[0]*=scalar;
		points[1]*=scalar;
		copyVals();
	}

	void operator+=(vec4 a){
		points[0]+=a.points[0];
		points[1]+=a.points[1];
		copyVals();
	}

	void operator-=(vec4 a){
		points[0]-=a.points[0];
		points[1]-=a.points[1];
		copyVals();
	}

	//dot
	float dot(vec4 a, vec4 b){
		return a.x*b.x+a.y*b.y;
	}

	void normalize(){
		float dt = x*x+y*y;
		float qrsqrt = Q_rsqrt(dt);
		points[0]*=qrsqrt;
		points[1]*=qrsqrt;
		copyVals();
	}

	void operator=(vec4 a){
		points[0]=a.points[0];
		points[1]=a.points[1];
		copyVals();
	}

	void log(){
		printf("%f,%f\n",x,y);
	}
	*/
};

vec4 operator+(vec4 a, vec4 b){
	vec4 sum(a.x+b.x,a.y+b.y,a.z+b.z,a.w+b.w);
	return sum;
}

vec4 invertColor(vec4 c,bool invertAlpha=false){
	vec4 inverted;
	inverted.setx(1-c.x);
	inverted.sety(1-c.y);
	inverted.setz(1-c.z);
	inverted.setw(invertAlpha?1-c.w : c.w);
	return inverted;
}

struct mat4{
	float points[16];
	mat4(){
		points[0]=1.0;
		points[1]=0.0;
		points[2]=0.0;
		points[3]=0.0;
		points[4]=0.0;
		points[5]=1.0;
		points[6]=0.0;
		points[7]=0.0;
		points[8]=0.0;
		points[9]=0.0;
		points[10]=1.0;
		points[11]=0.0;
		points[12]=0.0;
		points[13]=0.0;
		points[14]=0.0;
		points[15]=1.0;
	}

	mat4 translate(vec3 vec){
		mat4 translated;
		translated.points[0]=points[0]+points[3]*vec.x;
		translated.points[1]=points[1]+points[3]*vec.y;
		translated.points[2]=points[2]+points[3]*vec.z;
		translated.points[3]=points[3];
		translated.points[4]=points[4]+points[7]*vec.x;
		translated.points[5]=points[5]+points[7]*vec.y;
		translated.points[6]=points[6]+points[7]*vec.z;
		translated.points[7]=points[7];
		translated.points[8]=points[8]+points[11]*vec.x;
		translated.points[9]=points[9]+points[11]*vec.y;
		translated.points[10]=points[10]+points[11]*vec.z;
		translated.points[11]=points[11];
		translated.points[12]=points[12]+points[15]*vec.x;
		translated.points[13]=points[13]+points[15]*vec.y;
		translated.points[14]=points[14]+points[15]*vec.z;
		translated.points[15]=points[15];
		return translated;
	}

	mat4 rotate(float rads, vec3 axis){
		float qr=cos(rads*0.5);
		float sineTerm = -sin(rads*0.5);
		float qi=sineTerm*axis.x;
		float qj=sineTerm*axis.y;
		float qk=sineTerm*axis.z;
		float a = 1-2*(qj*qj+qk*qk);
		float b = 2*(qi*qj+qk*qr);
		float c = 2*(qi*qk-qj*qr);
		float d = 2*(qi*qj-qk*qr);
		float e = 1-2*(qi*qi+qk*qk);
		float f = 2*(qj*qk+qi*qr);
		float g = 2*(qi*qk+qj*qr);
		float h = 2*(qj*qk-qi*qr);
		float i = 1-2*(qi*qi+qj*qj);
		mat4 rotated;
		rotated.points[0]=points[0]*a+points[1]*d+points[2]*g;
		rotated.points[1]=points[0]*b+points[1]*e+points[2]*h;
		rotated.points[2]=points[0]*c+points[1]*f+points[2]*i;
		rotated.points[3]=points[3];
		rotated.points[4]=points[4]*a+points[5]*d+points[6]*g;
		rotated.points[5]=points[4]*b+points[5]*e+points[6]*h;
		rotated.points[6]=points[4]*c+points[5]*f+points[6]*i;
		rotated.points[7]=points[7];
		rotated.points[8]=points[8]*a+points[9]*d+points[10]*g;
		rotated.points[9]=points[8]*b+points[9]*e+points[10]*h;
		rotated.points[10]=points[8]*c+points[9]*f+points[10]*i;
		rotated.points[11]=points[11];
		rotated.points[12]=points[12]*a+points[13]*d+points[14]*g;
		rotated.points[13]=points[12]*b+points[13]*e+points[14]*h;
		rotated.points[14]=points[12]*c+points[13]*f+points[14]*i;
		rotated.points[15]=points[15];
		return rotated;
	}

	vec3 rotateVec(float rads, vec3 axis){
		float qr=cos(rads*0.5);
		float sineTerm = -sin(rads*0.5);
		float qi=sineTerm*axis.x;
		float qj=sineTerm*axis.y;
		float qk=sineTerm*axis.z;
		float a = 1-2*(qj*qj+qk*qk);
		float b = 2*(qi*qj+qk*qr);
		float c = 2*(qi*qk-qj*qr);
		float d = 2*(qi*qj-qk*qr);
		float e = 1-2*(qi*qi+qk*qk);
		float f = 2*(qj*qk+qi*qr);
		float g = 2*(qi*qk+qj*qr);
		float h = 2*(qj*qk-qi*qr);
		float i = 1-2*(qi*qi+qj*qj);
		float x=points[12]*a+points[13]*d+points[14]*g;
		float y=points[12]*b+points[13]*e+points[14]*h;
		float z=points[12]*c+points[13]*f+points[14]*i;
		vec3 ret = vec3(x,y,z);
		return ret;
	}

	mat4 scale(vec3 axis){
		mat4 scaled;
		scaled.points[0]=points[0]*axis.x;
		scaled.points[1]=points[1]*axis.y;
		scaled.points[2]=points[2]*axis.z;
		scaled.points[3]=points[3];
		scaled.points[4]=points[4]*axis.x;
		scaled.points[5]=points[5]*axis.y;
		scaled.points[6]=points[6]*axis.z;
		scaled.points[7]=points[7];
		scaled.points[8]=points[8]*axis.x;
		scaled.points[9]=points[9]*axis.y;
		scaled.points[10]=points[10]*axis.z;
		scaled.points[11]=points[11];
		scaled.points[12]=points[12]*axis.x;
		scaled.points[13]=points[13]*axis.y;
		scaled.points[14]=points[14]*axis.z;
		scaled.points[15]=points[15];
		return scaled;
	}


	mat4 operator*(mat4 other){
		mat4 product;
		product.points[0]=points[0]*other.points[0]+points[4]*other.points[1]+
						points[8]*other.points[2]+points[12]*other.points[3];
		product.points[1]=points[1]*other.points[0]+points[5]*other.points[1]+
						points[9]*other.points[2]+points[13]*other.points[3];
		product.points[2]=points[2]*other.points[0]+points[6]*other.points[1]+
						points[10]*other.points[2]+points[14]*other.points[3];
		product.points[3]=points[3]*other.points[0]+points[7]*other.points[1]+
						points[11]*other.points[2]+points[15]*other.points[3];
		product.points[4]=points[0]*other.points[4]+points[4]*other.points[5]+
						points[8]*other.points[6]+points[12]*other.points[7];
		product.points[5]=points[1]*other.points[4]+points[5]*other.points[5]+
						points[9]*other.points[6]+points[13]*other.points[7];
		product.points[6]=points[2]*other.points[4]+points[6]*other.points[5]+
						points[10]*other.points[6]+points[14]*other.points[7];
		product.points[7]=points[3]*other.points[4]+points[7]*other.points[5]+
						points[11]*other.points[6]+points[15]*other.points[7];
		product.points[8]=points[0]*other.points[8]+points[4]*other.points[9]+
						points[8]*other.points[10]+points[12]*other.points[11];
		product.points[9]=points[1]*other.points[8]+points[5]*other.points[9]+
						points[9]*other.points[10]+points[13]*other.points[11];
		product.points[10]=points[2]*other.points[8]+points[6]*other.points[9]+
						points[10]*other.points[10]+points[14]*other.points[11];
		product.points[11]=points[3]*other.points[8]+points[7]*other.points[9]+
						points[11]*other.points[10]+points[15]*other.points[11];
		product.points[12]=points[0]*other.points[12]+points[4]*other.points[13]+
						points[8]*other.points[14]+points[12]*other.points[15];
		product.points[13]=points[1]*other.points[12]+points[5]*other.points[13]+
						points[9]*other.points[14]+points[13]*other.points[15];
		product.points[14]=points[2]*other.points[12]+points[6]*other.points[13]+
						points[10]*other.points[14]+points[14]*other.points[15];
		product.points[15]=points[3]*other.points[12]+points[7]*other.points[13]+
						points[11]*other.points[14]+points[15]*other.points[15];
		return product;
	}

	vec3 operator*(vec3 other){
		vec3 prod(points[0]*other.x+points[4]*other.y+points[8]*other.z+points[12],
				points[1]*other.x+points[5]*other.y+points[9]*other.z+points[13],
				points[2]*other.x+points[6]*other.y+points[10]*other.z+points[14]);
		return prod;
	}

	vec4 operator*(vec4 other){
		vec4 prod(points[0]*other.x+points[4]*other.y+points[8]*other.z+points[12]*other.w,
				points[1]*other.x+points[5]*other.y+points[9]*other.z+points[13]*other.w,
				points[2]*other.x+points[6]*other.y+points[10]*other.z+points[14]*other.w,
				points[3]*other.x+points[7]*other.y+points[11]*other.z+points[15]*other.w);
		return prod;
	}

	void log(){
		for(int i=0; i<16; i++){
			printf("%f...",points[i]);
		}
		printf("\n");
	}
};

mat4 identity;

vec4 zero4 = vec4(0,0,0,0);

//more matrix functions
mat4 lookAt(vec3 eye, vec3 at, vec3 up){
	//RH
	/*
	//vec3 z = normal(eye-at);
	//vec3 x = normal(cross(up,z));
	//vec3 y = cross(z,x);
	//float a = dot(x,eye);
	//float b = dot(y,eye);
	//float c = dot(z,eye);
	*/
	//LH
	vec3 z = normal(at-eye);
	vec3 x = normal(cross(up,z));
	vec3 y = cross(z,x);
	float a = -dot(x,eye);
	float b = -dot(y,eye);
	float c = -dot(z,eye);

	mat4 view;
	view.points[0]=x.x;
	view.points[1]=y.x;
	view.points[2]=z.x;
	view.points[3]=0;
	view.points[4]=x.y;
	view.points[5]=y.y;
	view.points[6]=z.y;
	view.points[7]=0;
	view.points[8]=x.z;
	view.points[9]=y.z;
	view.points[10]=z.z;
	view.points[11]=0;
	view.points[12]=a;
	view.points[13]=b;
	view.points[14]=c;
	view.points[15]=1;
	return view;
}	

mat4 perspective(float fovy, float aspect, float nr, float fr){
	float t = 1/tan(fovy*0.5f);
	//LH - directx
	float a = t/aspect;
	float b = t;
	float c = fr/(fr-nr);
	float d = 1;
	float e = -nr*fr/(fr-nr);

	mat4 proj;
	proj.points[0]=a;
	proj.points[1]=0;
	proj.points[2]=0;
	proj.points[3]=0;
	proj.points[4]=0;
	proj.points[5]=b;
	proj.points[6]=0;
	proj.points[7]=0;
	proj.points[8]=0;
	proj.points[9]=0;
	proj.points[10]=c;
	proj.points[11]=d;
	proj.points[12]=0;
	proj.points[13]=0;
	proj.points[14]=e;
	proj.points[15]=0;
	return proj;
}

mat4 orthographic(float left, float right, float bottom, float top, float nr, float fr){
	mat4 proj;
	//directx
	proj.points[0]=2/(right-left);
	proj.points[1]=0;
	proj.points[2]=0;
	proj.points[3]=(right+left)/(right-left);
	proj.points[4]=0;
	proj.points[5]=2/(top-bottom);
	proj.points[6]=0;
	proj.points[7]=(top+bottom)/(top-bottom);
	proj.points[8]=0;
	proj.points[9]=0;
	proj.points[10]=1/(fr-nr);
	proj.points[11]=nr/(nr-fr);
	proj.points[12]=0;
	proj.points[13]=0;
	proj.points[14]=0;
	proj.points[15]=1;
	return proj;
}

struct vec2{
	float points[2];
	float x;
	float y;
	vec2(){
		points[0]=0.0;
		points[1]=0.0;
		copyVals();
	}
	vec2(float a, float b){
		points[0]=a;
		points[1]=b;
		copyVals();
	}
	vec2(float a){
		points[0]=a;
		points[1]=a;
		copyVals();
	}
	void copyVals(){
		x=points[0];
		y=points[1];
	}

	void zero(){
		points[0]=0;
		points[1]=0;
		copyVals();
	}

	bool isZero(){
		return points[0]==0&&points[1]==0;
	}

	void set(float a, float b){
		points[0]=a;
		points[1]=b;
		copyVals();
	}

	//scale
	vec2 operator*(float scalar){
		vec2 scaled(points[0]*scalar,
				points[1]*scalar);
		return scaled;
	}

	void operator*=(float scalar){
		points[0]*=scalar;
		points[1]*=scalar;
		copyVals();
	}

	void operator+=(vec2 a){
		points[0]+=a.points[0];
		points[1]+=a.points[1];
		copyVals();
	}

	void operator-=(vec2 a){
		points[0]-=a.points[0];
		points[1]-=a.points[1];
		copyVals();
	}

	//dot
	float dot(vec2 a, vec2 b){
		return a.x*b.x+a.y*b.y;
	}

	void normalize(){
		float dt = x*x+y*y;
		float qrsqrt = Q_rsqrt(dt);
		points[0]*=qrsqrt;
		points[1]*=qrsqrt;
		copyVals();
	}

	void operator=(vec2 a){
		points[0]=a.points[0];
		points[1]=a.points[1];
		copyVals();
	}

	bool operator==(vec2 a){
		return x==a.x && y==a.y;
	}

	void log(){
		printf("%f,%f\n",x,y);
	}

	float magnitude(){
		float dt = x*x+y*y;
		return sqrt(dt);
		//return Q_rsqrt(dt);
	}
	void setx(float x1){
		x=x1;
		points[0]=x;
	}

	void sety(float y1){
		y=y1;
		points[1]=y;
	}

	void round(){
		x=(int)x;
		y=(int)y;
		points[0]=x;
		points[1]=y;
	}
};


//vector2 operator overloads
vec2 operator-(vec2 a, vec2 b){
	vec2 diff(a.x-b.x,a.y-b.y);
	return diff;
}

float distance(vec2 a, vec2 b){
	vec2 diff = a-b; 
	float distSqr=(diff.x*diff.x+diff.y*diff.y);
	float dist = 1.0f/Q_rsqrt((float)distSqr);
	return dist;
}

vec2 operator+(vec2 a, vec2 b){
	vec2 sum(a.x+b.x,a.y+b.y);
	return sum;
}

vec2 operator*(float scalar,vec2 v){
	vec2 scaled(v.x*scalar,
			v.y*scalar);
	return scaled;
}

float dot(vec2 a, vec2 b){
	return a.x*b.x+a.y*b.y;
}

vec2 lerp(vec2 a, vec2 b, float t){
	vec2 lerped(a.x+(b.x-a.x)*t,
			a.y+(b.y-a.y)*t);
	return lerped;
}

//cross not yet defined

vec2 normal(vec2 v){
	vec2 norm = v*Q_rsqrt(dot(v,v));
	return norm;
}


//hybrid overloads between matrix and vector

#endif
