// (c) by Stefan Roettger

#ifndef MINILOD_H
#define MINILOD_H

#include "datacalc.h"
#include "minibrick.h"

typedef struct
   {
   char *bname;
   minibrick *brick;

   float brad;
   unsigned int lods;
   float stagger;
   }
minibrickdata;

typedef struct
   {
   unsigned int bindex;

   float x,y,e;
   float dx,dy,de;
   float r,g,b,a;

   float dx1,dy1,dz1;
   float dx2,dy2,dz2;
   float dx3,dy3,dz3;
   }
miniloddata;

class minilod
   {
   public:

   //! default constructor
   minilod(float offsetlat=0.0f,float offsetlon=0.0f,
           float scalex=1.0f,float scaley=1.0f,float scaleelev=1.0f);

   //! destructor
   ~minilod();

   //! add minibrick volume
   unsigned int addbrick(const char *brickname,
                         float brad=0.0f,
                         unsigned int lods=0,
                         float stagger=0.0f);

   //! add volume at specific location
   unsigned int addvolume(unsigned int bindex,
                          float midx,float midy,float basez,
                          float dx,float dy,float de,
                          float r,float g,float b,float a);

   //! add orientation to volume
   void addorientation(unsigned int vindex,
                       float dx1,float dy1,float dz1,
                       float dx2,float dy2,float dz2,
                       float dx3,float dy3,float dz3);

   //! clear volumes
   void clearvolumes() {VNUM=0;}

   //! render the volumes
   void render(float ex,float ey,float ez,
               float farp,
               float fovy,float aspect,
               int phase=MINIBRICK_ONE_RENDER_PHASE);

   //! get number of bricks
   unsigned int getbricks() {return(BNUM);}

   //! get number of volumes
   unsigned int getvolumes() {return(VNUM);}

   //! configuring of brick renderer
   void configure_brickpasses(int brickpasses=1); // render passes for brick display
   void configure_brickoffset(float brickoffset=0.0f); // offset of brick stripes

   protected:

   float OFFSETLAT,OFFSETLON;
   float SCALEX,SCALEY,SCALEELEV;

   minibrickdata *BRICKS;
   unsigned int BNUM,BMAX;

   miniloddata *VOLS;
   unsigned int VNUM,VMAX;

   int CONFIGURE_BRICKPASSES;
   float CONFIGURE_BRICKOFFSET;

   private:

   static datacalc CALC;

   static int DBavailable_callback(int col,int row,int lod,void *data);
   static void DBload_callback(int col,int row,int lod,databuf *volume,void *data);
   };

#endif
