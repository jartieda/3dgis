// (c) by Stefan Roettger

#ifndef MINISTUB_H
#define MINISTUB_H

class ministub
   {
   public:

   //! default constructors:

   ministub(short int *image, // height field is a short int array
            int *size,float *dim,float scale,float cellaspect, // grid definition
            float centerx,float centery,float centerz, // grid center
            void (*beginfan)(),void (*fanvertex)(float i,float y,float j), // mandatory callbacks
            void (*notify)(int i,int j,int s)=0, // optional callback
            short int (*getelevation)(int i,int j,int S,void *data)=0, // optional elevation callback
            void *objref=0, // optional data pointer that is passed to the elevation callback
            unsigned char *fogimage=0, // optional ground fog layer
            int fogsize=0,float lambda=0.0f,float displace=0.0f,float attenuation=1.0f,
            void (*prismedge)(float x,float y,float yf,float z)=0,
            void **d2map2=0,int *size2=0,
            float minres=0.0f,float minoff=0.0f,
            float maxd2=0.0f,float sead2=0.0f,
            float seamin=0.0f,float seamax=0.0f,
            int maxcull=0);

   ministub(float *image, // height field is a float array
            int *size,float *dim,float scale,float cellaspect, // grid definition
            float centerx,float centery,float centerz, // grid center
            void (*beginfan)(),void (*fanvertex)(float i,float y,float j), // mandatory callbacks
            void (*notify)(int i,int j,int s)=0, // optional callback
            float (*getelevation)(int i,int j,int S,void *data)=0, // optional elevation callback
            void *objref=0, // optional data pointer that is passed to the elevation callback
            unsigned char *fogimage=0, // optional ground fog layer
            int fogsize=0,float lambda=0.0f,float displace=0.0f,float attenuation=1.0f,
            void (*prismedge)(float x,float y,float yf,float z)=0,
            void **d2map2=0,int *size2=0,
            float minres=0.0f,float minoff=0.0f,
            float maxd2=0.0f,float sead2=0.0f,
            float seamin=0.0f,float seamax=0.0f,
            int maxcull=0);

   //! destructor
   ~ministub();

   //! draw scene
   void draw(float res, // resolution
             float ex,float ey,float ez, // eye point
             float dx,float dy,float dz, // view direction
             float ux,float uy,float uz, // up vector
             float fovy,float aspect, // field of view and aspect
             float nearp,float farp); // near and far plane

   //! set focus of interest (equal to eye point by default)
   void setfocus(int on,float fx=0.0f,float fy=0.0f,float fz=0.0f);

   //! define relative scaling (0<=scale<=1)
   void setrelscale(float scale=1.0f);

   //! set sea level
   void setsealevel(float level);

   //! get-functions for geometric properties
   float getheight(int i,int j);
   float getheight(float x,float z);
   float getfogheight(float x,float z);
   void getnormal(float x,float z,float *nx,float *ny,float *nz);

   //! get-functions for geometric settings
   int getsize() {return(SIZE);}
   float getdim() {return(DIM);}
   float getscale() {return(SCALE);}
   float getcellaspect() {return(CELLASPECT);}
   float getcenterx() {return(CENTERX);}
   float getcentery() {return(CENTERY);}
   float getcenterz() {return(CENTERZ);}
   float getlambda() {return(LAMBDA);}
   float getdisplace() {return(DISPLACE);}
   float getrelscale() {return(RELSCALE);}
   float getsealevel() {return(SEALEVEL);}

   //! modify the terrain at run time
   void setheight(int i,int j,float h);
   void setheight(float x,float z,float h);
   void setrealheight(int i,int j,float h);
   void setrealheight(float x,float z,float h);

   //! enable fast update
   void setfastupdate(int on);

   protected:

   int FLOAT;

   void *MAP,*D2MAP,*FOGMAP;

   int SIZE;
   float DIM,SCALE,CELLASPECT;
   float CENTERX,CENTERY,CENTERZ;
   float RELSCALE,SEALEVEL;
   float LAMBDA,DISPLACE;
   float ATTENUATION;

   void (*BEGINFAN_CALLBACK)();
   void (*FANVERTEX_CALLBACK)(float i,float y,float j);
   void (*NOTIFY_CALLBACK)(int i,int j,int s);
   void (*PRISMEDGE_CALLBACK)(float x,float y,float yf,float z);

   void *D2MAP2[4];
   int SIZE2[4];

   float MINRES,MINOFF;
   float MAXD2,SEAD2;
   float SEAMIN,SEAMAX;
   int MAXCULL;

   int FOCUS;
   float FX,FY,FZ;

   int MODIFIED;

   int FAST;

   private:
   };

#endif
