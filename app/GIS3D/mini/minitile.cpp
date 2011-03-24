// (c) by Stefan Roettger

#include "minibase.h"

#include "mini.h"

#include "minimath.h"

#include "miniutm.h"

#include "pnmbase.h"
#include "pnmsample.h"

#include "minitile.h"

const int minitile::MAX_STR=1024;

float minitile::CONFIGURE_MINRES=9.0f;
float minitile::CONFIGURE_MAXD2=100.0f;
float minitile::CONFIGURE_SEAD2=0.1f;
float minitile::CONFIGURE_MINOFF=0.1f;
int minitile::CONFIGURE_MAXCULL=8;

float minitile::CONFIGURE_SEAMIN=-MAXFLOAT;
float minitile::CONFIGURE_SEAMAX=-MAXFLOAT;
int minitile::CONFIGURE_MIPMAPS=1;

char minitile::CONFIGURE_TILESETPATH[MAX_STR]="";

// default constructor
minitile::minitile(unsigned char **hfields,unsigned char **textures,
                   int cols,int rows,float coldim,float rowdim,float scale,
                   float centerx,float centery,float centerz,
                   unsigned char **fogmaps,float lambda,float displace,
                   float emission,float attenuation,
                   float fogR,float fogG,float fogB,
                   float minres,float minoff,
                   int loaded)
   {
   int i,j,k;

   unsigned char *image;
   int width,height,components;

   short int *hfield;

   if (cols<1 || rows<1) ERRORMSG();
   if (coldim<=0.0f || rowdim<=0.0f) ERRORMSG();

   FAST=0;
   AVGD2=0.0f;

   if (minres>=1.0f)
      {
      CONFIGURE_MINRES=minres;
      CONFIGURE_MINOFF=minoff;
      }

   mini::setparams(CONFIGURE_MINRES,CONFIGURE_MAXD2,CONFIGURE_SEAD2,CONFIGURE_MINOFF,CONFIGURE_MAXCULL);

   mini::setsearange(CONFIGURE_SEAMIN,CONFIGURE_SEAMAX);

   ID=0;

   COLS=cols;
   ROWS=rows;

   COLDIM=coldim;
   ROWDIM=rowdim;

   if ((MAP=(void **)malloc(cols*rows*sizeof(void *)))==NULL) ERRORMSG();
   if ((D2MAP=(void **)malloc(cols*rows*sizeof(void *)))==NULL) ERRORMSG();
   if ((FOGMAP=(void **)malloc(cols*rows*sizeof(void *)))==NULL) ERRORMSG();

   if ((MAP2=(void **)malloc(cols*rows*sizeof(void *)))==NULL) ERRORMSG();
   if ((D2MAP2=(void **)malloc(cols*rows*sizeof(void *)))==NULL) ERRORMSG();
   if ((FOGMAP2=(void **)malloc(cols*rows*sizeof(void *)))==NULL) ERRORMSG();

   if ((SIZE=(int *)malloc(cols*rows*sizeof(int)))==NULL) ERRORMSG();
   if ((DIM=(float *)malloc(cols*rows*sizeof(float)))==NULL) ERRORMSG();

   if ((SIZE2=(int *)malloc(cols*rows*sizeof(int)))==NULL) ERRORMSG();
   if ((DIM2=(float *)malloc(cols*rows*sizeof(float)))==NULL) ERRORMSG();

   if ((TEXID=(int *)malloc(cols*rows*sizeof(int)))==NULL) ERRORMSG();
   if ((TEXWIDTH=(int *)malloc(cols*rows*sizeof(int)))==NULL) ERRORMSG();
   if ((TEXHEIGHT=(int *)malloc(cols*rows*sizeof(int)))==NULL) ERRORMSG();
   if ((TEXMIPMAPS=(int *)malloc(cols*rows*sizeof(int)))==NULL) ERRORMSG();

   if ((USEFLOAT=(int *)malloc(cols*rows*sizeof(int)))==NULL) ERRORMSG();
   if ((USEFLOAT2=(int *)malloc(cols*rows*sizeof(int)))==NULL) ERRORMSG();

   if ((RELOADED=(int *)malloc(cols*rows*sizeof(int)))==NULL) ERRORMSG();
   if ((UPDATED=(int *)malloc(cols*rows*sizeof(int)))==NULL) ERRORMSG();
   if ((MODIFIED=(int *)malloc(cols*rows*sizeof(int)))==NULL) ERRORMSG();

   SCALE=scale;
   CELLASPECT=rowdim/coldim;

   CENTERX=centerx;
   CENTERY=centery;
   CENTERZ=centerz;

   for (i=0; i<cols; i++)
      for (j=0; j<rows; j++)
         {
         MAP[i+j*cols]=NULL;
         TEXID[i+j*cols]=0;
         FOGMAP[i+j*cols]=NULL;

         MAP2[i+j*cols]=NULL;
         FOGMAP2[i+j*cols]=NULL;

         SIZE[i+j*cols]=0;
         TEXWIDTH[i+j*cols]=TEXHEIGHT[i+j*cols]=0;
         TEXMIPMAPS[i+j*cols]=CONFIGURE_MIPMAPS; // mipmaps are enabled by default

         USEFLOAT[i+j*cols]=0;

         if (hfields!=NULL)
            {
            if (hfields[i+j*cols]==NULL) ERRORMSG();

            if ((image=readPNMimage(hfields[i+j*cols],&width,&height,&components,loaded))==NULL) ERRORMSG();
            if (width<2 || width!=height || (components!=1 && components!=2)) ERRORMSG();

            if ((hfield=(short int *)malloc(width*height*sizeof(short int)))==NULL) ERRORMSG();

            if (components==1)
               for (k=0; k<width*height; k++) hfield[k]=image[k];
            else
               for (k=0; k<width*height; k++)
                  {
                  int v=256*image[2*k]+image[2*k+1];
                  if (v<32768) hfield[k]=v;
                  else hfield[k]=v-65536;
                  }

            if (loaded==0) free(image);

            SIZE[i+j*cols]=width;
            DIM[i+j*cols]=coldim/(width-1);

            MAP[i+j*cols]=mini::initmap(hfield,&D2MAP[i+j*cols],
                                        &SIZE[i+j*cols],&DIM[i+j*cols],scale,
                                        CELLASPECT,NULL,NULL,FAST,AVGD2);

            free(hfield);

            if (textures!=NULL)
               if (textures[i+j*cols]!=NULL)
                  {
                  if ((image=readPNMimage(textures[i+j*cols],&width,&height,&components,loaded))==NULL) ERRORMSG();
                  if (width<2 || height<2 || components!=3) ERRORMSG();

                  TEXWIDTH[i+j*cols]=width;
                  TEXHEIGHT[i+j*cols]=height;
                  TEXMIPMAPS[i+j*cols]=CONFIGURE_MIPMAPS; // mipmaps are enabled by default

                  TEXID[i+j*cols]=mini::inittexmap(image,&TEXWIDTH[i+j*cols],&TEXHEIGHT[i+j*cols],TEXMIPMAPS[i+j*cols]);
                  if (loaded==0) free(image);
                  }

            if (fogmaps!=NULL)
               if (fogmaps[i+j*cols]!=NULL)
                  {
                  if ((image=readPNMimage(fogmaps[i+j*cols],&width,&height,&components,loaded))==NULL) ERRORMSG();
                  if (width<2 || width!=height || components!=1) ERRORMSG();

                  FOGMAP[i+j*cols]=mini::initfogmap(image,width,lambda,displace,emission,attenuation,fogR,fogG,fogB,FAST);
                  if (loaded==0) free(image);
                  }
            }

         RELOADED[i+j*cols]=0;
         UPDATED[i+j*cols]=0;
         MODIFIED[i+j*cols]=0;
         }

   TEXMODE=0;
   FOGMODE=1;

   LAMBDA=lambda;
   DISPLACE=displace;
   EMISSION=emission;
   ATTENUATION=attenuation;

   FR=fogR;
   FG=fogG;
   FB=fogB;

   BEGINFAN_CALLBACK=NULL;
   FANVERTEX_CALLBACK=NULL;
   NOTIFY_CALLBACK=NULL;
   TEXMAP_CALLBACK=NULL;
   PRISMEDGE_CALLBACK=NULL;
   TRIGGER_CALLBACK=NULL;
   SYNC_CALLBACK=NULL;

   REQUEST_CALLBACK=NULL;
   PRELOAD_CALLBACK=NULL;
   REQUEST_OBJ=NULL;

   CENTERI=COLS/2.0f;
   CENTERJ=ROWS/2.0f;

   LEFT=0;
   RIGHT=COLS-1;
   BOTTOM=0;
   TOP=ROWS-1;

   COL=ROW=0;
   PHASE=-1;

   SWIZZLE=1;

   FOCUS=FALSE;

   RELSCALE0=1.0f;
   RELSCALE=1.0f;

   SEALEVEL0=-MAXFLOAT;
   SEALEVEL=-MAXFLOAT;

   MOD=FALSE;

   PFARP=0.0f;

   PLEFT=0;
   PRIGHT=COLS-1;
   PBOTTOM=0;
   PTOP=ROWS-1;

   PCOL=PROW=0;
   PUPDATE=0;

   PSWIZZLE=1;

   REDUCTION=1.0f;
   RATIO=1.0f;

   WARP=NULL;
   }

// destructor
minitile::~minitile()
   {
   int i,j;

   for (i=0; i<COLS; i++)
      for (j=0; j<ROWS; j++)
         {
         freetile(i,j,1);
         freetile2(i,j);
         }

   free(MAP);
   free(D2MAP);
   free(FOGMAP);

   free(MAP2);
   free(D2MAP2);
   free(FOGMAP2);

   free(SIZE);
   free(DIM);

   free(SIZE2);
   free(DIM2);

   free(TEXID);
   free(TEXWIDTH);
   free(TEXHEIGHT);
   free(TEXMIPMAPS);

   free(USEFLOAT);
   free(USEFLOAT2);

   free(RELOADED);
   free(UPDATED);
   free(MODIFIED);

   if (WARP!=NULL) delete WARP;
   }

// set focus of interest
void minitile::setfocus(int on,
                        float fx,float fy,float fz)
   {
   FOCUS=on;

   FX=fx;
   FY=fy;
   FZ=fz;
   }

// set relative scaling (0<=scale<=1)
void minitile::setrelscale(float scale)
   {
   if (scale<0.0f || scale>1.0f) ERRORMSG();
   RELSCALE0=scale;
   }

// set sea level
void minitile::setsealevel(float level)
   {SEALEVEL0=level;}

// set resolution reduction of invisible tiles
void minitile::setreduction(float reduction,float ratio)
   {
   if (reduction<1.0f || ratio<1.0f) ERRORMSG();

   REDUCTION=reduction;
   RATIO=ratio;
   }

// set callbacks
void minitile::setcallbacks(void (*beginfan)(),
                            void (*fanvertex)(float i,float y,float j),
                            void (*notify)(int i,int j,int s),
                            void (*texmap)(int m,int n,int S),
                            void (*prismedge)(float x,float y,float yf,float z),
                            void (*trigger)(int phase,float scale,float ex,float ey,float ez),
                            void (*sync)(int id),
                            int id)
   {
   ID=id;

   BEGINFAN_CALLBACK=beginfan;
   FANVERTEX_CALLBACK=fanvertex;
   NOTIFY_CALLBACK=notify;
   TEXMAP_CALLBACK=texmap;
   PRISMEDGE_CALLBACK=prismedge;
   TRIGGER_CALLBACK=trigger;
   SYNC_CALLBACK=sync;

   PHASE=-1;
   }

// copy warp object
void minitile::copywarp(miniwarp *warp)
   {
   if (WARP==NULL) WARP=new miniwarp(*warp);
   else *WARP=*warp;
   }

// check the visibility of the tiles
void minitile::checktiles(float ex,float ez,
                          float farp,float fovy,float aspect)
   {
   float c,d;

   // account for tilted views
   aspect=fmax(aspect,1.0f/aspect);

   // calculate visible range
   if (fovy>0.0f) c=fsqrt(1.0f+fsqr(ftan(fovy*PI/360.0f)*aspect));
   else c=fsqrt(1.0f+fsqr(fovy/2.0f*aspect/farp));

   // calculate rendering center
   CENTERI=(ex-(CENTERX-COLDIM*COLS/2.0f))/COLDIM;
   CENTERJ=(ez-(CENTERZ-ROWDIM*ROWS/2.0f))/ROWDIM;

   // calculate rendering bounding box:

   d=ffloor(((ex-c*farp)-(CENTERX-COLDIM*COLS/2.0f))/COLDIM);
   LEFT=min(max(ftrc(d)-1,0),COLS-1);

   d=ffloor(((CENTERX+COLDIM*COLS/2.0f)-(ex+c*farp))/COLDIM);
   RIGHT=COLS-1-min(max(ftrc(d)-1,0),COLS-1);

   d=ffloor(((ez-c*farp)-(CENTERZ-ROWDIM*ROWS/2.0f))/ROWDIM);
   BOTTOM=min(max(ftrc(d)-1,0),ROWS-1);

   d=ffloor(((CENTERZ+ROWDIM*ROWS/2.0f)-(ez+c*farp))/ROWDIM);
   TOP=ROWS-1-min(max(ftrc(d)-1,0),ROWS-1);

   // calculate preloading bounding box:

   d=ffloor(((ex-c*fmax(farp,PFARP))-(CENTERX-COLDIM*COLS/2.0f))/COLDIM);
   PLEFT=min(min(max(ftrc(d)-1,0),COLS-1),max(LEFT-1,0));

   d=ffloor(((CENTERX+COLDIM*COLS/2.0f)-(ex+c*fmax(farp,PFARP)))/COLDIM);
   PRIGHT=max(COLS-1-min(max(ftrc(d)-1,0),COLS-1),min(RIGHT+1,COLS-1));

   d=ffloor(((ez-c*fmax(farp,PFARP))-(CENTERZ-ROWDIM*ROWS/2.0f))/ROWDIM);
   PBOTTOM=min(min(max(ftrc(d)-1,0),ROWS-1),max(BOTTOM-1,0));

   d=ffloor(((CENTERZ+ROWDIM*ROWS/2.0f)-(ez+c*fmax(farp,PFARP)))/ROWDIM);
   PTOP=max(ROWS-1-min(max(ftrc(d)-1,0),ROWS-1),min(TOP+1,ROWS-1));

   // calculate optimal swizzle parameters
   for (SWIZZLE=13; gcd((RIGHT-LEFT+1)*(TOP-BOTTOM+1),SWIZZLE)!=1; SWIZZLE+=2);
   for (PSWIZZLE=13; gcd((PRIGHT-PLEFT+1)*(PTOP-PBOTTOM+1),PSWIZZLE)!=1; PSWIZZLE+=2);
   }

// update a reloaded tile
void minitile::updatetile(int col,int row)
   {
   if (RELOADED[col+row*COLS]!=0)
      {
      if (MAP2[col+row*COLS]!=NULL)
         {
         freetile(col,row,0);

         MAP[col+row*COLS]=MAP2[col+row*COLS];
         D2MAP[col+row*COLS]=D2MAP2[col+row*COLS];
         FOGMAP[col+row*COLS]=FOGMAP2[col+row*COLS];

         SIZE[col+row*COLS]=SIZE2[col+row*COLS];
         DIM[col+row*COLS]=DIM2[col+row*COLS];

         USEFLOAT[col+row*COLS]=USEFLOAT2[col+row*COLS];
         }
      else
         {
         freetile(col,row,1);

         MAP[col+row*COLS]=NULL;
         TEXID[col+row*COLS]=0;
         FOGMAP[col+row*COLS]=NULL;

         SIZE[col+row*COLS]=0;
         TEXWIDTH[col+row*COLS]=TEXHEIGHT[col+row*COLS]=0;
         TEXMIPMAPS[col+row*COLS]=CONFIGURE_MIPMAPS; // mipmaps are enabled by default

         USEFLOAT[col+row*COLS]=0;
         }

      MAP2[col+row*COLS]=NULL;
      FOGMAP2[col+row*COLS]=NULL;

      RELOADED[col+row*COLS]=0;
      UPDATED[col+row*COLS]=1;
      MODIFIED[col+row*COLS]=0;
      }
   }

// process one tile of the terrain
void minitile::drawtile(float res,
                        float ex,float ey,float ez,
                        float dx,float dy,float dz,
                        float ux,float uy,float uz,
                        float fovy,float aspect,
                        float nearp,float farp,
                        int col,int row,int phase,
                        int update)
   {
   float ox,oz;

   void *d2map2[4];
   int size2[4];

   float fx,fy,fz;

   if (FOCUS)
      {
      fx=FX;
      fy=FY;
      fz=FZ;
      }
   else
      {
      fx=ex;
      fy=ey;
      fz=ez;
      }

   if (phase==1)
      if (REQUEST_CALLBACK!=NULL) REQUEST_CALLBACK(col,row,1,REQUEST_OBJ);

   if (MAP[col+row*COLS]==NULL) return;

   ox=COLDIM*(col-(COLS-1)/2.0f)+CENTERX;
   oz=ROWDIM*(row-(ROWS-1)/2.0f)+CENTERZ;

   d2map2[0]=d2map2[1]=d2map2[2]=d2map2[3]=NULL;

   if (col>0) // left
      if (MAP[col-1+row*COLS]!=NULL)
         {
         d2map2[0]=D2MAP[col-1+row*COLS];
         size2[0]=SIZE[col-1+row*COLS];
         }
   if (col<COLS-1) // right
      if (MAP[col+1+row*COLS]!=NULL)
         {
         d2map2[1]=D2MAP[col+1+row*COLS];
         size2[1]=SIZE[col+1+row*COLS];
         }
   if (row<ROWS-1) // bottom
      if (MAP[col+(row+1)*COLS]!=NULL)
         {
         d2map2[2]=D2MAP[col+(row+1)*COLS];
         size2[2]=SIZE[col+(row+1)*COLS];
         }
   if (row>0) // top
      if (MAP[col+(row-1)*COLS]!=NULL)
         {
         d2map2[3]=D2MAP[col+(row-1)*COLS];
         size2[3]=SIZE[col+(row-1)*COLS];
         }

   if (phase==2 || phase==3)
      if (TEXMAP_CALLBACK!=NULL) TEXMAP_CALLBACK(col,row,SIZE[col+row*COLS]);

   if (USEFLOAT[col+row*COLS]==0)
      {
      mini::setparams(CONFIGURE_MINRES,CONFIGURE_MAXD2,CONFIGURE_SEAD2,CONFIGURE_MINOFF,CONFIGURE_MAXCULL);

      mini::setmaps(MAP[col+row*COLS],D2MAP[col+row*COLS],
                    SIZE[col+row*COLS],DIM[col+row*COLS],SCALE*RELSCALE,
                    (TEXMODE==0)?TEXID[col+row*COLS]:0,TEXWIDTH[col+row*COLS],TEXHEIGHT[col+row*COLS],TEXMIPMAPS[col+row*COLS],
                    CELLASPECT,ox,CENTERY,oz,d2map2,size2,
                    (FOGMODE!=0 && (update<=1 || PRISMEDGE_CALLBACK!=NULL))?FOGMAP[col+row*COLS]:NULL,LAMBDA,DISPLACE,
                    (FOGMODE==1)?EMISSION:0.0f,ATTENUATION,FR,FG,FB);

      mini::setsea(SEALEVEL);

      if (MODIFIED[col+row*COLS])
         {
         mini::updatemaps(FAST,AVGD2);
         MODIFIED[col+row*COLS]=0;
         }

      if (update>0)
         {
         if (phase==1)
            if (REDUCTION>1.0f)
               if (mini::checklandscape(ex,ey,ez,
                                        dx,dy,dz,
                                        ux,uy,uz,
                                        fovy,RATIO*aspect,
                                        nearp,farp)==0) res/=REDUCTION;

         fovy=0.0f;
         }

      mini::drawlandscape(res,
                          ex,ey,ez,
                          fx,fy,fz,
                          dx,dy,dz,
                          ux,uy,uz,
                          fovy,aspect,
                          nearp,farp,
                          BEGINFAN_CALLBACK,
                          FANVERTEX_CALLBACK,
                          NOTIFY_CALLBACK,
                          PRISMEDGE_CALLBACK,
                          phase);
      }
   else
      {
      Mini::setparams(CONFIGURE_MINRES,CONFIGURE_MAXD2,CONFIGURE_SEAD2,CONFIGURE_MINOFF,CONFIGURE_MAXCULL);

      Mini::setmaps(MAP[col+row*COLS],D2MAP[col+row*COLS],
                    SIZE[col+row*COLS],DIM[col+row*COLS],SCALE*RELSCALE,
                    (TEXMODE==0)?TEXID[col+row*COLS]:0,TEXWIDTH[col+row*COLS],TEXHEIGHT[col+row*COLS],TEXMIPMAPS[col+row*COLS],
                    CELLASPECT,ox,CENTERY,oz,d2map2,size2,
                    (FOGMODE!=0 && (update<=1 || PRISMEDGE_CALLBACK!=NULL))?FOGMAP[col+row*COLS]:NULL,LAMBDA,DISPLACE,
                    (FOGMODE==1)?EMISSION:0.0f,ATTENUATION,FR,FG,FB);

      Mini::setsea(SEALEVEL);

      if (MODIFIED[col+row*COLS])
         {
         Mini::updatemaps(FAST,AVGD2);
         MODIFIED[col+row*COLS]=0;
         }

      if (update>0)
         {
         if (phase==1)
            if (REDUCTION>1.0f)
               if (Mini::checklandscape(ex,ey,ez,
                                        dx,dy,dz,
                                        ux,uy,uz,
                                        fovy,RATIO*aspect,
                                        nearp,farp)==0) res/=REDUCTION;

         fovy=0.0f;
         }

      Mini::drawlandscape(res,
                          ex,ey,ez,
                          fx,fy,fz,
                          dx,dy,dz,
                          ux,uy,uz,
                          fovy,aspect,
                          nearp,farp,
                          BEGINFAN_CALLBACK,
                          FANVERTEX_CALLBACK,
                          NOTIFY_CALLBACK,
                          PRISMEDGE_CALLBACK,
                          phase);
      }
   }

// render tiled terrain
void minitile::draw(float res,
                    float ex,float ey,float ez,
                    float dx,float dy,float dz,
                    float ux,float uy,float uz,
                    float fovy,float aspect,
                    float nearp,float farp,
                    int update)
   {
   int i;

   int steps;
   int psteps;

   int col,row,
       swizzle;

   int width,height;

   if (update<0) ERRORMSG();

   if (update>0 &&
      (BEGINFAN_CALLBACK==NULL ||
       FANVERTEX_CALLBACK==NULL ||
       TRIGGER_CALLBACK==NULL)) ERRORMSG();

   if (MOD)
      {
      update=min(update,1);
      MOD=FALSE;
      }

   if (PHASE<=0 || update<=1)
      {
      checktiles(ex,ez,farp,fovy,aspect);

      for (col=PLEFT; col<=PRIGHT; col++)
         for (row=PBOTTOM; row<=PTOP; row++) updatetile(col,row);

      RELSCALE=RELSCALE0;
      SEALEVEL=SEALEVEL0;

      COL=LEFT;
      ROW=BOTTOM;
      }

   if (PRELOAD_CALLBACK!=NULL)
      {
      width=PRIGHT-PLEFT+1;
      height=PTOP-PBOTTOM+1;

      if (PUPDATE<=0) psteps=1;
      else psteps=max(width*height/max(max(PUPDATE,update),1),1);

      for (i=0; i<psteps; i++)
         {
         if (PCOL<PLEFT) PCOL=PLEFT;
         else if (PCOL>PRIGHT) PCOL=PRIGHT;

         if (PROW<PBOTTOM) PROW=PBOTTOM;
         else if (PROW>PTOP) PROW=PTOP;

         col=PCOL-PLEFT;
         row=PROW-PBOTTOM;

         swizzle=(PSWIZZLE*(col+row*width))%(width*height);

         col=swizzle%width+PLEFT;
         row=swizzle/width+PBOTTOM;

         PRELOAD_CALLBACK(col,row,REQUEST_OBJ);

         if (++PCOL>PRIGHT)
            {
            PCOL=PLEFT;
            if (++PROW>PTOP) PROW=PBOTTOM;
            }
         }
      }

   if (SYNC_CALLBACK!=NULL) SYNC_CALLBACK(ID);

   width=RIGHT-LEFT+1;
   height=TOP-BOTTOM+1;

   if (PHASE<0 || update<=1)
      {
      PHASE=0;
      if (TRIGGER_CALLBACK!=NULL) TRIGGER_CALLBACK(PHASE,0.0f,ex,ey,ez);

      if (SEALEVEL==-MAXFLOAT) steps=2*width*height;
      else steps=3*width*height;
      }
   else
      if (SEALEVEL==-MAXFLOAT) steps=(2*width*height+update-1)/update;
      else steps=(3*width*height+update-1)/update;

   if (PHASE==0)
      {
      PHASE=1;
      if (TRIGGER_CALLBACK!=NULL) TRIGGER_CALLBACK(PHASE,SCALE*RELSCALE,ex,ey,ez);
      }

   for (i=0; i<steps; i++)
      {
      col=COL-LEFT;
      row=ROW-BOTTOM;

      swizzle=(SWIZZLE*(col+row*width))%(width*height);

      col=swizzle%width+LEFT;
      row=swizzle/width+BOTTOM;

      if ((PHASE!=2 && PHASE!=3) || UPDATED[col+row*COLS]==0)
         drawtile(res,
                  ex,ey,ez,
                  dx,dy,dz,
                  ux,uy,uz,
                  fovy,aspect,
                  nearp,farp,
                  col,row,PHASE,
                  update);

      if (PHASE==1) UPDATED[col+row*COLS]=0;

      if (++COL>RIGHT)
         {
         COL=LEFT;
         if (++ROW>TOP)
            {
            ROW=BOTTOM;

            PHASE++;
            if (TRIGGER_CALLBACK!=NULL) TRIGGER_CALLBACK(PHASE,(PHASE!=4)?SCALE*RELSCALE:LAMBDA,ex,ey,ez);

            if (PHASE==3 && SEALEVEL==-MAXFLOAT)
               {
               PHASE++;
               if (TRIGGER_CALLBACK!=NULL) TRIGGER_CALLBACK(PHASE,LAMBDA,ex,ey,ez);
               }

            if (PHASE==4)
               {
               if (PRISMEDGE_CALLBACK==NULL)
                  {
                  mini::drawprismcache(ex,ey,ez,
                                       dx,dy,dz,
                                       nearp,farp,
                                       (FOGMODE==1)?EMISSION:0.0f,FR,FG,FB);

                  Mini::drawprismcache(ex,ey,ez,
                                       dx,dy,dz,
                                       nearp,farp,
                                       (FOGMODE==1)?EMISSION:0.0f,FR,FG,FB);
                  }

               PHASE=0;
               if (TRIGGER_CALLBACK!=NULL) TRIGGER_CALLBACK(PHASE,0.0f,ex,ey,ez);

               break;
               }
            }
         }
      }
   }

// loader for tile sets
// positive latitudes are transformed into negative Z-values
minitile *minitile::load(int cols,int rows,
                         const char *basepath1,const char *basepath2,const char *basepath3,
                         float offsetx,float offsety,float offseth,
                         float stretch,float stretchx,float stretchy,
                         float exaggeration,float scale,
                         void (*shader)(float nx,float ny,float nz,float elev,float *rgb),
                         float sealevel,float snowline,
                         float lambda,float attenuation,
                         float minres,float minoff,
                         float outparams[5],
                         float outscale[3])
   {
   int i,j;

   char **hmaps=NULL,**tmaps=NULL,**fmaps=NULL;

   unsigned char *image;
   int width,height,components;
   PNMcomment comment;

   float coord[8],cellsize[2],scaling;
   int utm_zone,utm_datum,missing;

   float xdim,zdim;
   float centerx,centerz;

   float as2m[2];
   float maxelev=0.0f;

   unsigned char *tmap;
   int twidth,theight,tcomps;

   minitile *terrain;

   if (cols<1 || rows<1) ERRORMSG();
   if (exaggeration<=0.0f || scale<=0.0f) ERRORMSG();

   hmaps=new char *[cols*rows];
   tmaps=new char *[cols*rows];

   if (basepath3!=NULL) fmaps=new char *[cols*rows];

   for (i=0; i<cols; i++)
      for (j=0; j<rows; j++)
         {
         hmaps[i+j*cols]=new char[MAX_STR];

         if (basepath1==NULL) snprintf(hmaps[i+j*cols],MAX_STR,"tile.%d-%d.pgm",i+1,j+1);
         else snprintf(hmaps[i+j*cols],MAX_STR,"%s%s/tile.%d-%d.pgm",CONFIGURE_TILESETPATH,basepath1,i+1,j+1);

         tmaps[i+j*cols]=new char[MAX_STR];

         if (basepath2==NULL) snprintf(tmaps[i+j*cols],MAX_STR,"tile.%d-%d.ppm",i+1,j+1);
         else snprintf(tmaps[i+j*cols],MAX_STR,"%s%s/tile.%d-%d.ppm",CONFIGURE_TILESETPATH,basepath2,i+1,j+1);

         if (basepath3!=NULL)
            {
            fmaps[i+j*cols]=new char[MAX_STR];
            snprintf(fmaps[i+j*cols],MAX_STR,"%s%s/tile.%d-%d.8.pgm",CONFIGURE_TILESETPATH,basepath3,i+1,j+1);
            }
         }

   image=NULL;

   for (i=0; i<cols && image==NULL; i++)
      for (j=0; j<rows && image==NULL; j++)
         {
         image=readPNMfile(hmaps[i+j*cols],&width,&height,&components,&comment);

         if (image!=NULL)
            {
            if (components==2) maxelev=32767.0f;
            else maxelev=255.0f;

            if (getPNMparams(&comment,
                             coord,cellsize,
                             &scaling,&missing,
                             &utm_zone,&utm_datum)==0) ERRORMSG();

            if (stretch!=1.0f)
               {
               coord[0]*=stretch;
               coord[1]*=stretch;
               coord[2]*=stretch;
               coord[3]*=stretch;
               coord[4]*=stretch;
               coord[5]*=stretch;
               coord[6]*=stretch;
               coord[7]*=stretch;
               }

            if (utm_zone!=0)
               {
               miniutm::UTM2LL(coord[0],coord[1],utm_zone,utm_datum,&coord[1],&coord[0]);
               miniutm::UTM2LL(coord[2],coord[3],utm_zone,utm_datum,&coord[3],&coord[2]);
               miniutm::UTM2LL(coord[4],coord[5],utm_zone,utm_datum,&coord[5],&coord[4]);
               miniutm::UTM2LL(coord[6],coord[7],utm_zone,utm_datum,&coord[7],&coord[6]);
               }

            offsetx-=i*(coord[6]-coord[0]);
            offsety-=j*(coord[3]-coord[1]);
            }
         }

   if (image==NULL) ERRORMSG();

   free(image);

   if (offsetx!=0.0f || offsety!=0.0f)
      {
      coord[0]=LONSUB(coord[0],-offsetx);
      coord[1]+=offsety;

      if (coord[1]<-90*60*60 || coord[1]>90*60*60) ERRORMSG();

      coord[2]=LONSUB(coord[2],-offsetx);
      coord[3]+=offsety;

      if (coord[3]<-90*60*60 || coord[3]>90*60*60) ERRORMSG();

      coord[4]=LONSUB(coord[4],-offsetx);
      coord[5]+=offsety;

      if (coord[5]<-90*60*60 || coord[5]>90*60*60) ERRORMSG();

      coord[6]=LONSUB(coord[6],-offsetx);
      coord[7]+=offsety;

      if (coord[7]<-90*60*60 || coord[7]>90*60*60) ERRORMSG();
      }

   scaling*=exaggeration;
   lambda*=exaggeration;

   xdim=fsqrt(fsqr(LONADD(coord[6],-coord[0]))+fsqr(coord[7]-coord[1]));
   zdim=fsqrt(fsqr(coord[3]-coord[1])+fsqr(LONSUB(coord[2],coord[0])));

   centerx=LONSUB(coord[2],-cols/2.0f*LONADD(coord[6],-coord[0])+rows/2.0f*LONSUB(coord[2],coord[0]));
   centerz=coord[3]-rows/2.0f*(coord[3]-coord[1])+cols/2.0f*(coord[7]-coord[1]);

   miniutm::arcsec2meter(centerz,as2m);

   as2m[0]*=stretchx;
   as2m[1]*=stretchy;

   xdim*=as2m[0];
   zdim*=as2m[1];

   centerx*=as2m[0];
   centerz*=as2m[1];

   if (shader!=NULL)
      for (i=0; i<cols; i++)
         for (j=0; j<rows; j++)
         {
         tmap=readPNMfile(tmaps[i+j*cols],&twidth,&theight,&tcomps);

         if (tmap==NULL)
            {
            twidth=2;
            while (twidth<width-1) twidth*=2;

            theight=2;
            while (theight<height-1) theight*=2;

            texturemap(hmaps[i+j*cols],
                       tmaps[i+j*cols],
                       twidth,theight,shader,
                       sealevel,snowline);
            }
         else
            {
            if (twidth<2 || theight<2 || tcomps!=3) ERRORMSG();
            free(tmap);
            }
         }

   if (basepath3==NULL)
      terrain=new minitile((unsigned char **)hmaps,
                           (unsigned char **)tmaps,
                           cols,rows,xdim/scale,zdim/scale,scaling/scale,
                           centerx/scale,offseth/scale,-centerz/scale,
                           NULL,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,
                           minres,minoff);
   else
      terrain=new minitile((unsigned char **)hmaps,
                           (unsigned char **)tmaps,
                           cols,rows,xdim/scale,zdim/scale,scaling/scale,
                           centerx/scale,offseth/scale,-centerz/scale,
                           (unsigned char **)fmaps,
                           lambda/scale,0.0f,0.0f,attenuation,0.0f,0.0f,0.0f,
                           minres,minoff);

   for (i=0; i<cols; i++)
      for (j=0; j<rows; j++)
         {
         delete[] hmaps[i+j*cols];
         delete[] tmaps[i+j*cols];

         if (basepath3!=NULL) delete[] fmaps[i+j*cols];
         }

   delete[] hmaps;
   delete[] tmaps;

   if (basepath3!=NULL) delete[] fmaps;

   // 5 output parameters
   if (outparams!=NULL)
      {
      outparams[0]=xdim/scale;
      outparams[1]=zdim/scale;

      outparams[2]=centerx/scale;
      outparams[3]=centerz/scale;

      outparams[4]=maxelev*scaling/scale;
      }

   // 3 output parameters
   if (outscale!=NULL)
      {
      outscale[0]=as2m[0]/scale;
      outscale[1]=as2m[1]/scale;
      outscale[2]=scaling/scale/exaggeration;
      }

   return(terrain);
   }

// choose actual tile from position (x,z)
int minitile::setblock(float x,float z,int approx)
   {
   int bi,bj;

   float pi,pj;

   float ox,oz;

   pi=(x-CENTERX)/COLDIM+COLS/2.0f;
   pj=(z-CENTERZ)/ROWDIM+ROWS/2.0f;

   if (pi<0.0f || pi>COLS || pj<0.0f || pj>ROWS) return(0);

   bi=ftrc(pi);
   bj=ftrc(pj);

   if (pi==COLS) bi--;
   if (pj==ROWS) bj--;

   if (MAP[bi+bj*COLS]==NULL)
      {
      if (REQUEST_CALLBACK!=NULL) REQUEST_CALLBACK(bi,bj,approx==0?0:-1,REQUEST_OBJ);
      if (MAP[bi+bj*COLS]==NULL) return(0);
      }

   ox=COLDIM*(bi-(COLS-1)/2.0f)+CENTERX;
   oz=ROWDIM*(bj-(ROWS-1)/2.0f)+CENTERZ;

   if (USEFLOAT[bi+bj*COLS]==0)
      {
      mini::setmaps(MAP[bi+bj*COLS],D2MAP[bi+bj*COLS],
                    SIZE[bi+bj*COLS],DIM[bi+bj*COLS],SCALE*RELSCALE0,
                    0,0,0,0,CELLASPECT,ox,CENTERY,oz,NULL,NULL,
                    FOGMAP[bi+bj*COLS],LAMBDA);

      return(1);
      }
   else
      {
      Mini::setmaps(MAP[bi+bj*COLS],D2MAP[bi+bj*COLS],
                    SIZE[bi+bj*COLS],DIM[bi+bj*COLS],SCALE*RELSCALE0,
                    0,0,0,0,CELLASPECT,ox,CENTERY,oz,NULL,NULL,
                    FOGMAP[bi+bj*COLS],LAMBDA);

      return(2);
      }
   }

// get-functions for geometric properties:

// calculate the elevation at position (x,z)
float minitile::getheight(float x,float z,int approx)
   {
   switch(setblock(x,z,approx))
      {
      case 1: return(mini::getheight(x,z));
      case 2: return(Mini::getheight(x,z));
      default: return(-MAXFLOAT);
      }
   }

// calculate the height of the fog layer at position (x,z)
float minitile::getfogheight(float x,float z,int approx)
   {
   switch (setblock(x,z,approx))
      {
      case 1: return(mini::getfogheight(x,z));
      case 2: return(Mini::getfogheight(x,z));
      default: return(0.0f);
      }
   }

// calculate the normal vector at position (x,z)
void minitile::getnormal(float x,float z,float *nx,float *ny,float *nz)
   {
   switch (setblock(x,z))
      {
      case 1: mini::getnormal(x,z,nx,ny,nz); break;
      case 2: Mini::getnormal(x,z,nx,ny,nz); break;
      default: *nx=*ny=*nz=0.0f;
      }
   }

// calculate the cell dimension at position (x,z)
int minitile::getdim(float x,float z,float *dimx,float *dimz,float *radius2)
   {
   int bi,bj;

   float pi,pj;

   float dx,dz;
   float bx,bz;

   pi=(x-CENTERX)/COLDIM+COLS/2.0f;
   pj=(z-CENTERZ)/ROWDIM+ROWS/2.0f;

   if (pi<0.0f || pi>COLS || pj<0.0f || pj>ROWS) return(0);

   bi=ftrc(pi);
   bj=ftrc(pj);

   if (pi==COLS) bi--;
   if (pj==ROWS) bj--;

   if (MAP[bi+bj*COLS]==NULL)
      {
      if (REQUEST_CALLBACK!=NULL) REQUEST_CALLBACK(bi,bj,0,REQUEST_OBJ);
      if (MAP[bi+bj*COLS]==NULL) return(0);
      }

   dx=DIM[bi+bj*COLS];
   dz=dx*CELLASPECT;

   *dimx=dx;
   *dimz=dz;

   if (radius2!=NULL)
      {
      bx=pi-bi;
      if (bx>0.5f) bx=1.0f-bx;

      bz=pj-bj;
      if (bz>0.5f) bz=1.0f-bz;

      *radius2=fsqr(fmax(bx*COLDIM,dx/2.0f))+fsqr(fmax(bz*ROWDIM,dz/2.0f));
      }

   return(1);
   }

// functions for grid reloading:

// check whether or not a tile has already been loaded
int minitile::isloaded(int col,int row)
   {
   if (col<0 || col>=COLS || row<0 || row>=ROWS) ERRORMSG();

   if (MAP[col+row*COLS]==NULL) return(0);
   else return(1);
   }

// check whether or not a tile is visible and has already been loaded
int minitile::isvisible(int col,int row)
   {
   if (col<0 || col>=COLS || row<0 || row>=ROWS) ERRORMSG();

   if (col<LEFT || col>RIGHT || row<BOTTOM || row>TOP) return(0);

   if (MAP[col+row*COLS]==NULL) return(0);
   else return(1);
   }

// check whether or not a tile has already been preloaded
int minitile::ispreloaded(int col,int row)
   {
   if (col<0 || col>=COLS || row<0 || row>=ROWS) ERRORMSG();

   if (col<PLEFT || col>PRIGHT || row<PBOTTOM || row>PTOP) return(0);

   if (MAP[col+row*COLS]==NULL) return(0);
   else return(1);
   }

// calculate unused LODs
int minitile::getunusedlods(int col,int row,
                            float res,float fx,float fy,float fz)
   {
   int maxsize,actsize;
   float ox,oz;

   int unusedlods;

   if (col<0 || col>=COLS || row<0 || row>=ROWS) ERRORMSG();

   if (MAP2[col+row*COLS]!=NULL)
      {
      maxsize=SIZE2[col+row*COLS];

      ox=COLDIM*(col-(COLS-1)/2.0f)+CENTERX;
      oz=ROWDIM*(row-(ROWS-1)/2.0f)+CENTERZ;

      if (USEFLOAT2[col+row*COLS]==0)
         {
         mini::setparams(CONFIGURE_MINRES,CONFIGURE_MAXD2,CONFIGURE_SEAD2,CONFIGURE_MINOFF,CONFIGURE_MAXCULL);

         mini::setmaps(MAP2[col+row*COLS],D2MAP2[col+row*COLS],
                       SIZE2[col+row*COLS],DIM2[col+row*COLS],SCALE*RELSCALE0,
                       0,0,0,0,CELLASPECT,ox,CENTERY,oz);

         actsize=mini::getmaxsize(res,fx,fy,fz);
         }
      else
         {
         Mini::setparams(CONFIGURE_MINRES,CONFIGURE_MAXD2,CONFIGURE_SEAD2,CONFIGURE_MINOFF,CONFIGURE_MAXCULL);

         Mini::setmaps(MAP2[col+row*COLS],D2MAP2[col+row*COLS],
                       SIZE2[col+row*COLS],DIM2[col+row*COLS],SCALE*RELSCALE0,
                       0,0,0,0,CELLASPECT,ox,CENTERY,oz);

         actsize=Mini::getmaxsize(res,fx,fy,fz);
         }
      }
   else if (MAP[col+row*COLS]!=NULL)
      {
      maxsize=SIZE[col+row*COLS];

      ox=COLDIM*(col-(COLS-1)/2.0f)+CENTERX;
      oz=ROWDIM*(row-(ROWS-1)/2.0f)+CENTERZ;

      if (USEFLOAT[col+row*COLS]==0)
         {
         mini::setparams(CONFIGURE_MINRES,CONFIGURE_MAXD2,CONFIGURE_SEAD2,CONFIGURE_MINOFF,CONFIGURE_MAXCULL);

         mini::setmaps(MAP[col+row*COLS],D2MAP[col+row*COLS],
                       SIZE[col+row*COLS],DIM[col+row*COLS],SCALE*RELSCALE0,
                       0,0,0,0,CELLASPECT,ox,CENTERY,oz);

         actsize=mini::getmaxsize(res,fx,fy,fz);
         }
      else
         {
         Mini::setparams(CONFIGURE_MINRES,CONFIGURE_MAXD2,CONFIGURE_SEAD2,CONFIGURE_MINOFF,CONFIGURE_MAXCULL);

         Mini::setmaps(MAP[col+row*COLS],D2MAP[col+row*COLS],
                       SIZE[col+row*COLS],DIM[col+row*COLS],SCALE*RELSCALE0,
                       0,0,0,0,CELLASPECT,ox,CENTERY,oz);

         actsize=Mini::getmaxsize(res,fx,fy,fz);
         }
      }
   else return(0);

   unusedlods=ftrc(fceil(flog((float)(maxsize-1)/max(actsize-1,2))/flog(2.0f)));

   return(unusedlods);
   }

// reload a specific tile
void minitile::reload(int col,int row,
                      unsigned char *hmap,
                      unsigned char *tmap,
                      unsigned char *fmap,
                      int updatetex,
                      int loaded)
   {
   int k;

   unsigned char *image;
   int width,height,components;

   short int *hfield;

   if (col<0 || col>=COLS || row<0 || row>=ROWS) ERRORMSG();

   freetile2(col,row);

   MAP2[col+row*COLS]=NULL;
   FOGMAP2[col+row*COLS]=NULL;

   RELOADED[col+row*COLS]=1;

   if (ispreloaded(col,row)==0) updatetile(col,row);

   if (hmap==NULL) return;

   RELOADED[col+row*COLS]=1;

   mini::setparams(CONFIGURE_MINRES,CONFIGURE_MAXD2,CONFIGURE_SEAD2,CONFIGURE_MINOFF,CONFIGURE_MAXCULL);

   mini::setsearange(CONFIGURE_SEAMIN,CONFIGURE_SEAMAX);

   if ((image=readPNMimage(hmap,&width,&height,&components,loaded))==NULL) ERRORMSG();
   if (width<2 || width!=height || (components!=1 && components!=2)) ERRORMSG();

   if ((hfield=(short int *)malloc(width*height*sizeof(short int)))==NULL) ERRORMSG();

   if (components==1)
      for (k=0; k<width*height; k++) hfield[k]=image[k];
   else
      for (k=0; k<width*height; k++)
         {
         int v=256*image[2*k]+image[2*k+1];
         if (v<32768) hfield[k]=v;
         else hfield[k]=v-65536;
         }

   if (loaded==0) free(image);

   SIZE2[col+row*COLS]=width;
   DIM2[col+row*COLS]=COLDIM/(width-1);

   MAP2[col+row*COLS]=mini::initmap(hfield,&D2MAP2[col+row*COLS],
                                    &SIZE2[col+row*COLS],&DIM2[col+row*COLS],SCALE,
                                    CELLASPECT,NULL,NULL,FAST,AVGD2);

   free(hfield);

   USEFLOAT2[col+row*COLS]=0;

   if (updatetex!=0)
      {
      mini::setmaps(MAP2[col+row*COLS],D2MAP2[col+row*COLS],
                    SIZE2[col+row*COLS],DIM2[col+row*COLS],SCALE,
                    TEXID[col+row*COLS],TEXWIDTH[col+row*COLS],TEXHEIGHT[col+row*COLS],TEXMIPMAPS[col+row*COLS],
                    CELLASPECT);

      if (tmap!=NULL)
         {
         if ((image=readPNMimage(tmap,&width,&height,&components,loaded))==NULL) ERRORMSG();
         if (width<2 || height<2 || components!=3) ERRORMSG();

         TEXWIDTH[col+row*COLS]=width;
         TEXHEIGHT[col+row*COLS]=height;
         TEXMIPMAPS[col+row*COLS]=CONFIGURE_MIPMAPS; // mipmaps are enabled by default

         TEXID[col+row*COLS]=mini::inittexmap(image,&TEXWIDTH[col+row*COLS],&TEXHEIGHT[col+row*COLS],TEXMIPMAPS[col+row*COLS]);
         if (loaded==0) free(image);
         }
      else
         {
         TEXWIDTH[col+row*COLS]=TEXHEIGHT[col+row*COLS]=0;
         TEXMIPMAPS[col+row*COLS]=CONFIGURE_MIPMAPS; // mipmaps are enabled by default

         TEXID[col+row*COLS]=mini::inittexmap(NULL,&TEXWIDTH[col+row*COLS],&TEXHEIGHT[col+row*COLS],TEXMIPMAPS[col+row*COLS]);
         }
      }

   if (fmap!=NULL)
      {
      if ((image=readPNMimage(fmap,&width,&height,&components,loaded))==NULL) ERRORMSG();
      if (width<2 || width!=height || components!=1) ERRORMSG();

      FOGMAP2[col+row*COLS]=mini::initfogmap(image,width,LAMBDA,DISPLACE,EMISSION,ATTENUATION,FR,FG,FB,FAST);
      if (loaded==0) free(image);
      }

   if (ispreloaded(col,row)==0) updatetile(col,row);

   if (MAP[col+row*COLS]==NULL) updatetile(col,row);
   }

// reload a specific tile from data buffers
void minitile::reload(int col,int row,
                      databuf &hmap,
                      databuf &tmap,
                      databuf &fmap,
                      int updatetex)
   {
   int k;

   int width,height;

   short int *hfield;

   if (col<0 || col>=COLS || row<0 || row>=ROWS) ERRORMSG();

   freetile2(col,row);

   MAP2[col+row*COLS]=NULL;
   FOGMAP2[col+row*COLS]=NULL;

   RELOADED[col+row*COLS]=1;

   if (hmap.missing()) return;

   RELOADED[col+row*COLS]=1;

   width=hmap.xsize;
   height=hmap.ysize;

   if (width<2 || height!=width) ERRORMSG();

   SIZE2[col+row*COLS]=width;
   DIM2[col+row*COLS]=COLDIM/(width-1);

   if (hmap.type!=databuf::DATABUF_TYPE_FLOAT)
      {
      mini::setparams(CONFIGURE_MINRES,CONFIGURE_MAXD2,CONFIGURE_SEAD2,CONFIGURE_MINOFF,CONFIGURE_MAXCULL);

      mini::setsearange(CONFIGURE_SEAMIN,CONFIGURE_SEAMAX);

      if (hmap.type==databuf::DATABUF_TYPE_BYTE)
         {
         if ((hfield=(short int *)malloc(width*height*sizeof(short int)))==NULL) ERRORMSG();
         for (k=0; k<width*height; k++) hfield[k]=((unsigned char *)hmap.data)[k];
         }
      else if (hmap.type==databuf::DATABUF_TYPE_SHORT) hfield=(short int *)(hmap.data);
      else ERRORMSG();

      MAP2[col+row*COLS]=mini::initmap(hfield,&D2MAP2[col+row*COLS],
                                       &SIZE2[col+row*COLS],&DIM2[col+row*COLS],SCALE,
                                       CELLASPECT,NULL,NULL,FAST,AVGD2);

      if (hmap.type==databuf::DATABUF_TYPE_BYTE) free(hfield);

      USEFLOAT2[col+row*COLS]=0;

      if (updatetex!=0)
         {
         mini::setmaps(MAP2[col+row*COLS],D2MAP2[col+row*COLS],
                       SIZE2[col+row*COLS],DIM2[col+row*COLS],SCALE,
                       TEXID[col+row*COLS],TEXWIDTH[col+row*COLS],TEXHEIGHT[col+row*COLS],TEXMIPMAPS[col+row*COLS],
                       CELLASPECT);

         if (!tmap.missing())
            {
            width=tmap.xsize;
            height=tmap.ysize;

            if (width<2 || height<2) ERRORMSG();

            TEXWIDTH[col+row*COLS]=width;
            TEXHEIGHT[col+row*COLS]=height;

            if (tmap.type==databuf::DATABUF_TYPE_RGB)
               {
               TEXMIPMAPS[col+row*COLS]=CONFIGURE_MIPMAPS; // enable mipmaps if the texture is not compressed
               TEXID[col+row*COLS]=mini::inittexmap((unsigned char *)tmap.data,&TEXWIDTH[col+row*COLS],&TEXHEIGHT[col+row*COLS],TEXMIPMAPS[col+row*COLS],0,0);
               }
            else if (tmap.type==databuf::DATABUF_TYPE_RGBA)
               {
               TEXMIPMAPS[col+row*COLS]=CONFIGURE_MIPMAPS; // enable mipmaps if the texture is not compressed
               TEXID[col+row*COLS]=mini::inittexmap((unsigned char *)tmap.data,&TEXWIDTH[col+row*COLS],&TEXHEIGHT[col+row*COLS],TEXMIPMAPS[col+row*COLS],0,1);
               }
            else if (tmap.type==databuf::DATABUF_TYPE_RGB_S3TC)
               {
               TEXMIPMAPS[col+row*COLS]=0; // disable mipmaps if the texture is compressed
               TEXID[col+row*COLS]=mini::inittexmap((unsigned char *)tmap.data,&TEXWIDTH[col+row*COLS],&TEXHEIGHT[col+row*COLS],TEXMIPMAPS[col+row*COLS],1,0,tmap.bytes);
               }
            else if (tmap.type==databuf::DATABUF_TYPE_RGBA_S3TC)
               {
               TEXMIPMAPS[col+row*COLS]=0; // disable mipmaps if the texture is compressed
               TEXID[col+row*COLS]=mini::inittexmap((unsigned char *)tmap.data,&TEXWIDTH[col+row*COLS],&TEXHEIGHT[col+row*COLS],TEXMIPMAPS[col+row*COLS],1,1,tmap.bytes);
               }
            else if (tmap.type==databuf::DATABUF_TYPE_RGB_MM)
               {
               TEXMIPMAPS[col+row*COLS]=1; // enable mipmaps
               TEXID[col+row*COLS]=mini::inittexmap((unsigned char *)tmap.data,&TEXWIDTH[col+row*COLS],&TEXHEIGHT[col+row*COLS],TEXMIPMAPS[col+row*COLS],0,0,0,1);
               }
            else if (tmap.type==databuf::DATABUF_TYPE_RGBA_MM)
               {
               TEXMIPMAPS[col+row*COLS]=1; // enable mipmaps
               TEXID[col+row*COLS]=mini::inittexmap((unsigned char *)tmap.data,&TEXWIDTH[col+row*COLS],&TEXHEIGHT[col+row*COLS],TEXMIPMAPS[col+row*COLS],0,1,0,1);
               }
            else if (tmap.type==databuf::DATABUF_TYPE_RGB_MM_S3TC)
               {
               TEXMIPMAPS[col+row*COLS]=1; // enable compressed mipmaps
               TEXID[col+row*COLS]=mini::inittexmap((unsigned char *)tmap.data,&TEXWIDTH[col+row*COLS],&TEXHEIGHT[col+row*COLS],TEXMIPMAPS[col+row*COLS],1,0,tmap.bytes,1);
               }
            else if (tmap.type==databuf::DATABUF_TYPE_RGBA_MM_S3TC)
               {
               TEXMIPMAPS[col+row*COLS]=1; // enable compressed mipmaps
               TEXID[col+row*COLS]=mini::inittexmap((unsigned char *)tmap.data,&TEXWIDTH[col+row*COLS],&TEXHEIGHT[col+row*COLS],TEXMIPMAPS[col+row*COLS],1,1,tmap.bytes,1);
               }
            else ERRORMSG();
            }
         else
            {
            TEXWIDTH[col+row*COLS]=TEXHEIGHT[col+row*COLS]=0;
            TEXMIPMAPS[col+row*COLS]=CONFIGURE_MIPMAPS; // mipmaps are enabled by default

            TEXID[col+row*COLS]=mini::inittexmap(NULL,&TEXWIDTH[col+row*COLS],&TEXHEIGHT[col+row*COLS],TEXMIPMAPS[col+row*COLS]);
            }
         }

      if (!fmap.missing())
         {
         width=fmap.xsize;
         height=fmap.ysize;

         if (width<2 || width!=height || fmap.type!=databuf::DATABUF_TYPE_BYTE) ERRORMSG();

         FOGMAP2[col+row*COLS]=mini::initfogmap((unsigned char *)fmap.data,width,LAMBDA,DISPLACE,EMISSION,ATTENUATION,FR,FG,FB,FAST);
         }
      }
   else
      {
      Mini::setparams(CONFIGURE_MINRES,CONFIGURE_MAXD2,CONFIGURE_SEAD2,CONFIGURE_MINOFF,CONFIGURE_MAXCULL);

      Mini::setsearange(CONFIGURE_SEAMIN,CONFIGURE_SEAMAX);

      MAP2[col+row*COLS]=Mini::initmap((float *)(hmap.data),&D2MAP2[col+row*COLS],
                                       &SIZE2[col+row*COLS],&DIM2[col+row*COLS],SCALE,
                                       CELLASPECT,NULL,NULL,FAST,AVGD2);

      USEFLOAT2[col+row*COLS]=1;

      if (updatetex!=0)
         {
         Mini::setmaps(MAP2[col+row*COLS],D2MAP2[col+row*COLS],
                       SIZE2[col+row*COLS],DIM2[col+row*COLS],SCALE,
                       TEXID[col+row*COLS],TEXWIDTH[col+row*COLS],TEXHEIGHT[col+row*COLS],TEXMIPMAPS[col+row*COLS],
                       CELLASPECT);

         if (!tmap.missing())
            {
            width=tmap.xsize;
            height=tmap.ysize;

            if (width<2 || height<2) ERRORMSG();

            TEXWIDTH[col+row*COLS]=width;
            TEXHEIGHT[col+row*COLS]=height;

            if (tmap.type==databuf::DATABUF_TYPE_RGB)
               {
               TEXMIPMAPS[col+row*COLS]=CONFIGURE_MIPMAPS; // enable mipmaps if the texture is not compressed
               TEXID[col+row*COLS]=Mini::inittexmap((unsigned char *)tmap.data,&TEXWIDTH[col+row*COLS],&TEXHEIGHT[col+row*COLS],TEXMIPMAPS[col+row*COLS],0,0);
               }
            else if (tmap.type==databuf::DATABUF_TYPE_RGBA)
               {
               TEXMIPMAPS[col+row*COLS]=CONFIGURE_MIPMAPS; // enable mipmaps if the texture is not compressed
               TEXID[col+row*COLS]=Mini::inittexmap((unsigned char *)tmap.data,&TEXWIDTH[col+row*COLS],&TEXHEIGHT[col+row*COLS],TEXMIPMAPS[col+row*COLS],0,1);
               }
            else if (tmap.type==databuf::DATABUF_TYPE_RGB_S3TC)
               {
               TEXMIPMAPS[col+row*COLS]=0; // disable mipmaps if the texture is compressed
               TEXID[col+row*COLS]=Mini::inittexmap((unsigned char *)tmap.data,&TEXWIDTH[col+row*COLS],&TEXHEIGHT[col+row*COLS],TEXMIPMAPS[col+row*COLS],1,0,tmap.bytes);
               }
            else if (tmap.type==databuf::DATABUF_TYPE_RGBA_S3TC)
               {
               TEXMIPMAPS[col+row*COLS]=0; // disable mipmaps if the texture is compressed
               TEXID[col+row*COLS]=Mini::inittexmap((unsigned char *)tmap.data,&TEXWIDTH[col+row*COLS],&TEXHEIGHT[col+row*COLS],TEXMIPMAPS[col+row*COLS],1,1,tmap.bytes);
               }
            else if (tmap.type==databuf::DATABUF_TYPE_RGB_MM)
               {
               TEXMIPMAPS[col+row*COLS]=1; // enable mipmaps
               TEXID[col+row*COLS]=Mini::inittexmap((unsigned char *)tmap.data,&TEXWIDTH[col+row*COLS],&TEXHEIGHT[col+row*COLS],TEXMIPMAPS[col+row*COLS],0,0,0,1);
               }
            else if (tmap.type==databuf::DATABUF_TYPE_RGBA_MM)
               {
               TEXMIPMAPS[col+row*COLS]=1; // enable mipmaps
               TEXID[col+row*COLS]=Mini::inittexmap((unsigned char *)tmap.data,&TEXWIDTH[col+row*COLS],&TEXHEIGHT[col+row*COLS],TEXMIPMAPS[col+row*COLS],0,1,0,1);
               }
            else if (tmap.type==databuf::DATABUF_TYPE_RGB_MM_S3TC)
               {
               TEXMIPMAPS[col+row*COLS]=1; // enable compressed mipmaps
               TEXID[col+row*COLS]=Mini::inittexmap((unsigned char *)tmap.data,&TEXWIDTH[col+row*COLS],&TEXHEIGHT[col+row*COLS],TEXMIPMAPS[col+row*COLS],1,0,tmap.bytes,1);
               }
            else if (tmap.type==databuf::DATABUF_TYPE_RGBA_MM_S3TC)
               {
               TEXMIPMAPS[col+row*COLS]=1; // enable compressed mipmaps
               TEXID[col+row*COLS]=Mini::inittexmap((unsigned char *)tmap.data,&TEXWIDTH[col+row*COLS],&TEXHEIGHT[col+row*COLS],TEXMIPMAPS[col+row*COLS],1,1,tmap.bytes,1);
               }
            else ERRORMSG();
            }
         else
            {
            TEXWIDTH[col+row*COLS]=TEXHEIGHT[col+row*COLS]=0;
            TEXMIPMAPS[col+row*COLS]=CONFIGURE_MIPMAPS; // mipmaps are enabled by default

            TEXID[col+row*COLS]=Mini::inittexmap(NULL,&TEXWIDTH[col+row*COLS],&TEXHEIGHT[col+row*COLS],TEXMIPMAPS[col+row*COLS]);
            }
         }

      if (!fmap.missing())
         {
         width=fmap.xsize;
         height=fmap.ysize;

         if (width<2 || width!=height || fmap.type!=databuf::DATABUF_TYPE_BYTE) ERRORMSG();

         FOGMAP2[col+row*COLS]=Mini::initfogmap((unsigned char *)fmap.data,width,LAMBDA,DISPLACE,EMISSION,ATTENUATION,FR,FG,FB,FAST);
         }
      }

   if (isloaded(col,row)==0) updatetile(col,row);
   }

// set callback for requesting visible tiles
void minitile::setrequest(void (*request)(int col,int row,int needtex,void *data),void *obj,
                          void (*preload)(int col,int row,void *data),
                          float pfarp,int pupdate)
   {
   REQUEST_CALLBACK=request;
   PRELOAD_CALLBACK=preload;
   REQUEST_OBJ=obj;

   if (preload!=NULL)
      {
      PFARP=pfarp;
      PUPDATE=pupdate;
      }
   }

// set preloading
void minitile::setpreload(float pfarp,int pupdate)
   {
   if (PRELOAD_CALLBACK!=NULL)
      {
      PFARP=pfarp;
      PUPDATE=pupdate;
      }
   }

// set fast initialization
void minitile::setfastinit(int fast,float avgd2)
   {
   FAST=fast;
   AVGD2=avgd2;
   }

// modify a heixel of the tile set at run time
void minitile::setheight(float x,float z,float h)
   {
   int i,j;

   int bi,bj;
   float pi,pj;

   int mi,mj;
   float ri,rj;

   float x1,x2,z1,z2;
   float h1,h2,h3,h4;

   int size;

   pi=(x-CENTERX)/COLDIM+COLS/2.0f;
   pj=(z-CENTERZ)/ROWDIM+ROWS/2.0f;

   if (pi<0.0f || pi>COLS || pj<0.0f || pj>ROWS) return;

   bi=ftrc(pi);
   bj=ftrc(pj);

   if (pi==COLS) bi--;
   if (pj==ROWS) bj--;

   if (MAP[bi+bj*COLS]==NULL) return;

   size=SIZE[bi+bj*COLS];

   pi-=bi;
   pj-=bj;

   pi*=size-1;
   pj*=size-1;

   mi=ftrc(pi);
   mj=ftrc(pj);

   ri=pi-mi;
   rj=pj-mj;

   if (mi==size-1)
      {
      mi=size-2;
      ri=1.0f;
      }

   if (mj==size-1)
      {
      mj=size-2;
      rj=1.0f;
      }

   if (mi==0)
      if (bi>0)
         if (MAP[bi-1+bj*COLS]!=NULL)
            if (SIZE[bi-1+bj*COLS]!=size) return;

   if (mi==size-2)
      if (bi<COLS-1)
         if (MAP[bi+1+bj*COLS]!=NULL)
            if (SIZE[bi+1+bj*COLS]!=size) return;

   if (mj==0)
      if (bj>0)
         if (MAP[bi+(bj-1)*COLS]!=NULL)
            if (SIZE[bi+(bj-1)*COLS]!=size) return;

   if (mj==size-2)
      if (bj<ROWS-1)
         if (MAP[bi+(bj+1)*COLS]!=NULL)
            if (SIZE[bi+(bj+1)*COLS]!=size) return;

   if (USEFLOAT[bi+bj*COLS]==0)
      {
      mini::setmaps(MAP[bi+bj*COLS],D2MAP[bi+bj*COLS],
                    SIZE[bi+bj*COLS],DIM[bi+bj*COLS],SCALE*RELSCALE0,
                    0,0,0,0,CELLASPECT,getcenterx(bi),CENTERY,getcenterz(bj));

      mini::setheight(x,z,h);
      }
   else
      {
      Mini::setmaps(MAP[bi+bj*COLS],D2MAP[bi+bj*COLS],
                    SIZE[bi+bj*COLS],DIM[bi+bj*COLS],SCALE*RELSCALE0,
                    0,0,0,0,CELLASPECT,getcenterx(bi),CENTERY,getcenterz(bj));

      Mini::setheight(x,z,h);
      }

   MOD=TRUE;

   MODIFIED[bi+bj*COLS]=TRUE;

   if (pi>=1 && pi<=size-2 && pj>=1 && pj<=size-2) return;

   x1=x-ri*COLDIM/(size-1);
   x2=x+(1.0f-ri)*COLDIM/(size-1);

   z1=z-rj*ROWDIM/(size-1);
   z2=z+(1.0f-rj)*ROWDIM/(size-1);

   if (USEFLOAT[bi+bj*COLS]==0)
      {
      h1=mini::getheight(mi,mj)*ri*rj+h*(1.0f-ri)*(1.0f-rj);
      h2=mini::getheight(mi+1,mj)*(1.0f-ri)*rj+h*ri*(1.0f-rj);
      h3=mini::getheight(mi,mj+1)*ri*(1.0f-rj)+h*(1.0f-ri)*rj;
      h4=mini::getheight(mi+1,mj+1)*(1.0f-ri)*(1.0f-rj)+h*ri*rj;
      }
   else
      {
      h1=Mini::getheight(mi,mj)*ri*rj+h*(1.0f-ri)*(1.0f-rj);
      h2=Mini::getheight(mi+1,mj)*(1.0f-ri)*rj+h*ri*(1.0f-rj);
      h3=Mini::getheight(mi,mj+1)*ri*(1.0f-rj)+h*(1.0f-ri)*rj;
      h4=Mini::getheight(mi+1,mj+1)*(1.0f-ri)*(1.0f-rj)+h*ri*rj;
      }

   for (i=bi-1; i<=bi+1; i++)
      for (j=bj-1; j<=bj+1; j++)
         if (i>=0 && i<COLS && j>=0 && j<ROWS)
            if (i!=bi || j!=bj)
               {
               if (MAP[i+j*COLS]==NULL) continue;

               if (USEFLOAT[i+j*COLS]==0)
                  {
                  mini::setmaps(MAP[i+j*COLS],D2MAP[i+j*COLS],
                                SIZE[i+j*COLS],DIM[i+j*COLS],SCALE*RELSCALE0,
                                0,0,0,0,CELLASPECT,getcenterx(i),CENTERY,getcenterz(j));

                  if (mini::setheight(x1,z1,h1)!=0) MODIFIED[i+j*COLS]=TRUE;
                  if (mini::setheight(x2,z1,h2)!=0) MODIFIED[i+j*COLS]=TRUE;
                  if (mini::setheight(x1,z2,h3)!=0) MODIFIED[i+j*COLS]=TRUE;
                  if (mini::setheight(x2,z2,h4)!=0) MODIFIED[i+j*COLS]=TRUE;
                  }
               else
                  {
                  Mini::setmaps(MAP[i+j*COLS],D2MAP[i+j*COLS],
                                SIZE[i+j*COLS],DIM[i+j*COLS],SCALE*RELSCALE0,
                                0,0,0,0,CELLASPECT,getcenterx(i),CENTERY,getcenterz(j));

                  if (Mini::setheight(x1,z1,h1)!=0) MODIFIED[i+j*COLS]=TRUE;
                  if (Mini::setheight(x2,z1,h2)!=0) MODIFIED[i+j*COLS]=TRUE;
                  if (Mini::setheight(x1,z2,h3)!=0) MODIFIED[i+j*COLS]=TRUE;
                  if (Mini::setheight(x2,z2,h4)!=0) MODIFIED[i+j*COLS]=TRUE;
                  }
               }
   }

// set the real height of a heixel at run time
void minitile::setrealheight(float x,float z,float h)
   {
   if (SCALE*RELSCALE0==0.0f) ERRORMSG();
   setheight(x,z,(h-CENTERY)/(SCALE*RELSCALE0));
   }

// free all maps of a tile and the texture map if requested
void minitile::freetile(int col,int row,int freetex)
   {
   if (MAP[col+row*COLS]!=NULL)
      if (USEFLOAT[col+row*COLS]==0)
         {
         if (freetex!=0)
            mini::setmaps(MAP[col+row*COLS],D2MAP[col+row*COLS],
                          SIZE[col+row*COLS],DIM[col+row*COLS],SCALE,
                          TEXID[col+row*COLS],TEXWIDTH[col+row*COLS],TEXHEIGHT[col+row*COLS],TEXMIPMAPS[col+row*COLS],
                          CELLASPECT,0.0f,0.0f,0.0f,NULL,NULL,
                          FOGMAP[col+row*COLS]);
         else
            mini::setmaps(MAP[col+row*COLS],D2MAP[col+row*COLS],
                          SIZE[col+row*COLS],DIM[col+row*COLS],SCALE,
                          0,0,0,0,CELLASPECT,0.0f,0.0f,0.0f,NULL,NULL,
                          FOGMAP[col+row*COLS]);

         mini::deletemaps();
         }
      else
         {
         if (freetex!=0)
            Mini::setmaps(MAP[col+row*COLS],D2MAP[col+row*COLS],
                          SIZE[col+row*COLS],DIM[col+row*COLS],SCALE,
                          TEXID[col+row*COLS],TEXWIDTH[col+row*COLS],TEXHEIGHT[col+row*COLS],TEXMIPMAPS[col+row*COLS],
                          CELLASPECT,0.0f,0.0f,0.0f,NULL,NULL,
                          FOGMAP[col+row*COLS]);
         else
            Mini::setmaps(MAP[col+row*COLS],D2MAP[col+row*COLS],
                          SIZE[col+row*COLS],DIM[col+row*COLS],SCALE,
                          0,0,0,0,CELLASPECT,0.0f,0.0f,0.0f,NULL,NULL,
                          FOGMAP[col+row*COLS]);

         Mini::deletemaps();
         }
   }

// free all temporary maps of a tile
void minitile::freetile2(int col,int row)
   {
   if (MAP2[col+row*COLS]!=NULL)
      if (USEFLOAT2[col+row*COLS]==0)
         {
         mini::setmaps(MAP2[col+row*COLS],D2MAP2[col+row*COLS],
                       SIZE2[col+row*COLS],DIM2[col+row*COLS],SCALE,
                       0,0,0,0,CELLASPECT,0.0f,0.0f,0.0f,NULL,NULL,
                       FOGMAP2[col+row*COLS]);

         mini::deletemaps();
         }
      else
         {
         Mini::setmaps(MAP2[col+row*COLS],D2MAP2[col+row*COLS],
                       SIZE2[col+row*COLS],DIM2[col+row*COLS],SCALE,
                       0,0,0,0,CELLASPECT,0.0f,0.0f,0.0f,NULL,NULL,
                       FOGMAP2[col+row*COLS]);

         Mini::deletemaps();
         }
   }

// configure core parameters:

void minitile::configure_minres(float minres)
   {
   if (minres<1.0f) minres=9.0f;
   CONFIGURE_MINRES=minres;
   }

void minitile::configure_maxd2(float maxd2)
   {
   if (maxd2<=0.0f) maxd2=100.0f;
   CONFIGURE_MAXD2=maxd2;
   }

void minitile::configure_sead2(float sead2)
   {
   if (sead2<0.0f) sead2=0.1f;
   CONFIGURE_SEAD2=sead2;
   }

void minitile::configure_minoff(float minoff)
   {
   if (minoff<0.0f) minoff=0.1f;
   CONFIGURE_MINOFF=minoff;
   }

void minitile::configure_maxcull(int maxcull)
   {
   if (maxcull<0) maxcull=8;
   CONFIGURE_MAXCULL=maxcull;
   }

// configuring:

void minitile::configure_seamin(float seamin)
   {CONFIGURE_SEAMIN=seamin;}

void minitile::configure_seamax(float seamax)
   {CONFIGURE_SEAMAX=seamax;}

void minitile::configure_mipmaps(int mipmaps)
   {CONFIGURE_MIPMAPS=mipmaps;}

// configuring the loader:

void minitile::configure_tilesetpath(char *tilesetpath)
   {strncpy(CONFIGURE_TILESETPATH,tilesetpath,MAX_STR);}
