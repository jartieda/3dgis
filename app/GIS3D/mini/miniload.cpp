// (c) by Stefan Roettger

#include "minibase.h"

#include "miniio.h"
#include "miniutm.h"

#include "pnmbase.h"
#include "pnmsample.h"

#include "miniload.h"

// initialize
void miniload::init()
   {
   TILE=NULL;

   RELSCALE=1.0f;
   SEALEVEL=-MAXFLOAT;

   REQUEST_CALLBACK=NULL;
   PRELOAD_CALLBACK=NULL;
   DELIVER_CALLBACK=NULL;
   REQUEST_DATA=NULL;

   INQUIRY_CALLBACK=NULL;
   INQUIRY_DATA=NULL;

   QUERY_CALLBACK=NULL;
   QUERY_DATA=NULL;

   MAX_STR=1024;

   PAGING=0;

   PFARP=0.0f;
   PRANGE=PRANGE0=0.0f;
   PBASESIZE=0;
   PUPDATE=0;

   PRADIUS=PRADIUS0=0.0f;
   PDROPOFF=0.0f;

   PRELOD=PRETEX=0;
   LAZYNESS=LAZYTEX=0;

   RELOADED=0;

   CONFIGURE_MINRES=9.0f;
   CONFIGURE_MAXD2=100.0f;
   CONFIGURE_SEAD2=0.1f;
   CONFIGURE_MINOFF=0.1f;
   CONFIGURE_MAXCULL=8;

   CONFIGURE_SEAMIN=-MAXFLOAT;
   CONFIGURE_SEAMAX=-MAXFLOAT;
   CONFIGURE_MIPMAPS=1;

   CONFIGURE_SZONE=0.01f;
   CONFIGURE_TSAFETY=0.5f;
   CONFIGURE_MINLAZY=0.25f;
   CONFIGURE_MAXTEXLOD=10;
   CONFIGURE_MAXMAPLOD=10;
   CONFIGURE_DONTFREE=0;

   CONFIGURE_USEZERONUMBERING=0;
   CONFIGURE_TILESETPATH=NULL;
   CONFIGURE_USEPNM=1;

   EXPIRE=0;
   ECOL=EROW=0;

   RES=MAXFLOAT;

   FOCUS=FALSE;
   }

// default constructor
miniload::miniload()
   {init();}

// constructor
miniload::miniload(unsigned char **hfields,unsigned char **textures,
                   int cols,int rows,float coldim,float rowdim,float scale,
                   float centerx,float centery,float centerz,
                   unsigned char **fogmaps,float lambda,float displace,
                   float emission,float attenuation,
                   float fogR,float fogG,float fogB)
   {
   int i;

   init();

   if (hfields==NULL || textures==NULL) ERRORMSG();

   TILE=new minitile(NULL,NULL,
                     cols,rows,coldim,rowdim,scale,
                     centerx,centery,centerz,
                     NULL,lambda,displace,
                     emission,attenuation,
                     fogR,fogG,fogB);

   TILE->setrequest(request,this,preload,PFARP,PUPDATE);

   COLS=cols;
   ROWS=rows;

   COLDIM=coldim;
   ROWDIM=rowdim;
   SCALE=scale;

   CENTERX=centerx;
   CENTERY=centery;
   CENTERZ=centerz;

   if ((HFIELDS=(unsigned char **)malloc(COLS*ROWS*sizeof(unsigned char *)))==NULL) ERRORMSG();

   for (i=0; i<COLS*ROWS; i++)
      if (hfields[i]!=NULL) HFIELDS[i]=(unsigned char *)strdup((char *)hfields[i]);
      else HFIELDS[i]=NULL;

   if ((TEXTURES=(unsigned char **)malloc(COLS*ROWS*sizeof(unsigned char *)))==NULL) ERRORMSG();

   for (i=0; i<COLS*ROWS; i++)
      if (textures[i]!=NULL) TEXTURES[i]=(unsigned char *)strdup((char *)textures[i]);
      else TEXTURES[i]=NULL;

   if (fogmaps==NULL) FOGMAPS=NULL;
   else
      {
      if ((FOGMAPS=(unsigned char **)malloc(COLS*ROWS*sizeof(unsigned char *)))==NULL) ERRORMSG();

      for (i=0; i<COLS*ROWS; i++)
         if (fogmaps[i]!=NULL) FOGMAPS[i]=(unsigned char *)strdup((char *)fogmaps[i]);
         else FOGMAPS[i]=NULL;
      }

   if ((MANDATORY=(int *)malloc(COLS*ROWS*sizeof(int)))==NULL) ERRORMSG();

   for (i=0; i<COLS*ROWS; i++) MANDATORY[i]=1;

   if ((LODS=(unsigned char **)malloc(COLS*ROWS*sizeof(unsigned char *)))==NULL) ERRORMSG();

   if ((MAXLOD=(int *)malloc(COLS*ROWS*sizeof(int)))==NULL) ERRORMSG();
   if ((ACTLOD=(int *)malloc(COLS*ROWS*sizeof(int)))==NULL) ERRORMSG();
   if ((NEWLOD=(int *)malloc(COLS*ROWS*sizeof(int)))==NULL) ERRORMSG();

   for (i=0; i<COLS*ROWS; i++)
      {
      LODS[i]=NULL;
      MAXLOD[i]=-1;
      }

   if ((TEXS=(unsigned char **)malloc(COLS*ROWS*sizeof(unsigned char *)))==NULL) ERRORMSG();

   if ((MAXTEX=(int *)malloc(COLS*ROWS*sizeof(int)))==NULL) ERRORMSG();
   if ((ACTTEX=(int *)malloc(COLS*ROWS*sizeof(int)))==NULL) ERRORMSG();
   if ((NEWTEX=(int *)malloc(COLS*ROWS*sizeof(int)))==NULL) ERRORMSG();
   if ((MINTEX=(int *)malloc(COLS*ROWS*sizeof(int)))==NULL) ERRORMSG();

   for (i=0; i<COLS*ROWS; i++)
      {
      TEXS[i]=NULL;
      MAXTEX[i]=-1;
      }

   if ((FOGS=(unsigned char **)malloc(COLS*ROWS*sizeof(unsigned char *)))==NULL) ERRORMSG();

   if ((MAXFOG=(int *)malloc(COLS*ROWS*sizeof(int)))==NULL) ERRORMSG();

   for (i=0; i<COLS*ROWS; i++)
      {
      FOGS[i]=NULL;
      MAXFOG[i]=-1;
      }

   TIME=0;

   if ((LRU=(int *)malloc(COLS*ROWS*sizeof(int)))==NULL) ERRORMSG();
   for (i=0; i<COLS*ROWS; i++) LRU[i]=TIME;
   }

// cleanup
void miniload::cleanup()
   {
   int i;

   for (i=0; i<COLS*ROWS; i++)
      if (HFIELDS[i]!=NULL) free(HFIELDS[i]);

   free(HFIELDS);

   for (i=0; i<COLS*ROWS; i++)
      if (TEXTURES[i]!=NULL) free(TEXTURES[i]);

   free(TEXTURES);

   if (FOGMAPS!=NULL)
      {
      for (i=0; i<COLS*ROWS; i++)
         if (FOGMAPS[i]!=NULL) free(FOGMAPS[i]);

      free(FOGMAPS);
      }

   free(MANDATORY);

   for (i=0; i<COLS*ROWS; i++)
      if (LODS[i]!=NULL) free(LODS[i]);

   free(LODS);

   free(MAXLOD);
   free(ACTLOD);
   free(NEWLOD);

   for (i=0; i<COLS*ROWS; i++)
      if (TEXS[i]!=NULL) free(TEXS[i]);

   free(TEXS);

   free(MAXTEX);
   free(ACTTEX);
   free(NEWTEX);
   free(MINTEX);

   for (i=0; i<COLS*ROWS; i++)
      if (FOGS[i]!=NULL) free(FOGS[i]);

   free(FOGS);

   free(MAXFOG);

   free(LRU);
   }

// destructor
miniload::~miniload()
   {
   if (TILE!=NULL)
      {
      delete TILE;
      cleanup();
      }

   if (CONFIGURE_TILESETPATH!=NULL) free(CONFIGURE_TILESETPATH);
   }

// set texture mapping mode
void miniload::settexmode(int texmode)
   {
   if (TILE==NULL) ERRORMSG();
   TILE->settexmode(texmode);
   }

// set fog rendering mode
void miniload::setfogmode(int fogmode)
   {
   if (TILE==NULL) ERRORMSG();
   TILE->setfogmode(fogmode);
   }

// update actual LOD
unsigned char *miniload::updatelod(int col,int row,int lodinc)
   {
   int lod;
   unsigned char *newlod;

   if (HFIELDS[col+row*COLS]==NULL) return(NULL);

   lod=min(max(ACTLOD[col+row*COLS]+lodinc,0),min(MAXLOD[col+row*COLS],CONFIGURE_MAXMAPLOD));

   if (TILE->isloaded(col,row)!=0)
      if (lod==NEWLOD[col+row*COLS]) return(NULL);

   if (lod==0) newlod=(unsigned char *)strdup((char *)HFIELDS[col+row*COLS]);
   else
      {
      if ((newlod=(unsigned char *)malloc(MAX_STR))==NULL) ERRORMSG();
      snprintf((char *)newlod,MAX_STR,"%s%d",(char *)HFIELDS[col+row*COLS],lod);
      }

   NEWLOD[col+row*COLS]=lod;

   return(newlod);
   }

// update actual fog LOD
void miniload::updatefog(int col,int row)
   {
   int lod;
   unsigned char *newlod;

   if (FOGMAPS==NULL) return;
   if (FOGMAPS[col+row*COLS]==NULL) return;

   lod=min(NEWLOD[col+row*COLS],MAXFOG[col+row*COLS]);

   if (lod==0) newlod=(unsigned char *)strdup((char *)FOGMAPS[col+row*COLS]);
   else
      {
      if ((newlod=(unsigned char *)malloc(MAX_STR))==NULL) ERRORMSG();
      snprintf((char *)newlod,MAX_STR,"%s%d",(char *)FOGMAPS[col+row*COLS],lod);
      }

   if (FOGS[col+row*COLS]!=NULL) free(FOGS[col+row*COLS]);
   FOGS[col+row*COLS]=newlod;
   }

// check existence of LODs and calculate LOD increment
void miniload::checklods(int col,int row,int prelod)
   {
   BOOLINT check;

   int unused,lodinc;
   unsigned char *newlod;

   int present;

   if (MAXLOD[col+row*COLS]<0)
      {
      MAXLOD[col+row*COLS]=ACTLOD[col+row*COLS]=NEWLOD[col+row*COLS]=0;

      if (HFIELDS[col+row*COLS]==NULL) return;

      if (REQUEST_CALLBACK==NULL) present=(checkfile((char *)HFIELDS[col+row*COLS])!=0);
      else present=(REQUEST_CALLBACK(col,row,HFIELDS[col+row*COLS],0,NULL,0,NULL,REQUEST_DATA,NULL,NULL,NULL)!=0);

      if (present==0)
         {
         free(HFIELDS[col+row*COLS]);
         HFIELDS[col+row*COLS]=NULL;

         return;
         }

      LODS[col+row*COLS]=(unsigned char *)strdup((char *)HFIELDS[col+row*COLS]);

      check=TRUE;

      while (check)
         {
         MAXLOD[col+row*COLS]++;
         newlod=updatelod(col,row,1);
         ACTLOD[col+row*COLS]=NEWLOD[col+row*COLS];

         if (REQUEST_CALLBACK==NULL) present=(checkfile((char *)newlod)!=0);
         else present=(REQUEST_CALLBACK(col,row,newlod,NEWLOD[col+row*COLS],NULL,0,NULL,REQUEST_DATA,NULL,NULL,NULL)!=0);

         if (present==0)
            {
            check=FALSE;
            ACTLOD[col+row*COLS]=NEWLOD[col+row*COLS]=--MAXLOD[col+row*COLS];
            free(newlod);
            }
         else
            {
            free(LODS[col+row*COLS]);
            LODS[col+row*COLS]=newlod;
            }
         }

      if (FOGMAPS!=NULL)
         if (FOGMAPS[col+row*COLS]!=NULL)
            {
            MAXFOG[col+row*COLS]=MAXLOD[col+row*COLS];
            NEWLOD[col+row*COLS]=1;

            while (NEWLOD[col+row*COLS]<=MAXFOG[col+row*COLS])
               {
               updatefog(col,row);

               if (REQUEST_CALLBACK==NULL) present=(checkfile((char *)FOGS[col+row*COLS])!=0);
               else present=(REQUEST_CALLBACK(col,row,NULL,NEWLOD[col+row*COLS],NULL,0,FOGS[col+row*COLS],REQUEST_DATA,NULL,NULL,NULL)!=0);

               if (present==0)
                  {
                  MAXFOG[col+row*COLS]=--NEWLOD[col+row*COLS];
                  break;
                  }

               NEWLOD[col+row*COLS]++;
               }

            NEWLOD[col+row*COLS]=MAXLOD[col+row*COLS];
            updatefog(col,row);
            }
      }

   if (MAXLOD[col+row*COLS]>0)
      {
      if (PAGING==0 || RES==MAXFLOAT) unused=-MAXLOD[col+row*COLS];
      else unused=TILE->getunusedlods(col,row,RES,FX,FY,FZ);

      if (TILE->isloaded(col,row)==0 && PAGING!=0) lodinc=0;
      else if (unused<=1+prelod) lodinc=unused-1-prelod;
      else if (prelod==0) lodinc=max(unused-1-PRELOD-LAZYNESS,0);
      else lodinc=max(unused-PRELOD-LAZYNESS,0);

      newlod=updatelod(col,row,lodinc);

      if (newlod!=NULL)
         {
         free(LODS[col+row*COLS]);
         LODS[col+row*COLS]=newlod;

         updatefog(col,row);
         }
      }
   }

// update actual texture LOD
unsigned char *miniload::updatetex(int col,int row,int texinc)
   {
   int tex;
   unsigned char *newtex;

   if (TEXTURES[col+row*COLS]==NULL) return(NULL);

   tex=min(max(ACTTEX[col+row*COLS]+texinc,0),MAXTEX[col+row*COLS]);

   if (TILE->gettexid(col,row)!=0)
      if (tex==NEWTEX[col+row*COLS]) return(NULL);

   if (tex==0) newtex=(unsigned char *)strdup((char *)TEXTURES[col+row*COLS]);
   else
      {
      if ((newtex=(unsigned char *)malloc(MAX_STR))==NULL) ERRORMSG();
      snprintf((char *)newtex,MAX_STR,"%s%d",(char *)TEXTURES[col+row*COLS],tex);
      }

   NEWTEX[col+row*COLS]=tex;

   return(newtex);
   }

// check existence of texture LODs and calculate LOD increment
void miniload::checktexs(int col,int row,int pretex)
   {
   BOOLINT check;

   float px,pz,boff;
   float sizex2,sizez2;
   float minvalue,maxvalue;
   float dist,diff;

   int unused,texinc;
   unsigned char *newtex;

   int present;

   unsigned char *image;
   int width,height,components;

   databuf hfield,texture,fogmap;

   if (MAXTEX[col+row*COLS]<0)
      {
      MAXTEX[col+row*COLS]=ACTTEX[col+row*COLS]=NEWTEX[col+row*COLS]=0;

      if (HFIELDS[col+row*COLS]==NULL || TEXTURES[col+row*COLS]==NULL) return;

      if (REQUEST_CALLBACK==NULL) present=(checkfile((char *)TEXTURES[col+row*COLS])!=0);
      else present=(REQUEST_CALLBACK(col,row,NULL,0,TEXTURES[col+row*COLS],0,NULL,REQUEST_DATA,NULL,NULL,NULL)!=0);

      if (present==0)
         {
         free(TEXTURES[col+row*COLS]);
         TEXTURES[col+row*COLS]=NULL;

         return;
         }

      TEXS[col+row*COLS]=(unsigned char *)strdup((char *)TEXTURES[col+row*COLS]);

      check=TRUE;

      while (check)
         {
         MAXTEX[col+row*COLS]++;
         newtex=updatetex(col,row,1);
         ACTTEX[col+row*COLS]=NEWTEX[col+row*COLS];

         if (REQUEST_CALLBACK==NULL) present=(checkfile((char *)newtex)!=0);
         else present=(REQUEST_CALLBACK(col,row,NULL,0,newtex,NEWTEX[col+row*COLS],NULL,REQUEST_DATA,NULL,NULL,NULL)!=0);

         if (present==0)
            {
            check=FALSE;
            ACTTEX[col+row*COLS]=NEWTEX[col+row*COLS]=MINTEX[col+row*COLS]=--MAXTEX[col+row*COLS];
            free(newtex);

            if (QUERY_CALLBACK!=NULL)
               QUERY_CALLBACK(col,row,TEXS[col+row*COLS],NEWTEX[col+row*COLS],QUERY_DATA,
                              &width,&height);
            else
               if (REQUEST_CALLBACK==NULL)
                  {
                  if ((image=readPNMfile((char *)TEXS[col+row*COLS],&width,&height,&components))==NULL) ERRORMSG();
                  free(image);
                  }
               else
                  {
                  hfield.reset();
                  texture.reset();
                  fogmap.reset();

                  REQUEST_CALLBACK(col,row,NULL,0,TEXS[col+row*COLS],NEWTEX[col+row*COLS],NULL,REQUEST_DATA,
                                   &hfield,&texture,&fogmap);

                  if (!hfield.missing() || texture.missing() || !fogmap.missing()) ERRORMSG();

                  width=texture.xsize;
                  height=texture.ysize;

                  if (CONFIGURE_DONTFREE==0) texture.release();
                  }

            while (width>2 && height>2)
               {
               width/=2;
               height/=2;
               MINTEX[col+row*COLS]++;
               }
            }
         else
            {
            free(TEXS[col+row*COLS]);
            TEXS[col+row*COLS]=newtex;
            }
         }
      }

   if (MAXTEX[col+row*COLS]>0)
      {
      if (PRANGE==0.0f || RES==MAXFLOAT) unused=-MAXTEX[col+row*COLS];
      else
         {
         px=FX;
         pz=FZ;

         sizex2=TILE->getcoldim()/2.0f*(1.0f-CONFIGURE_SZONE);
         sizez2=TILE->getrowdim()/2.0f*(1.0f-CONFIGURE_SZONE);

         if (px<TILE->getcenterx(col)-sizex2) px=TILE->getcenterx(col)-sizex2;
         else if (px>TILE->getcenterx(col)+sizex2) px=TILE->getcenterx(col)+sizex2;

         if (pz<TILE->getcenterz(row)-sizez2) pz=TILE->getcenterz(row)-sizez2;
         else if (pz>TILE->getcenterz(row)+sizez2) pz=TILE->getcenterz(row)+sizez2;

         boff=fabs(FY-CENTERY);

         if (TILE->isloaded(col,row)!=0) boff=fabs(FY-TILE->getheight(px,pz,1));
         else if (INQUIRY_CALLBACK!=NULL)
            if (INQUIRY_CALLBACK(col,row,LODS[col+row*COLS],ACTLOD[col+row*COLS],INQUIRY_DATA,&minvalue,&maxvalue)!=0)
               {
               minvalue=minvalue*SCALE*RELSCALE+CENTERY;
               maxvalue=maxvalue*SCALE*RELSCALE+CENTERY;

               if (FY>minvalue) boff=FY-minvalue;
               else boff=maxvalue-FY;
               }

         dist=fsqrt(fsqr(FX-px)+fsqr(FZ-pz)+fsqr(boff));

         if (CONFIGURE_TSAFETY>0.0f) dist=fmax(dist-CONFIGURE_TSAFETY*fmax(TILE->getcoldim(),TILE->getrowdim()),0.0f);

         diff=flog(dist/PRANGE+1.0f)/flog(2.0f); // linear LOD term
         if (PRADIUS>0.0f) diff+=fmin(dist/PRADIUS,1.0f)*PDROPOFF; // non-linear LOD term
         diff=fmin(diff,CONFIGURE_MAXTEXLOD)-ACTTEX[col+row*COLS]; // LOD difference

         if (diff>=0.0f) unused=ftrc(fmax(diff-CONFIGURE_MINLAZY/2.0f,0.0f));
         else unused=-ftrc(1.0f-CONFIGURE_MINLAZY/2.0f-diff);
         }

      if (PBASESIZE>0) unused-=ftrc(flog(PBASESIZE)/flog(2.0f)+0.5f)-1-MINTEX[col+row*COLS];

      if (unused<=pretex) texinc=unused-pretex;
      else if (pretex==0) texinc=max(unused-PRETEX-LAZYTEX,0);
      else texinc=max(unused-PRETEX-LAZYTEX+1,0);

      newtex=updatetex(col,row,texinc);

      if (newtex!=NULL)
         {
         free(TEXS[col+row*COLS]);
         TEXS[col+row*COLS]=newtex;
         }
      }
   }

// request callback
void miniload::request(int col,int row,int needtex,void *data)
   {
   miniload *obj=(miniload *)data;
   minitile *tile=obj->getminitile();
   int cols=obj->COLS;

   int oldlodinc,newlodinc;
   int updatetex;

   databuf hfield,texture,fogmap;

   if (obj->HFIELDS[col+row*cols]==NULL) return;

   obj->LRU[col+row*cols]=obj->TIME;

   minitile::configure_minres(obj->CONFIGURE_MINRES);
   minitile::configure_maxd2(obj->CONFIGURE_MAXD2);
   minitile::configure_sead2(obj->CONFIGURE_SEAD2);
   minitile::configure_minoff(obj->CONFIGURE_MINOFF);
   minitile::configure_maxcull(obj->CONFIGURE_MAXCULL);

   minitile::configure_seamin(obj->CONFIGURE_SEAMIN);
   minitile::configure_seamax(obj->CONFIGURE_SEAMAX);
   minitile::configure_mipmaps(obj->CONFIGURE_MIPMAPS);

   obj->checklods(col,row,0);
   if (needtex>0) obj->checktexs(col,row,0);

   if (obj->PRELOAD_CALLBACK!=NULL)
      {
      if (tile->isloaded(col,row)!=0 && tile->gettexid(col,row)!=0) return;
      if (obj->MANDATORY[col+row*cols]==0) return;
      }

   oldlodinc=newlodinc=obj->NEWLOD[col+row*cols]-obj->ACTLOD[col+row*cols];

   updatetex=0;

   if (needtex>0)
      if (obj->TEXTURES[col+row*cols]!=NULL && tile->gettexid(col,row)==0) updatetex=1;
      else if (obj->NEWTEX[col+row*cols]!=obj->ACTTEX[col+row*cols]) updatetex=1;

   while (tile->isloaded(col,row)==0 || (newlodinc!=0 && needtex>=0) || updatetex!=0)
      {
      if (obj->REQUEST_CALLBACK==NULL)
         {
         if (obj->FOGMAPS==NULL) tile->reload(col,row,obj->LODS[col+row*cols],obj->TEXS[col+row*cols],NULL,updatetex);
         else tile->reload(col,row,obj->LODS[col+row*cols],obj->TEXS[col+row*cols],obj->FOGS[col+row*cols],updatetex);

         obj->ACTLOD[col+row*cols]=obj->NEWLOD[col+row*cols];
         if (updatetex!=0) obj->ACTTEX[col+row*cols]=obj->NEWTEX[col+row*cols];
         }
      else
         {
         hfield.reset();
         texture.reset();
         fogmap.reset();

         if (updatetex!=0)
            if (obj->FOGMAPS==NULL)
               obj->REQUEST_CALLBACK(col,row,obj->LODS[col+row*cols],obj->NEWLOD[col+row*cols],obj->TEXS[col+row*cols],obj->NEWTEX[col+row*cols],NULL,obj->REQUEST_DATA,
                                     &hfield,&texture,&fogmap);
            else
               obj->REQUEST_CALLBACK(col,row,obj->LODS[col+row*cols],obj->NEWLOD[col+row*cols],obj->TEXS[col+row*cols],obj->NEWTEX[col+row*cols],obj->FOGS[col+row*cols],obj->REQUEST_DATA,
                                     &hfield,&texture,&fogmap);
         else
            if (obj->FOGMAPS==NULL)
               obj->REQUEST_CALLBACK(col,row,obj->LODS[col+row*cols],obj->NEWLOD[col+row*cols],NULL,0,NULL,obj->REQUEST_DATA,
                                     &hfield,&texture,&fogmap);
            else
               obj->REQUEST_CALLBACK(col,row,obj->LODS[col+row*cols],obj->NEWLOD[col+row*cols],NULL,0,obj->FOGS[col+row*cols],obj->REQUEST_DATA,
                                     &hfield,&texture,&fogmap);

         if (hfield.missing()) break;

         if (updatetex!=0)
            {
            tile->reload(col,row,hfield,texture,fogmap,1);

            obj->ACTLOD[col+row*cols]=obj->NEWLOD[col+row*cols];
            obj->ACTTEX[col+row*cols]=obj->NEWTEX[col+row*cols];
            }
         else
            {
            tile->reload(col,row,hfield,texture,fogmap,0);
            obj->ACTLOD[col+row*cols]=obj->NEWLOD[col+row*cols];
            }

         if (obj->CONFIGURE_DONTFREE==0)
            {
            hfield.release();
            texture.release();
            fogmap.release();
            }
         }

      if (oldlodinc<0 && newlodinc>0) break;
      if (oldlodinc>0 && newlodinc<0) break;

      obj->checklods(col,row,0);

      newlodinc=obj->NEWLOD[col+row*cols]-obj->ACTLOD[col+row*cols];

      updatetex=0;
      }
   }

// preload callback
void miniload::preload(int col,int row,void *data)
   {
   miniload *obj=(miniload *)data;
   minitile *tile=obj->getminitile();
   int cols=obj->COLS;

   int updatelod,updatetex;

   if (obj->HFIELDS[col+row*cols]==NULL) return;

   if (obj->PRELOAD_CALLBACK==NULL)
      {
      request(col,row,1,data);
      return;
      }

   minitile::configure_minres(obj->CONFIGURE_MINRES);
   minitile::configure_maxd2(obj->CONFIGURE_MAXD2);
   minitile::configure_sead2(obj->CONFIGURE_SEAD2);
   minitile::configure_minoff(obj->CONFIGURE_MINOFF);
   minitile::configure_maxcull(obj->CONFIGURE_MAXCULL);

   minitile::configure_seamin(obj->CONFIGURE_SEAMIN);
   minitile::configure_seamax(obj->CONFIGURE_SEAMAX);
   minitile::configure_mipmaps(obj->CONFIGURE_MIPMAPS);

   obj->checklods(col,row,obj->PRELOD);
   obj->checktexs(col,row,obj->PRETEX);

   if (tile->isloaded(col,row)==0 || tile->isloaded(col,row)==0)
      if (obj->MANDATORY[col+row*cols]!=0) return;

   updatelod=updatetex=0;

   if (tile->isloaded(col,row)==0 || obj->ACTLOD[col+row*cols]!=obj->NEWLOD[col+row*cols]) updatelod=1;
   if ((obj->TEXTURES[col+row*cols]!=NULL && tile->gettexid(col,row)==0) || obj->ACTTEX[col+row*cols]!=obj->NEWTEX[col+row*cols]) updatetex=1;

   if (updatelod!=0 || updatetex!=0)
      if (updatetex!=0)
         if (obj->FOGMAPS==NULL)
            obj->PRELOAD_CALLBACK(col,row,
                                  obj->LODS[col+row*cols],obj->NEWLOD[col+row*cols],
                                  obj->TEXS[col+row*cols],obj->NEWTEX[col+row*cols],
                                  NULL,obj->REQUEST_DATA);
         else
            obj->PRELOAD_CALLBACK(col,row,
                                  obj->LODS[col+row*cols],obj->NEWLOD[col+row*cols],
                                  obj->TEXS[col+row*cols],obj->NEWTEX[col+row*cols],
                                  obj->FOGS[col+row*cols],obj->REQUEST_DATA);
      else
         if (obj->FOGMAPS==NULL)
            obj->PRELOAD_CALLBACK(col,row,
                                  obj->LODS[col+row*cols],obj->NEWLOD[col+row*cols],
                                  NULL,0,NULL,obj->REQUEST_DATA);
         else
            obj->PRELOAD_CALLBACK(col,row,
                                  obj->LODS[col+row*cols],obj->NEWLOD[col+row*cols],
                                  NULL,0,obj->FOGS[col+row*cols],obj->REQUEST_DATA);
   }

// deliver preloaded tiles
void miniload::deliver()
   {
   int col,row;
   databuf hfield,texture,fogmap;
   int hlod,tlod;

   int updatelod,updatetex;

   minitile::configure_minres(CONFIGURE_MINRES);
   minitile::configure_maxd2(CONFIGURE_MAXD2);
   minitile::configure_sead2(CONFIGURE_SEAD2);
   minitile::configure_minoff(CONFIGURE_MINOFF);
   minitile::configure_maxcull(CONFIGURE_MAXCULL);

   minitile::configure_seamin(CONFIGURE_SEAMIN);
   minitile::configure_seamax(CONFIGURE_SEAMAX);
   minitile::configure_mipmaps(CONFIGURE_MIPMAPS);

   if (DELIVER_CALLBACK!=NULL)
      {
      hfield.reset();
      texture.reset();
      fogmap.reset();

      DELIVER_CALLBACK(&col,&row,
                       &hfield,&hlod,&texture,&tlod,&fogmap,
                       REQUEST_DATA);

      while (!hfield.missing())
         if (HFIELDS[col+row*COLS]!=NULL)
            {
            updatelod=updatetex=0;

            if (TILE->isloaded(col,row)==0) updatelod=1;
            else if (hlod!=ACTLOD[col+row*COLS]) updatelod=1;

            if (!texture.missing())
               if (TEXTURES[col+row*COLS]!=NULL)
                  if (TILE->gettexid(col,row)==0) updatetex=1;
                  else if (tlod!=ACTTEX[col+row*COLS]) updatetex=1;

            if (updatelod!=0 || updatetex!=0)
               {
               TILE->reload(col,row,hfield,texture,fogmap,updatetex);

               ACTLOD[col+row*COLS]=hlod;
               if (updatetex!=0) ACTTEX[col+row*COLS]=tlod;

               LRU[col+row*COLS]=TIME;
               }

            if (CONFIGURE_DONTFREE==0)
               {
               hfield.release();
               texture.release();
               fogmap.release();
               }

            hfield.reset();
            texture.reset();
            fogmap.reset();

            DELIVER_CALLBACK(&col,&row,
                             &hfield,&hlod,&texture,&tlod,&fogmap,
                             REQUEST_DATA);
            }
      }
   }

// remove expired tiles
void miniload::expire()
   {
   int i;

   int steps;

   if (EXPIRE>0)
      {
      if (PUPDATE==0) steps=1;
      else steps=max(COLS*ROWS/PUPDATE,1);

      for (i=0; i<steps; i++)
         {
         if (TILE->isloaded(ECOL,EROW)!=0)
            if (TILE->ispreloaded(ECOL,EROW)==0)
               if (TIME-LRU[ECOL+EROW*COLS]>EXPIRE) TILE->reload(ECOL,EROW,NULL,NULL,NULL);

         if (++ECOL>=COLS)
            {
            ECOL=0;
            if (++EROW>=ROWS) EROW=0;
            }
         }
      }
   }

// render tiled terrain
void miniload::draw(float res,
                    float ex,float ey,float ez,
                    float dx,float dy,float dz,
                    float ux,float uy,float uz,
                    float fovy,float aspect,
                    float nearp,float farp,
                    int update)
   {
   if (TILE==NULL) ERRORMSG();

   minitile::configure_minres(CONFIGURE_MINRES);
   minitile::configure_maxd2(CONFIGURE_MAXD2);
   minitile::configure_sead2(CONFIGURE_SEAD2);
   minitile::configure_minoff(CONFIGURE_MINOFF);
   minitile::configure_maxcull(CONFIGURE_MAXCULL);

   minitile::configure_seamin(CONFIGURE_SEAMIN);
   minitile::configure_seamax(CONFIGURE_SEAMAX);
   minitile::configure_mipmaps(CONFIGURE_MIPMAPS);

   RES=res;

   if (!FOCUS)
      {
      FX=ex;
      FY=ey;
      FZ=ez;
      }

   if (RELOADED!=0)
      {
      if (update>1) update=1;
      RELOADED=0;
      }

   TILE->setsealevel(SEALEVEL);

   TILE->draw(res,
              ex,ey,ez,
              dx,dy,dz,
              ux,uy,uz,
              fovy,aspect,
              nearp,farp,
              update);

   deliver();
   expire();

   TIME++;
   }

// set focus of interest
void miniload::setfocus(int on,
                        float fx,float fy,float fz)
   {
   FOCUS=on;

   FX=fx;
   FY=fy;
   FZ=fz;
   }

// set relative scaling (0<=scale<=1)
void miniload::setrelscale(float scale)
   {
   if (TILE==NULL) ERRORMSG();
   TILE->setrelscale(RELSCALE=scale);
   }

// get relative scaling
float miniload::getrelscale()
   {return(TILE->getrelscale());}

// set sea level
void miniload::setsealevel(float level)
   {SEALEVEL=level;}

// get sea level
float miniload::getsealevel()
   {return(SEALEVEL);}

// calculate the elevation at position (x,z)
float miniload::getheight(float x,float z,int approx)
   {
   if (TILE==NULL) ERRORMSG();
   return(TILE->getheight(x,z,approx));
   }

// calculate the height of the fog layer at position (x,z)
float miniload::getfogheight(float x,float z,int approx)
   {
   if (TILE==NULL) ERRORMSG();
   return(TILE->getfogheight(x,z,approx));
   }

// calculate the normal vector at position (x,z)
void miniload::getnormal(float x,float z,float *nx,float *ny,float *nz)
   {
   if (TILE==NULL) ERRORMSG();
   TILE->getnormal(x,z,nx,ny,nz);
   }

// calculate the cell dimension at position (x,z)
int miniload::getdim(float x,float z,float *dimx,float *dimz,float *radius2)
   {
   if (TILE==NULL) ERRORMSG();
   return(TILE->getdim(x,z,dimx,dimz,radius2));
   }

// return the scaling of the fog layer
float miniload::getlambda()
   {
   if (TILE==NULL) ERRORMSG();
   return(TILE->getlambda());
   }

// return the displacement of the fog layer
float miniload::getdisplace()
   {
   if (TILE==NULL) ERRORMSG();
   return(TILE->getdisplace());
   }

// return memory foot print of visible and preloaded tiles in mega bytes
float miniload::getmem()
   {
   int i,j;

   float mem;

   if (TILE==NULL) ERRORMSG();

   mem=0.0f;

   for (i=TILE->getpreloadedleft(); i<=TILE->getpreloadedright(); i++)
      for (j=TILE->getpreloadedbottom(); j<=TILE->getpreloadedtop(); j++)
         {
         if (TILE->isloaded(i,j)!=0)
            if (TILE->usesfloat(i,j)==0) mem+=(sizeof(short int)+1)*fsqr(TILE->getsize(i,j));
            else mem+=(sizeof(float)+1)*fsqr(TILE->getsize(i,j));

         if (TILE->hasfog(i,j)!=0) mem+=sizeof(unsigned char)*fsqr(TILE->getsize(i,j));
         }

   return(mem/fsqr(1024));
   }

// return texture memory foot print of visible and preloaded tiles in raw mega bytes
// if compressed with S3TC divide by 6
float miniload::gettexmem()
   {
   int i,j;

   float tmem;

   if (TILE==NULL) ERRORMSG();

   tmem=0.0f;

   for (i=TILE->getpreloadedleft(); i<=TILE->getpreloadedright(); i++)
      for (j=TILE->getpreloadedbottom(); j<=TILE->getpreloadedtop(); j++)
         if (TILE->gettexid(i,j)!=0)
            if (TILE->gettexmm(i,j)==0) tmem+=3*TILE->gettexw(i,j)*TILE->gettexh(i,j);
            else tmem+=1.5f*3*TILE->gettexw(i,j)*TILE->gettexh(i,j);

   return(tmem/fsqr(1024));
   }

// reload a specific tile
void miniload::reload(int col,int row,
                      const unsigned char *hmap,
                      const unsigned char *tmap,
                      const unsigned char *fmap)
   {
   if (TILE==NULL) ERRORMSG();

   if (HFIELDS[col+row*COLS]!=NULL)
      {
      free(HFIELDS[col+row*COLS]);
      HFIELDS[col+row*COLS]=NULL;

      if (TEXTURES[col+row*COLS]!=NULL)
         {
         free(TEXTURES[col+row*COLS]);
         TEXTURES[col+row*COLS]=NULL;
         }

      if (FOGMAPS!=NULL)
         if (FOGMAPS[col+row*COLS]!=NULL)
            {
            free(FOGMAPS[col+row*COLS]);
            FOGMAPS[col+row*COLS]=NULL;
            }
      }

   TILE->reload(col,row,NULL,NULL,NULL);

   if (hmap!=NULL)
      {
      HFIELDS[col+row*COLS]=(unsigned char *)strdup((char *)hmap);
      if (tmap!=NULL) TEXTURES[col+row*COLS]=(unsigned char *)strdup((char *)tmap);

      if (FOGMAPS!=NULL)
         if (fmap!=NULL) FOGMAPS[col+row*COLS]=(unsigned char *)strdup((char *)fmap);
      }

   MAXLOD[col+row*COLS]=-1;
   MAXTEX[col+row*COLS]=-1;
   MAXFOG[col+row*COLS]=-1;

   RELOADED=1;
   }

// loader for paged tile sets
// positive latitudes are transformed into negative Z-values
int miniload::load(int cols,int rows,
                   const char *basepath1,const char *basepath2,const char *basepath3,
                   float offsetx,float offsety,float offseth,
                   float stretch,float stretchx,float stretchy,
                   float exaggeration,float scale,
                   float lambda,float attenuation,
                   float minres,float minoff,
                   float outparams[5],
                   float outscale[3])
   {
   int i,j;

   unsigned char *image;
   int width,height,components;
   PNMcomment comment;

   databuf hfield,texture,fogmap;

   float coord[8],cellsize[2],scaling;
   int utm_zone,utm_datum,missing;

   float as2m[2];
   float maxelev=0.0f;

   if (cols<1 || rows<1) ERRORMSG();
   if (exaggeration<=0.0f || scale<=0.0f) ERRORMSG();

   if (TILE!=NULL)
      {
      delete TILE;
      cleanup();
      }

   COLS=cols;
   ROWS=rows;

   if ((HFIELDS=(unsigned char **)malloc(COLS*ROWS*sizeof(unsigned char *)))==NULL) ERRORMSG();
   if ((TEXTURES=(unsigned char **)malloc(COLS*ROWS*sizeof(unsigned char *)))==NULL) ERRORMSG();

   if (basepath3==NULL) FOGMAPS=NULL;
   else if ((FOGMAPS=(unsigned char **)malloc(COLS*ROWS*sizeof(unsigned char *)))==NULL) ERRORMSG();

   for (i=0; i<COLS; i++)
      for (j=0; j<ROWS; j++)
         if (CONFIGURE_USEPNM!=0)
            if (CONFIGURE_USEZERONUMBERING==0)
               // .pnm format with numbering starting from 1
               {
               if ((HFIELDS[i+j*COLS]=(unsigned char *)malloc(MAX_STR))==NULL) ERRORMSG();

               if (basepath1==NULL) snprintf((char *)HFIELDS[i+j*COLS],MAX_STR,"tile.%d-%d.pgm",i+1,j+1);
               else if (CONFIGURE_TILESETPATH==NULL) snprintf((char *)HFIELDS[i+j*COLS],MAX_STR,"%s/tile.%d-%d.pgm",basepath1,i+1,j+1);
               else snprintf((char *)HFIELDS[i+j*COLS],MAX_STR,"%s%s/tile.%d-%d.pgm",CONFIGURE_TILESETPATH,basepath1,i+1,j+1);

               if ((TEXTURES[i+j*COLS]=(unsigned char *)malloc(MAX_STR))==NULL) ERRORMSG();

               if (basepath2==NULL) snprintf((char *)TEXTURES[i+j*COLS],MAX_STR,"tile.%d-%d.ppm",i+1,j+1);
               else if (CONFIGURE_TILESETPATH==NULL) snprintf((char *)TEXTURES[i+j*COLS],MAX_STR,"%s/tile.%d-%d.ppm",basepath2,i+1,j+1);
               else snprintf((char *)TEXTURES[i+j*COLS],MAX_STR,"%s%s/tile.%d-%d.ppm",CONFIGURE_TILESETPATH,basepath2,i+1,j+1);

               if (basepath3!=NULL)
                  {
                  if ((FOGMAPS[i+j*COLS]=(unsigned char *)malloc(MAX_STR))==NULL) ERRORMSG();
                  if (CONFIGURE_TILESETPATH==NULL) snprintf((char *)FOGMAPS[i+j*COLS],MAX_STR,"%s/tile.%d-%d.pgm",basepath3,i+1,j+1);
                  else snprintf((char *)FOGMAPS[i+j*COLS],MAX_STR,"%s%s/tile.%d-%d.pgm",CONFIGURE_TILESETPATH,basepath3,i+1,j+1);
                  }
               }
            else
               // .pnm format with numbering starting from 0
               {
               if ((HFIELDS[i+j*COLS]=(unsigned char *)malloc(MAX_STR))==NULL) ERRORMSG();

               if (basepath1==NULL) snprintf((char *)HFIELDS[i+j*COLS],MAX_STR,"tile.%d-%d.pgm",i,j);
               else if (CONFIGURE_TILESETPATH==NULL) snprintf((char *)HFIELDS[i+j*COLS],MAX_STR,"%s/tile.%d-%d.pgm",basepath1,i,j);
               else snprintf((char *)HFIELDS[i+j*COLS],MAX_STR,"%s%s/tile.%d-%d.pgm",CONFIGURE_TILESETPATH,basepath1,i,j);

               if ((TEXTURES[i+j*COLS]=(unsigned char *)malloc(MAX_STR))==NULL) ERRORMSG();

               if (basepath2==NULL) snprintf((char *)TEXTURES[i+j*COLS],MAX_STR,"tile.%d-%d.ppm",i,j);
               else if (CONFIGURE_TILESETPATH==NULL) snprintf((char *)TEXTURES[i+j*COLS],MAX_STR,"%s/tile.%d-%d.ppm",basepath2,i,j);
               else snprintf((char *)TEXTURES[i+j*COLS],MAX_STR,"%s%s/tile.%d-%d.ppm",CONFIGURE_TILESETPATH,basepath2,i,j);

               if (basepath3!=NULL)
                  {
                  if ((FOGMAPS[i+j*COLS]=(unsigned char *)malloc(MAX_STR))==NULL) ERRORMSG();
                  if (CONFIGURE_TILESETPATH==NULL) snprintf((char *)FOGMAPS[i+j*COLS],MAX_STR,"%s/tile.%d-%d.pgm",basepath3,i,j);
                  else snprintf((char *)FOGMAPS[i+j*COLS],MAX_STR,"%s%s/tile.%d-%d.pgm",CONFIGURE_TILESETPATH,basepath3,i,j);
                  }
               }
         else
            if (CONFIGURE_USEZERONUMBERING==0)
               // .db format with numbering starting from 1
               {
               if ((HFIELDS[i+j*COLS]=(unsigned char *)malloc(MAX_STR))==NULL) ERRORMSG();

               if (basepath1==NULL) ERRORMSG();
               if (CONFIGURE_TILESETPATH==NULL) snprintf((char *)HFIELDS[i+j*COLS],MAX_STR,"%s/tile.%d-%d.db",basepath1,i+1,j+1);
               else snprintf((char *)HFIELDS[i+j*COLS],MAX_STR,"%s%s/tile.%d-%d.db",CONFIGURE_TILESETPATH,basepath1,i+1,j+1);

               if ((TEXTURES[i+j*COLS]=(unsigned char *)malloc(MAX_STR))==NULL) ERRORMSG();

               if (basepath2==NULL) snprintf((char *)TEXTURES[i+j*COLS],MAX_STR,"tile.%d-%d.db",i+1,j+1);
               if (CONFIGURE_TILESETPATH==NULL) snprintf((char *)TEXTURES[i+j*COLS],MAX_STR,"%s/tile.%d-%d.db",basepath2,i+1,j+1);
               else snprintf((char *)TEXTURES[i+j*COLS],MAX_STR,"%s%s/tile.%d-%d.db",CONFIGURE_TILESETPATH,basepath2,i+1,j+1);

               if (basepath3!=NULL)
                  {
                  if ((FOGMAPS[i+j*COLS]=(unsigned char *)malloc(MAX_STR))==NULL) ERRORMSG();
                  if (CONFIGURE_TILESETPATH==NULL) snprintf((char *)FOGMAPS[i+j*COLS],MAX_STR,"%s/tile.%d-%d.db",basepath3,i+1,j+1);
                  else snprintf((char *)FOGMAPS[i+j*COLS],MAX_STR,"%s%s/tile.%d-%d.db",CONFIGURE_TILESETPATH,basepath3,i+1,j+1);
                  }
               }
            else
               // .db format with numbering starting from 0
               {
               if ((HFIELDS[i+j*COLS]=(unsigned char *)malloc(MAX_STR))==NULL) ERRORMSG();

               if (basepath1==NULL) ERRORMSG();
               if (CONFIGURE_TILESETPATH==NULL) snprintf((char *)HFIELDS[i+j*COLS],MAX_STR,"%s/tile.%d-%d.db",basepath1,i,j);
               else snprintf((char *)HFIELDS[i+j*COLS],MAX_STR,"%s%s/tile.%d-%d.db",CONFIGURE_TILESETPATH,basepath1,i,j);

               if ((TEXTURES[i+j*COLS]=(unsigned char *)malloc(MAX_STR))==NULL) ERRORMSG();

               if (basepath2==NULL) snprintf((char *)TEXTURES[i+j*COLS],MAX_STR,"tile.%d-%d.db",i,j);
               if (CONFIGURE_TILESETPATH==NULL) snprintf((char *)TEXTURES[i+j*COLS],MAX_STR,"%s/tile.%d-%d.db",basepath2,i,j);
               else snprintf((char *)TEXTURES[i+j*COLS],MAX_STR,"%s%s/tile.%d-%d.db",CONFIGURE_TILESETPATH,basepath2,i,j);

               if (basepath3!=NULL)
                  {
                  if ((FOGMAPS[i+j*COLS]=(unsigned char *)malloc(MAX_STR))==NULL) ERRORMSG();
                  if (CONFIGURE_TILESETPATH==NULL) snprintf((char *)FOGMAPS[i+j*COLS],MAX_STR,"%s/tile.%d-%d.db",basepath3,i,j);
                  else snprintf((char *)FOGMAPS[i+j*COLS],MAX_STR,"%s%s/tile.%d-%d.db",CONFIGURE_TILESETPATH,basepath3,i,j);
                  }
               }

   if ((MANDATORY=(int *)malloc(COLS*ROWS*sizeof(int)))==NULL) ERRORMSG();

   for (i=0; i<COLS*ROWS; i++) MANDATORY[i]=1;

   if ((LODS=(unsigned char **)malloc(COLS*ROWS*sizeof(unsigned char *)))==NULL) ERRORMSG();

   if ((MAXLOD=(int *)malloc(COLS*ROWS*sizeof(int)))==NULL) ERRORMSG();
   if ((ACTLOD=(int *)malloc(COLS*ROWS*sizeof(int)))==NULL) ERRORMSG();
   if ((NEWLOD=(int *)malloc(COLS*ROWS*sizeof(int)))==NULL) ERRORMSG();

   for (i=0; i<COLS*ROWS; i++)
      {
      LODS[i]=NULL;
      MAXLOD[i]=-1;
      }

   if ((TEXS=(unsigned char **)malloc(COLS*ROWS*sizeof(unsigned char *)))==NULL) ERRORMSG();

   if ((MAXTEX=(int *)malloc(COLS*ROWS*sizeof(int)))==NULL) ERRORMSG();
   if ((ACTTEX=(int *)malloc(COLS*ROWS*sizeof(int)))==NULL) ERRORMSG();
   if ((NEWTEX=(int *)malloc(COLS*ROWS*sizeof(int)))==NULL) ERRORMSG();
   if ((MINTEX=(int *)malloc(COLS*ROWS*sizeof(int)))==NULL) ERRORMSG();

   for (i=0; i<COLS*ROWS; i++)
      {
      TEXS[i]=NULL;
      MAXTEX[i]=-1;
      }

   if ((FOGS=(unsigned char **)malloc(COLS*ROWS*sizeof(unsigned char *)))==NULL) ERRORMSG();

   if ((MAXFOG=(int *)malloc(COLS*ROWS*sizeof(int)))==NULL) ERRORMSG();

   for (i=0; i<COLS*ROWS; i++)
      {
      FOGS[i]=NULL;
      MAXFOG[i]=0;
      }

   TIME=0;

   if ((LRU=(int *)malloc(COLS*ROWS*sizeof(int)))==NULL) ERRORMSG();
   for (i=0; i<COLS*ROWS; i++) LRU[i]=TIME;

   if (REQUEST_CALLBACK==NULL)
      {
      if (CONFIGURE_USEPNM==0) ERRORMSG();

      image=NULL;

      for (i=0; i<COLS && image==NULL; i++)
         for (j=0; j<ROWS && image==NULL; j++)
            {
            image=readPNMfile((char *)HFIELDS[i+j*COLS],&width,&height,&components,&comment);

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

               SCALE=scaling*exaggeration;

               offsetx-=i*(coord[6]-coord[0]);
               offsety-=j*(coord[3]-coord[1]);
               }
            }

      if (image==NULL)
         {
         cleanup();
         return(0);
         }

      free(image);
      }
   else
      {
      hfield.reset();
      texture.reset();
      fogmap.reset();

      for (i=0; i<COLS && hfield.missing(); i++)
         for (j=0; j<ROWS && hfield.missing(); j++)
            {
            REQUEST_CALLBACK(0,0,HFIELDS[i+j*COLS],0,NULL,0,NULL,REQUEST_DATA,
                             &hfield,&texture,&fogmap);

            if (!texture.missing() || !fogmap.missing()) ERRORMSG();

            if (!hfield.missing())
               {
               if (hfield.type==databuf::DATABUF_TYPE_SHORT) maxelev=32767.0f;
               else if (hfield.type==databuf::DATABUF_TYPE_FLOAT) maxelev=MAXFLOAT;
               else maxelev=255.0f;

               coord[0]=hfield.swx*stretch;
               coord[1]=hfield.swy*stretch;
               coord[2]=hfield.nwx*stretch;
               coord[3]=hfield.nwy*stretch;
               coord[4]=hfield.nex*stretch;
               coord[5]=hfield.ney*stretch;
               coord[6]=hfield.sex*stretch;
               coord[7]=hfield.sey*stretch;

               SCALE=hfield.scaling*exaggeration;

               offsetx-=i*(coord[6]-coord[0]);
               offsety-=j*(coord[3]-coord[1]);
               }
            }

      if (hfield.missing())
         {
         cleanup();
         return(0);
         }

      if (CONFIGURE_DONTFREE==0) hfield.release();
      }

   if (CONFIGURE_USEPNM!=0)
      {
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

      COLDIM=fsqrt(fsqr(LONADD(coord[6],-coord[0]))+fsqr(coord[7]-coord[1]));
      ROWDIM=fsqrt(fsqr(coord[3]-coord[1])+fsqr(LONSUB(coord[2],coord[0])));

      CENTERX=LONSUB(coord[2],-cols/2.0f*LONADD(coord[6],-coord[0])+rows/2.0f*LONSUB(coord[2],coord[0]));
      CENTERY=offseth;
      CENTERZ=coord[3]-rows/2.0f*(coord[3]-coord[1])+cols/2.0f*(coord[7]-coord[1]);

      miniutm::arcsec2meter(CENTERZ,as2m);

      COLDIM*=as2m[0];
      ROWDIM*=as2m[1];

      CENTERX*=as2m[0];
      CENTERZ*=as2m[1];
      }
   else
      {
      if (offsetx!=0.0f || offsety!=0.0f)
         {
         coord[0]+=offsetx;
         coord[1]+=offsety;

         coord[2]+=offsetx;
         coord[3]+=offsety;

         coord[4]+=offsetx;
         coord[5]+=offsety;

         coord[6]+=offsetx;
         coord[7]+=offsety;
         }

      COLDIM=fsqrt(fsqr(coord[6]-coord[0])+fsqr(coord[7]-coord[1]));
      ROWDIM=fsqrt(fsqr(coord[3]-coord[1])+fsqr(coord[2]-coord[0]));

      CENTERX=coord[2]+cols/2.0f*(coord[6]-coord[0])+rows/2.0f*(coord[2]-coord[0]);
      CENTERY=offseth;
      CENTERZ=coord[3]-rows/2.0f*(coord[3]-coord[1])+cols/2.0f*(coord[7]-coord[1]);
      }

   COLDIM*=stretchx/scale;
   ROWDIM*=stretchy/scale;

   SCALE/=scale;

   CENTERX*=stretchx/scale;
   CENTERY/=scale;
   CENTERZ*=-stretchy/scale;

   TILE=new minitile(NULL,NULL,
                     COLS,ROWS,COLDIM,ROWDIM,SCALE,
                     CENTERX,CENTERY,CENTERZ,
                     NULL,lambda*exaggeration/scale,0.0f,0.0f,attenuation,0.0f,0.0f,0.0f,
                     minres,minoff);

   TILE->setrequest(request,this,preload,PFARP,PUPDATE);

   // 5 output parameters
   if (outparams!=NULL)
      {
      outparams[0]=COLDIM; // dimension of one column
      outparams[1]=ROWDIM; // dimension of one row

      outparams[2]=CENTERX; // x-center of the grid
      outparams[3]=-CENTERZ; // z-center of the grid

      outparams[4]=(maxelev==MAXFLOAT)?MAXFLOAT:maxelev*SCALE; // maximum elevation
      }

   // 3 output parameters
   if (outscale!=NULL)
      if (CONFIGURE_USEPNM!=0)
         {
         outscale[0]=as2m[0]*stretch*stretchx/scale; // x-size of one arcsec in meters
         outscale[1]=as2m[1]*stretch*stretchy/scale; // z-size of one arcsec in meters
         outscale[2]=SCALE/exaggeration; // one scaled meter
         }
      else
         {
         outscale[0]=stretch*stretchx/scale; // x-scaling
         outscale[1]=stretch*stretchy/scale; // z-scaling
         outscale[2]=SCALE/exaggeration; // one scaled meter
         }

   return(1);
   }

// set callbacks for requesting and preloading tiles
void miniload::setloader(int (*request)(int col,int row,const unsigned char *mapfile,int hlod,const unsigned char *texfile,int tlod,const unsigned char *fogfile,void *data,databuf *hfield,databuf *texture,databuf *fogmap),void *data,
                         void (*preload)(int col,int row,const unsigned char *mapfile,int hlod,const unsigned char *texfile,int tlod,const unsigned char *fogfile,void *data),
                         void (*deliver)(int *col,int *row,databuf *hfield,int *hlod,databuf *texture,int *tlod,databuf *fogmap,void *data),
                         int paging,
                         float pfarp,
                         float prange,int pbasesize,
                         int plazyness,int pupdate,
                         int expire)
   {
   if (paging<0 ||
       pfarp<0.0f ||
       prange<0.0f || pbasesize<0 ||
       plazyness<0 || pupdate<0 ||
       expire<0) ERRORMSG();

   REQUEST_CALLBACK=request;
   PRELOAD_CALLBACK=preload;
   DELIVER_CALLBACK=deliver;
   REQUEST_DATA=data;

   PAGING=paging;

   PFARP=pfarp;
   PRANGE=PRANGE0=prange;
   PBASESIZE=pbasesize;
   PUPDATE=pupdate;

   if (preload==NULL || paging==0) PRELOD=0;
   else PRELOD=paging;

   if (preload==NULL || prange==0.0f) PRETEX=0;
   else PRETEX=1;

   LAZYNESS=max(plazyness,1);
   LAZYTEX=plazyness;

   if (PRETEX>0)
      {
      PRANGE/=2.0f;
      LAZYTEX=max(LAZYTEX,1);
      }

   if (TILE!=NULL) TILE->setrequest(miniload::request,this,miniload::preload,PFARP,PUPDATE);

   EXPIRE=expire;
   }

// set callback for inquiry of height map elevation range
void miniload::setinquiry(int (*inquiry)(int col,int row,const unsigned char *mapfile,int hlod,void *data,float *minvalue,float *maxvalue),void *data)
   {
   INQUIRY_CALLBACK=inquiry;
   INQUIRY_DATA=data;
   }

// set callback for query of texture map base size
void miniload::setquery(void (*query)(int col,int row,const unsigned char *texfile,int tlod,void *data,int *tsizex,int *tsizey),void *data)
   {
   QUERY_CALLBACK=query;
   QUERY_DATA=data;
   }

// compute texture paging range from screen space error
float miniload::calcrange(float dim,int height,float fovy,float thres)
   {
   if (fovy>0.0f) return(dim*height/(thres*tan(PI/360.0f*fovy)));
   else return(0.0f); // disable texture paging
   }

// set preloading
void miniload::setpreload(float pfarp,int pupdate)
   {
   PFARP=pfarp;
   PUPDATE=pupdate;

   TILE->setpreload(pfarp,pupdate);
   }

// set expiration
void miniload::setexpire(int expire)
   {EXPIRE=expire;}

// set texture range
void miniload::setrange(float prange)
   {
   if (PRETEX>0) PRANGE=prange/2.0f;
   else PRANGE=prange;

   PRANGE0=prange;
   }

// define radius at which the non-linear texture LOD term kicks in
void miniload::setradius(float radius,float dropoff)
   {
   if (PRETEX>0) PRADIUS=radius/2.0f;
   else PRADIUS=radius;

   PRADIUS0=radius;

   PDROPOFF=dropoff;
   }

// define texture size of LOD0
void miniload::setbasesize(int basesize)
   {PBASESIZE=basesize;}

// define restricted roi in which tiles are mandatory at startup
void miniload::restrictroi(float rx,float rz,
                           float rrad)
   {
   int i,j;

   float d;

   int left,right;
   int top,bottom;

   if (TILE==NULL) ERRORMSG();

   if (rrad<0.0f) ERRORMSG();

   // calculate bounding box of roi
   if (rrad>0.0f)
      {
      d=ffloor(((rx-rrad)-(CENTERX-COLDIM*COLS/2.0f))/COLDIM);
      left=min(max(ftrc(d),0),COLS-1);

      d=ffloor(((CENTERX+COLDIM*COLS/2.0f)-(rx+rrad))/COLDIM);
      right=COLS-1-min(max(ftrc(d),0),COLS-1);

      d=ffloor(((rz-rrad)-(CENTERZ-ROWDIM*ROWS/2.0f))/ROWDIM);
      bottom=min(max(ftrc(d),0),ROWS-1);

      d=ffloor(((CENTERZ+ROWDIM*ROWS/2.0f)-(rz+rrad))/ROWDIM);
      top=ROWS-1-min(max(ftrc(d),0),ROWS-1);
      }
   else
      {
      left=0;
      right=COLS-1;
      bottom=0;
      top=ROWS-1;
      }

   // define all tiles not to be mandatory
   for (i=0; i<COLS; i++)
      for (j=0; j<ROWS; j++) MANDATORY[i+j*COLS]=0;

   // redefine tiles inside the bounding box to be mandatory
   for (i=left; i<=right; i++)
      for (j=bottom; j<=top; j++) MANDATORY[i+j*COLS]=1;
   }

// update roi so that missing tiles are requested immediately
void miniload::updateroi(float res,
                         float ex,float ey,float ez,
                         float rx,float rz,
                         float rrad,
                         float farp,
                         float fovy,float aspect)
   {
   int i,j;

   float c,d;

   int left,right;
   int top,bottom;

   int needtex;

   if (TILE==NULL) ERRORMSG();

   if (rrad<0.0f || farp<0.0f) ERRORMSG();

   RES=res;

   FX=ex;
   FY=ey;
   FZ=ez;

   // increase roi by visible range
   if (farp>0.0f)
      {
      aspect=fmax(aspect,1.0f/aspect);

      if (fovy>0.0f) c=fsqrt(1.0f+fsqr(ftan(fovy*PI/360.0f)*aspect));
      else c=fsqrt(1.0f+fsqr(fovy/2.0f*aspect/farp));

      rrad+=c*farp;
      }

   // calculate bounding box of roi
   if (rrad>0.0f)
      {
      d=ffloor(((rx-rrad)-(CENTERX-COLDIM*COLS/2.0f))/COLDIM);
      left=min(max(ftrc(d),0),COLS-1);

      d=ffloor(((CENTERX+COLDIM*COLS/2.0f)-(rx+rrad))/COLDIM);
      right=COLS-1-min(max(ftrc(d),0),COLS-1);

      d=ffloor(((rz-rrad)-(CENTERZ-ROWDIM*ROWS/2.0f))/ROWDIM);
      bottom=min(max(ftrc(d),0),ROWS-1);

      d=ffloor(((CENTERZ+ROWDIM*ROWS/2.0f)-(rz+rrad))/ROWDIM);
      top=ROWS-1-min(max(ftrc(d),0),ROWS-1);
      }
   else
      {
      left=0;
      right=COLS-1;
      bottom=0;
      top=ROWS-1;
      }

   // request missing tiles inside the bounding box
   for (i=left; i<=right; i++)
      for (j=bottom; j<=top; j++)
         {
         needtex=(TEXTURES[i+j*COLS]!=NULL && TILE->gettexid(i,j)==0)?1:0;
         if (TILE->isloaded(i,j)==0 || needtex!=0) request(i,j,needtex,this);
         }
   }

// update entire tile set
void miniload::updateall()
   {
   static const float uplift=1000.0f;

   float ex,ey,ez;

   ex=CENTERX;
   ey=CENTERY+uplift*(COLS*COLDIM+ROWS*ROWDIM)/2.0f;
   ez=CENTERZ;

   updateroi(1.0,
             ex,ey,ez,ex,ez,
             0.0f,0.0f);
   }

// configure core parameters:

void miniload::configure_minres(float minres)
   {
   if (minres<1.0f) minres=9.0f;
   CONFIGURE_MINRES=minres;
   }

void miniload::configure_maxd2(float maxd2)
   {
   if (maxd2<=0.0f) maxd2=100.0f;
   CONFIGURE_MAXD2=maxd2;
   }

void miniload::configure_sead2(float sead2)
   {
   if (sead2<0.0f) sead2=0.1f;
   CONFIGURE_SEAD2=sead2;
   }

void miniload::configure_minoff(float minoff)
   {
   if (minoff<0.0f) minoff=0.1f;
   CONFIGURE_MINOFF=minoff;
   }

void miniload::configure_maxcull(int maxcull)
   {
   if (maxcull<0) maxcull=8;
   CONFIGURE_MAXCULL=maxcull;
   }

// configuring:

void miniload::configure_seamin(float seamin)
   {CONFIGURE_SEAMIN=seamin;}

void miniload::configure_seamax(float seamax)
   {CONFIGURE_SEAMAX=seamax;}

void miniload::configure_mipmaps(int mipmaps)
   {CONFIGURE_MIPMAPS=mipmaps;}

// configuring the pager:

void miniload::configure_szone(float szone)
   {
   if (szone<0.0f || szone>=1.0f) ERRORMSG();
   CONFIGURE_SZONE=szone;
   }

void miniload::configure_tsafety(float tsafety)
   {
   if (tsafety<0.0f) ERRORMSG();
   CONFIGURE_TSAFETY=tsafety;
   }

void miniload::configure_minlazy(float minlazy)
   {
   if (minlazy<=0.0f || minlazy>=1.0f) ERRORMSG();
   CONFIGURE_MINLAZY=minlazy;
   }

void miniload::configure_maxtexlod(int maxtexlod)
   {
   if (maxtexlod<0) ERRORMSG();
   CONFIGURE_MAXTEXLOD=maxtexlod;
   }

void miniload::configure_maxmaplod(int maxmaplod)
   {
   if (maxmaplod<0) ERRORMSG();
   CONFIGURE_MAXMAPLOD=maxmaplod;
   }

void miniload::configure_dontfree(int dontfree)
   {CONFIGURE_DONTFREE=dontfree;}

// configuring the loader:

void miniload::configure_usezeronumbering(int usezeronumbering)
   {CONFIGURE_USEZERONUMBERING=usezeronumbering;}

void miniload::configure_tilesetpath(char *tilesetpath)
   {
   if (CONFIGURE_TILESETPATH!=NULL) free(CONFIGURE_TILESETPATH);
   CONFIGURE_TILESETPATH=NULL;

   if (tilesetpath!=NULL) CONFIGURE_TILESETPATH=strdup(tilesetpath);
   }

void miniload::configure_usepnm(int usepnm)
   {CONFIGURE_USEPNM=usepnm;}
