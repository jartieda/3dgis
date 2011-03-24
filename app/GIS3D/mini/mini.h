// (c) by Stefan Roettger

#ifndef MINI_H
#define MINI_H

void setminierrorhandler(void (*handler)(const char *file,int line,int fatal)=0);

namespace mini {

void setparams(float minr=9.0f,
               float maxd=100.0f,
               float sead=0.1f,
               float mino=0.1f,
               int maxc=8);

void *initmap(short int *image,void **d2map,
              int *size,float *dim,float scale,
              float cellaspect=1.0f,
              short int (*getelevation)(int i,int j,int size,void *data)=0,
              void *objref=0,
              int fast=0,float avgd2=0.0f);

int inittexmap(unsigned char *image=0,int *width=0,int *height=0,
               int mipmaps=1,int s3tc=0,int rgba=0,int bytes=0,int mipmapped=0);

void *initfogmap(unsigned char *image,int size,
                 float lambda,float displace,float emission,
                 float fogatt=1.0f,float fogR=1.0f,float fogG=1.0f,float fogB=1.0f,
                 int fast=0);

void setmaps(void *map,void *d2map,
             int size,float dim,float scale,
             int texid=0,int width=0,int height=0,int mipmaps=1,
             float cellaspect=1.0f,
             float ox=0.0f,float oy=0.0f,float oz=0.0f,
             void **d2map2=0,int *size2=0,
             void *fogmap=0,float lambda=0.0f,float displace=0.0f,
             float emission=0.0f,float fogatt=1.0f,float fogR=1.0f,float fogG=1.0f,float fogB=1.0f);

void setsea(float level);
void setsearange(float seamin,float seamax);

void drawlandscape(float res,
                   float ex,float ey,float ez,
                   float fx,float fy,float fz,
                   float dx,float dy,float dz,
                   float ux,float uy,float uz,
                   float fovy,float aspect,
                   float nearp,float farp,
                   void (*beginfan)()=0,
                   void (*fanvertex)(float i,float y,float j)=0,
                   void (*notify)(int i,int j,int s)=0,
                   void (*prismedge)(float x,float y,float yf,float z)=0,
                   int state=0);

int checklandscape(float ex,float ey,float ez,
                   float dx,float dy,float dz,
                   float ux,float uy,float uz,
                   float fovy,float aspect,
                   float nearp,float farp);

void drawprismcache(float ex,float ey,float ez,
                    float dx,float dy,float dz,
                    float nearp,float farp,
                    float emission=0.0f,float fogR=1.0f,float fogG=1.0f,float fogB=1.0f,
                    float *prismcache=0,int prismcnt=0);

float getheight(int i,int j);
void getheight(float s,float t,float *height);
float getheight(float x,float z);
float getfogheight(float x,float z);
void getnormal(float s,float t,float *nx,float *nz);
void getnormal(float x,float z,float *nx,float *ny,float *nz);
int getmaxsize(float res,float fx,float fy,float fz,float fovy=0.0f);

void setheight(int i,int j,float h);
int setheight(float x,float z,float h);
void setrealheight(int i,int j,float h);
int setrealheight(float x,float z,float h);

void updatemaps(int fast=0,float avgd2=0.0f,int recalc=0);

void deletemaps();

// helper functions:

extern int S;
extern float Dx,SCALE,Dz;
extern float OX,OY,OZ;

inline float getX(const float i)
   {return((i-S/2)*Dx+OX);}
inline float getY(const float y)
   {return(y*SCALE+OY);}
inline float getZ(const float j)
   {return((S/2-j)*Dz+OZ);}

}

namespace Mini {

void setparams(float minr=9.0f,
               float maxd=100.0f,
               float sead=0.1f,
               float mino=0.1f,
               int maxc=8);

void *initmap(float *image,void **d2map,
              int *size,float *dim,float scale,
              float cellaspect=1.0f,
              float (*getelevation)(int i,int j,int size,void *data)=0,
              void *objref=0,
              int fast=0,float avgd2=0.0f);

int inittexmap(unsigned char *image=0,int *width=0,int *height=0,
               int mipmaps=1,int s3tc=0,int rgba=0,int bytes=0,int mipmapped=0);

void *initfogmap(unsigned char *image,int size,
                 float lambda,float displace,float emission,
                 float fogatt=1.0f,float fogR=1.0f,float fogG=1.0f,float fogB=1.0f,
                 int fast=0);

void setmaps(void *map,void *d2map,
             int size,float dim,float scale,
             int texid=0,int width=0,int height=0,int mipmaps=1,
             float cellaspect=1.0f,
             float ox=0.0f,float oy=0.0f,float oz=0.0f,
             void **d2map2=0,int *size2=0,
             void *fogmap=0,float lambda=0.0f,float displace=0.0f,
             float emission=0.0f,float fogatt=1.0f,float fogR=1.0f,float fogG=1.0f,float fogB=1.0f);

void setsea(float level);
void setsearange(float seamin,float seamax);

void drawlandscape(float res,
                   float ex,float ey,float ez,
                   float fx,float fy,float fz,
                   float dx,float dy,float dz,
                   float ux,float uy,float uz,
                   float fovy,float aspect,
                   float nearp,float farp,
                   void (*beginfan)()=0,
                   void (*fanvertex)(float i,float y,float j)=0,
                   void (*notify)(int i,int j,int s)=0,
                   void (*prismedge)(float x,float y,float yf,float z)=0,
                   int state=0);

int checklandscape(float ex,float ey,float ez,
                   float dx,float dy,float dz,
                   float ux,float uy,float uz,
                   float fovy,float aspect,
                   float nearp,float farp);

void drawprismcache(float ex,float ey,float ez,
                    float dx,float dy,float dz,
                    float nearp,float farp,
                    float emission=0.0f,float fogR=1.0f,float fogG=1.0f,float fogB=1.0f,
                    float *prismcache=0,int prismcnt=0);

float getheight(int i,int j);
void getheight(float s,float t,float *height);
float getheight(float x,float z);
float getfogheight(float x,float z);
void getnormal(float s,float t,float *nx,float *nz);
void getnormal(float x,float z,float *nx,float *ny,float *nz);
int getmaxsize(float res,float fx,float fy,float fz,float fovy=0.0f);

void setheight(int i,int j,float h);
int setheight(float x,float z,float h);
void setrealheight(int i,int j,float h);
int setrealheight(float x,float z,float h);

void updatemaps(int fast=0,float avgd2=0.0f,int recalc=0);

void deletemaps();

// helper functions:

extern int S;
extern float Dx,SCALE,Dz;
extern float OX,OY,OZ;

inline float getX(const float i)
   {return((i-S/2)*Dx+OX);}
inline float getY(const float y)
   {return(y*SCALE+OY);}
inline float getZ(const float j)
   {return((S/2-j)*Dz+OZ);}

}

#endif
