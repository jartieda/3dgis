// (c) by Stefan Roettger

#include "minibase.h"

#include "pnmbase.h"

#include "minitree.h"

// default constructor
minitree::minitree(minicache *cache,minitile *tile)
   {
   CACHE=cache;
   ATTACHED_ID=tile->getid();

   TREEMODE=0;

   PRISM_ID=0;

   TREECACHE_NUM=1;

   TREECACHE_SIZE1=TREECACHE_SIZE2=0;
   TREECACHE_MAXSIZE1=TREECACHE_MAXSIZE2=1;

   if ((TREECACHE_CACHE1=(float *)malloc(TREECACHE_MAXSIZE1*3*sizeof(float)))==NULL) ERRORMSG();
   if ((TREECACHE_CACHE2=(float *)malloc(TREECACHE_MAXSIZE2*3*sizeof(float)))==NULL) ERRORMSG();

   if ((TREECACHE_COORD1=(float *)malloc(TREECACHE_MAXSIZE1*3*sizeof(float)))==NULL) ERRORMSG();
   if ((TREECACHE_COORD2=(float *)malloc(TREECACHE_MAXSIZE2*3*sizeof(float)))==NULL) ERRORMSG();

   GRASSCACHE_SIZE1=GRASSCACHE_SIZE2=0;
   GRASSCACHE_MAXSIZE1=GRASSCACHE_MAXSIZE2=1;

   if ((GRASSCACHE_CACHE1=(float *)malloc(GRASSCACHE_MAXSIZE1*3*sizeof(float)))==NULL) ERRORMSG();
   if ((GRASSCACHE_CACHE2=(float *)malloc(GRASSCACHE_MAXSIZE2*3*sizeof(float)))==NULL) ERRORMSG();

   if ((GRASSCACHE_COORD1=(float *)malloc(GRASSCACHE_MAXSIZE1*3*sizeof(float)))==NULL) ERRORMSG();
   if ((GRASSCACHE_COORD2=(float *)malloc(GRASSCACHE_MAXSIZE2*3*sizeof(float)))==NULL) ERRORMSG();

   TREECACHE_TREES1=TREECACHE_TREES2=0;

   TREECACHE_LAMBDA=0.0f;

   TREECACHE_TEXID=GRASSCACHE_TEXID=GRASSCACHE_PERTURBID=RENDERCACHE_TEXID=0;

   TREECACHE_VTXPROGID1=TREECACHE_VTXPROGID2=RENDERCACHE_VTXPROGID=0;
   GRASSCACHE_VTXPROGID=GRASSCACHE_FRAGPROGID1=GRASSCACHE_FRAGPROGID2=GRASSCACHE_FRAGPROGID3=GRASSCACHE_FRAGPROGID4=0;
   RENDERCACHE_FRAGPROGID1=RENDERCACHE_FRAGPROGID2=0;

   CONFIGURE_ZSCALE=0.95f;
   CONFIGURE_BLEND=0;

   WGLSETUP=0;

   // initialize parameters for negative modes:

   TREEMODE_MX_BASE=0.5f;
   TREEMODE_MX_TR=0.0f;
   TREEMODE_MX_TG=0.25f;
   TREEMODE_MX_TB=0.05f;
   TREEMODE_MX_TA=0.9f;

   // initialize parameters for mode -2:

   TREEMODE_M2_TEXFILE=NULL;
   TREEMODE_M2_TEXSIZE=250.0f;

   // initialize parameters for positive modes:

   TREEMODE_X_RES=50.0f;
   TREEMODE_X_SQRFACTOR=5.0f;
   TREEMODE_X_MAXLEVEL=4;
   TREEMODE_X_MINSPACE=50.0f; // must be lower than or equal to grid spacing in order to prevent excessive popping
   TREEMODE_X_MINHEIGHT=75.0f;
   TREEMODE_X_TREEWIDTH=70.0f;
   TREEMODE_X_TREEVARIANCE=60.0f;
   TREEMODE_X_TR=0.0f;
   TREEMODE_X_TG=0.25f;
   TREEMODE_X_TB=0.05f;

   // initialize parameters for mode 3:

   TREEMODE_3_COLFLUCT=0.1f;

   // initialize parameters for mode 4+:

   TREEMODE_4_TEXFILE_RGB=NULL;
   TREEMODE_4_TEXFILE_A=NULL;
   TREEMODE_4_TREEASPECT=0.0f;
   TREEMODE_4_MINALPHA=0.5f;

   // initialize parameters for mode 6+:

   TREEMODE_6_SHRUBFRAC=0.4f;
   TREEMODE_6_SHRUBPROB=0.7f;

   // initialize parameters for mode 7+:

   TREEMODE_7_TEXFILE_RGB=NULL;
   TREEMODE_7_TEXFILE_A=NULL;
   TREEMODE_7_TREENUM=4;

   // initialize parameters for mode 8+:

   TREEMODE_8_TREESTEP=10.0f;
   TREEMODE_8_TREERAND=0.25f;

   // initialize parameters for mode 9+:

   TREEMODE_9_VOLFILE=NULL;
   TREEMODE_9_TEXSIZE=30.0f;
   TREEMODE_9_GRASSDEPTH=3.0f;
   TREEMODE_9_MINHEIGHT=0.005f;
   TREEMODE_9_BASEHEIGHT=0.01f;
   TREEMODE_9_MAXHEIGHT=1.25f;
   TREEMODE_9_TREESTART=0.75f;
   TREEMODE_9_MIPMAPRANGE=15.0f;
   TREEMODE_9_MIPMAPLEVELS=8;
   TREEMODE_9_GRASSALPHA=1.0f;

   // initialize parameters for mode 10+:

   TREEMODE_10_PERTURBSIZE=250.0f;
   TREEMODE_10_PERTURBRES=1024;
   TREEMODE_10_PERTURBSTART=16;
   TREEMODE_10_PERTURBPERS=0.5f;
   TREEMODE_10_PERTURBSEED=0.0f;
   TREEMODE_10_PERTURBFX=0.25f;

   // initialize parameters for mode 12:

   TREEMODE_12_REDWGT=0.3f;
   TREEMODE_12_GREENWGT=0.6f;
   TREEMODE_12_BLUEWGT=0.1f;
   TREEMODE_12_ALPHATHRES=0.25f;
   TREEMODE_12_ALPHASLOPE=1.0f;
   }

// destructor
minitree::~minitree()
   {
   free(TREECACHE_CACHE1);
   free(TREECACHE_CACHE2);

   free(TREECACHE_COORD1);
   free(TREECACHE_COORD2);

   free(GRASSCACHE_CACHE1);
   free(GRASSCACHE_CACHE2);

   free(GRASSCACHE_COORD1);
   free(GRASSCACHE_COORD2);

   deletetexmap(TREECACHE_TEXID);
   deletetexmap(GRASSCACHE_TEXID);
   deletetexmap(GRASSCACHE_PERTURBID);
   deletetexmap(RENDERCACHE_TEXID);

#ifndef NOOGL

#if defined(GL_ARB_vertex_program) && defined(GL_ARB_fragment_program)

   GLuint progid;

   if (TREECACHE_VTXPROGID1!=0)
      {
      progid=TREECACHE_VTXPROGID1;
      glDeleteProgramsARB(1,&progid);
      }

   if (TREECACHE_VTXPROGID2!=0)
      {
      progid=TREECACHE_VTXPROGID2;
      glDeleteProgramsARB(1,&progid);
      }

   if (GRASSCACHE_VTXPROGID!=0)
      {
      progid=GRASSCACHE_VTXPROGID;
      glDeleteProgramsARB(1,&progid);
      }

   if (GRASSCACHE_FRAGPROGID1!=0)
      {
      progid=GRASSCACHE_FRAGPROGID1;
      glDeleteProgramsARB(1,&progid);
      }

   if (GRASSCACHE_FRAGPROGID2!=0)
      {
      progid=GRASSCACHE_FRAGPROGID2;
      glDeleteProgramsARB(1,&progid);
      }

   if (GRASSCACHE_FRAGPROGID3!=0)
      {
      progid=GRASSCACHE_FRAGPROGID3;
      glDeleteProgramsARB(1,&progid);
      }

   if (GRASSCACHE_FRAGPROGID4!=0)
      {
      progid=GRASSCACHE_FRAGPROGID4;
      glDeleteProgramsARB(1,&progid);
      }

   if (RENDERCACHE_VTXPROGID!=0)
      {
      progid=RENDERCACHE_VTXPROGID;
      glDeleteProgramsARB(1,&progid);
      }

   if (RENDERCACHE_FRAGPROGID1!=0)
      {
      progid=RENDERCACHE_FRAGPROGID1;
      glDeleteProgramsARB(1,&progid);
      }

   if (RENDERCACHE_FRAGPROGID2!=0)
      {
      progid=RENDERCACHE_FRAGPROGID2;
      glDeleteProgramsARB(1,&progid);
      }

#endif

#endif

   if (TREEMODE_M2_TEXFILE!=NULL) free(TREEMODE_M2_TEXFILE);

   if (TREEMODE_4_TEXFILE_RGB!=NULL) free(TREEMODE_4_TEXFILE_RGB);
   if (TREEMODE_4_TEXFILE_A!=NULL) free(TREEMODE_4_TEXFILE_A);

   if (TREEMODE_7_TEXFILE_RGB!=NULL) free(TREEMODE_7_TEXFILE_RGB);
   if (TREEMODE_7_TEXFILE_A!=NULL) free(TREEMODE_7_TEXFILE_A);

   if (TREEMODE_9_VOLFILE!=NULL) free(TREEMODE_9_VOLFILE);
   }

// configuring
void minitree::configure_zfight(float zscale) {CONFIGURE_ZSCALE=zscale;}
void minitree::configure_blend(int blend) {CONFIGURE_BLEND=blend;}

// set tree render mode
// -2: transparent textured vegetation surface
// -1: transparent vegetation surface
// 0: transparent surface
// 1: lines
// 2: flat triangles
// 3: pseudo-lit triangles
// 4: textured quads
// 5: textured billboards
// 6: textured billboards with random thinning
// 7: randomized billboards
// 8: randomized billboards with height-based selection
// 9: randomized billboards with grass layer
// 10: randomized billboards with perturbed grass layer
// 11: randomized billboards with interpolated grass layer
// 12: randomized billboards with translucent grass layer
void minitree::setmode(int treemode)
   {
   if (treemode<-2 || treemode>12) ERRORMSG();

   TREEMODE=treemode;

   TREECACHE_SIZE1=TREECACHE_SIZE2=0;
   GRASSCACHE_SIZE1=GRASSCACHE_SIZE2=0;
   TREECACHE_TREES1=TREECACHE_TREES2=0;

   deletetexmap(TREECACHE_TEXID);
   TREECACHE_TEXID=0;

   deletetexmap(GRASSCACHE_TEXID);
   GRASSCACHE_TEXID=0;

   deletetexmap(GRASSCACHE_PERTURBID);
   GRASSCACHE_PERTURBID=0;

   deletetexmap(RENDERCACHE_TEXID);
   RENDERCACHE_TEXID=0;

   if (TREEMODE<0) CACHE->attach(NULL,NULL,NULL,prismrender,NULL,NULL,this);
   else if (TREEMODE>0) CACHE->attach(NULL,prismedge,prismcache,NULL,prismtrigger,prismsync,this);
   else CACHE->attach(NULL,NULL,NULL,NULL,NULL,NULL,NULL);
   }

// set parameters for negative modes
void minitree::setmode_mx(float base,float tr,float tg,float tb,float ta)
   {
   if (base<0.0f || base>1.0f) ERRORMSG();

   TREEMODE_MX_BASE=base;
   TREEMODE_MX_TR=tr;
   TREEMODE_MX_TG=tg;
   TREEMODE_MX_TB=tb;
   TREEMODE_MX_TA=ta;
   }

// set parameters for mode -2
void minitree::setmode_m2(char *texfile,float texsize)
   {
   if (texfile==NULL || texsize<=0.0f) ERRORMSG();

   if (TREEMODE_M2_TEXFILE!=NULL) free(TREEMODE_M2_TEXFILE);
   TREEMODE_M2_TEXFILE=strdup(texfile);

   deletetexmap(RENDERCACHE_TEXID);
   RENDERCACHE_TEXID=0;

   TREEMODE_M2_TEXSIZE=texsize;
   }

// set parameters for positive modes
void minitree::setmode_x(float res,float sqrfactor,int maxlevel,float minspace,float minheight,
                         float treewidth,float treevariance,float tr,float tg,float tb)
   {
   if (res<=0.0f) ERRORMSG();
   if (sqrfactor<=0.0f) ERRORMSG();

   TREEMODE_X_RES=res;
   TREEMODE_X_SQRFACTOR=sqrfactor;
   TREEMODE_X_MAXLEVEL=maxlevel;
   TREEMODE_X_MINSPACE=minspace;
   TREEMODE_X_MINHEIGHT=minheight;
   TREEMODE_X_TREEWIDTH=treewidth;
   TREEMODE_X_TREEVARIANCE=treevariance;
   TREEMODE_X_TR=tr;
   TREEMODE_X_TG=tg;
   TREEMODE_X_TB=tb;
   }

// set parameters for mode 3
void minitree::setmode_3(float colfluct)
   {
   if (colfluct<0.0f || colfluct>1.0f) ERRORMSG();
   TREEMODE_3_COLFLUCT=colfluct;
   }

// set parameters for mode 4+
void minitree::setmode_4(char *texfile_rgb,char *texfile_a,float treeaspect,float minalpha)
   {
   if (texfile_rgb==NULL) ERRORMSG();
   if (treeaspect<0.0f) ERRORMSG();

   if (TREEMODE_4_TEXFILE_RGB!=NULL) free(TREEMODE_4_TEXFILE_RGB);
   TREEMODE_4_TEXFILE_RGB=strdup(texfile_rgb);
   if (TREEMODE_4_TEXFILE_A!=NULL) free(TREEMODE_4_TEXFILE_A);
   if (texfile_a!=NULL) TREEMODE_4_TEXFILE_A=strdup(texfile_a);
   else TREEMODE_4_TEXFILE_A=NULL;

   deletetexmap(TREECACHE_TEXID);
   TREECACHE_TEXID=0;

   TREEMODE_4_TREEASPECT=treeaspect;
   TREEMODE_4_MINALPHA=minalpha;
   }

// set parameters for mode 6+
void minitree::setmode_6(float shrubfrac,float shrubprob)
   {
   if (shrubfrac<0.0f || shrubfrac>1.0f) ERRORMSG();
   if (shrubprob<0.0f || shrubprob>1.0f) ERRORMSG();

   TREEMODE_6_SHRUBFRAC=shrubfrac;
   TREEMODE_6_SHRUBPROB=shrubprob;
   }

// set parameters for mode 7+
void minitree::setmode_7(char *texfile_rgb,char *texfile_a,int treenum)
   {
   if (texfile_rgb==NULL) ERRORMSG();
   if (treenum<=0) ERRORMSG();

   if (TREEMODE_7_TEXFILE_RGB!=NULL) free(TREEMODE_7_TEXFILE_RGB);
   TREEMODE_7_TEXFILE_RGB=strdup(texfile_rgb);
   if (TREEMODE_7_TEXFILE_A!=NULL) free(TREEMODE_7_TEXFILE_A);
   if (texfile_a!=NULL) TREEMODE_7_TEXFILE_A=strdup(texfile_a);
   else TREEMODE_7_TEXFILE_A=NULL;

   deletetexmap(TREECACHE_TEXID);
   TREECACHE_TEXID=0;

   TREEMODE_7_TREENUM=treenum;
   }

// set parameters for mode 8+
void minitree::setmode_8(float treestep,float treerand)
   {
   if (treestep<=0.0f) ERRORMSG();
   if (treerand<0.0f || treerand>1.0f) ERRORMSG();

   TREEMODE_8_TREESTEP=treestep;
   TREEMODE_8_TREERAND=treerand;
   }

// set parameters for mode 9+
void minitree::setmode_9(char *volfile,float texsize,
                         float grassdepth,float minheight,float baseheight,float maxheight,
                         float treestart,float mipmaprange,int mipmaplevels,float grassalpha)
   {
   if (volfile==NULL ||
       texsize<=0.0f || grassdepth<0.0f ||
       minheight<0.0f || minheight>=1.0f ||
       baseheight<=minheight || maxheight<=1.0f ||
       treestart<0.0f || treestart>1.0f ||
       mipmaprange<=0.0f || mipmaplevels<1 ||
       grassalpha<0.0f || grassalpha>1.0f) ERRORMSG();

   if (TREEMODE_9_VOLFILE!=NULL) free(TREEMODE_9_VOLFILE);
   TREEMODE_9_VOLFILE=strdup(volfile);

   deletetexmap(GRASSCACHE_TEXID);
   GRASSCACHE_TEXID=0;

   TREEMODE_9_TEXSIZE=texsize;
   TREEMODE_9_GRASSDEPTH=grassdepth;
   TREEMODE_9_MINHEIGHT=minheight*TREEMODE_X_MINHEIGHT;
   TREEMODE_9_BASEHEIGHT=baseheight*TREEMODE_X_MINHEIGHT;
   TREEMODE_9_MAXHEIGHT=maxheight*TREEMODE_X_MINHEIGHT;
   TREEMODE_9_TREESTART=treestart;
   TREEMODE_9_MIPMAPRANGE=mipmaprange;
   TREEMODE_9_MIPMAPLEVELS=mipmaplevels;
   TREEMODE_9_GRASSALPHA=grassalpha;
   }

// set parameters for mode 10+
void minitree::setmode_10(float perturbsize,int perturbres,int perturbstart,float perturbpers,float perturbseed,float perturbfx)
   {
   if (perturbsize<=0.0f ||
       perturbres<2 || perturbstart<2 || perturbstart>perturbres ||
       perturbpers<0.0f || perturbpers>1.0f ||
       perturbseed<0.0f || perturbseed>1.0f ||
       perturbfx<0.0f || perturbfx>1.0f) ERRORMSG();

   deletetexmap(GRASSCACHE_PERTURBID);
   GRASSCACHE_PERTURBID=0;

   TREEMODE_10_PERTURBSIZE=perturbsize;
   TREEMODE_10_PERTURBRES=perturbres;
   TREEMODE_10_PERTURBSTART=perturbstart;
   TREEMODE_10_PERTURBPERS=perturbpers;
   TREEMODE_10_PERTURBSEED=perturbseed;
   TREEMODE_10_PERTURBFX=perturbfx;
   }

// set parameters for mode 12
void minitree::setmode_12(float redwgt,float greenwgt,float bluewgt,float alphathres,float alphaslope)
   {
   float wgtsum;

   if (redwgt<=0.0f || greenwgt<=0.0f || bluewgt<=0.0f ||
       alphathres<0.0f || alphathres>1.0f || alphaslope<1.0f) ERRORMSG();

   wgtsum=redwgt+greenwgt+bluewgt;

   TREEMODE_12_REDWGT=redwgt/wgtsum;
   TREEMODE_12_GREENWGT=greenwgt/wgtsum;
   TREEMODE_12_BLUEWGT=bluewgt/wgtsum;

   TREEMODE_12_ALPHATHRES=alphathres;
   TREEMODE_12_ALPHASLOPE=alphaslope;
   }

// render callbacks:

void minitree::prismedge(float x,float y,float yf,float z,void *data)
   {
   minitree *tree=(minitree *)data;
   tree->treeedge(x,y,yf,z);
   }

void minitree::prismcache(int phase,float scale,float ex,float ey,float ez,void *data)
   {
   minitree *tree=(minitree *)data;
   tree->treecache(phase,scale,ex,ey,ez);
   }

int minitree::prismrender(float *cache,int cnt,float lambda,miniwarp *warp,void *data)
   {
   minitree *tree=(minitree *)data;
   return(tree->renderprisms(cache,cnt,lambda,warp,tree->TREEMODE_MX_TR,tree->TREEMODE_MX_TG,tree->TREEMODE_MX_TB,tree->TREEMODE_MX_TA));
   }

int minitree::prismtrigger(int phase,void *data)
   {
   minitree *tree=(minitree *)data;
   return(tree->treetrigger(phase));
   }

void minitree::prismsync(int id,void *data)
   {
   minitree *tree=(minitree *)data;
   tree->treesync(id);
   }

// process tree prism edges
void minitree::treeedge(float x,float y,float yf,float z)
   {
   if (PRISM_ID==ATTACHED_ID)
      switch (TREECACHE_COUNT++%3)
         {
         case 0:
            TREECACHE_X1=x,TREECACHE_Y1=y,TREECACHE_Z1=z,TREECACHE_H1=yf;
            break;
         case 1:
            TREECACHE_X2=x,TREECACHE_Y2=y,TREECACHE_Z2=z,TREECACHE_H2=yf;
            break;
         case 2:
            treedata(TREECACHE_X1,TREECACHE_Y1,TREECACHE_Z1,TREECACHE_H1,
                     TREECACHE_X2,TREECACHE_Y2,TREECACHE_Z2,TREECACHE_H2,
                     x,y,z,yf);
            break;
         }
   }

// switch tree cache buffers
void minitree::treecache(int phase,float scale,float ex,float ey,float ez)
   {
   if (PRISM_ID==ATTACHED_ID)
      {
      if (phase==0)
         {
         if (TREECACHE_NUM==1)
            {
            TREECACHE_SIZE2=0;
            GRASSCACHE_SIZE2=0;
            TREECACHE_TREES2=0;
            TREECACHE_NUM=2;
            }
         else
            {
            TREECACHE_SIZE1=0;
            GRASSCACHE_SIZE1=0;
            TREECACHE_TREES1=0;
            TREECACHE_NUM=1;
            }

         TREECACHE_COUNT=0;

         TREECACHE_EX=ex;
         TREECACHE_EY=ey;
         TREECACHE_EZ=ez;
         }

      if (phase==4) TREECACHE_LAMBDA=scale;
      }
   }

// render tree cache
int minitree::treetrigger(int phase)
   {
   int vtx=0;

   if (PRISM_ID==ATTACHED_ID)
      if (phase==4)
         {
         if (TREECACHE_NUM==1) vtx+=rendertrees(TREECACHE_CACHE2,TREECACHE_COORD2,TREECACHE_SIZE2,CACHE->gettile(PRISM_ID)->getwarp(),TREEMODE_X_TR,TREEMODE_X_TG,TREEMODE_X_TB);
         else vtx+=rendertrees(TREECACHE_CACHE1,TREECACHE_COORD1,TREECACHE_SIZE1,CACHE->gettile(PRISM_ID)->getwarp(),TREEMODE_X_TR,TREEMODE_X_TG,TREEMODE_X_TB);

         if (TREEMODE>=9)
            if (TREECACHE_NUM==1) vtx+=rendergrass(GRASSCACHE_CACHE2,GRASSCACHE_COORD2,GRASSCACHE_SIZE2,CACHE->gettile(PRISM_ID)->getwarp());
            else vtx+=rendergrass(GRASSCACHE_CACHE1,GRASSCACHE_COORD1,GRASSCACHE_SIZE1,CACHE->gettile(PRISM_ID)->getwarp());
         }

   return(vtx);
   }

// receive actual terrain id
void minitree::treesync(int id)
   {PRISM_ID=id;}

// process tree data
void minitree::treedata(float x1,float y1,float z1,float h1,
                        float x2,float y2,float z2,float h2,
                        float x3,float y3,float z3,float h3,
                        int level)
   {
   float f;
   float dx,dy,dz;

   float x12,y12,z12,h12;
   float x23,y23,z23,h23;
   float x31,y31,z31,h31;

   float x,y,z,h,b,d,w,t,r;
   float g1,g2,g3,t1,t2,t3;

   float rand;
   int choice;

   h=fmax(fmax(h1,h2),h3);

   if (TREEMODE>=9 && level==0)
      if (h>=TREEMODE_9_MINHEIGHT)
         {
         g1=fmin(fmax((h1-TREEMODE_9_MINHEIGHT)/(TREEMODE_9_BASEHEIGHT-TREEMODE_9_MINHEIGHT),0.0f),1.0f);
         g2=fmin(fmax((h2-TREEMODE_9_MINHEIGHT)/(TREEMODE_9_BASEHEIGHT-TREEMODE_9_MINHEIGHT),0.0f),1.0f);
         g3=fmin(fmax((h3-TREEMODE_9_MINHEIGHT)/(TREEMODE_9_BASEHEIGHT-TREEMODE_9_MINHEIGHT),0.0f),1.0f);

         if (h1<TREEMODE_X_MINHEIGHT)
            t1=fmin(TREEMODE_9_TREESTART*fmax((h1-TREEMODE_9_MINHEIGHT)/(TREEMODE_X_MINHEIGHT-TREEMODE_9_MINHEIGHT),0.0f),1.0f);
         else
            t1=TREEMODE_9_TREESTART+fmin((1.0f-TREEMODE_9_TREESTART)*fmax((h1-TREEMODE_X_MINHEIGHT)/(TREEMODE_9_MAXHEIGHT-TREEMODE_X_MINHEIGHT),0.0f),1.0f);

         if (h2<TREEMODE_X_MINHEIGHT)
            t2=fmin(TREEMODE_9_TREESTART*fmax((h2-TREEMODE_9_MINHEIGHT)/(TREEMODE_X_MINHEIGHT-TREEMODE_9_MINHEIGHT),0.0f),1.0f);
         else
            t2=TREEMODE_9_TREESTART+fmin((1.0f-TREEMODE_9_TREESTART)*fmax((h2-TREEMODE_X_MINHEIGHT)/(TREEMODE_9_MAXHEIGHT-TREEMODE_X_MINHEIGHT),0.0f),1.0f);

         if (h3<TREEMODE_X_MINHEIGHT)
            t3=fmin(TREEMODE_9_TREESTART*fmax((h3-TREEMODE_9_MINHEIGHT)/(TREEMODE_X_MINHEIGHT-TREEMODE_9_MINHEIGHT),0.0f),1.0f);
         else
            t3=TREEMODE_9_TREESTART+fmin((1.0f-TREEMODE_9_TREESTART)*fmax((h3-TREEMODE_X_MINHEIGHT)/(TREEMODE_9_MAXHEIGHT-TREEMODE_X_MINHEIGHT),0.0f),1.0f);

         cachegrass(x1,y1+g1*TREEMODE_9_GRASSDEPTH,z1,g1,t1);
         cachegrass(x2,y2+g2*TREEMODE_9_GRASSDEPTH,z2,g2,t2);
         cachegrass(x3,y3+g3*TREEMODE_9_GRASSDEPTH,z3,g3,t3);
         }

   if (h<TREEMODE_X_MINHEIGHT) return;

   b=(fabs(x1-x2)+fabs(z1-z2)+
      fabs(x2-x3)+fabs(z2-z3)+
      fabs(x3-x1)+fabs(z3-z1))/3.0f;

   x=(x1+x2+x3)/3.0f;
   y=(y1+y2+y3)/3.0f;
   z=(z1+z2+z3)/3.0f;

   dx=x-TREECACHE_EX;
   dy=y-TREECACHE_EY;
   dz=z-TREECACHE_EZ;

   d=(b*TREEMODE_X_SQRFACTOR+fsqr(b))*fsqr(TREEMODE_X_RES);

   // evaluate the subdivision variable
   f=(dx*dx+dy*dy+dz*dz)/d;

   if (f<1.0f) // subdivision condition
      if (level<TREEMODE_X_MAXLEVEL)
         if (b>1.5f*TREEMODE_X_MINSPACE)
            {
            x12=(x1+x2)/2.0f;
            y12=(y1+y2)/2.0f;
            z12=(z1+z2)/2.0f;
            h12=(h1+h2)/2.0f;

            x23=(x2+x3)/2.0f;
            y23=(y2+y3)/2.0f;
            z23=(z2+z3)/2.0f;
            h23=(h2+h3)/2.0f;

            x31=(x3+x1)/2.0f;
            y31=(y3+y1)/2.0f;
            z31=(z3+z1)/2.0f;
            h31=(h3+h1)/2.0f;

            treedata(x1,y1,z1,h1,
                     x12,y12,z12,h12,
                     x31,y31,z31,h31,
                     level+1);

            treedata(x12,y12,z12,h12,
                     x2,y2,z2,h2,
                     x23,y23,z23,h23,
                     level+1);

            treedata(x31,y31,z31,h31,
                     x23,y23,z23,h23,
                     x3,y3,z3,h3,
                     level+1);

            treedata(x12,y12,z12,h12,
                     x23,y23,z23,h23,
                     x31,y31,z31,h31,
                     level+1);

            return;
            }

   rand=(x/0.271f+z/0.331f)/TREEMODE_X_MINSPACE;

   if (TREEMODE>=6)
      if (h<TREEMODE_X_MINHEIGHT*(1.0f+TREEMODE_6_SHRUBFRAC))
         {
         r=TREEMODE_6_SHRUBFRAC*TREEMODE_X_MINHEIGHT;

         if (r>0.0f) r=(h-TREEMODE_X_MINHEIGHT)/r;
         else r=1.0f;

         rand+=271.331f*(rand+1.0f/3);
         rand=rand-ftrc(rand);

         if (rand>r*TREEMODE_6_SHRUBPROB) return;
         }

   rand+=271.331f*(rand+1.0f/3);
   rand=rand-ftrc(rand);

   x+=(rand-0.5f)*TREEMODE_X_TREEVARIANCE*(b/TREEMODE_X_MINSPACE);

   rand+=271.331f*(rand+1.0f/3);
   rand=rand-ftrc(rand);

   z+=(rand-0.5f)*TREEMODE_X_TREEVARIANCE*(b/TREEMODE_X_MINSPACE);

   h=(h1+h2+h3)/3.0f;

   switch (TREEMODE)
      {
      case 1: // lines

         cachedata(x,y,z);
         cachedata(x,y+h,z);

         break;

      case 2: // triangles

      case 3: // pseudo-lit triangles

         w=0.5f*TREEMODE_X_TREEWIDTH;

         cachedata(x-w,y,z);
         cachedata(x+w,y,z);
         cachedata(x,y+h,z);

         cachedata(x,y,z-w);
         cachedata(x,y,z+w);
         cachedata(x,y+h,z);

         break;

      case 4: // textured quads

         w=0.5f*TREEMODE_X_TREEWIDTH;

         if (TREEMODE_4_TREEASPECT==0.0f) t=0.0f;
         else t=1.0f-h/(TREEMODE_X_TREEWIDTH*TREEMODE_4_TREEASPECT);

         cachedata(x-w,y,z,0.0f,1.0f);
         cachedata(x+w,y,z,1.0f,1.0f);
         cachedata(x+w,y+h,z,1.0f,t);
         cachedata(x-w,y+h,z,0.0f,t);

         cachedata(x,y,z-w,0.0f,1.0f);
         cachedata(x,y,z+w,1.0f,1.0f);
         cachedata(x,y+h,z+w,1.0f,t);
         cachedata(x,y+h,z-w,0.0f,t);

         break;

      case 5: // textured billboards
      case 6:

         w=0.5f*TREEMODE_X_TREEWIDTH;

         if (TREEMODE_4_TREEASPECT==0.0f) t=0.0f;
         else t=1.0f-h/(TREEMODE_X_TREEWIDTH*TREEMODE_4_TREEASPECT);

         cachedata(x,y,z,0.0f,1.0f,-w);
         cachedata(x,y,z,1.0f,1.0f,w);
         cachedata(x,y+h,z,1.0f,t,w);
         cachedata(x,y+h,z,0.0f,t,-w);

         break;

      case 7: // randomized billboards

         w=0.5f*TREEMODE_X_TREEWIDTH;

         if (TREEMODE_4_TREEASPECT==0.0f) t=0.0f;
         else t=1.0f-h/(TREEMODE_X_TREEWIDTH*TREEMODE_4_TREEASPECT);

         rand+=271.331f*(rand+1.0f/3);
         rand=rand-ftrc(rand);

         choice=ftrc(TREEMODE_7_TREENUM*rand);
         r=1.0f/TREEMODE_7_TREENUM;

         cachedata(x,y,z,choice*r,1.0f,-w);
         cachedata(x,y,z,(choice+1)*r,1.0f,w);
         cachedata(x,y+h,z,(choice+1)*r,t,w);
         cachedata(x,y+h,z,choice*r,t,-w);

         break;

      case 8: // height-based billboards
      case 9:
      case 10:
      case 11:
      case 12:

         w=0.5f*TREEMODE_X_TREEWIDTH;

         if (TREEMODE_4_TREEASPECT==0.0f) t=0.0f;
         else t=1.0f-h/(TREEMODE_X_TREEWIDTH*TREEMODE_4_TREEASPECT);

         rand+=271.331f*(rand+1.0f/3);
         rand=rand-ftrc(rand);

         r=TREEMODE_7_TREENUM-1-(h-TREEMODE_X_MINHEIGHT)/TREEMODE_8_TREESTEP;
         r+=TREEMODE_8_TREERAND*TREEMODE_7_TREENUM*(rand-0.5f);

         choice=ftrc(r);
         if (choice<0) choice=0;
         else if (choice>TREEMODE_7_TREENUM-1) choice=TREEMODE_7_TREENUM-1;

         r=1.0f/TREEMODE_7_TREENUM;

         cachedata(x,y,z,choice*r,1.0f,-w);
         cachedata(x,y,z,(choice+1)*r,1.0f,w);
         cachedata(x,y+h,z,(choice+1)*r,t,w);
         cachedata(x,y+h,z,choice*r,t,-w);

         break;
      }

   if (TREECACHE_NUM==1) TREECACHE_TREES1++;
   else TREECACHE_TREES2++;
   }

// cache tree data (one vertex)
void minitree::cachedata(float x,float y,float z)
   {
   float *ptr;

   if (TREECACHE_NUM==1)
      {
      if (TREECACHE_SIZE1>=TREECACHE_MAXSIZE1)
         {
         TREECACHE_MAXSIZE1*=2;

         if ((TREECACHE_CACHE1=(float *)realloc(TREECACHE_CACHE1,TREECACHE_MAXSIZE1*3*sizeof(float)))==NULL) ERRORMSG();
         if ((TREECACHE_COORD1=(float *)realloc(TREECACHE_COORD1,TREECACHE_MAXSIZE1*3*sizeof(float)))==NULL) ERRORMSG();
         }

      ptr=&TREECACHE_CACHE1[3*TREECACHE_SIZE1++];
      }
   else
      {
      if (TREECACHE_SIZE2>=TREECACHE_MAXSIZE2)
         {
         TREECACHE_MAXSIZE2*=2;

         if ((TREECACHE_CACHE2=(float *)realloc(TREECACHE_CACHE2,TREECACHE_MAXSIZE2*3*sizeof(float)))==NULL) ERRORMSG();
         if ((TREECACHE_COORD2=(float *)realloc(TREECACHE_COORD2,TREECACHE_MAXSIZE2*3*sizeof(float)))==NULL) ERRORMSG();
         }

      ptr=&TREECACHE_CACHE2[3*TREECACHE_SIZE2++];
      }

   *ptr++=x;
   *ptr++=y;
   *ptr=z;
   }

// cache tree data (one vertex plus texture coords)
void minitree::cachedata(float x,float y,float z,float s,float t,float r)
   {
   float *ptr1,*ptr2;

   if (TREECACHE_NUM==1)
      {
      if (TREECACHE_SIZE1>=TREECACHE_MAXSIZE1)
         {
         TREECACHE_MAXSIZE1*=2;

         if ((TREECACHE_CACHE1=(float *)realloc(TREECACHE_CACHE1,TREECACHE_MAXSIZE1*3*sizeof(float)))==NULL) ERRORMSG();
         if ((TREECACHE_COORD1=(float *)realloc(TREECACHE_COORD1,TREECACHE_MAXSIZE1*3*sizeof(float)))==NULL) ERRORMSG();
         }

      ptr1=&TREECACHE_CACHE1[3*TREECACHE_SIZE1];
      ptr2=&TREECACHE_COORD1[3*TREECACHE_SIZE1++];
      }
   else
      {
      if (TREECACHE_SIZE2>=TREECACHE_MAXSIZE2)
         {
         TREECACHE_MAXSIZE2*=2;

         if ((TREECACHE_CACHE2=(float *)realloc(TREECACHE_CACHE2,TREECACHE_MAXSIZE2*3*sizeof(float)))==NULL) ERRORMSG();
         if ((TREECACHE_COORD2=(float *)realloc(TREECACHE_COORD2,TREECACHE_MAXSIZE2*3*sizeof(float)))==NULL) ERRORMSG();
         }

      ptr1=&TREECACHE_CACHE2[3*TREECACHE_SIZE2];
      ptr2=&TREECACHE_COORD2[3*TREECACHE_SIZE2++];
      }

   *ptr1++=x;
   *ptr1++=y;
   *ptr1=z;

   *ptr2++=s;
   *ptr2++=t;
   *ptr2=r;
   }

// cache grass data (one vertex plus texture coords)
void minitree::cachegrass(float x,float y,float z,float s,float t,float r)
   {
   float *ptr1,*ptr2;

   if (TREECACHE_NUM==1)
      {
      if (GRASSCACHE_SIZE1>=GRASSCACHE_MAXSIZE1)
         {
         GRASSCACHE_MAXSIZE1*=2;

         if ((GRASSCACHE_CACHE1=(float *)realloc(GRASSCACHE_CACHE1,GRASSCACHE_MAXSIZE1*3*sizeof(float)))==NULL) ERRORMSG();
         if ((GRASSCACHE_COORD1=(float *)realloc(GRASSCACHE_COORD1,GRASSCACHE_MAXSIZE1*3*sizeof(float)))==NULL) ERRORMSG();
         }

      ptr1=&GRASSCACHE_CACHE1[3*GRASSCACHE_SIZE1];
      ptr2=&GRASSCACHE_COORD1[3*GRASSCACHE_SIZE1++];
      }
   else
      {
      if (GRASSCACHE_SIZE2>=GRASSCACHE_MAXSIZE2)
         {
         GRASSCACHE_MAXSIZE2*=2;

         if ((GRASSCACHE_CACHE2=(float *)realloc(GRASSCACHE_CACHE2,GRASSCACHE_MAXSIZE2*3*sizeof(float)))==NULL) ERRORMSG();
         if ((GRASSCACHE_COORD2=(float *)realloc(GRASSCACHE_COORD2,GRASSCACHE_MAXSIZE2*3*sizeof(float)))==NULL) ERRORMSG();
         }

      ptr1=&GRASSCACHE_CACHE2[3*GRASSCACHE_SIZE2];
      ptr2=&GRASSCACHE_COORD2[3*GRASSCACHE_SIZE2++];
      }

   *ptr1++=x;
   *ptr1++=y;
   *ptr1=z;

   *ptr2++=s;
   *ptr2++=t;
   *ptr2=r;
   }

// render cached trees
int minitree::rendertrees(float *cache,float *coords,int cnt,miniwarp *warp,
                          float tr,float tg,float tb)
   {
   int vtx=0;

#ifndef NOOGL

#if defined(GL_ARB_vertex_program) && defined(GL_ARB_fragment_program)

   static const char *vtxprog1="!!ARBvp1.0 \n\
      PARAM c=program.env[0]; \n\
      PARAM mat[4]={state.matrix.mvp}; \n\
      TEMP vtx,col,pos; \n\
      MOV vtx,vertex.position; \n\
      MOV col,vertex.color; \n\
      DP4 pos.x,mat[0],vtx; \n\
      DP4 pos.y,mat[1],vtx; \n\
      DP4 pos.z,mat[2],vtx; \n\
      DP4 pos.w,mat[3],vtx; \n\
      MOV result.position,pos; \n\
      MUL vtx,vtx,c; \n\
      ADD vtx.x,vtx.x,vtx.y; \n\
      ADD vtx.x,vtx.x,vtx.z; \n\
      FRC vtx.x,vtx.x; \n\
      MUL vtx.x,vtx.x,c.y; \n\
      ADD col.xyz,col,vtx.x; \n\
      MOV result.color,col; \n\
      MOV result.fogcoord.x,pos.z; \n\
      END \n";

   static const char *vtxprog2="!!ARBvp1.0 \n\
      PARAM c=program.env[0]; \n\
      PARAM mat[4]={state.matrix.mvp}; \n\
      TEMP vtx,col,tex,pos; \n\
      MOV vtx,vertex.position; \n\
      MOV col,vertex.color; \n\
      MOV tex,vertex.texcoord; \n\
      MAD vtx,tex.z,c,vtx; \n\
      DP4 pos.x,mat[0],vtx; \n\
      DP4 pos.y,mat[1],vtx; \n\
      DP4 pos.z,mat[2],vtx; \n\
      DP4 pos.w,mat[3],vtx; \n\
      MOV result.position,pos; \n\
      MOV result.color,col; \n\
      MOV result.texcoord,tex; \n\
      MOV result.fogcoord.x,pos.z; \n\
      END \n";

   GLuint vtxprogid;

   GLfloat mvmtx[16];

#endif

   miniv4d mtx[3];
   double oglmtx[16];

   unsigned char *image;
   int width,height,components;

   unsigned char *image2;
   int width2,height2,components2;

   int prevdepth;

   if (cnt==0) return(vtx);

   initwglprocs();

   initstate();

   glDisable(GL_CULL_FACE);

   glPushMatrix();
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glScalef(CONFIGURE_ZSCALE,CONFIGURE_ZSCALE,CONFIGURE_ZSCALE); // prevent Z-fighting
   glMatrixMode(GL_TEXTURE);
   glPushMatrix();
   glLoadIdentity();
   glMatrixMode(GL_MODELVIEW);

   if (warp!=NULL)
      {
      warp->getwarp(mtx);

      oglmtx[0]=mtx[0].x;
      oglmtx[4]=mtx[0].y;
      oglmtx[8]=mtx[0].z;
      oglmtx[12]=mtx[0].w;

      oglmtx[1]=mtx[1].x;
      oglmtx[5]=mtx[1].y;
      oglmtx[9]=mtx[1].z;
      oglmtx[13]=mtx[1].w;

      oglmtx[2]=mtx[2].x;
      oglmtx[6]=mtx[2].y;
      oglmtx[10]=mtx[2].z;
      oglmtx[14]=mtx[2].w;

      oglmtx[3]=0.0;
      oglmtx[7]=0.0;
      oglmtx[11]=0.0;
      oglmtx[15]=1.0;

      mtxmult(oglmtx);
      }

   switch (TREEMODE)
      {
      case 1:

         glColor3f(tr,tg,tb);

         glVertexPointer(3,GL_FLOAT,0,cache);
         glEnableClientState(GL_VERTEX_ARRAY);
         glDrawArrays(GL_LINES,0,cnt);
         glDisableClientState(GL_VERTEX_ARRAY);

         vtx+=cnt;

         break;

      case 2:

         glColor3f(tr,tg,tb);

         glVertexPointer(3,GL_FLOAT,0,cache);
         glEnableClientState(GL_VERTEX_ARRAY);
         glDrawArrays(GL_TRIANGLES,0,cnt);
         glDisableClientState(GL_VERTEX_ARRAY);

         vtx+=cnt;

         break;

      case 3:

#if defined(GL_ARB_vertex_program) && defined(GL_ARB_fragment_program)

         if (TREECACHE_VTXPROGID1==0)
            {
            glGenProgramsARB(1,&vtxprogid);
            glBindProgramARB(GL_VERTEX_PROGRAM_ARB,vtxprogid);
            glProgramStringARB(GL_VERTEX_PROGRAM_ARB,GL_PROGRAM_FORMAT_ASCII_ARB,strlen(vtxprog1),vtxprog1);
            TREECACHE_VTXPROGID1=vtxprogid;
            }

         glBindProgramARB(GL_VERTEX_PROGRAM_ARB,TREECACHE_VTXPROGID1);
         glEnable(GL_VERTEX_PROGRAM_ARB);

         glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB,0,271.0f/TREEMODE_X_TREEWIDTH,TREEMODE_3_COLFLUCT,331.0f/TREEMODE_X_TREEWIDTH,0.0f);

         glColor3f(tr,tg,tb);

         glVertexPointer(3,GL_FLOAT,0,cache);
         glEnableClientState(GL_VERTEX_ARRAY);
         glDrawArrays(GL_TRIANGLES,0,cnt);
         glDisableClientState(GL_VERTEX_ARRAY);

         glBindProgramARB(GL_VERTEX_PROGRAM_ARB,0);
         glDisable(GL_VERTEX_PROGRAM_ARB);

         vtx+=cnt;

#endif

         break;

      case 4:

         if (TREECACHE_TEXID==0)
            {
            if (TREEMODE_4_TEXFILE_RGB==NULL) ERRORMSG();

            if (TREEMODE_4_TEXFILE_A==NULL)
               {
               if ((image=readPNMfile(TREEMODE_4_TEXFILE_RGB,&width,&height,&components))==NULL) ERRORMSG();
               if (components!=3) ERRORMSG();

               prevdepth=miniOGL::configure_depth(32);
               TREECACHE_TEXID=buildRGBtexmap(image,&width,&height);
               miniOGL::configure_depth(prevdepth);
               free(image);
               }
            else
               {
               if ((image=readPNMfile(TREEMODE_4_TEXFILE_RGB,&width,&height,&components))==NULL) ERRORMSG();
               if (components!=3) ERRORMSG();

               if ((image2=readPNMfile(TREEMODE_4_TEXFILE_A,&width2,&height2,&components2))==NULL) ERRORMSG();
               if (components2!=1) ERRORMSG();

               if (width2!=width || height2!=height) ERRORMSG();

               TREECACHE_TEXID=buildRGBAtexmap(image,image2,&width,&height);
               free(image2);
               free(image);
               }
            }

         bindtexmap(TREECACHE_TEXID,1,1);

         glColor3f(1.0f,1.0f,1.0f);

         if (CONFIGURE_BLEND!=0)
            {
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_BLEND);
            }

         glAlphaFunc(GL_GREATER,TREEMODE_4_MINALPHA);
         glEnable(GL_ALPHA_TEST);

         glVertexPointer(3,GL_FLOAT,0,cache);
         glEnableClientState(GL_VERTEX_ARRAY);
         glTexCoordPointer(3,GL_FLOAT,0,coords);
         glEnableClientState(GL_TEXTURE_COORD_ARRAY);
         glDrawArrays(GL_QUADS,0,cnt);
         glDisableClientState(GL_VERTEX_ARRAY);
         glDisableClientState(GL_TEXTURE_COORD_ARRAY);

         glDisable(GL_ALPHA_TEST);

         if (CONFIGURE_BLEND!=0) glDisable(GL_BLEND);

         bindtexmap(0,0,0);

         vtx+=cnt;

         break;

      case 5:
      case 6:

#if defined(GL_ARB_vertex_program) && defined(GL_ARB_fragment_program)

         if (TREECACHE_VTXPROGID2==0)
            {
            glGenProgramsARB(1,&vtxprogid);
            glBindProgramARB(GL_VERTEX_PROGRAM_ARB,vtxprogid);
            glProgramStringARB(GL_VERTEX_PROGRAM_ARB,GL_PROGRAM_FORMAT_ASCII_ARB,strlen(vtxprog2),vtxprog2);
            TREECACHE_VTXPROGID2=vtxprogid;
            }

         glBindProgramARB(GL_VERTEX_PROGRAM_ARB,TREECACHE_VTXPROGID2);
         glEnable(GL_VERTEX_PROGRAM_ARB);

         glGetFloatv(GL_MODELVIEW_MATRIX,mvmtx);
         glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB,0,-mvmtx[10],0.0f,mvmtx[2],0.0f);

         if (TREECACHE_TEXID==0)
            {
            if (TREEMODE_4_TEXFILE_RGB==NULL) ERRORMSG();

            if (TREEMODE_4_TEXFILE_A==NULL)
               {
               if ((image=readPNMfile(TREEMODE_4_TEXFILE_RGB,&width,&height,&components))==NULL) ERRORMSG();
               if (components!=3) ERRORMSG();

               prevdepth=miniOGL::configure_depth(32);
               TREECACHE_TEXID=buildRGBtexmap(image,&width,&height);
               miniOGL::configure_depth(prevdepth);
               free(image);
               }
            else
               {
               if ((image=readPNMfile(TREEMODE_4_TEXFILE_RGB,&width,&height,&components))==NULL) ERRORMSG();
               if (components!=3) ERRORMSG();

               if ((image2=readPNMfile(TREEMODE_4_TEXFILE_A,&width2,&height2,&components2))==NULL) ERRORMSG();
               if (components2!=1) ERRORMSG();

               if (width2!=width || height2!=height) ERRORMSG();

               TREECACHE_TEXID=buildRGBAtexmap(image,image2,&width,&height);
               free(image2);
               free(image);
               }
            }

         bindtexmap(TREECACHE_TEXID,1,1);

         glColor3f(1.0f,1.0f,1.0f);

         if (CONFIGURE_BLEND!=0)
            {
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_BLEND);
            }

         glAlphaFunc(GL_GREATER,TREEMODE_4_MINALPHA);
         glEnable(GL_ALPHA_TEST);

         glVertexPointer(3,GL_FLOAT,0,cache);
         glEnableClientState(GL_VERTEX_ARRAY);
         glTexCoordPointer(3,GL_FLOAT,0,coords);
         glEnableClientState(GL_TEXTURE_COORD_ARRAY);
         glDrawArrays(GL_QUADS,0,cnt);
         glDisableClientState(GL_VERTEX_ARRAY);
         glDisableClientState(GL_TEXTURE_COORD_ARRAY);

         glDisable(GL_ALPHA_TEST);

         if (CONFIGURE_BLEND!=0) glDisable(GL_BLEND);

         bindtexmap(0,0,0);

         glBindProgramARB(GL_VERTEX_PROGRAM_ARB,0);
         glDisable(GL_VERTEX_PROGRAM_ARB);

         vtx+=cnt;

#endif

         break;

      case 7:
      case 8:
      case 9:
      case 10:
      case 11:
      case 12:

#if defined(GL_ARB_vertex_program) && defined(GL_ARB_fragment_program)

         if (TREECACHE_VTXPROGID2==0)
            {
            glGenProgramsARB(1,&vtxprogid);
            glBindProgramARB(GL_VERTEX_PROGRAM_ARB,vtxprogid);
            glProgramStringARB(GL_VERTEX_PROGRAM_ARB,GL_PROGRAM_FORMAT_ASCII_ARB,strlen(vtxprog2),vtxprog2);
            TREECACHE_VTXPROGID2=vtxprogid;
            }

         glBindProgramARB(GL_VERTEX_PROGRAM_ARB,TREECACHE_VTXPROGID2);
         glEnable(GL_VERTEX_PROGRAM_ARB);

         glGetFloatv(GL_MODELVIEW_MATRIX,mvmtx);
         glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB,0,-mvmtx[10],0.0f,mvmtx[2],0.0f);

         if (TREECACHE_TEXID==0)
            {
            if (TREEMODE_7_TEXFILE_RGB==NULL) ERRORMSG();

            if (TREEMODE_7_TEXFILE_A==NULL)
               {
               if ((image=readPNMfile(TREEMODE_7_TEXFILE_RGB,&width,&height,&components))==NULL) ERRORMSG();
               if (components!=3) ERRORMSG();

               prevdepth=miniOGL::configure_depth(32);
               TREECACHE_TEXID=buildRGBtexmap(image,&width,&height);
               miniOGL::configure_depth(prevdepth);
               free(image);
               }
            else
               {
               if ((image=readPNMfile(TREEMODE_7_TEXFILE_RGB,&width,&height,&components))==NULL) ERRORMSG();
               if (components!=3) ERRORMSG();

               if ((image2=readPNMfile(TREEMODE_7_TEXFILE_A,&width2,&height2,&components2))==NULL) ERRORMSG();
               if (components2!=1) ERRORMSG();

               if (width2!=width || height2!=height) ERRORMSG();

               TREECACHE_TEXID=buildRGBAtexmap(image,image2,&width,&height);
               free(image2);
               free(image);
               }
            }

         bindtexmap(TREECACHE_TEXID,1,1);

         glColor3f(1.0f,1.0f,1.0f);

         if (CONFIGURE_BLEND!=0)
            {
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_BLEND);
            }

         glAlphaFunc(GL_GREATER,TREEMODE_4_MINALPHA);
         glEnable(GL_ALPHA_TEST);

         glVertexPointer(3,GL_FLOAT,0,cache);
         glEnableClientState(GL_VERTEX_ARRAY);
         glTexCoordPointer(3,GL_FLOAT,0,coords);
         glEnableClientState(GL_TEXTURE_COORD_ARRAY);
         glDrawArrays(GL_QUADS,0,cnt);
         glDisableClientState(GL_VERTEX_ARRAY);
         glDisableClientState(GL_TEXTURE_COORD_ARRAY);

         glDisable(GL_ALPHA_TEST);

         if (CONFIGURE_BLEND!=0) glDisable(GL_BLEND);

         bindtexmap(0,0,0);

         glBindProgramARB(GL_VERTEX_PROGRAM_ARB,0);
         glDisable(GL_VERTEX_PROGRAM_ARB);

         vtx+=cnt;

#endif

         break;
      }

   glPopMatrix();
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_TEXTURE);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);

   exitstate();

#endif

   return(vtx);
   }

// render cached grass
int minitree::rendergrass(float *cache,float *coords,int cnt,miniwarp *warp)
   {
   int vtx=0;

#ifndef NOOGL

#if defined(GL_ARB_multitexture) && defined(GL_ARB_vertex_program) && defined(GL_ARB_fragment_program)

   static const char *vtxprog="!!ARBvp1.0 \n\
      PARAM c=program.env[0]; \n\
      PARAM mat[4]={state.matrix.mvp}; \n\
      TEMP vtx,col,tex,pos; \n\
      MOV vtx,vertex.position; \n\
      MOV col,vertex.color; \n\
      MOV tex,vertex.texcoord; \n\
      DP4 pos.x,mat[0],vtx; \n\
      DP4 pos.y,mat[1],vtx; \n\
      DP4 pos.z,mat[2],vtx; \n\
      DP4 pos.w,mat[3],vtx; \n\
      MOV result.position,pos; \n\
      MOV result.color,col; \n\
      MOV result.texcoord,tex; \n\
      MUL result.texcoord[1].xyz,vtx,c.x; \n\
      MOV result.fogcoord.x,pos.z; \n\
      END \n";

   static const char *fragprog1="!!ARBfp1.0 \n\
      PARAM c=program.env[0]; \n\
      PARAM d=program.env[1]; \n\
      PARAM e=program.env[2]; \n\
      TEMP col,tex,pos; \n\
      MOV tex.x,fragment.texcoord[1].x; \n\
      MOV tex.y,fragment.texcoord[1].z; \n\
      MOV_SAT tex.z,fragment.texcoord.y; \n\
      MAD tex.z,tex.z,d.x,d.y; \n\
      FLR tex.z,tex.z; \n\
      MAD tex.z,tex.z,d.z,d.w; \n\
      MAD pos.z,fragment.fogcoord.x,e.x,e.y; \n\
      LG2 pos.z,pos.z; \n\
      MIN pos.w,pos.z,e.w; \n\
      SUB pos.z,pos.z,pos.w; \n\
      SUB pos.z,e.y,pos.z; \n\
      FLR pos.w,pos.w; \n\
      MAD tex.z,pos.w,e.z,tex.z; \n\
      TEX result.color.xyz,tex,texture[0],3D; \n\
      MUL col.w,fragment.color.w,fragment.texcoord.x; \n\
      MUL col.w,col.w,pos.z; \n\
      MAD_SAT pos.z,fragment.fogcoord.x,c.z,c.w; \n\
      MUL result.color.w,col.w,pos.z; \n\
      END \n";

   static const char *fragprog2="!!ARBfp1.0 \n\
      PARAM c=program.env[0]; \n\
      PARAM d=program.env[1]; \n\
      PARAM e=program.env[2]; \n\
      PARAM f=program.env[3]; \n\
      TEMP col,tex,ptb,pos; \n\
      MOV tex.x,fragment.texcoord[1].x; \n\
      MOV tex.y,fragment.texcoord[1].z; \n\
      MUL ptb.xy,tex,f.x; \n\
      TEX ptb,ptb,texture[1],2D; \n\
      MAD ptb.xyz,ptb,f.z,f.w; \n\
      MAD_SAT tex.z,ptb.z,f.y,fragment.texcoord.y; \n\
      MAD tex.z,tex.z,d.x,d.y; \n\
      FLR tex.z,tex.z; \n\
      MAD tex.z,tex.z,d.z,d.w; \n\
      MAD pos.z,fragment.fogcoord.x,e.x,e.y; \n\
      LG2 pos.z,pos.z; \n\
      MIN pos.w,pos.z,e.w; \n\
      SUB pos.z,pos.z,pos.w; \n\
      SUB pos.z,e.y,pos.z; \n\
      FLR pos.w,pos.w; \n\
      MAD tex.z,pos.w,e.z,tex.z; \n\
      TEX result.color.xyz,tex,texture[0],3D; \n\
      MUL col.w,fragment.color.w,fragment.texcoord.x; \n\
      MUL col.w,col.w,pos.z; \n\
      MAD_SAT pos.z,fragment.fogcoord.x,c.z,c.w; \n\
      MUL result.color.w,col.w,pos.z; \n\
      END \n";

   static const char *fragprog3="!!ARBfp1.0 \n\
      PARAM c=program.env[0]; \n\
      PARAM d=program.env[1]; \n\
      PARAM e=program.env[2]; \n\
      PARAM f=program.env[3]; \n\
      TEMP col,tex,ptb,pos; \n\
      MOV tex.x,fragment.texcoord[1].x; \n\
      MOV tex.y,fragment.texcoord[1].z; \n\
      MUL ptb.xy,tex,f.x; \n\
      TEX ptb,ptb,texture[1],2D; \n\
      MAD ptb.xyz,ptb,f.z,f.w; \n\
      MAD_SAT tex.z,ptb.z,f.y,fragment.texcoord.y; \n\
      MAD tex.z,tex.z,d.x,d.y; \n\
      MAD pos.z,fragment.fogcoord.x,e.x,e.y; \n\
      LG2 pos.z,pos.z; \n\
      MIN pos.w,pos.z,e.w; \n\
      SUB pos.z,pos.z,pos.w; \n\
      SUB pos.z,e.y,pos.z; \n\
      FLR pos.w,pos.w; \n\
      MAD tex.z,pos.w,e.z,tex.z; \n\
      TEX result.color.xyz,tex,texture[0],3D; \n\
      MUL col.w,fragment.color.w,fragment.texcoord.x; \n\
      MUL col.w,col.w,pos.z; \n\
      MAD_SAT pos.z,fragment.fogcoord.x,c.z,c.w; \n\
      MUL result.color.w,col.w,pos.z; \n\
      END \n";

   static const char *fragprog4="!!ARBfp1.0 \n\
      PARAM c=program.env[0]; \n\
      PARAM d=program.env[1]; \n\
      PARAM e=program.env[2]; \n\
      PARAM f=program.env[3]; \n\
      PARAM g=program.env[4]; \n\
      TEMP col,tex,ptb,pos; \n\
      MOV tex.x,fragment.texcoord[1].x; \n\
      MOV tex.y,fragment.texcoord[1].z; \n\
      MUL ptb.xy,tex,f.x; \n\
      TEX ptb,ptb,texture[1],2D; \n\
      MAD ptb.xyz,ptb,f.z,f.w; \n\
      MAD_SAT tex.z,ptb.z,f.y,fragment.texcoord.y; \n\
      MAD tex.z,tex.z,d.x,d.y; \n\
      MAD pos.z,fragment.fogcoord.x,e.x,e.y; \n\
      LG2 pos.z,pos.z; \n\
      MIN pos.w,pos.z,e.w; \n\
      SUB pos.z,pos.z,pos.w; \n\
      SUB pos.z,e.y,pos.z; \n\
      FLR pos.w,pos.w; \n\
      MAD tex.z,pos.w,e.z,tex.z; \n\
      TEX col.xyz,tex,texture[0],3D; \n\
      DP3 col.w,col,g; \n\
      ADD_SAT col.w,col.w,g.w; \n\
      MUL col.w,col.w,fragment.color.w; \n\
      MUL col.w,col.w,fragment.texcoord.x; \n\
      MUL col.w,col.w,pos.z; \n\
      MAD_SAT pos.z,fragment.fogcoord.x,c.z,c.w; \n\
      MUL col.w,col.w,pos.z; \n\
      MOV result.color,col; \n\
      END \n";

   GLuint vtxprogid,fragprogid;

   GLfloat fogstart,fogend;

   miniv4d mtx[3];
   double oglmtx[16];

   unsigned char *volume;
   int width,height,depth,components;

   unsigned char *perturbation;
   int psize,pwidth,pheight;

   if (cnt==0) return(vtx);

   initwglprocs();

   initstate();

   glDisable(GL_CULL_FACE);

   glPushMatrix();
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glScalef(CONFIGURE_ZSCALE,CONFIGURE_ZSCALE,CONFIGURE_ZSCALE); // prevent Z-fighting
   glMatrixMode(GL_TEXTURE);
   glPushMatrix();
   glLoadIdentity();
   glMatrixMode(GL_MODELVIEW);

   if (warp!=NULL)
      {
      warp->getwarp(mtx);

      oglmtx[0]=mtx[0].x;
      oglmtx[4]=mtx[0].y;
      oglmtx[8]=mtx[0].z;
      oglmtx[12]=mtx[0].w;

      oglmtx[1]=mtx[1].x;
      oglmtx[5]=mtx[1].y;
      oglmtx[9]=mtx[1].z;
      oglmtx[13]=mtx[1].w;

      oglmtx[2]=mtx[2].x;
      oglmtx[6]=mtx[2].y;
      oglmtx[10]=mtx[2].z;
      oglmtx[14]=mtx[2].w;

      oglmtx[3]=0.0;
      oglmtx[7]=0.0;
      oglmtx[11]=0.0;
      oglmtx[15]=1.0;

      mtxmult(oglmtx);
      }

   glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
   glEnable(GL_BLEND);

   glColor4f(1.0f,1.0f,1.0f,TREEMODE_9_GRASSALPHA);

   if (GRASSCACHE_VTXPROGID==0)
      {
      glGenProgramsARB(1,&vtxprogid);
      glBindProgramARB(GL_VERTEX_PROGRAM_ARB,vtxprogid);
      glProgramStringARB(GL_VERTEX_PROGRAM_ARB,GL_PROGRAM_FORMAT_ASCII_ARB,strlen(vtxprog),vtxprog);
      GRASSCACHE_VTXPROGID=vtxprogid;
      }

   glBindProgramARB(GL_VERTEX_PROGRAM_ARB,GRASSCACHE_VTXPROGID);
   glEnable(GL_VERTEX_PROGRAM_ARB);

   if (TREEMODE==9)
      {
      if (GRASSCACHE_FRAGPROGID1==0)
         {
         glGenProgramsARB(1,&fragprogid);
         glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,fragprogid);
         glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB,GL_PROGRAM_FORMAT_ASCII_ARB,strlen(fragprog1),fragprog1);
         GRASSCACHE_FRAGPROGID1=fragprogid;
         }

      glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,GRASSCACHE_FRAGPROGID1);
      glEnable(GL_FRAGMENT_PROGRAM_ARB);
      }
   else if (TREEMODE==10)
      {
      if (GRASSCACHE_FRAGPROGID2==0)
         {
         glGenProgramsARB(1,&fragprogid);
         glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,fragprogid);
         glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB,GL_PROGRAM_FORMAT_ASCII_ARB,strlen(fragprog2),fragprog2);
         GRASSCACHE_FRAGPROGID2=fragprogid;
         }

      glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,GRASSCACHE_FRAGPROGID2);
      glEnable(GL_FRAGMENT_PROGRAM_ARB);
      }
   else if (TREEMODE==11)
      {
      if (GRASSCACHE_FRAGPROGID3==0)
         {
         glGenProgramsARB(1,&fragprogid);
         glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,fragprogid);
         glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB,GL_PROGRAM_FORMAT_ASCII_ARB,strlen(fragprog3),fragprog3);
         GRASSCACHE_FRAGPROGID3=fragprogid;
         }

      glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,GRASSCACHE_FRAGPROGID3);
      glEnable(GL_FRAGMENT_PROGRAM_ARB);
      }
   else
      {
      if (GRASSCACHE_FRAGPROGID4==0)
         {
         glGenProgramsARB(1,&fragprogid);
         glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,fragprogid);
         glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB,GL_PROGRAM_FORMAT_ASCII_ARB,strlen(fragprog4),fragprog4);
         GRASSCACHE_FRAGPROGID4=fragprogid;
         }

      glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,GRASSCACHE_FRAGPROGID4);
      glEnable(GL_FRAGMENT_PROGRAM_ARB);
      }

   glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB,0,1.0f/TREEMODE_9_TEXSIZE,0.0f,0.0f,0.0f);

   // assume linear fog
   glGetFloatv(GL_FOG_START,&fogstart);
   glGetFloatv(GL_FOG_END,&fogend);
   if (fogend<=fogstart) ERRORMSG();

   glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,0,0.0f,0.0f,-1.0f/(fogend-fogstart),fogend/(fogend-fogstart));

   if (GRASSCACHE_TEXID==0)
      {
      if (TREEMODE_9_VOLFILE==NULL) ERRORMSG();
      if ((volume=readPVMvolume(TREEMODE_9_VOLFILE,&width,&height,&depth,&components))==NULL) ERRORMSG();
      if (components!=1 && components!=3 && components!=4) ERRORMSG();

      volume=build3Dmipmap(volume,width,height,depth,components,TREEMODE_9_MIPMAPLEVELS);
      GRASSCACHE_CLASSES=depth;
      depth*=TREEMODE_9_MIPMAPLEVELS;

      GRASSCACHE_TEXID=build3Dtexmap(volume,&width,&height,&depth,components);
      free(volume);
      }

   bind3Dtexmap(GRASSCACHE_TEXID);

   if (TREEMODE<=10)
      glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,1,
                                 GRASSCACHE_CLASSES-1,0.5f,
                                 1.0f/(GRASSCACHE_CLASSES*TREEMODE_9_MIPMAPLEVELS),0.5f/(GRASSCACHE_CLASSES*TREEMODE_9_MIPMAPLEVELS));
   else
      glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,1,
                                 (float)(GRASSCACHE_CLASSES-1)/(GRASSCACHE_CLASSES*TREEMODE_9_MIPMAPLEVELS),0.5f/(GRASSCACHE_CLASSES*TREEMODE_9_MIPMAPLEVELS),
                                 0.0f,0.0f);

   glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,2,
                              1.0f/TREEMODE_9_MIPMAPRANGE,1.0f,
                              1.0f/TREEMODE_9_MIPMAPLEVELS,TREEMODE_9_MIPMAPLEVELS-1);

   if (TREEMODE>=10)
      {
      if (GRASSCACHE_PERTURBID==0)
         {
         psize=pwidth=pheight=TREEMODE_10_PERTURBRES;

         perturbation=pn_perlin2D(psize,TREEMODE_10_PERTURBSTART,TREEMODE_10_PERTURBPERS,TREEMODE_10_PERTURBSEED);

         GRASSCACHE_PERTURBID=buildLtexmap(perturbation,&pwidth,&pheight);
         free(perturbation);
         }

      glActiveTextureARB(GL_TEXTURE1_ARB);
      bindtexmap(GRASSCACHE_PERTURBID);
      glActiveTextureARB(GL_TEXTURE0_ARB);

      glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,3,
                                 TREEMODE_9_TEXSIZE/TREEMODE_10_PERTURBSIZE,TREEMODE_10_PERTURBFX,
                                 2.0f,-1.0f);

      if (TREEMODE==12)
         glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,4,
                                    TREEMODE_12_ALPHASLOPE*TREEMODE_12_REDWGT/TREEMODE_12_ALPHATHRES,
                                    TREEMODE_12_ALPHASLOPE*TREEMODE_12_GREENWGT/TREEMODE_12_ALPHATHRES,
                                    TREEMODE_12_ALPHASLOPE*TREEMODE_12_BLUEWGT/TREEMODE_12_ALPHATHRES,
                                    -TREEMODE_12_ALPHASLOPE/2.0f);
      }

   glVertexPointer(3,GL_FLOAT,0,cache);
   glEnableClientState(GL_VERTEX_ARRAY);
   glTexCoordPointer(3,GL_FLOAT,0,coords);
   glEnableClientState(GL_TEXTURE_COORD_ARRAY);
   glDrawArrays(GL_TRIANGLES,0,cnt);
   glDisableClientState(GL_VERTEX_ARRAY);
   glDisableClientState(GL_TEXTURE_COORD_ARRAY);

   if (TREEMODE>=10)
      {
      glActiveTextureARB(GL_TEXTURE1_ARB);
      bindtexmap(0);
      glActiveTextureARB(GL_TEXTURE0_ARB);
      }

   bind3Dtexmap(0);

   glBindProgramARB(GL_VERTEX_PROGRAM_ARB,0);
   glDisable(GL_VERTEX_PROGRAM_ARB);

   glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,0);
   glDisable(GL_FRAGMENT_PROGRAM_ARB);

   glDisable(GL_BLEND);

   glPopMatrix();
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_TEXTURE);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);

   exitstate();

   vtx+=cnt;

#endif

#endif

   return(vtx);
   }

// render boundary of cached prisms
int minitree::renderprisms(float *cache,int cnt,float lambda,miniwarp *warp,
                           float tr,float tg,float tb,float ta)
   {
   int vtx=0;

#ifndef NOOGL

#if defined(GL_ARB_vertex_program) && defined(GL_ARB_fragment_program)

   static const char *vtxprog="!!ARBvp1.0 \n\
      PARAM c=program.env[0]; \n\
      PARAM mat[4]={state.matrix.mvp}; \n\
      TEMP vtx,col,pos; \n\
      MOV vtx,vertex.position.xywz; \n\
      MOV col.w,vtx.w; \n\
      MUL result.color.w,col.w,c.z; \n\
      MOV vtx.w,c.w; \n\
      DP4 pos.x,mat[0],vtx; \n\
      DP4 pos.y,mat[1],vtx; \n\
      DP4 pos.z,mat[2],vtx; \n\
      DP4 pos.w,mat[3],vtx; \n\
      MOV result.position,pos; \n\
      MUL result.texcoord[0].x,vtx.x,c.x; \n\
      MUL result.texcoord[0].y,vtx.z,c.x; \n\
      MOV result.color.xyz,vertex.color; \n\
      MOV result.fogcoord.x,pos.z; \n\
      END \n";

   static const char *fragprog1="!!ARBfp1.0 \n\
      PARAM c=program.env[0]; \n\
      TEMP col,pos; \n\
      MOV result.color.xyz,fragment.color; \n\
      MIN col.w,fragment.color.w,c.y; \n\
      MAD_SAT pos.z,fragment.fogcoord.x,c.z,c.w; \n\
      MUL result.color.w,col.w,pos.z; \n\
      END \n";

   static const char *fragprog2="!!ARBfp1.0 \n\
      PARAM c=program.env[0]; \n\
      TEMP col,pos; \n\
      TEX result.color.xyz,fragment.texcoord[0],texture[0],2D; \n\
      MIN col.w,fragment.color.w,c.y; \n\
      MAD_SAT pos.z,fragment.fogcoord.x,c.z,c.w; \n\
      MUL result.color.w,col.w,pos.z; \n\
      END \n";

   GLuint vtxprogid,fragprogid;

   GLfloat fogstart,fogend;

   miniv4d mtx[3];
   double oglmtx[16];

   unsigned char *image;
   int width,height,components;

   if (lambda<=0.0f || cnt==0) return(vtx);

   initwglprocs();

   initstate();

   glDisable(GL_CULL_FACE);
   glDepthMask(GL_FALSE);

   glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
   glEnable(GL_BLEND);

   glPushMatrix();
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glScalef(CONFIGURE_ZSCALE,CONFIGURE_ZSCALE,CONFIGURE_ZSCALE); // prevent Z-fighting
   glMatrixMode(GL_TEXTURE);
   glPushMatrix();
   glLoadIdentity();
   glMatrixMode(GL_MODELVIEW);

   if (warp!=NULL)
      {
      warp->getwarp(mtx);

      oglmtx[0]=mtx[0].x;
      oglmtx[4]=mtx[0].y;
      oglmtx[8]=mtx[0].z;
      oglmtx[12]=mtx[0].w;

      oglmtx[1]=mtx[1].x;
      oglmtx[5]=mtx[1].y;
      oglmtx[9]=mtx[1].z;
      oglmtx[13]=mtx[1].w;

      oglmtx[2]=mtx[2].x;
      oglmtx[6]=mtx[2].y;
      oglmtx[10]=mtx[2].z;
      oglmtx[14]=mtx[2].w;

      oglmtx[3]=0.0;
      oglmtx[7]=0.0;
      oglmtx[11]=0.0;
      oglmtx[15]=1.0;

      mtxmult(oglmtx);
      }

   if (TREEMODE==-2) glColor4f(1.0f,1.0f,1.0f,ta);
   else glColor4f(tr,tg,tb,ta);

   if (RENDERCACHE_VTXPROGID==0)
      {
      glGenProgramsARB(1,&vtxprogid);
      glBindProgramARB(GL_VERTEX_PROGRAM_ARB,vtxprogid);
      glProgramStringARB(GL_VERTEX_PROGRAM_ARB,GL_PROGRAM_FORMAT_ASCII_ARB,strlen(vtxprog),vtxprog);
      RENDERCACHE_VTXPROGID=vtxprogid;
      }

   glBindProgramARB(GL_VERTEX_PROGRAM_ARB,RENDERCACHE_VTXPROGID);
   glEnable(GL_VERTEX_PROGRAM_ARB);

   if (TREEMODE!=-2)
      {
      if (RENDERCACHE_FRAGPROGID1==0)
         {
         glGenProgramsARB(1,&fragprogid);
         glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,fragprogid);
         glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB,GL_PROGRAM_FORMAT_ASCII_ARB,strlen(fragprog1),fragprog1);
         RENDERCACHE_FRAGPROGID1=fragprogid;
         }

      glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,RENDERCACHE_FRAGPROGID1);
      glEnable(GL_FRAGMENT_PROGRAM_ARB);
      }
   else
      {
      if (RENDERCACHE_FRAGPROGID2==0)
         {
         glGenProgramsARB(1,&fragprogid);
         glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,fragprogid);
         glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB,GL_PROGRAM_FORMAT_ASCII_ARB,strlen(fragprog2),fragprog2);
         RENDERCACHE_FRAGPROGID2=fragprogid;
         }

      glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,RENDERCACHE_FRAGPROGID2);
      glEnable(GL_FRAGMENT_PROGRAM_ARB);
      }

   glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB,0,1.0f/TREEMODE_M2_TEXSIZE,0.0f,1.0f/(255.0f*lambda*TREEMODE_MX_BASE),1.0f);

   // assume linear fog
   glGetFloatv(GL_FOG_START,&fogstart);
   glGetFloatv(GL_FOG_END,&fogend);
   if (fogend<=fogstart) ERRORMSG();

   glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,0,0.0f,ta,-1.0f/(fogend-fogstart),fogend/(fogend-fogstart));

   if (TREEMODE==-2)
      {
      if (RENDERCACHE_TEXID==0)
         {
         if (TREEMODE_M2_TEXFILE==NULL) ERRORMSG();
         if ((image=readPNMfile(TREEMODE_M2_TEXFILE,&width,&height,&components))==NULL) ERRORMSG();
         if (components!=3) ERRORMSG();

         RENDERCACHE_TEXID=buildRGBtexmap(image,&width,&height);
         free(image);
         }

      bindtexmap(RENDERCACHE_TEXID);
      }

   glVertexPointer(4,GL_FLOAT,0,cache);
   glEnableClientState(GL_VERTEX_ARRAY);
   glDrawArrays(GL_TRIANGLES,0,3*cnt);
   glDisableClientState(GL_VERTEX_ARRAY);

   if (TREEMODE==-2) bindtexmap(0);

   glBindProgramARB(GL_VERTEX_PROGRAM_ARB,0);
   glDisable(GL_VERTEX_PROGRAM_ARB);

   glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,0);
   glDisable(GL_FRAGMENT_PROGRAM_ARB);

   glPopMatrix();
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_TEXTURE);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);

   exitstate();

   vtx+=3*cnt;

#endif

#endif

   return(vtx);
   }

// construct a pseudo 3D mipmap
unsigned char *minitree::build3Dmipmap(unsigned char *volume,
                                       int width,int height,int depth,int components,
                                       int levels)
   {
   int i;

   unsigned char *texture;

   int width2,height2;

   if ((texture=(unsigned char *)malloc(levels*width*height*depth*components))==NULL) ERRORMSG();

   width2=width;
   height2=height;

   put3Dtexture(texture,width,height,depth,components,
                volume,width2,height2);

   for (i=1; i<levels; i++)
      {
      if (width2>=4 && height2>=4)
         {
         volume=shrink3Dtexture(volume,width2,height2,depth,components);

         width2/=2;
         height2/=2;
         }

      put3Dtexture(&texture[i*width*height*depth*components],width,height,depth,components,
                   volume,width2,height2);
      }

   free(volume);

   return(texture);
   }

// construct the next pseudo 3D mipmap level
unsigned char *minitree::shrink3Dtexture(unsigned char *volume,
                                         int width,int height,int depth,int components)
   {
   int i,j,k,l;

   unsigned char *texture;

   int width2,height2;

   width2=width/2;
   height2=height/2;

   if ((texture=(unsigned char *)malloc(width2*height2*depth*components))==NULL) ERRORMSG();

   for (i=0; i<width2; i++)
      for (j=0; j<height2; j++)
         for (k=0; k<depth; k++)
            for (l=0; l<components; l++)
               texture[components*(i+(j+k*height2)*width2)+l]=(volume[components*(2*i+(2*j+k*height)*width)+l]+
                                                               volume[components*(2*i+1+(2*j+k*height)*width)+l]+
                                                               volume[components*(2*i+(2*j+1+k*height)*width)+l]+
                                                               volume[components*(2*i+1+(2*j+1+k*height)*width)+l]+2)/4;

   free(volume);

   return(texture);
   }

// interpolate a single pseudo 3D mipmap level
void minitree::put3Dtexture(unsigned char *texture,int width,int height,int depth,int components,
                            unsigned char *volume,int width2,int height2)
   {
   int i,j,k;

   for (i=0; i<width; i++)
      for (j=0; j<height; j++)
         for (k=0; k<depth; k++)
            put3Dcolor(&texture[components*(i+(j+k*height)*width)],components,
                       &volume[k*width2*height2*components],width2,height2,
                       (float)i/(width-1),(float)j/(height-1));
   }

// interpolate the color of the pseudo 3D mipmap
void minitree::put3Dcolor(unsigned char *color,int components,
                          unsigned char *slice,int width,int height,
                          float s,float t)
   {
   int i,j,k;

   float v,w;

   if (s<0.0f) s=0.0f;
   else if (s>1.0f) s=1.0f;

   if (t<0.0f) t=0.0f;
   else if (t>1.0f) t=1.0f;

   s*=width-1;
   t*=height-1;

   i=ftrc(s);
   j=ftrc(t);

   v=s-i;
   w=t-j;

   if (i==width-1)
      {
      i=width-2;
      v=1.0f;
      }

   if (j==height-1)
      {
      j=height-2;
      w=1.0f;
      }

   slice=&slice[components*(i+j*width)];

   for (k=0; k<components; k++)
      color[k]=ftrc((1.0f-w)*((1.0f-v)*slice[k]+v*slice[components+k])+
                    w*((1.0f-v)*slice[components*width+k]+v*slice[components*width+components+k])+0.5f);
   }

// deterministic random number generator
float minitree::pn_getrandom(float seed)
   {
   static const long maxbits=20;
   static const long maxnum=1<<20;

   static long number=0;

   if (seed>=0.0f && seed<=1.0f) number=ftrc(seed*(maxnum-1)+0.5f);

   number=271*(number+331);
   number=(number<<(maxbits/3))+(number>>(2*maxbits/3));
   number%=maxnum;

   return((float)number/(maxnum-1));
   }

// cubic interpolation
float minitree::pn_interpolateC(float v0,float v1,float v2,float v3,float x)
   {
   float p,q,r;

   p=v3-v2+v1-v0;
   q=v0-v1-p;
   r=v2-v0;

   return(((p*x+q)*x+r)*x+v1);
   }

// cubic 2D interpolation
float minitree::pn_interpolate2DC(float *octave,int size,float c1,float c2)
   {
   int k1,k2;
   float w1,w2;

   float v0,v1,v2,v3;

   k1=ftrc(c1*(size-1));
   w1=c1*(size-1)-k1;

   if (k1<0)
      {
      k1=0;
      w1=0.0f;
      }

   if (k1>size-2)
      {
      k1=size-2;
      w1=1.0f;
      }

   k2=ftrc(c2*(size-1));
   w2=c2*(size-1)-k2;

   if (k2<0)
      {
      k2=0;
      w2=0.0f;
      }

   if (k2>size-2)
      {
      k2=size-2;
      w2=1.0f;
      }

   v1=pn_interpolateC((k1>0)?octave[k1-1+k2*size]:octave[k1+k2*size],
                      octave[k1+k2*size],octave[k1+1+k2*size],
                      (k1<size-2)?octave[k1+2+k2*size]:octave[k1+1+k2*size],w1);

   if (k2>0)
      v0=pn_interpolateC((k1>0)?octave[k1-1+(k2-1)*size]:octave[k1+(k2-1)*size],
                         octave[k1+(k2-1)*size],octave[k1+1+(k2-1)*size],
                         (k1<size-2)?octave[k1+2+(k2-1)*size]:octave[k1+1+(k2-1)*size],w1);
   else v0=v1;

   v2=pn_interpolateC((k1>0)?octave[k1-1+(k2+1)*size]:octave[k1+(k2+1)*size],
                      octave[k1+(k2+1)*size],octave[k1+1+(k2+1)*size],
                      (k1<size-2)?octave[k1+2+(k2+1)*size]:octave[k1+1+(k2+1)*size],w1);

   if (k2<size-2)
      v3=pn_interpolateC((k1>0)?octave[k1-1+(k2+2)*size]:octave[k1+(k2+2)*size],
                         octave[k1+(k2+2)*size],octave[k1+1+(k2+2)*size],
                         (k1<size-2)?octave[k1+2+(k2+2)*size]:octave[k1+1+(k2+2)*size],w1);
   else v3=v2;

   return(pn_interpolateC(v0,v1,v2,v3,w2));
   }

// generate 2D Perlin Noise
unsigned char *minitree::pn_perlin2D(int size,int start,float persist,float seed)
   {
   int i,j,k;

   float scaling,maxr;

   float *noise,*octave;

   unsigned char *image;

   if ((size&(size-1))!=0 || size<2) ERRORMSG();
   if ((start&(start-1))!=0 || start<2 || start>size) ERRORMSG();

   if ((noise=(float *)malloc(size*size*sizeof(float)))==NULL) ERRORMSG();
   for (i=0; i<size*size; i++) noise[i]=0.0f;

   if ((octave=(float *)malloc(size*size*sizeof(float)))==NULL) ERRORMSG();

   pn_getrandom(seed);

   scaling=1.0f;

   for (i=start; i<=size; i*=2)
      {
      for (j=0; j<i-1; j++)
         for (k=0; k<i-1; k++) octave[j+k*i]=(2.0f*pn_getrandom()-1.0f)*scaling;

      // replicate edge and corner values for seamless tiling
      for (j=0; j<i-1; j++) octave[j+(i-1)*i]=octave[j];
      for (k=0; k<i; k++) octave[i-1+k*i]=octave[k*i];

      for (j=0; j<size; j++)
         for (k=0; k<size; k++)
            noise[j+k*size]+=pn_interpolate2DC(octave,i,
                                               (float)j/(size-1),
                                               (float)k/(size-1));

      scaling*=persist;
      }

   free(octave);

   maxr=1.0f;

   for (i=0; i<size*size; i++)
      if (fabs(noise[i])>maxr) maxr=fabs(noise[i]);

   maxr*=2.0f;

   for (i=0; i<size*size; i++) noise[i]=noise[i]/maxr+0.5f;

   if ((image=(unsigned char *)malloc(size*size))==NULL) ERRORMSG();

   for (i=0; i<size*size; i++)
      image[i]=ftrc(255.0f*noise[i]+0.5f);

   free(noise);

   return(image);
   }

// Windows OpenGL extension setup
void minitree::initwglprocs()
   {
#ifndef NOOGL

#ifdef _WIN32

   if (WGLSETUP==0)
      {
#ifdef GL_ARB_multitexture
      if ((glActiveTextureARB=(PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress("glActiveTextureARB"))==NULL) ERRORMSG();
#endif

#if defined(GL_ARB_vertex_program) && defined(GL_ARB_fragment_program)
      if ((glGenProgramsARB=(PFNGLGENPROGRAMSARBPROC)wglGetProcAddress("glGenProgramsARB"))==NULL) ERRORMSG();
      if ((glBindProgramARB=(PFNGLBINDPROGRAMARBPROC)wglGetProcAddress("glBindProgramARB"))==NULL) ERRORMSG();
      if ((glProgramStringARB=(PFNGLPROGRAMSTRINGARBPROC)wglGetProcAddress("glProgramStringARB"))==NULL) ERRORMSG();
      if ((glProgramEnvParameter4fARB=(PFNGLPROGRAMENVPARAMETER4FARBPROC)wglGetProcAddress("glProgramEnvParameter4fARB"))==NULL) ERRORMSG();
      if ((glDeleteProgramsARB=(PFNGLDELETEPROGRAMSARBPROC)wglGetProcAddress("glDeleteProgramsARB"))==NULL) ERRORMSG();
#endif

      WGLSETUP=1;
      }

#endif

#endif
   }
