#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "SDL/SDL.h"
#include "SDL/SDL_mixer.h"
#include "realfftf.h"
#include "realfftf.c"

#define WIDTH 720
#define HEIGHT 406

typedef unsigned char byte;

typedef struct {
   float    x;
   float    y;
} vec2f;

typedef struct {
   int    x;
   int    y;
} vec2i;

typedef struct {
   int    x;
   int    y;
   int    z;
} vec3i;

typedef struct {
   float    x;
   float    y;
   float    z;
} vec3;

typedef struct {
   vec3    x;
   vec3    y;
   vec3    z;
} matrix3d;

#include "fetus.h"

///////////////////////////////////////////////////////////////////

#define NUM_LEVELS 256
#define STACKSIZE 10000

struct plotter {
    float angle;
    float angle_increment;
    float dist;
    float x;
    float y;
    float z;
    float z_angle;
    int color;
    int toggle;
};

struct plotter p = {0,3.5,0.1,0,0.0,0,  0,0xFFFFFF, 1};

float temp_x=0, temp_y=0, temp_z=0;

#define LSYSTEM_COUNT 8

char lsystem_axiom[LSYSTEM_COUNT][65000];
char lsystem_rules[LSYSTEM_COUNT][8192];
int max_iterations[LSYSTEM_COUNT] = {4,4,10,5,0,0,0,0};

typedef struct {
    int top;
    struct plotter items[STACKSIZE];
} stack;

stack st = {0};
stack st2 = {0};

int pushes = 0;
int pops = 0;


void push(struct plotter x, stack *ps)
{
    pushes++;
    if(ps->top == STACKSIZE-1){
        printf("Error: stack overflow\n");
        exit(-1);
    } else
        ps->items[++(ps->top)] = x;

    return;
}

struct plotter pop(stack *ps)
{
    pops++;
    if(ps->top < 0){
        printf("Error: stack underflow\n");
        exit(-1);
    } else
        return(ps->items[(ps->top)--]);
}

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define abs(a) (((a)<0) ? -(a) : (a))
#define sign(a) (((a)<0) ? -1 : (a)>0 ? 1 : 0)

void drawLine32(SDL_Surface *screen, int x1, int y1, int x2, int y2, SDL_Color c);

void lineTo(float dx, float dy, float dz, float x0, float y0, SDL_Surface *screen, SDL_Color c) {

    drawLine32(screen, WIDTH/2-(int)(p.x*WIDTH), HEIGHT/2-(int)(p.y*HEIGHT), WIDTH/2-(int)(dx*WIDTH), HEIGHT/2-(int)(dy*HEIGHT), c);

    p.x = dx;
    p.y = dy;
    p.z = dz;
}

void moveRel(int dx, int dy) {
    int temp_x, temp_y;
    temp_x = p.x;
    temp_y = p.y;
    p.x = temp_x+dx;
    p.y = temp_y+dy;
}

void turnTo(float angle) {
    p.angle = -angle;
}

void turn(float angle) {
    p.angle -= angle;    //turn anti-clockwise for positive angle
}

void z_turn(float angle) {
    p.z_angle -= angle;    //turn anti-clockwise for positive angle
}

void forward(float dist, int visible, float x0, float y0, SDL_Surface *screen, SDL_Color c) {
    const float RadPerDeg = 0.017453393;
    float temp_x = (p.x + (dist * cos(RadPerDeg * p.angle)));
    float temp_y = (p.y + (dist * sin(RadPerDeg * p.angle)));
    float temp_z = (p.z + (dist * sin(RadPerDeg * p.z_angle)));

    if (visible == 1) lineTo(temp_x,temp_y,temp_z, x0, y0, screen, c);

    p.x = temp_x;
    p.y = temp_y;
    p.z = temp_z;


}

char *processed_rules[512][512];
char *split_rules[512];

int iteration = 0;

void append(char* s, char c)
{
        int len = strlen(s);
        s[len] = c;
        s[len+1] = '\0';
}

void lsystem_iteration() {

    int ff;

    int iter = 0;
    for (ff=0;ff<LSYSTEM_COUNT;ff++) {
        iteration = 0;

    while (iteration < max_iterations[ff]) {

    printf("%d", iter++);

    int i = 0;

    // parse rules for comma delimited rules

    char *rules_copy = (char *)malloc(strlen(lsystem_rules[ff]) + 1);
    strcpy(rules_copy, lsystem_rules[ff]);

    char *pch;
    pch = strtok(rules_copy,",");
    while (pch != NULL) {
        split_rules[i] = pch;
        pch = strtok(NULL, ",");
        i++;
      }

    // parse split rules by =

    int number_of_rules = i;

    int j;

    for(j=0;j<i;j++) {

        char *pch2;
        pch2 = strtok(split_rules[j], "=");
        processed_rules[j][0] = pch2;
        while (pch2 != NULL) {
            processed_rules[j][1] = pch2;
            pch2 = strtok(NULL, "=");
        }

    }

    // iterate through the axiom, producing the next generation


    int k = 0;

    char *new_text = "";
    char *lsystem_axiom_temp = "";

    int rule_found = 0;

    new_text = (char*)malloc(65000);
    lsystem_axiom_temp = (char*)malloc(65000);
    strcpy(lsystem_axiom_temp, "");

    for (i=0;i<strlen(lsystem_axiom[ff]);i++) {

        int rule_matched = 0;
        for (k=0;k<number_of_rules;k++) {
            if(processed_rules[k][0][0] == lsystem_axiom[ff][i]) { strcat(lsystem_axiom_temp,processed_rules[k][1]); rule_matched = 1; }
        }

        if (rule_matched == 0) append(lsystem_axiom_temp, lsystem_axiom[ff][i]);
    }

    strcpy(lsystem_axiom[ff], lsystem_axiom_temp);

    free(new_text);
    free(rules_copy);
    free(lsystem_axiom_temp);

    iteration++;

    }

    }
}

// < z vasemmalle
// > z oikealle



////////////////////////////////////////////////////////////////////////


byte HMap[256*256];       /* Height field */
byte CMap[256*256];       /* Color map */
byte Video[WIDTH*HEIGHT]; /* Off-screen buffer */
byte Depth[WIDTH*HEIGHT]; /* Off-screen buffer */

float millis = 0;

Sint16* music_mix_stream;
int music_mix_len;

fft_type data1[256],data2[256];
fft_type re,im,value;

void music_mix_callback(void *udata, Uint8 *stream, int len)
{
  music_mix_stream=(Sint16*)stream;
  music_mix_len=len;
}

void setPixel8(SDL_Surface *screen, int x, int y, Uint8 c)
{
  *((Uint8*)(screen->pixels+y*screen->pitch+x))=c;
}

Uint8 getPixel8(SDL_Surface *screen, int x, int y)
{
  return *((Uint8*)(screen->pixels+y*screen->pitch+x));
}


void setPixel32(SDL_Surface *screen, int x, int y, SDL_Color c)
{
  *((Uint8*)(screen->pixels+y*screen->pitch+x*4+2))=c.r;
  *((Uint8*)(screen->pixels+y*screen->pitch+x*4+1))=c.g;
  *((Uint8*)(screen->pixels+y*screen->pitch+x*4))=c.b;
}

SDL_Color getPixel32(SDL_Surface *screen, int x, int y)
{
  SDL_Color c;
  c.r = *((Uint8*)(screen->pixels+y*screen->pitch+x*4+2));
  c.g = *((Uint8*)(screen->pixels+y*screen->pitch+x*4+1));
  c.b = *((Uint8*)(screen->pixels+y*screen->pitch+x*4));
  return c;
}


/* line algorithm GPL by Julien Carme <julien.carme@acm.org> */

#define SWAP(x,y)  \
        x = x + y; \
        y = x - y; \
        x = x - y;

void drawLine8(SDL_Surface *screen, int x1, int y1, int x2, int y2, Uint8 c)
{
         int dx, dy, cxy,dxy;
        /* calculate the distances */
        dx = abs(x1 - x2);
        dy = abs(y1 - y2);

        cxy = 0;
        if (dy > dx) {
                /* Follow Y axis */
                if (y1 > y2) {
                        SWAP(y1, y2);
                        SWAP(x1, x2);
                }

                if (x1 > x2)
                        dxy = -1;
                else
                        dxy = 1;

                for (y1=y1; y1<y2; y1++) {
                        cxy += dx;
                        if (cxy >= dy) {
                                x1+= dxy;
                                cxy -= dy;
                        }
                      setPixel8(screen, x1, y1, c);
                }
        } else {
                /* Follow X axis */
                if (x1 > x2) {
                        SWAP(x1, x2);
                        SWAP(y1, y2);
                }

                if (y1 > y2)
                        dxy = -1;
                else
                        dxy = 1;

                for (x1=x1; x1<x2; x1++) {
                        cxy += dy;
                        if (cxy >= dx) {
                                y1+=dxy;
                                cxy -= dx;
                        }
                      setPixel8(screen, x1, y1, c);
                }
        }

}

void drawLine32(SDL_Surface *screen, int x1, int y1, int x2, int y2, SDL_Color c)
{
        if (x1 < 0 || y1 < 0 || x2 < 0 || y2 < 0) return;
        if (x1 > WIDTH-1 || y1 > HEIGHT-1 || x2 > WIDTH-1 || y2 > HEIGHT-1) return;

         int dx, dy, cxy,dxy;
        /* calculate the distances */
        dx = abs(x1 - x2);
        dy = abs(y1 - y2);

        cxy = 0;
        if (dy > dx) {
                /* Follow Y axis */
                if (y1 > y2) {
                        SWAP(y1, y2);
                        SWAP(x1, x2);
                }

                if (x1 > x2)
                        dxy = -1;
                else
                        dxy = 1;

                for (y1=y1; y1<y2; y1++) {
                        cxy += dx;
                        if (cxy >= dy) {
                                x1+= dxy;
                                cxy -= dy;
                        }
                      setPixel32(screen, x1, y1, c);
                }
        } else {
                /* Follow X axis */
                if (x1 > x2) {
                        SWAP(x1, x2);
                        SWAP(y1, y2);
                }

                if (y1 > y2)
                        dxy = -1;
                else
                        dxy = 1;

                for (x1=x1; x1<x2; x1++) {
                        cxy += dy;
                        if (cxy >= dx) {
                                y1+=dxy;
                                cxy -= dx;
                        }
                      setPixel32(screen, x1, y1, c);
                }
        }

}

void drawLineAdd32(SDL_Surface *screen, int x1, int y1, int x2, int y2, SDL_Color c)
{
        int dx, dy, cxy,dxy;
        SDL_Color gc;

        /* calculate the distances */
        dx = abs(x1 - x2);
        dy = abs(y1 - y2);

        cxy = 0;
        if (dy > dx) {
                /* Follow Y axis */
                if (y1 > y2) {
                        SWAP(y1, y2);
                        SWAP(x1, x2);
                }

                if (x1 > x2)
                        dxy = -1;
                else
                        dxy = 1;

                for (y1=y1; y1<y2; y1++) {
                        cxy += dx;
                        if (cxy >= dy) {
                                x1+= dxy;
                                cxy -= dy;
                        }
                      gc=getPixel32(screen, x1, y1);
                      gc.r+=c.r; gc.g+=c.g; gc.b+=c.b;
                      setPixel32(screen, x1, y1, gc);
                }
        } else {
                /* Follow X axis */
                if (x1 > x2) {
                        SWAP(x1, x2);
                        SWAP(y1, y2);
                }

                if (y1 > y2)
                        dxy = -1;
                else
                        dxy = 1;

                for (x1=x1; x1<x2; x1++) {
                        cxy += dy;
                        if (cxy >= dx) {
                                y1+=dxy;
                                cxy -= dx;
                        }
                      gc=getPixel32(screen, x1, y1);
                      gc.r+=c.r; gc.g+=c.g; gc.b+=c.b;
                      setPixel32(screen, x1, y1, gc);
                }
        }

}

/* Reduces a value to 0..255 (used in height field computation) */
int Clamp(int x)
{
  return (x<0 ? 0 : (x>255 ? 255 : x));
}

int lasty[WIDTH],         /* Last pixel drawn on a given column */
    lastc[WIDTH];         /* Color of last pixel on a column */

/*
   Draw a "section" of the landscape; x0,y0 and x1,y1 and the xy coordinates
   on the height field, hy is the viewpoint height, s is the scaling factor
   for the distance. x0,y0,x1,y1 are 16.16 fixed point numbers and the
   scaling factor is a 16.8 fixed point value.
 */
void Line(int x0,int y0,int x1,int y1,int hy,int s)
{
  int i,sx,sy;

  /* Compute xy speed */
  sx=(x1-x0)/WIDTH; sy=(y1-y0)/WIDTH;
  for ( i=0; i<WIDTH; i++ )
  {
    int c,y,h,u0,v0,u1,v1,a,b,h0,h1,h2,h3;

    /* Compute the xy coordinates; a and b will be the position inside the
       single map cell (0..255).
     */
    u0=(x0>>16)&0xFF;    a=(x0>>8)&255;
    v0=((y0>>8)&0xFF00); b=(y0>>8)&255;
    u1=(u0+1)&0xFF;
    v1=(v0+256)&0xFF00;

    /* Fetch the height at the four corners of the square the point is in */
    h0=HMap[u0+v0]; h2=HMap[u0+v1];
    h1=HMap[u1+v0]; h3=HMap[u1+v1];

    /* Compute the height using bilinear interpolation */
    h0=(h0<<8)+a*(h1-h0);
    h2=(h2<<8)+a*(h3-h2);
    h=((h0<<8)+b*(h2-h0))>>16;

    /* Fetch the color at the four corners of the square the point is in */
    h0=CMap[u0+v0]; h2=CMap[u0+v1];
    h1=CMap[u1+v0]; h3=CMap[u1+v1];

    /* Compute the color using bilinear interpolation (in 16.16) */
    h0=(h0<<8)+a*(h1-h0);
    h2=(h2<<8)+a*(h3-h2);
    c=((h0<<8)+b*(h2-h0));

    /* Compute screen height using the scaling factor */
    y=(((h-hy)*s)>>11)+HEIGHT/8;

    /* Draw the column */
    if ( y<(a=lasty[i]) )
    {
      unsigned char *b=Video+a*WIDTH+i;
      int sc,cc;


      if ( lastc[i]==-1 )
      	lastc[i]=c;

      sc=(c-lastc[i])/(a-y);
      cc=lastc[i];

      if ( a>HEIGHT-1 ) { b-=(a-(HEIGHT-1))*WIDTH; cc+=(a-(HEIGHT-1))*sc; a=HEIGHT-1; }
      if ( y<0 ) y=0;
      int dd = 0;
      dd = lasty[i]%2;
      while ( y<a )
      {
	       *b+=cc>>18; 
         *b-=cc>>14&(dd);
         cc+=sc;
         b-=WIDTH; a--;
      }
      lasty[i]=y;
    }
    lastc[i]=c;

    /* Advance to next xy position */
    x0+=sx; y0+=sy;
  }
}




matrix3d buildMatrix(vec3 rotate) {
	float t1 = rotate.y-rotate.z;
	float t2 = rotate.y+rotate.z;
	float t3 = rotate.x+rotate.z;
	float t4 = rotate.x-rotate.z;
	float t5 = rotate.x+t2;
	float t6 = rotate.x-t1;
	float t7 = rotate.x+t1;
	float t8 = t2-rotate.x;
	float t9 = rotate.y-rotate.x;
	float t10= rotate.y+rotate.x;

	float a = (cos(t1)+cos(t2))/2;
	float b = (sin(t1)-sin(t2))/2;
	float c = sin(rotate.y);
	float d = (sin(t3)-sin(t4))/2 + (cos(t6)-cos(t5)+cos(t8)-cos(t7))/4;
	float e = (cos(t3)+cos(t4))/2 + (sin(t5)-sin(t6)-sin(t7)-sin(t8))/4;
	float f = (sin(t9)-sin(t10))/2;
	float g = (cos(t4)-cos(t3))/2 + (sin(t6)-sin(t5)-sin(t8)-sin(t7))/4;
	float h = (sin(t3)+sin(t4))/2 + (cos(t6)-cos(t5)+cos(t7)-cos(t8))/4;
	float i = (cos(t9)+cos(t10))/2;

	matrix3d rotateMatrix;
	rotateMatrix.x = (vec3){a,b,c};
	rotateMatrix.y = (vec3){d,e,f};
	rotateMatrix.z = (vec3){g,h,i};
	return rotateMatrix;
}

void swapVec3i(vec3i *s1,vec3i *s2){
	vec3i temp = *s2;
	*s2 = *s1;
	*s1 = temp;
}

void fswap(float *s1,float *s2){
	float temp = *s2;
	*s2 = *s1;
	*s1 = temp;
}

void hLine(float x1, float x2, float y, int color, int depth) {
	if (x1 > x2) fswap(&x1,&x2);
	//unsigned char *start=Video+(int)y*WIDTH+(int)x1;
	int start=(int)y*WIDTH+(int)x1;
	int length = (int)(x2-x1)+2;
	//memset(start,color,length);
	for (int i = start;i<start+length;i+=1)
	{ 
		if (Depth[i] < depth) { 
			Depth[i] = depth; 
			Video[i] = .5*depth+color; 
		}
	}
}



void drawTriangle(vec3i A, vec3i B, vec3i C,int shade){
	
	if (A.y < B.y) {
		if (C.y < A.y) swapVec3i(&A,&C);
	} else {
	if (B.y < C.y) swapVec3i(&A,&B);
	else swapVec3i(&A,&C);
	} 
	if(C.y<B.y) swapVec3i(&B,&C);
	

	float dx1,dx2,dx3;
	if (B.y-A.y > 0) dx1=(float)(B.x-A.x)/(float)(B.y-A.y); else dx1=0;
	if (C.y-A.y > 0) dx2=(float)(C.x-A.x)/(float)(C.y-A.y); else dx2=0;
	if (C.y-B.y > 0) dx3=(float)(C.x-B.x)/(float)(C.y-B.y); else dx3=0;	
	
	vec2f S,E;
	S.x = (float)A.x;
	S.y = (float)A.y;
	E.x = (float)A.x;
	E.y = (float)A.y;
	int depth=128-(A.z+B.z+C.z)/3;
	int color=shade;
	
	
	if(dx1 > dx2) {
		for(;S.y<=B.y;S.y++,E.y++,S.x+=dx2,E.x+=dx1)
		{
			hLine(S.x,E.x,S.y,color,depth);
		}

		E.x=(float)B.x;
		E.y=(float)B.y;
		
		for(;S.y<=C.y;S.y++,E.y++,S.x+=dx2,E.x+=dx3)
		{
			hLine(S.x,E.x,S.y,color,depth);
		}
	} else {
		for(;S.y<=B.y;S.y++,E.y++,S.x+=dx1,E.x+=dx2)
		{
			hLine(S.x,E.x,S.y,color,depth);
		}
		S.x=(float)B.x;
		S.y=(float)B.y;

		for(;S.y<=C.y;S.y++,E.y++,S.x+=dx3,E.x+=dx2)
		{
			hLine(S.x,E.x,S.y,color,depth);
		}
	}


}







void render_3d_model(int x0,int y0,float aa,SDL_Surface *screen)
{

	memset(Video,0,WIDTH*HEIGHT);
	memset(Depth,0,WIDTH*HEIGHT);

	vec2i aa = {20,320};
	vec2i bb = {180,160};
	vec2i cc = {40,200};
	
	float t = SDL_GetTicks()*.0001;
	vec3 rotation = {3.14*.5,0,t*5};
	vec3 light = {.5,.5,.5};

	matrix3d rotationMatrix = buildMatrix(rotation);

	vec3i projectedPoints[120];
	int zorderPoints[120];
	
	for (int i=0;i<120;i++){
	vec3 rotatedPoint;
	
	vec3 rVert = vertices[i];
	vec3 temp;

	temp = rVert;
	temp.x*=rotationMatrix.x.x;
	temp.y*=rotationMatrix.x.y;
	temp.z*=rotationMatrix.x.z;
	rotatedPoint.x = temp.x + temp.y + temp.z;

	temp = rVert;
	temp.x*=rotationMatrix.y.x;
	temp.y*=rotationMatrix.y.y;
	temp.z*=rotationMatrix.y.z;
	rotatedPoint.y  = temp.x + temp.y + temp.z;

	temp = rVert;
	temp.x*=rotationMatrix.z.x;
	temp.y*=rotationMatrix.z.y;
	temp.z*=rotationMatrix.z.z;
	rotatedPoint.z  = temp.x + temp.y + temp.z;

	
	rotatedPoint.x*=40;
	rotatedPoint.y*=40;
	rotatedPoint.z+=150;
	rotatedPoint.x/=rotatedPoint.z;
	rotatedPoint.y/=rotatedPoint.z;
	rotatedPoint.x*=8;
	rotatedPoint.y*=8;
	rotatedPoint.x+=WIDTH/2;
	rotatedPoint.y+=HEIGHT/2;
	
	
	projectedPoints[i].x = rotatedPoint.x;
	projectedPoints[i].y = rotatedPoint.y;
	projectedPoints[i].z = rotatedPoint.z-150;
	}

	for (int i = 0; i < 236; i++){
		vec3 face = faces[i];
		vec3i v1 = projectedPoints[(int)face.x];
		vec3i v2 = projectedPoints[(int)face.y];
		vec3i v3 = projectedPoints[(int)face.z];
	
		int shade = 100+(int)(100* (light.x * normals[i].x + light.y * normals[i].y + light.z * normals[i].z));
		byte clockwise = ((v3.x - v1.x) * (v2.y - v1.y)) > ((v3.y - v1.y) * (v2.x - v1.x));
	    if (clockwise)  drawTriangle(projectedPoints[(int)face.x],projectedPoints[(int)face.y],projectedPoints[(int)face.z],shade);
	}


  /* Blit the final image to the screen */
  if ( SDL_LockSurface(screen) == 0 ) {
    int row;
    Uint8 *src, *dst;

    src = Video;
    dst = (Uint8 *)screen->pixels;
    for ( row=screen->h; row>0; --row )
    {
      memcpy(dst, src, WIDTH);
      src += WIDTH;
      dst += screen->pitch;
    }
    SDL_UnlockSurface(screen);
  }
  SDL_UpdateRect(screen, 0, 0, 0, 0);
}














float FOV=3.141592654/16;   /* half of the xy field of view */

/*
// Draw the view from the point x0,y0 (16.16) looking at angle a
*/
void render_heightmap(int x0,int y0,float aa,SDL_Surface *screen)
{
  int d;
  int a,b,h,u0,v0,u1,v1,h0,h1,h2,h3;

  /* Clear offscreen buffer */
  memset(Video,0,WIDTH*HEIGHT);

  /* Initialize last-y and last-color arrays */
  for ( d=0; d<WIDTH; d++ )
  {
    lasty[d]=HEIGHT;
    lastc[d]=-1;
  }

  /* Compute viewpoint height value */

  /* Compute the xy coordinates; a and b will be the position inside the
     single map cell (0..255).
   */
  u0=(x0>>16)&0xFF;    a=(x0>>8)&255;
  v0=((y0>>8)&0xFF00); b=(y0>>8)&255;
  u1=(u0+1)&0xFF;
  v1=(v0+256)&0xFF00;

  /* Fetch the height at the four corners of the square the point is in */
  h0=HMap[u0+v0]; h2=HMap[u0+v1];
  h1=HMap[u1+v0]; h3=HMap[u1+v1];

  /* Compute the height using bilinear interpolation */
  h0=(h0<<8)+a*(h1-h0);
  h2=(h2<<8)+a*(h3-h2);
  h=((h0<<8)+b*(h2-h0))>>16;

  h=0;

  /* Draw the landscape from near to far without overdraw */
  for ( d=0; d<HEIGHT/2; d+=1+(d>>6) )
  {
    Line(x0+d*65536*cos(aa-FOV),y0+d*65536*sin(aa-FOV),
         x0+d*65536*cos(aa+FOV),y0+d*65536*sin(aa+FOV),
         h-30,(HEIGHT/2*atan(millis*0.001-d))*256/(d+1));
  }

  /* Blit the final image to the screen */
  if ( SDL_LockSurface(screen) == 0 ) {
    int row;
    Uint8 *src, *dst;

    src = Video;
    dst = (Uint8 *)screen->pixels;
    for ( row=screen->h; row>0; --row )
    {
      memcpy(dst, src, WIDTH);
      src += WIDTH;
      dst += screen->pitch;
    }
    SDL_UnlockSurface(screen);
  }
  SDL_UpdateRect(screen, 0, 0, 0, 0);
}

///////////////////////////////////////////////////////// LSYSTEM_RENDER

int lastframe = 0;

void render_lsystem(int x0,int y0,float aa,SDL_Surface *screen, float millis)
{
  int j = 0;  // lsystem_index

  float t = millis*0.01;

//  push(p, &st);

  int li = 1;

  SDL_LockSurface(screen);
  
    int beg = lastframe-10;
    if (beg < 0) beg = 0;

    p.angle_increment=cos(t);
    p.dist = 0.007+cos(t*0.01)*0.0001;

    SDL_Color cc = { cos(t)*255, sin(t)*255, cos(t)*255 };

    for(j=beg;j<lastframe;j+=1) 
    {

      if (lsystem_axiom[li][j] == 'f') 
      {
          forward(p.dist, 1, x0, y0, screen, cc);
      }

      else if (lsystem_axiom[li][j] == '-' && p.toggle == 1) turn(-((360/(p.angle_increment))));
      else if (lsystem_axiom[li][j] == '+' && p.toggle == 1) turn((360/(p.angle_increment)));

      else if (lsystem_axiom[li][j] == '<'  && p.toggle == 1) z_turn(-((360/p.angle_increment)));
      else if (lsystem_axiom[li][j] == '>' && p.toggle == 1) z_turn((360/p.angle_increment));

      else if (lsystem_axiom[li][j] == '!') p.toggle = -p.toggle;

      else if (lsystem_axiom[li][j] == '[') push(p, &st);
      else if (lsystem_axiom[li][j] == ']') p = pop(&st);
    }

  SDL_UnlockSurface(screen);

  //p = pop(&st);

  lastframe=(int)(t*10+cos(t)*2);
  if (lastframe >= strlen(lsystem_axiom[li])) lastframe = 0;

  SDL_UpdateRect(screen, 0, 0, 0, 0);

}

/////////////////////////////////////////////////////////////// MAIN


main(int argc, char *argv[])
{
  SDL_Surface *screen;
  int done;
  int i,k;
  float ss,sa,a,s;
  int x0,y0;
  SDL_Color colors[255];
  SDL_Event event;
  Uint8 *keystate;

  /* Initialize SDL */
  if ( SDL_Init(SDL_INIT_VIDEO) < 0 )
  {
    fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
    exit(1);
  }
  atexit(SDL_Quit);

  if ( Mix_OpenAudio(44100, AUDIO_S16, 2, 512) < 0 )
  {
    fprintf(stderr, "Couldn't initialize SDL audio: %s\n", SDL_GetError());
    exit(1);
  }

  Mix_Music* music = Mix_LoadMUS("data\\music.ogg");

  if ( music == NULL )
    fprintf(stderr, "Couldn't load music: %s\n", Mix_GetError());

  Mix_SetPostMix(music_mix_callback,0);

  InitializeFFT(256);

  screen = SDL_SetVideoMode(WIDTH, HEIGHT, 32,
			(SDL_HWSURFACE)); //|SDL_FULLSCREEN
  if ( screen == NULL )
  {
    fprintf(stderr, "Couldn't init video mode: %s\n", SDL_GetError());
    exit(1);
  }

  /* Set up the first 64 colors to a grayscale */
  /*
  for ( i=0; i<255; i+=2 )
  {
    colors[i].r = i*0.5;
    colors[i].g = i*2.5;
    colors[i].b = i*4;
  }
  for ( i=1; i<255; i+=2 )
  {
    colors[i].r = i*0.5;
    colors[i].g = i*1.5;
    colors[i].b = i*4;
  }
  */

	/*ilmaripaletti*/
  for ( i=0; i<255; i+=1 )
  {
    colors[i].r = i;
    colors[i].g = i;
    colors[i].b = i;
  }


  SDL_SetColors(screen, colors, 0, 255);

  /* Main loop
    
       a     = angle
       x0,y0 = current position
       s     = speed constant
       ss    = current forward/backward speed
       sa    = angular speed
   */
  done=0;
  a=0; k=x0=y0=0;
  s=1024*1; /*s=4096;*/
  ss=0; sa=0;

  float t = 0.0;
  float dt = 1 / 60.0f;

  float currentTime = SDL_GetTicks();
  float deltaTime = 0.0;


    int j;

  int p,k2,p2;

  /* Start from a plasma clouds fractal */
  HMap[0]=128;
  for ( p=256; p>1; p=p2 )
  {
    p2=p>>1;
    k=p*8+20; k2=k>>1;
    for ( i=0; i<256; i+=p )
    {
      for ( j=0; j<256; j+=p )
      {
  int a,b,c,d;

  a=HMap[(i<<8)+j];
  b=HMap[(((i+p)&255)<<8)+j];
  c=HMap[(i<<8)+((j+p)&255)];
  d=HMap[(((i+p)&255)<<8)+((j+p)&255)];

  HMap[(i<<8)+((j+p2)&255)]=
    Clamp(((a+c)>>1)+(rand()%k-k2));
  HMap[(((i+p2)&255)<<8)+((j+p2)&255)]=
    Clamp(((a+b+c+d)>>2)+(rand()%k-k2));
  HMap[(((i+p2)&255)<<8)+j]=
    Clamp(((a+b)>>1)+(rand()%k-k2));
      }
    }
  }

  /* Smoothing */
  for ( k=0; k<3; k++ )
    for ( i=0; i<256*256; i+=256 )
      for ( j=0; j<256; j++ )
      {
  HMap[i+j]=(HMap[((i+256)&0xFF00)+j]+HMap[i+((j+1)&0xFF)]+
       HMap[((i-256)&0xFF00)+j]+HMap[i+((j-1)&0xFF)])>>2;
      }


  /* Color computation (derivative of the height field) */
  for ( i=0; i<256*256; i+=256 )
    for ( j=0; j<256; j++ )
    {
      k=128+(HMap[((i+256)&0xFF00)+((j+1)&255)]-HMap[i+j])*6;
      if ( k<0 ) k=0; if (k>255) k=255;
      CMap[i+j]=k-CMap[i-1+j];
    }

///////////// LSYSTEM INIT

  int zx, zy, zz;

  for (zx=0;zx<512;zx++) {
      split_rules[zx] = 0;
  }

  for (zy=0;zy<512;zy++) {
      for (zx=0;zx<512;zx++) {
          processed_rules[zx][zy] = 0;
      }
  }

  strcpy(lsystem_axiom[0], "fx"); // cross, angle 4, 0.08
  strcpy(lsystem_rules[0], "x=fx+fx+fxfy-fy-,y=+fx+fxfy-fy-fy");

  strcpy(lsystem_axiom[1], "++++f");  // bush, angle 16
  strcpy(lsystem_rules[1], "f=ff-[-f+f+f]+[+f-f-f]");

  strcpy(lsystem_axiom[2], "fx"); // dragon, angle 8
  strcpy(lsystem_rules[2], "y=+fx--fy+,x=-fx++fy-");

  strcpy(lsystem_axiom[3], "f"); // sierpinski
  strcpy(lsystem_rules[3], "f=fxf,x=+fxf-fxf-fxf+");

  lsystem_iteration();

  fft_type data1[256],data2[256];
  fft_type re,im,value;

  while(!done)
  {
    if (!Mix_PlayingMusic())
      Mix_PlayMusic(music, 0);

    float newTime = SDL_GetTicks();
    float frameTime = newTime - currentTime;
    currentTime = newTime;
    millis+=frameTime;

    for (i=0;i<256;i++)
    {
      data1[i]=music_mix_stream[i*2];
      data2[i]=music_mix_stream[i*2+1];
    }

    RealFFT(&data1[0]);
    RealFFT(&data2[0]);

   

    for (i=0;i<128;i++)
    {
      re=data1[BitReversed[i]];
      im=data1[BitReversed[i]+1];
      value=sqrt(re*re+im*im)/100;
    }



    // newmap
    for (j = 0; j < 256; j+=4)
    {
      for (i = 0; i < 256; i+=4)
      {
        HMap[j*256+i]=128-cos(j-i*millis*0.0005/atan(j*1+i))*255*tan(value);
      }
    }
  /* Smoothing */
  for ( k=0; k<1; k++ )
    for ( i=0; i<256*256; i+=256 )
      for ( j=0; j<256; j++ )
      {
  HMap[i+j]=(HMap[((i+256)&0xFF00)+j]+HMap[i+((j+1)&0xFF)]+
       HMap[((i-256)&0xFF00)+j]+HMap[i+((j-1)&0xFF)])>>2;
      }
  /* Color computation (derivative of the height field) */
  for ( i=0; i<256*256; i+=512 )
    for ( j=0; j<256; j+=2 )
    {
      k=128+(HMap[((i+256)&0xFF00)+((j+1)&255)]-HMap[i+j])*4;
      if ( k<0 ) k=0; if (k>255) k=255;
      CMap[i+j]=k;
    }

    while (frameTime > 0.0f)
    {
      deltaTime = min(frameTime, dt);

      // LOGIC --------------------------------------

      /* Update position/angle */
      x0+=ss*cos(a)*deltaTime; y0+=ss*sin(a)*deltaTime;

      a=cos(millis*0.000001)*255;
      ss = 2;


      // /LOGIC --------------------------------------

      frameTime -= deltaTime;
      t += deltaTime;
    }
    FOV=M_PI+millis*0.00001;

    // RENDER --------------------------------------
    /* Draw the frame */
   render_lsystem(0,0,a,screen, millis);
   //render_heightmap(x0,y0,a,screen);
   // render_3d_model(x0,y0,a,screen);
    // /RENDER -------------------------------------


	/*palettitweak*/
/*
  for ( i=1; i<255; i+=3 )
  {
    colors[i].r = i*4.5+value*cos(millis)*50;
    colors[i].g = i*1.5+value*120;
    colors[i].b = i*4-value*10;
  }
  SDL_SetColors(screen, colors, 0, 255);
*/

    /* User input */
    while ( SDL_PollEvent(&event) )
    {
      if ( event.type == SDL_QUIT )
      {
          done = 1;
      }
    }
    keystate = SDL_GetKeyState(NULL);
    if ( keystate[SDLK_ESCAPE] ) {
      done = 1;
    }
  }

  /* Exit to text mode */
  exit(0);
}