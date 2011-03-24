// (c) by Stefan Roettger

#include "minibase.h"

#include "miniOGL.h"
#include "miniutm.h"

#include "pnmbase.h"

#include "miniglobe.h"

// default constructor
miniglobe::miniglobe()
   {
   SHAPE=SHAPE_EARTH;

   SCALE=1.0f;
   STRIPES=64;

   STRIP=new ministrip(0,3,2);
   SLOT=STRIP->getfreeslot();

   SHADE=1;

   DONE=0;

   CONFIGURE_FRONTNAME=strdup("Front.ppm");
   CONFIGURE_BACKNAME=strdup("Back.ppm");

   CONFIGURE_FRONTBUF=NULL;
   CONFIGURE_BACKBUF=NULL;
   }

// destructor
miniglobe::~miniglobe()
   {
   delete STRIP;

   free(CONFIGURE_FRONTNAME);
   free(CONFIGURE_BACKNAME);
   }

// set shape of globe
void miniglobe::setshape(int shape)
   {SHAPE=shape;}

// set scaling of globe
void miniglobe::setscale(float scale)
   {SCALE=scale;}

// set dynamic scaling of globe
void miniglobe::setdynscale(float scale)
   {STRIP->setscale(scale);}

// set tesselation of globe
void miniglobe::settess(int stripes)
   {STRIPES=stripes;}

// set matrix
void miniglobe::setmatrix(double mtx[16])
   {STRIP->setmatrix(mtx);}

// set direct shading parameters
void miniglobe::setshadedirectparams(float lightdir[3],
                                     float lightbias,float lightoffset)
   {
   if (SHADE!=0)
      STRIP->setshadedirectparams(SLOT,lightdir,lightbias,lightoffset);
   }

// set direct texturing parameters
void miniglobe::settexturedirectparams(float lightdir[3],float transbias,float transoffset)
   {
   if (SHADE==0)
      STRIP->settexturedirectparams(SLOT,lightdir,transbias,transoffset);
   }

// set fog parameters
void miniglobe::setfogparams(float fogstart,float fogend,
                             float fogdensity,
                             float *fogcolor)
   {STRIP->setfogparams(SLOT,fogstart,fogend,fogdensity,fogcolor);}

// render the globe
void miniglobe::render(int phase)
   {
   if (DONE==0)
      {
      if (SHAPE==SHAPE_SUN) create_sun();
      else if (SHAPE==SHAPE_EARTH) create_earth();
      else if (SHAPE==SHAPE_MOON) create_moon();
      else ERRORMSG();

      create_shader(CONFIGURE_FRONTNAME,CONFIGURE_BACKNAME,
                    CONFIGURE_FRONTBUF,CONFIGURE_BACKBUF);

      DONE=1;
      }

   initstate();

   if (phase==FIRST_RENDER_PHASE) disableZwriting();
   if (phase==LAST_RENDER_PHASE) disableRGBAwriting();

   STRIP->render();

   if (phase==FIRST_RENDER_PHASE) enableZwriting();
   if (phase==LAST_RENDER_PHASE) enableRGBAwriting();

   exitstate();
   }

// create a globe with constant radius and color
void miniglobe::create_globe(float radius,const float color[3])
   {
   int i,j;

   int const alpha_steps=4*STRIPES;
   int const beta_steps=STRIPES;

   float u,v;
   float alpha,beta;
   float nx,ny,nz;

   STRIP->setcol(color[0],color[1],color[2]);

   for (j=-beta_steps; j<beta_steps; j++)
      {
      STRIP->beginstrip();

      for (i=alpha_steps; i>=0; i--)
         {
         u=(float)i/alpha_steps;
         v=(float)j/beta_steps;

         alpha=u*2*PI;
         beta=v*PI/2;

         nx=fcos(alpha)*fcos(beta);
         nz=-fsin(alpha)*fcos(beta);
         ny=fsin(beta);

         STRIP->setnrm(nx,ny,nz);
         STRIP->settex(u,0.5f-v/2);
         STRIP->addvtx(nx*radius,ny*radius,nz*radius);

         v=(float)(j+1)/beta_steps;
         beta=v*PI/2;

         nx=fcos(alpha)*fcos(beta);
         nz=-fsin(alpha)*fcos(beta);
         ny=fsin(beta);

         STRIP->setnrm(nx,ny,nz);
         STRIP->settex(u,0.5f-v/2);
         STRIP->addvtx(nx*radius,ny*radius,nz*radius);
         }
      }
   }

// create the earth with ECEF coords
void miniglobe::create_earth(const float color[3])
   {
   int i,j;

   const int alpha_steps=4*STRIPES;
   const int beta_steps=STRIPES;

   float u,v;
   float alpha,beta;
   float xyz[3];

   float len;
   float nx,ny,nz;

   STRIP->setcol(color[0],color[1],color[2]);

   for (j=-beta_steps; j<beta_steps; j++)
      {
      STRIP->beginstrip();

      for (i=alpha_steps; i>=0; i--)
         {
         u=(float)i/alpha_steps;
         v=(float)j/beta_steps;

         alpha=u*360*60*60;
         beta=v*90*60*60;

         miniutm::LLH2ECEF(beta,alpha,0.0f,xyz);

         xyz[0]/=SCALE;
         xyz[1]/=SCALE;
         xyz[2]/=SCALE;

         len=fsqrt(fsqr(xyz[0])+fsqr(xyz[1])+fsqr(xyz[2]));

         nx=xyz[0]/len;
         ny=xyz[1]/len;
         nz=xyz[2]/len;

         STRIP->setnrm(nx,nz,-ny);
         STRIP->settex(u,0.5f-v/2);
         STRIP->addvtx(xyz[0],xyz[2],-xyz[1]);

         v=(float)(j+1)/beta_steps;
         beta=v*90*60*60;

         miniutm::LLH2ECEF(beta,alpha,0.0f,xyz);

         xyz[0]/=SCALE;
         xyz[1]/=SCALE;
         xyz[2]/=SCALE;

         len=fsqrt(fsqr(xyz[0])+fsqr(xyz[1])+fsqr(xyz[2]));

         nx=xyz[0]/len;
         ny=xyz[1]/len;
         nz=xyz[2]/len;

         STRIP->setnrm(nx,nz,-ny);
         STRIP->settex(u,0.5f-v/2);
         STRIP->addvtx(xyz[0],xyz[2],-xyz[1]);
         }
      }
   }

// create the sun globe
void miniglobe::create_sun()
   {
   static const float radius=695.5E6f;
   static const float color[3]={1.0f,0.75f,0.5f};

   create_globe(radius/SCALE,color);
   }

// create the earth globe
void miniglobe::create_earth()
   {
   static const float color[3]={0.5f,0.75f,1.0f};

   create_earth(color);
   }

// create the moon globe
void miniglobe::create_moon()
   {
   static const float radius=1.7374E6f;
   static const float color[3]={0.9f,0.9f,0.9f};

   create_globe(radius/SCALE,color);
   }

// create shader
void miniglobe::create_shader(const char *frontname,const char *backname,
                              databuf *frontbuf,databuf *backbuf)
   {
   int i;

   unsigned char *image1;
   int width1,height1,comps1;

   unsigned char *image2;
   int width2,height2,comps2;

   double texmtx[16];

   if (frontname!=NULL && backname!=NULL)
      {
      image1=readPNMfile(frontname,&width1,&height1,&comps1);
      if (image1!=NULL && comps1!=3) ERRORMSG();

      image2=readPNMfile(backname,&width2,&height2,&comps2);
      if (image2!=NULL && (width1!=width2 || height2!=height1 || comps2!=3)) ERRORMSG();
      }
   else image1=image2=NULL;

   if (frontbuf!=NULL && backbuf!=NULL)
      {
      width1=frontbuf->xsize;
      height1=frontbuf->ysize;

      width2=backbuf->xsize;
      height2=backbuf->ysize;

      if (width1!=width2 || height2!=height1) ERRORMSG();
      }

   if ((image1!=NULL && image2!=NULL) ||
       (frontbuf!=NULL && backbuf!=NULL))
      {
      SHADE=0;

      STRIP->setcol(1.0f,1.0f,1.0f);

      if (frontbuf==NULL || backbuf==NULL)
         {
         STRIP->setpixshadertex(SLOT,image1,width1,height1,comps1,1,0);
         STRIP->setpixshadertex(SLOT,image2,width2,height2,comps2,1,1);
         }
      else
         {
         STRIP->setpixshadertexbuf(SLOT,frontbuf,1,0);
         STRIP->setpixshadertexbuf(SLOT,backbuf,1,1);
         }

      for (i=0; i<16; i++) texmtx[i]=0.0;
      for (i=0; i<4; i++) texmtx[i+4*i]=1.0;

      texmtx[5]=(double)(height1-1)/height1;

      texmtx[12]=0.5/width1-0.5;
      texmtx[13]=0.5/height1;

      STRIP->settexmatrix(texmtx);

      if (image1!=NULL && image2!=NULL)
         {
         free(image1);
         free(image2);
         }
      }

   STRIP->concatvtxshader(SLOT,MINI_SNIPPET_VTX_BEGIN);
   STRIP->concatvtxshader(SLOT,MINI_SNIPPET_VTX_HEADER);
   STRIP->concatvtxshader(SLOT,MINI_SNIPPET_VTX_BASIC);
   STRIP->concatvtxshader(SLOT,MINI_SNIPPET_VTX_NORMAL_DIRECT);
   if (SHADE==0) STRIP->concatvtxshader(SLOT,MINI_SNIPPET_VTX_TEX);
   STRIP->concatvtxshader(SLOT,MINI_SNIPPET_VTX_FOG);
   STRIP->concatvtxshader(SLOT,MINI_SNIPPET_VTX_FOOTER);
   STRIP->concatvtxshader(SLOT,MINI_SNIPPET_VTX_END);

   STRIP->concatpixshader(SLOT,MINI_SNIPPET_FRG_BEGIN);
   STRIP->concatpixshader(SLOT,MINI_SNIPPET_FRG_HEADER);
   STRIP->concatpixshader(SLOT,MINI_SNIPPET_FRG_BASIC);
   if (SHADE==0) STRIP->concatpixshader(SLOT,MINI_SNIPPET_FRG_TEX2_DIRECT);
   else STRIP->concatpixshader(SLOT,MINI_SNIPPET_FRG_SHADE_DIRECT);
   STRIP->concatpixshader(SLOT,MINI_SNIPPET_FRG_FOG);
   STRIP->concatpixshader(SLOT,MINI_SNIPPET_FRG_FOOTER);
   STRIP->concatpixshader(SLOT,MINI_SNIPPET_FRG_END);

   STRIP->useshader(SLOT);
   }

// configuring:

void miniglobe::configure_frontname(const char *frontname)
   {CONFIGURE_FRONTNAME=strdup(frontname);}

void miniglobe::configure_backname(const char *backname)
   {CONFIGURE_BACKNAME=strdup(backname);}

void miniglobe::configure_frontbuf(databuf *frontbuf)
   {CONFIGURE_FRONTBUF=frontbuf;}

void miniglobe::configure_backbuf(databuf *backbuf)
   {CONFIGURE_BACKBUF=backbuf;}
