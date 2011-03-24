// (c) by Stefan Roettger

#include "minibase.h"

#include "minimath.h"

#include "miniOGL.h"

#include "minicache.h"

// there is only one cache
minicache *minicache::CACHE;

// default constructor
minicache::minicache()
   {
   int i;

   CACHE=NULL;

   TERRAIN=NULL;
   NUMTERRAIN=MAXTERRAIN=0;

   CACHE_ID=0;
   RENDER_ID=0;

   CULLMODE=1;

   OPACITY=1.0f;
   ALPHATEST=1.0f;

   SEA_R=0.0f;
   SEA_G=0.5f;
   SEA_B=1.0f;
   SEA_A=1.0f;

   PRISM_R=PRISM_G=PRISM_B=1.0f;
   PRISM_A=0.9f;

   PRISMEDGE_CALLBACK=NULL;
   PRISMCACHE_CALLBACK=NULL;
   PRISMRENDER_CALLBACK=NULL;
   PRISMTRIGGER_CALLBACK=NULL;
   PRISMSYNC_CALLBACK=NULL;
   CALLBACK_DATA=NULL;

   VTXPROG=NULL;
   VTXDIRTY=0;

   VTXPROGID=0;
   USEVTXSHADER=0;

   for (i=0; i<8; i++)
      {
      VTXSHADERPAR1[i]=0.0f;
      VTXSHADERPAR2[i]=0.0f;
      VTXSHADERPAR3[i]=0.0f;
      VTXSHADERPAR4[i]=0.0f;
      }

   FRAGPROG=NULL;
   FRAGDIRTY=0;

   FRAGPROGID=0;
   USEPIXSHADER=0;

   for (i=0; i<8; i++)
      {
      PIXSHADERPAR1[i]=0.0f;
      PIXSHADERPAR2[i]=0.0f;
      PIXSHADERPAR3[i]=0.0f;
      PIXSHADERPAR4[i]=0.0f;
      }

   PIXSHADERTEXID=0;

   SEAPROG=NULL;
   SEADIRTY=0;

   SEAPROGID=0;
   USESEASHADER=0;

   for (i=0; i<8; i++)
      {
      SEASHADERPAR1[i]=0.0f;
      SEASHADERPAR2[i]=0.0f;
      SEASHADERPAR3[i]=0.0f;
      SEASHADERPAR4[i]=0.0f;
      }

   SEASHADERTEXID=0;

   PRISMCACHE_VTXPROGID=0;
   PRISMCACHE_FRAGPROGID=0;

   PRESEA_CB=NULL;
   POSTSEA_CB=NULL;

   GLSETUP=0;
   WGLSETUP=0;

   GLEXT_MT=0;
   GLEXT_VP=0;
   GLEXT_FP=0;

   CONFIGURE_OVERLAP=0.02f;
   CONFIGURE_MINSIZE=33;
   CONFIGURE_SEATWOSIDED=1;
   CONFIGURE_SEAENABLETEX=0;
   CONFIGURE_ZSCALE_SEA=0.99f;
   CONFIGURE_ZSCALE_PRISMS=0.95f;
   CONFIGURE_ENABLERAY=0;
   CONFIGURE_OMITSEA=0;
   }

// destructor
minicache::~minicache()
   {
   int id;

   if (TERRAIN!=NULL)
      {
      for (id=0; id<MAXTERRAIN; id++)
         if (TERRAIN[id].tile!=NULL) freeterrain(&TERRAIN[id]);

      free(TERRAIN);
      }

#ifndef NOOGL

   GLuint progid;

   if (VTXPROG!=NULL) free(VTXPROG);
   if (FRAGPROG!=NULL) free(FRAGPROG);
   if (SEAPROG!=NULL) free(SEAPROG);

#if defined(GL_ARB_vertex_program) && defined(GL_ARB_fragment_program)

   if (GLEXT_VP!=0 && GLEXT_FP!=0)
      {
      if (VTXPROGID!=0)
         {
         progid=VTXPROGID;
         glDeleteProgramsARB(1,&progid);
         }

      if (FRAGPROGID!=0)
         {
         progid=FRAGPROGID;
         glDeleteProgramsARB(1,&progid);
         }

      if (SEAPROGID!=0)
         {
         progid=SEAPROGID;
         glDeleteProgramsARB(1,&progid);
         }

      if (PRISMCACHE_VTXPROGID!=0)
         {
         progid=PRISMCACHE_VTXPROGID;
         glDeleteProgramsARB(1,&progid);
         }

      if (PRISMCACHE_FRAGPROGID!=0)
         {
         progid=PRISMCACHE_FRAGPROGID;
         glDeleteProgramsARB(1,&progid);
         }
      }

#endif

   if (PIXSHADERTEXID!=0) deletetexmap(PIXSHADERTEXID);
   if (SEASHADERTEXID!=0) deletetexmap(SEASHADERTEXID);

#endif
   }

// initialize terrain
void minicache::initterrain(TERRAIN_TYPE *t)
   {
   int i;

   CACHE_TYPE *c;

   t->cache_num=0;

   for (i=0; i<2; i++)
      {
      c=&t->cache[i];

      c->size=0;
      c->maxsize=1;

      if ((c->op=(unsigned char *)malloc(c->maxsize))==NULL) ERRORMSG();
      if ((c->arg=(float *)malloc(3*c->maxsize*sizeof(float)))==NULL) ERRORMSG();

      c->fancnt=0;
      c->vtxcnt=0;

      c->prism_size=0;
      c->prism_maxsize=1;

      if ((c->prism_cache=(float *)malloc(4*c->prism_maxsize*sizeof(float)))==NULL) ERRORMSG();
      }

   t->cache_phase=-1;

   t->ray=new miniray;
   t->first_fancnt=0;

   t->render_phase=-1;

   t->isvisible=1;

   t->lx=0.0f;
   t->ly=0.0f;
   t->lz=0.0f;

   t->ls=0.0f;
   t->lo=1.0f;
   }

// free terrain
void minicache::freeterrain(TERRAIN_TYPE *t)
   {
   int i;

   CACHE_TYPE *c;

   for (i=0; i<2; i++)
      {
      c=&t->cache[i];

      free(c->op);
      free(c->arg);

      free(c->prism_cache);
      }

   delete t->ray;
   }

// static callback functions:

void minicache::cache_beginfan()
   {CACHE->cache(BEGINFAN_OP);}

void minicache::cache_fanvertex(const float i,const float y,const float j)
   {CACHE->cache(FANVERTEX_OP,i,y,j);}

void minicache::cache_texmap(const int m,const int n,const int S)
   {CACHE->cache(TEXMAP_OP,m,n,S);}

void minicache::cache_prismedge(const float x,const float y,const float yf,const float z)
   {CACHE->cacheprismedge(x,y,yf,z);}

void minicache::cache_trigger(const int phase,const float scale,const float ex,const float ey,const float ez)
   {CACHE->cachetrigger(phase,scale,ex,ey,ez);}

void minicache::cache_sync(const int id)
   {CACHE->cachesync(id);}

// caching functions:

void minicache::cache(const int op,const float arg1,const float arg2,const float arg3)
   {
   int cols,rows;
   float xdim,zdim;
   float centerx,centery,centerz;

   miniv3d s,o;

   TERRAIN_TYPE *t;
   CACHE_TYPE *c;

   float *ptr;

   t=&TERRAIN[CACHE_ID];
   c=&t->cache[t->cache_num];

   // enlarge vertex buffer
   if (c->size>=c->maxsize)
      {
      c->maxsize*=2;

      if ((c->op=(unsigned char *)realloc(c->op,c->maxsize))==NULL) ERRORMSG();
      if ((c->arg=(float *)realloc(c->arg,3*c->maxsize*sizeof(float)))==NULL) ERRORMSG();
      }

   // update state
   if (op==BEGINFAN_OP)
      {
      c->fancnt++;
      t->first_fancnt++;
      t->last_beginfan=c->size;
      }
   else if (op==FANVERTEX_OP)
      {
      c->vtxcnt++;
      c->arg[3*t->last_beginfan]++;
      }
   else
      // update ray object
      if (CONFIGURE_ENABLERAY!=0)
         {
         if (t->first_fancnt>0)
            {
            cols=t->tile->getcols();
            rows=t->tile->getrows();

            xdim=t->tile->getcoldim();
            zdim=t->tile->getrowdim();

            centerx=t->tile->getcenterx();
            centery=t->tile->getcentery();
            centerz=t->tile->getcenterz();

            s.x=xdim/(t->first_size-1);
            s.y=t->first_scale;
            s.z=-zdim/(t->first_size-1);

            o.x=xdim*(t->first_col-(cols-1)/2.0f)+centerx-xdim/2.0f;
            o.y=centery;
            o.z=zdim*(t->first_row-(rows-1)/2.0f)+centerz+zdim/2.0f;

            if (t->cache_phase!=3 || CONFIGURE_OMITSEA==0)
               t->ray->addtrianglefans(&c->arg,3*t->first_beginfan,t->first_fancnt,0,&s,&o,0,t->tile->getwarp());
            }

         if (op==TRIGGER_OP)
            {
            if (t->cache_phase==0) t->ray->clearbuffer();
            else if (t->cache_phase==1) t->first_scale=arg2;
            else if (t->cache_phase==4) t->ray->swapbuffer();
            }
         else if (op==TEXMAP_OP)
            {
            t->first_col=ftrc(arg1+0.5f);
            t->first_row=ftrc(arg2+0.5f);
            t->first_size=ftrc(arg3+0.5f);
            }

         t->first_beginfan=c->size+1;
         t->first_fancnt=0;
         }

   // append operand
   c->op[c->size]=op;

   // append vertex cache
   ptr=&c->arg[3*c->size++];
   *ptr++=arg1;
   *ptr++=arg2;
   *ptr=arg3;
   }

void minicache::cacheprismedge(const float x,const float y,const float yf,const float z)
   {
   miniv4d v1;

   TERRAIN_TYPE *t;
   CACHE_TYPE *c;

   float *ptr;

   if (PRISMEDGE_CALLBACK!=NULL) PRISMEDGE_CALLBACK(x,y-yf,yf,z,CALLBACK_DATA);
   else
      {
      t=&TERRAIN[CACHE_ID];
      c=&t->cache[t->cache_num];

      // enlarge prism cache
      if (c->prism_size>=c->prism_maxsize)
         {
         c->prism_maxsize*=2;

         if ((c->prism_cache=(float *)realloc(c->prism_cache,4*c->prism_maxsize*sizeof(float)))==NULL) ERRORMSG();
         }

      // append prism cache
      ptr=&c->prism_cache[4*c->prism_size++];
      *ptr++=x;
      *ptr++=y;
      *ptr++=yf;
      *ptr=z;
      }
   }

void minicache::cachetrigger(const int phase,const float scale,const float ex,const float ey,const float ez)
   {
   miniv4d v1;

   TERRAIN_TYPE *t;
   CACHE_TYPE *c1,*c2;

   t=&TERRAIN[CACHE_ID];
   c1=&t->cache[t->cache_num];
   c2=&t->cache[1-t->cache_num];

   t->cache_phase=phase;

   cache(TRIGGER_OP,phase,scale);

   if (t->cache_phase==0)
      {
      // reset size of back buffer
      c2->size=0;
      c2->prism_size=0;

      // reset counts of back buffer
      c2->fancnt=0;
      c2->vtxcnt=0;

      // swap vertex buffers
      t->cache_num=1-t->cache_num;

      // shrink front vertex buffer
      if (c1->size<c1->maxsize/4)
         {
         c1->maxsize/=2;

         if ((c1->op=(unsigned char *)realloc(c1->op,c1->maxsize))==NULL) ERRORMSG();
         if ((c1->arg=(float *)realloc(c1->arg,3*c1->maxsize*sizeof(float)))==NULL) ERRORMSG();
         }

      // shrink front prism buffer
      if (c1->prism_size<c1->prism_maxsize/4)
         {
         c1->prism_maxsize/=2;

         if ((c1->prism_cache=(float *)realloc(c1->prism_cache,4*c1->prism_maxsize*sizeof(float)))==NULL) ERRORMSG();
         }
      }

   if (PRISMCACHE_CALLBACK!=NULL) PRISMCACHE_CALLBACK(phase,scale,ex,ey,ez,CALLBACK_DATA);
   }

void minicache::cachesync(const int id)
   {
   CACHE_ID=id;

   if (PRISMSYNC_CALLBACK!=NULL) PRISMSYNC_CALLBACK(id,CALLBACK_DATA);
   }

// render all back buffers of the cache
int minicache::rendercache()
   {
   int vtx=0;

   int id,phase;

   for (id=0; id<MAXTERRAIN; id++)
      if (TERRAIN[id].tile!=NULL)
         {
         TERRAIN[id].render_phase=0;
         TERRAIN[id].render_count=0;
         }

   for (phase=0; phase<=4; phase++)
      {
      rendertrigger(phase);

      for (id=0; id<MAXTERRAIN; id++)
         if (TERRAIN[id].tile!=NULL)
            if (TERRAIN[id].isvisible!=0) vtx+=rendercache(id,phase);
      }

   return(vtx);
   }

// render back buffer of the cache
int minicache::rendercache(int id,int phase)
   {
   int vtx=0;

#ifndef NOOGL

   int i,p;

   TERRAIN_TYPE *t;
   CACHE_TYPE *c;

   RENDER_ID=id;

   t=&TERRAIN[RENDER_ID];
   c=&t->cache[1-t->cache_num];

   if (PRISMSYNC_CALLBACK!=NULL) PRISMSYNC_CALLBACK(RENDER_ID,CALLBACK_DATA);

   if (phase==0) vtx+=getvtxcnt(RENDER_ID);

   i=t->render_count;

   while (i<c->size)
      {
      switch (c->op[i])
         {
         case BEGINFAN_OP:
            p=ftrc(c->arg[3*i]+0.5f);
            glDrawArrays(GL_TRIANGLE_FAN,i+1,p);
            i+=p;
            break;
         case FANVERTEX_OP:
            break;
         case TEXMAP_OP:
            rendertexmap(ftrc(c->arg[3*i]+0.5f),ftrc(c->arg[3*i+1]+0.5f),ftrc(c->arg[3*i+2]+0.5f));
            break;
         case TRIGGER_OP:
            t->render_count=i;
            vtx+=rendertrigger(ftrc(c->arg[3*i]+0.5f),c->arg[3*i+1]);
            if (t->render_phase!=phase) return(vtx);
            break;
         }
      i++;
      }

   t->render_count=i;

#endif

   return(vtx);
   }

// rendering functions:

void minicache::rendertexmap(int m,int n,int S)
   {
#ifndef NOOGL

   int cols,rows;
   float xdim,zdim;
   float centerx,centery,centerz;

   GLfloat mvmtx[16];
   miniv3d invtra[3];
   miniv3d light;

   miniwarp *warp;

   miniv4d mtx[3];
   double oglmtx[16];

   float ox,oz;

   int texid,texw,texh,texmm;

   TERRAIN_TYPE *t;

   t=&TERRAIN[RENDER_ID];

   mtxpop();

   cols=t->tile->getcols();
   rows=t->tile->getrows();

   xdim=t->tile->getcoldim();
   zdim=t->tile->getrowdim();

   centerx=t->tile->getcenterx();
   centery=t->tile->getcentery();
   centerz=t->tile->getcenterz();

   mtxpush();

   glGetFloatv(GL_MODELVIEW_MATRIX,mvmtx);

   warp=t->tile->getwarp();

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

   ox=xdim*(m-(cols-1)/2.0f)+centerx;
   oz=zdim*(n-(rows-1)/2.0f)+centerz;

   mtxtranslate(ox-xdim/2.0f,centery,oz+zdim/2.0f);

   // avoid gaps between tiles (excluding the sea surface)
   if (t->render_phase==2)
      if (CONFIGURE_OVERLAP!=0.0f)
         if (S>=CONFIGURE_MINSIZE) mtxscale((S-1+CONFIGURE_OVERLAP)/(S-1),1.0f,(S-1+CONFIGURE_OVERLAP)/(S-1));

   mtxscale(xdim/(S-1),t->scale,-zdim/(S-1));

   if (t->render_phase==2 || t->render_phase==3)
      {
      texid=t->tile->gettexid(m,n);
      texw=t->tile->gettexw(m,n);
      texh=t->tile->gettexh(m,n);
      texmm=t->tile->gettexmm(m,n);

      if (t->render_phase==2 || CONFIGURE_SEAENABLETEX!=0) bindtexmap(texid,texw,texh,S,texmm);
      else texid=0;

      if (USEVTXSHADER!=0)
         setvtxshadertexprm(1.0f/(S-1)*(texw-1)/texw,
                            -1.0f/(S-1)*(texh-1)/texh,
                            0.5f/texh,
                            1.0f-0.5f/texh,
                            t->scale);

      if (USEPIXSHADER!=0 || USESEASHADER!=0)
         {
         invtra[0].x=mvmtx[0];
         invtra[1].x=mvmtx[1];
         invtra[2].x=mvmtx[2];
         invtra[0].y=mvmtx[4];
         invtra[1].y=mvmtx[5];
         invtra[2].y=mvmtx[6];
         invtra[0].z=mvmtx[8];
         invtra[1].z=mvmtx[9];
         invtra[2].z=mvmtx[10];

         inv_mtx(invtra,invtra);
         tra_mtx(invtra,invtra);

         light=miniv3d(t->lx,t->ly,t->lz);
         light=miniv3d(invtra[0]*light,invtra[1]*light,invtra[2]*light);
         light.normalize();

         if (texid==0) setpixshadertexprm(0.0f,1.0f,light.x,light.y,light.z,t->ls,t->lo);
         else setpixshadertexprm(1.0f,0.0f,light.x,light.y,light.z,t->ls,t->lo);
         }
      }

#endif
   }

int minicache::rendertrigger(int phase)
   {
   int vtx=0;

#ifndef NOOGL

   if (phase==2)
      {
      initstate();
      mtxpush();

      if (CULLMODE==0) disableculling();

      if (ALPHATEST<1.0f) enableAtest(ALPHATEST);

      if (OPACITY<1.0f)
         {
         if (OPACITY<=0.0f) disableRGBAwriting();

         disableZwriting();
         enableblending();

         color(1.0f,1.0f,1.0f,OPACITY);
         }
      else color(1.0f,1.0f,1.0f);

      normal(0.0f,1.0f,0.0f);

      if (USEVTXSHADER!=0) enablevtxshader();
      if (USEPIXSHADER!=0) enablepixshader();

      glEnableClientState(GL_VERTEX_ARRAY);
      }
   else if (phase==3)
      {
      if (CULLMODE==0) enableBFculling();

      if (ALPHATEST<1.0f) disableAtest();

      if (OPACITY<1.0f)
         {
         if (OPACITY<=0.0f) enableRGBAwriting();

         enableZwriting();
         disableblending();
         }

      if (USEPIXSHADER!=0) disablepixshader();

      if (PRESEA_CB!=NULL)
         {
         bindtexmap(0,0,0,0);

         glDisableClientState(GL_VERTEX_ARRAY);

         if (USEVTXSHADER!=0) disablevtxshader();

         mtxpop();
         exitstate();

         PRESEA_CB(CB_DATA);

         initstate();
         mtxpush();

         if (USEVTXSHADER!=0) enablevtxshader();

         glEnableClientState(GL_VERTEX_ARRAY);
         }

      if (CONFIGURE_SEATWOSIDED!=0) disableculling();

      if (SEA_A!=1.0f) enableblending();

      color(SEA_R,SEA_G,SEA_B,SEA_A);
      normal(0.0f,1.0f,0.0f);

      if (USESEASHADER!=0) enableseashader();

      if (CONFIGURE_ZSCALE_SEA!=1.0f)
         {
         mtxproj();
         mtxpush();
         mtxscale(CONFIGURE_ZSCALE_SEA,CONFIGURE_ZSCALE_SEA,CONFIGURE_ZSCALE_SEA); // prevent Z-fighting
         mtxmodel();
         }
      }
   else if (phase==4)
      {
      if (CONFIGURE_SEATWOSIDED!=0) enableBFculling();

      if (SEA_A!=1.0f) disableblending();

      if (USESEASHADER!=0) disableseashader();
      if (USEVTXSHADER!=0) disablevtxshader();

      bindtexmap(0,0,0,0);

      glDisableClientState(GL_VERTEX_ARRAY);

      if (CONFIGURE_ZSCALE_SEA!=1.0f)
         {
         mtxproj();
         mtxpop();
         mtxmodel();
         }

      mtxpop();
      exitstate();

      if (POSTSEA_CB!=NULL) POSTSEA_CB(CB_DATA);
      }

   if (phase==4) vtx+=rendertrigger();

#endif

   return(vtx);
   }

int minicache::rendertrigger(int phase,float scale)
   {
   int vtx=0;

#ifndef NOOGL

   TERRAIN_TYPE *t;
   CACHE_TYPE *c;

   t=&TERRAIN[RENDER_ID];
   c=&t->cache[1-t->cache_num];

   t->render_phase=phase;

   if (t->render_phase==1) t->scale=scale;
   else if (t->render_phase==2 || t->render_phase==3) glVertexPointer(3,GL_FLOAT,0,c->arg);
   else if (t->render_phase==4) t->lambda=scale;

   if (PRISMTRIGGER_CALLBACK!=NULL) vtx+=PRISMTRIGGER_CALLBACK(phase,CALLBACK_DATA);

#endif

   return(vtx);
   }

int minicache::rendertrigger()
   {
   int vtx=0;

#ifndef NOOGL

   int id;

   TERRAIN_TYPE *t;
   CACHE_TYPE *c;

   if (PRISMEDGE_CALLBACK==NULL)
      for (id=0; id<MAXTERRAIN; id++)
         {
         t=&TERRAIN[id];
         c=&t->cache[1-t->cache_num];

         if (t->tile!=NULL)
            {
            if (PRISMRENDER_CALLBACK!=NULL)
               vtx+=PRISMRENDER_CALLBACK(c->prism_cache,c->prism_size/3,t->lambda,t->tile->getwarp(),CALLBACK_DATA);
            else
               vtx+=renderprisms(c->prism_cache,c->prism_size/3,t->lambda,t->tile->getwarp(),
                                 PRISM_R,PRISM_G,PRISM_B,PRISM_A,
                                 t->lx,t->ly,t->lz,
                                 t->ls,t->lo);
            }
         }

#endif

   return(vtx);
   }

int minicache::renderprisms(float *cache,int cnt,float lambda,miniwarp *warp,
                            float pr,float pg,float pb,float pa,
                            float lx,float ly,float lz,
                            float ls,float lo)
   {
   int vtx=0;

#ifndef NOOGL

#if defined(GL_ARB_vertex_program) && defined(GL_ARB_fragment_program)

   static const char *vtxprog="!!ARBvp1.0 \n\
      PARAM c=program.env[0]; \n\
      PARAM mat[4]={state.matrix.mvp}; \n\
      PARAM invtra[4]={state.matrix.modelview.invtrans}; \n\
      TEMP vtx,col,nrm,pos,vec; \n\
      ### fetch actual vertex \n\
      MOV vtx,vertex.position.xywz; \n\
      MOV vtx.w,c.w; \n\
      MOV col,vertex.color; \n\
      MOV nrm,vertex.normal; \n\
      ### transform vertex with modelview \n\
      DP4 pos.x,mat[0],vtx; \n\
      DP4 pos.y,mat[1],vtx; \n\
      DP4 pos.z,mat[2],vtx; \n\
      DP4 pos.w,mat[3],vtx; \n\
      ### transform normal with inverse transpose \n\
      DP4 vec.x,invtra[0],nrm; \n\
      DP4 vec.y,invtra[1],nrm; \n\
      DP4 vec.z,invtra[2],nrm; \n\
      DP4 vec.w,invtra[3],nrm; \n\
      ### write resulting vertex \n\
      MOV result.position,pos; \n\
      MOV result.color,col; \n\
      ### pass normal as tex coords \n\
      MOV result.texcoord[1],vec; \n\
      ### calculate spherical fog coord \n\
      DP3 result.fogcoord.x,pos,pos; \n\
      END \n";

   static const char *fragprog="!!ARBfp1.0 \n\
      PARAM l=program.env[0]; \n\
      PARAM p=program.env[1]; \n\
      TEMP col,nrm,len; \n\
      ### fetch fragment color \n\
      MOV col,fragment.color; \n\
      ### modulate with directional light \n\
      MOV nrm,fragment.texcoord[1]; \n\
      DP3 len.x,nrm,nrm; \n\
      RSQ len.x,len.x; \n\
      MUL nrm,nrm,len.x; \n\
      DP3_SAT nrm.z,nrm,l; \n\
      MAD nrm.z,nrm.z,p.x,p.y; \n\
      MUL_SAT col.xyz,col,nrm.z; \n\
      ### write resulting color \n\
      MOV result.color,col; \n\
      END \n";

   GLuint vtxprogid,fragprogid;

   GLfloat mvmtx[16];
   miniv3d invtra[3];
   miniv3d light;

   miniv4d mtx[3];
   double oglmtx[16];

   if (lambda<=0.0f || cnt==0) return(vtx);

   initglexts();
   initwglprocs();

   if (GLEXT_VP!=0 && GLEXT_FP!=0)
      {
      if (PRISMCACHE_VTXPROGID==0)
         {
         glGenProgramsARB(1,&vtxprogid);
         glBindProgramARB(GL_VERTEX_PROGRAM_ARB,vtxprogid);
         glProgramStringARB(GL_VERTEX_PROGRAM_ARB,GL_PROGRAM_FORMAT_ASCII_ARB,strlen(vtxprog),vtxprog);
         PRISMCACHE_VTXPROGID=vtxprogid;
         }

      if (PRISMCACHE_FRAGPROGID==0)
         {
         glGenProgramsARB(1,&fragprogid);
         glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,fragprogid);
         glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB,GL_PROGRAM_FORMAT_ASCII_ARB,strlen(fragprog),fragprog);
         PRISMCACHE_FRAGPROGID=fragprogid;
         }

      initstate();

      enableblending();

      mtxpush();
      mtxproj();
      mtxpush();
      mtxscale(CONFIGURE_ZSCALE_PRISMS,CONFIGURE_ZSCALE_PRISMS,CONFIGURE_ZSCALE_PRISMS); // prevent Z-fighting
      mtxmodel();

      glGetFloatv(GL_MODELVIEW_MATRIX,mvmtx);

      invtra[0].x=mvmtx[0];
      invtra[1].x=mvmtx[1];
      invtra[2].x=mvmtx[2];
      invtra[0].y=mvmtx[4];
      invtra[1].y=mvmtx[5];
      invtra[2].y=mvmtx[6];
      invtra[0].z=mvmtx[8];
      invtra[1].z=mvmtx[9];
      invtra[2].z=mvmtx[10];

      inv_mtx(invtra,invtra);
      tra_mtx(invtra,invtra);

      light=miniv3d(lx,ly,lz);
      light=miniv3d(invtra[0]*light,invtra[1]*light,invtra[2]*light);
      light.normalize();

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

      glBindProgramARB(GL_VERTEX_PROGRAM_ARB,PRISMCACHE_VTXPROGID);
      glEnable(GL_VERTEX_PROGRAM_ARB);

      glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB,0,0.0f,0.0f,0.0f,1.0f);

      glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,PRISMCACHE_FRAGPROGID);
      glEnable(GL_FRAGMENT_PROGRAM_ARB);

      glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,0,light.x,light.y,light.z,0.0f);
      glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,1,ls,lo,0.0f,0.0f);

      color(pr,pg,pb,pa);

      glVertexPointer(4,GL_FLOAT,0,cache);
      glEnableClientState(GL_VERTEX_ARRAY);
      glDrawArrays(GL_TRIANGLES,0,3*cnt);
      glDisableClientState(GL_VERTEX_ARRAY);

      vtx+=3*cnt;

      glBindProgramARB(GL_VERTEX_PROGRAM_ARB,0);
      glDisable(GL_VERTEX_PROGRAM_ARB);

      glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,0);
      glDisable(GL_FRAGMENT_PROGRAM_ARB);

      mtxpop();
      mtxproj();
      mtxpop();
      mtxmodel();

      exitstate();
      }

#endif

#endif

   return(vtx);
   }

// attach a tileset for scene double buffering
void minicache::attach(minitile *terrain,
                       void (*prismedge)(float x,float y,float yf,float z,void *data),
                       void (*prismcache)(int phase,float scale,float ex,float ey,float ez,void *data),
                       int (*prismrender)(float *cache,int cnt,float lambda,miniwarp *warp,void *data),
                       int (*prismtrigger)(int phase,void *data),
                       void (*prismsync)(int id,void *data),
                       void *data)
   {
   int id;

   makecurrent();

   if (TERRAIN==NULL)
      {
      MAXTERRAIN=1;
      if ((TERRAIN=(TERRAIN_TYPE *)malloc(MAXTERRAIN*sizeof(TERRAIN_TYPE)))==NULL) ERRORMSG();
      TERRAIN[0].tile=NULL;
      }

   if (NUMTERRAIN>=MAXTERRAIN)
      {
      if ((TERRAIN=(TERRAIN_TYPE *)realloc(TERRAIN,2*MAXTERRAIN*sizeof(TERRAIN_TYPE)))==NULL) ERRORMSG();
      for (id=NUMTERRAIN; id<2*MAXTERRAIN; id++) TERRAIN[id].tile=NULL;
      MAXTERRAIN*=2;
      }

   if (terrain!=NULL)
      for (id=0; id<MAXTERRAIN; id++)
         if (TERRAIN[id].tile==NULL)
            {
            TERRAIN[id].tile=terrain;

            terrain->setcallbacks(cache_beginfan,
                                  cache_fanvertex,
                                  NULL,cache_texmap,
                                  cache_prismedge,
                                  cache_trigger,
                                  cache_sync,
                                  id);

            initterrain(&TERRAIN[id]);

            NUMTERRAIN++;

            break;
            }

   PRISMEDGE_CALLBACK=prismedge;
   PRISMCACHE_CALLBACK=prismcache;
   PRISMRENDER_CALLBACK=prismrender;
   PRISMTRIGGER_CALLBACK=prismtrigger;
   PRISMSYNC_CALLBACK=prismsync;
   CALLBACK_DATA=data;
   }

// detach a tileset
void minicache::detach(minitile *terrain)
   {
   if (terrain==NULL) ERRORMSG();

   TERRAIN[terrain->getid()].tile=NULL;

   freeterrain(&TERRAIN[terrain->getid()]);
   }

// determine whether or not a tileset is displayed
void minicache::display(minitile *terrain,int yes)
   {
   if (terrain==NULL) ERRORMSG();

   TERRAIN[terrain->getid()].isvisible=yes;
   }

// specify per-tileset lighting
void minicache::setlight(minitile *terrain,float lx,float ly,float lz,float ls,float lo)
   {
   int id;

   if (terrain==NULL) ERRORMSG();

   id=terrain->getid();

   TERRAIN[id].lx=lx;
   TERRAIN[id].ly=ly;
   TERRAIN[id].lz=lz;

   TERRAIN[id].ls=ls;
   TERRAIN[id].lo=lo;
   }

// make cache current
void minicache::makecurrent()
   {CACHE=this;}

// set culling mode
void minicache::setculling(int on)
   {CULLMODE=on;}

// define triangle mesh opacity
void minicache::setopacity(float alpha)
   {OPACITY=alpha;}

// define alpha test threshold
void minicache::setalphatest(float alpha)
   {ALPHATEST=alpha;}

// define color of semi-transparent sea
void minicache::setseacolor(float r,float g,float b,float a)
   {
   SEA_R=r;
   SEA_G=g;
   SEA_B=b;
   SEA_A=a;
   }

// define rendering color of prism boundary
void minicache::setprismcolor(float prismR,float prismG,float prismB,float prismA)
   {
   PRISM_R=prismR;
   PRISM_G=prismG;
   PRISM_B=prismB;
   PRISM_A=prismA;
   }

// set vertex shader plugin
void minicache::setvtxshader(const char *vp)
   {
#ifndef NOOGL

   // default vertex shader
   static const char *vtxprog="!!ARBvp1.0 \n\
      PARAM t=program.env[0]; \n\
      PARAM e=program.env[1]; \n\
      PARAM c0=program.env[2]; \n\
      PARAM c1=program.env[3]; \n\
      PARAM c2=program.env[4]; \n\
      PARAM c3=program.env[5]; \n\
      PARAM c4=program.env[6]; \n\
      PARAM c5=program.env[7]; \n\
      PARAM c6=program.env[8]; \n\
      PARAM c7=program.env[9]; \n\
      PARAM mat[4]={state.matrix.mvp}; \n\
      PARAM invtra[4]={state.matrix.modelview.invtrans}; \n\
      TEMP vtx,col,nrm,pos,vec; \n\
      ### fetch actual vertex \n\
      MOV vtx,vertex.position; \n\
      MOV col,vertex.color; \n\
      MOV nrm,vertex.normal; \n\
      ### transform vertex with modelview \n\
      DP4 pos.x,mat[0],vtx; \n\
      DP4 pos.y,mat[1],vtx; \n\
      DP4 pos.z,mat[2],vtx; \n\
      DP4 pos.w,mat[3],vtx; \n\
      ### transform normal with inverse transpose \n\
      DP4 vec.x,invtra[0],nrm; \n\
      DP4 vec.y,invtra[1],nrm; \n\
      DP4 vec.z,invtra[2],nrm; \n\
      DP4 vec.w,invtra[3],nrm; \n\
      ### write resulting vertex \n\
      MOV result.position,pos; \n\
      MOV result.color,col; \n\
      ### calculate tex coords \n\
      MAD result.texcoord[0].x,vtx.x,t.x,t.z; \n\
      MAD result.texcoord[0].y,vtx.z,t.y,t.w; \n\
      MUL result.texcoord[0].z,vtx.y,e.y; \n\
      ### pass normal as tex coords \n\
      MOV result.texcoord[1],vec; \n\
      ### calculate spherical fog coord \n\
      DP3 result.fogcoord.x,pos,pos; \n\
      END \n";

   if (vp==NULL) vp=vtxprog;

   if (VTXPROG!=NULL)
      {
      if (strcmp(vp,VTXPROG)==0) return;
      free(VTXPROG);
      }

   VTXPROG=strdup(vp);
   VTXDIRTY=1;

#endif
   }

// set vertex shader parameter vector
void minicache::setvtxshaderparams(float p1,float p2,float p3,float p4,int n)
   {
   if (n<0 || n>=8) ERRORMSG();

   VTXSHADERPAR1[n]=p1;
   VTXSHADERPAR2[n]=p2;
   VTXSHADERPAR3[n]=p3;
   VTXSHADERPAR4[n]=p4;
   }

// enable vertex shader plugin
void minicache::enablevtxshader()
   {
#ifndef NOOGL

#if defined(GL_ARB_vertex_program) && defined(GL_ARB_fragment_program)

   int i;

   GLuint vtxprogid;

   initglexts();
   initwglprocs();

   if (GLEXT_VP!=0 && GLEXT_FP!=0)
      {
      if (VTXDIRTY!=0)
         {
         if (VTXPROGID!=0)
            {
            vtxprogid=VTXPROGID;
            glDeleteProgramsARB(1,&vtxprogid);
            }

         glGenProgramsARB(1,&vtxprogid);
         glBindProgramARB(GL_VERTEX_PROGRAM_ARB,vtxprogid);
         glProgramStringARB(GL_VERTEX_PROGRAM_ARB,GL_PROGRAM_FORMAT_ASCII_ARB,strlen(VTXPROG),VTXPROG);
         VTXPROGID=vtxprogid;

         VTXDIRTY=0;
         }

      if (VTXPROGID!=0)
         {
         glBindProgramARB(GL_VERTEX_PROGRAM_ARB,VTXPROGID);
         glEnable(GL_VERTEX_PROGRAM_ARB);

         for (i=0; i<8; i++)
            glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB,2+i,VTXSHADERPAR1[i],VTXSHADERPAR2[i],VTXSHADERPAR3[i],VTXSHADERPAR4[i]);
         }
      }

#endif

#endif
   }

// set vertex shader texture mapping parameters
void minicache::setvtxshadertexprm(float s1,float s2,float o1,float o2,float scale)
   {
#ifndef NOOGL

#if defined(GL_ARB_vertex_program) && defined(GL_ARB_fragment_program)

   if (GLEXT_VP!=0 && GLEXT_FP!=0)
      if (VTXPROGID!=0)
         {
         glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB,0,s1,s2,o1,o2);
         glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB,1,0.0f,scale,0.0f,0.0f);
         }

#endif

#endif
   }

// disable vertex shader plugin
void minicache::disablevtxshader()
   {
#ifndef NOOGL

#if defined(GL_ARB_vertex_program) && defined(GL_ARB_fragment_program)

   if (GLEXT_VP!=0 && GLEXT_FP!=0)
      if (VTXPROGID!=0)
         {
         glBindProgramARB(GL_VERTEX_PROGRAM_ARB,0);
         glDisable(GL_VERTEX_PROGRAM_ARB);
         }

#endif

#endif
   }

// switch vertex shader plugin on/off
void minicache::usevtxshader(int on)
   {USEVTXSHADER=on;}

// set pixel shader plugin
void minicache::setpixshader(const char *fp)
   {
#ifndef NOOGL

   // default pixel shader
   static const char *fragprog="!!ARBfp1.0 \n\
      PARAM c0=program.env[0]; \n\
      PARAM c1=program.env[1]; \n\
      PARAM c2=program.env[2]; \n\
      PARAM c3=program.env[3]; \n\
      PARAM c4=program.env[4]; \n\
      PARAM c5=program.env[5]; \n\
      PARAM c6=program.env[6]; \n\
      PARAM c7=program.env[7]; \n\
      PARAM a=program.env[8]; \n\
      PARAM t=program.env[9]; \n\
      PARAM l=program.env[10]; \n\
      PARAM p=program.env[11]; \n\
      TEMP col,nrm,len; \n\
      ### fetch texture color \n\
      TEX col,fragment.texcoord[0],texture[0],2D; \n\
      MAD col,col,a.x,a.y; \n\
      ### modulate with directional light \n\
      MOV nrm,fragment.texcoord[1]; \n\
      DP3 len.x,nrm,nrm; \n\
      RSQ len.x,len.x; \n\
      MUL nrm,nrm,len.x; \n\
      DP3_SAT nrm.z,nrm,l; \n\
      MAD nrm.z,nrm.z,p.x,p.y; \n\
      MUL_SAT col.xyz,col,nrm.z; \n\
      ### modulate with fragment color \n\
      MUL result.color,col,fragment.color; \n\
      END \n";

   if (fp==NULL) fp=fragprog;

   if (FRAGPROG!=NULL)
      {
      if (strcmp(fp,FRAGPROG)==0) return;
      free(FRAGPROG);
      }

   FRAGPROG=strdup(fp);
   FRAGDIRTY=1;

#endif
   }

// set pixel shader parameter vector
void minicache::setpixshaderparams(float p1,float p2,float p3,float p4,int n)
   {
   if (n<0 || n>=8) ERRORMSG();

   PIXSHADERPAR1[n]=p1;
   PIXSHADERPAR2[n]=p2;
   PIXSHADERPAR3[n]=p3;
   PIXSHADERPAR4[n]=p4;
   }

// set pixel shader RGB texture map
void minicache::setpixshadertexRGB(unsigned char *image,int width,int height)
   {setpixshadertex(image,width,height,3);}

// set pixel shader RGBA texture map
void minicache::setpixshadertexRGBA(unsigned char *image,int width,int height)
   {setpixshadertex(image,width,height,4);}

// set pixel shader RGB[A] texture map
void minicache::setpixshadertex(unsigned char *image,int width,int height,int components)
   {
   if (width<2 || height<2) ERRORMSG();

   if (PIXSHADERTEXID!=0)
      {
      deletetexmap(PIXSHADERTEXID);
      PIXSHADERTEXID=0;
      }

   if (image!=NULL)
      {
      if (components==3) PIXSHADERTEXID=buildRGBtexmap(image,&width,&height,0);
      else if (components==4) PIXSHADERTEXID=buildRGBAtexmap(image,&width,&height,0);
      else ERRORMSG();

      PIXSHADERTEXWIDTH=width;
      PIXSHADERTEXHEIGHT=height;
      }
   }

// switch pixel shader plugin on/off
void minicache::usepixshader(int on)
   {USEPIXSHADER=on;}

// set sea shader plugin
void minicache::setseashader(const char *sp)
   {
#ifndef NOOGL

   // default sea shader
   static const char *seaprog="!!ARBfp1.0 \n\
      PARAM c0=program.env[0]; \n\
      PARAM c1=program.env[1]; \n\
      PARAM c2=program.env[2]; \n\
      PARAM c3=program.env[3]; \n\
      PARAM c4=program.env[4]; \n\
      PARAM c5=program.env[5]; \n\
      PARAM c6=program.env[6]; \n\
      PARAM c7=program.env[7]; \n\
      PARAM a=program.env[8]; \n\
      PARAM t=program.env[9]; \n\
      PARAM l=program.env[10]; \n\
      PARAM p=program.env[11]; \n\
      TEMP col,nrm,len; \n\
      ### fetch texture color \n\
      TEX col,fragment.texcoord[0],texture[0],2D; \n\
      MAD col,col,a.x,a.y; \n\
      ### modulate with directional light \n\
      MOV nrm,fragment.texcoord[1]; \n\
      DP3 len.x,nrm,nrm; \n\
      RSQ len.x,len.x; \n\
      MUL nrm,nrm,len.x; \n\
      DP3_SAT nrm.z,nrm,l; \n\
      MAD nrm.z,nrm.z,p.x,p.y; \n\
      MUL_SAT col.xyz,col,nrm.z; \n\
      ### modulate with fragment color \n\
      MUL result.color,col,fragment.color; \n\
      END \n";

   if (sp==NULL) sp=seaprog;

   if (SEAPROG!=NULL)
      {
      if (strcmp(sp,SEAPROG)==0) return;
      free(SEAPROG);
      }

   SEAPROG=strdup(sp);
   SEADIRTY=1;

#endif
   }

// set sea shader parameter vector
void minicache::setseashaderparams(float p1,float p2,float p3,float p4,int n)
   {
   if (n<0 || n>=8) ERRORMSG();

   SEASHADERPAR1[n]=p1;
   SEASHADERPAR2[n]=p2;
   SEASHADERPAR3[n]=p3;
   SEASHADERPAR4[n]=p4;
   }

// set sea shader RGB texture map
void minicache::setseashadertexRGB(unsigned char *image,int width,int height)
   {setseashadertex(image,width,height,3);}

// set sea shader RGBA texture map
void minicache::setseashadertexRGBA(unsigned char *image,int width,int height)
   {setseashadertex(image,width,height,4);}

// set sea shader RGB[A] texture map
void minicache::setseashadertex(unsigned char *image,int width,int height,int components)
   {
   if (width<2 || height<2) ERRORMSG();

   if (SEASHADERTEXID!=0)
      {
      deletetexmap(SEASHADERTEXID);
      SEASHADERTEXID=0;
      }

   if (image!=NULL)
      {
      if (components==3) SEASHADERTEXID=buildRGBtexmap(image,&width,&height,0);
      else if (components==4) SEASHADERTEXID=buildRGBAtexmap(image,&width,&height,0);
      else ERRORMSG();

      SEASHADERTEXWIDTH=width;
      SEASHADERTEXHEIGHT=height;
      }
   }

// switch sea shader plugin on/off
void minicache::useseashader(int on)
   {USESEASHADER=on;}

// define optional sea callbacks
void minicache::setseacb(void (*preseacb)(void *data),
                         void (*postseacb)(void *data),
                         void *data)
   {
   PRESEA_CB=preseacb;
   POSTSEA_CB=postseacb;
   CB_DATA=data;
   }

// enable pixel shader plugin
void minicache::enablepixshader()
   {
#ifndef NOOGL

#if defined(GL_ARB_vertex_program) && defined(GL_ARB_fragment_program)

   int i;

   GLuint fragprogid;

   initglexts();
   initwglprocs();

   if (GLEXT_VP!=0 && GLEXT_FP!=0)
      {
      if (FRAGDIRTY!=0)
         {
         if (FRAGPROGID!=0)
            {
            fragprogid=FRAGPROGID;
            glDeleteProgramsARB(1,&fragprogid);
            }

         glGenProgramsARB(1,&fragprogid);
         glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,fragprogid);
         glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB,GL_PROGRAM_FORMAT_ASCII_ARB,strlen(FRAGPROG),FRAGPROG);
         FRAGPROGID=fragprogid;

         FRAGDIRTY=0;
         }

      if (FRAGPROGID!=0)
         {
         glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,FRAGPROGID);
         glEnable(GL_FRAGMENT_PROGRAM_ARB);

         for (i=0; i<8; i++)
            glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,i,PIXSHADERPAR1[i],PIXSHADERPAR2[i],PIXSHADERPAR3[i],PIXSHADERPAR4[i]);

         glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,8,1.0f,0.0f,0.0f,0.0f);

         if (GLEXT_MT!=0)
            if (PIXSHADERTEXID!=0)
               {
#ifdef GL_ARB_multitexture
               glActiveTextureARB(GL_TEXTURE1_ARB);
               bindtexmap(PIXSHADERTEXID,PIXSHADERTEXWIDTH,PIXSHADERTEXHEIGHT,0,0);
               glActiveTextureARB(GL_TEXTURE0_ARB);

               glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,9,
                                          (float)(PIXSHADERTEXWIDTH-1)/PIXSHADERTEXWIDTH,0.5f/PIXSHADERTEXWIDTH,
                                          (float)(PIXSHADERTEXHEIGHT-1)/PIXSHADERTEXHEIGHT,0.5f/PIXSHADERTEXHEIGHT);
#endif
               }
         }
      }

#endif

#endif
   }

// set pixel shader texture mapping parameters
void minicache::setpixshadertexprm(float s,float o,
                                   float lx,float ly,float lz,
                                   float ls,float lo)
   {
#ifndef NOOGL

#if defined(GL_ARB_vertex_program) && defined(GL_ARB_fragment_program)

   if (GLEXT_VP!=0 && GLEXT_FP!=0)
      if (FRAGPROGID!=0 || SEAPROGID!=0)
         {
         glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,8,s,o,0.0f,0.0f);

         glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,10,lx,ly,lz,0.0f);
         glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,11,ls,lo,0.0f,0.0f);
         }

#endif

#endif
   }

// disable pixel shader plugin
void minicache::disablepixshader()
   {
#ifndef NOOGL

#if defined(GL_ARB_vertex_program) && defined(GL_ARB_fragment_program)

   if (GLEXT_VP!=0 && GLEXT_FP!=0)
      if (FRAGPROGID!=0)
         {
         glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,0);
         glDisable(GL_FRAGMENT_PROGRAM_ARB);

         if (GLEXT_MT!=0)
            if (PIXSHADERTEXID!=0)
               {
#ifdef GL_ARB_multitexture
               glActiveTextureARB(GL_TEXTURE1_ARB);
               bindtexmap(0);
               glActiveTextureARB(GL_TEXTURE0_ARB);
#endif
               }
         }

#endif

#endif
   }

// enable sea shader plugin
void minicache::enableseashader()
   {
#ifndef NOOGL

#if defined(GL_ARB_vertex_program) && defined(GL_ARB_fragment_program)

   int i;

   GLuint seaprogid;

   initglexts();
   initwglprocs();

   if (GLEXT_VP!=0 && GLEXT_FP!=0)
      {
      if (SEADIRTY!=0)
         {
         if (SEAPROGID!=0)
            {
            seaprogid=SEAPROGID;
            glDeleteProgramsARB(1,&seaprogid);
            }

         glGenProgramsARB(1,&seaprogid);
         glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,seaprogid);
         glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB,GL_PROGRAM_FORMAT_ASCII_ARB,strlen(SEAPROG),SEAPROG);
         SEAPROGID=seaprogid;

         SEADIRTY=0;
         }

      if (SEAPROGID!=0)
         {
         glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,SEAPROGID);
         glEnable(GL_FRAGMENT_PROGRAM_ARB);

         for (i=0; i<8; i++)
            glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,i,SEASHADERPAR1[i],SEASHADERPAR2[i],SEASHADERPAR3[i],PIXSHADERPAR4[i]);

         glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,8,1.0f,0.0f,0.0f,0.0f);

         if (GLEXT_MT!=0)
            if (SEASHADERTEXID!=0)
               {
#ifdef GL_ARB_multitexture
               glActiveTextureARB(GL_TEXTURE1_ARB);
               bindtexmap(SEASHADERTEXID,SEASHADERTEXWIDTH,SEASHADERTEXHEIGHT,0,0);
               glActiveTextureARB(GL_TEXTURE0_ARB);

               glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,9,
                                          (float)(SEASHADERTEXWIDTH-1)/SEASHADERTEXWIDTH,0.5f/SEASHADERTEXWIDTH,
                                          (float)(SEASHADERTEXHEIGHT-1)/SEASHADERTEXHEIGHT,0.5f/SEASHADERTEXHEIGHT);
#endif
               }
         }
      }

#endif

#endif
   }

// disable sea shader plugin
void minicache::disableseashader()
   {
#ifndef NOOGL

#if defined(GL_ARB_vertex_program) && defined(GL_ARB_fragment_program)

   if (GLEXT_VP!=0 && GLEXT_FP!=0)
      if (SEAPROGID!=0)
         {
         glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,0);
         glDisable(GL_FRAGMENT_PROGRAM_ARB);

         if (GLEXT_MT!=0)
            if (SEASHADERTEXID!=0)
               {
#ifdef GL_ARB_multitexture
               glActiveTextureARB(GL_TEXTURE1_ARB);
               bindtexmap(0);
               glActiveTextureARB(GL_TEXTURE0_ARB);
#endif
               }
         }

#endif

#endif
   }

// check for OpenGL extensions
void minicache::initglexts()
   {
#ifndef NOOGL

   char *gl_exts;

   if (GLSETUP==0)
      {
      GLEXT_MT=0;
      GLEXT_VP=0;
      GLEXT_FP=0;

      if ((gl_exts=(char *)glGetString(GL_EXTENSIONS))==NULL) ERRORMSG();

      if (strstr(gl_exts,"GL_ARB_multitexture")!=NULL) GLEXT_MT=1;
      if (strstr(gl_exts,"GL_ARB_vertex_program")!=NULL) GLEXT_VP=1;
      if (strstr(gl_exts,"GL_ARB_fragment_program")!=NULL) GLEXT_FP=1;

      GLSETUP=1;
      }

#endif
   }

// Windows OpenGL extension setup
void minicache::initwglprocs()
   {
#ifndef NOOGL

#ifdef _WIN32

   if (WGLSETUP==0)
      {
#ifdef GL_ARB_multitexture
      glActiveTextureARB=(PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress("glActiveTextureARB");
      glClientActiveTextureARB=(PFNGLCLIENTACTIVETEXTUREARBPROC)wglGetProcAddress("glClientActiveTextureARB");
#endif

#if defined(GL_ARB_vertex_program) && defined(GL_ARB_fragment_program)
      glGenProgramsARB=(PFNGLGENPROGRAMSARBPROC)wglGetProcAddress("glGenProgramsARB");
      glBindProgramARB=(PFNGLBINDPROGRAMARBPROC)wglGetProcAddress("glBindProgramARB");
      glProgramStringARB=(PFNGLPROGRAMSTRINGARBPROC)wglGetProcAddress("glProgramStringARB");
      glProgramEnvParameter4fARB=(PFNGLPROGRAMENVPARAMETER4FARBPROC)wglGetProcAddress("glProgramEnvParameter4fARB");
      glDeleteProgramsARB=(PFNGLDELETEPROGRAMSARBPROC)wglGetProcAddress("glDeleteProgramsARB");
#endif

      WGLSETUP=1;
      }

#endif

#endif
   }

// get triangle fan count of active cache
int minicache::getfancnt()
   {
   int id,fancnt;

   fancnt=0;

   for (id=0; id<MAXTERRAIN; id++)
      if (TERRAIN[id].tile!=NULL) fancnt+=getfancnt(id);

   return(fancnt);
   }

// get vertex count of active cache
int minicache::getvtxcnt()
   {
   int id,vtxcnt;

   vtxcnt=0;

   for (id=0; id<MAXTERRAIN; id++)
      if (TERRAIN[id].tile!=NULL) vtxcnt+=getvtxcnt(id);

   return(vtxcnt);
   }

// get triangle fan count of active buffer
int minicache::getfancnt(int id)
   {
   TERRAIN_TYPE *t;
   CACHE_TYPE *c;

   t=&TERRAIN[id];
   c=&t->cache[1-t->cache_num];

   return(c->fancnt);
   }

// get vertex count of active buffer
int minicache::getvtxcnt(int id)
   {
   TERRAIN_TYPE *t;
   CACHE_TYPE *c;

   t=&TERRAIN[id];
   c=&t->cache[1-t->cache_num];

   return(c->vtxcnt);
   }

// get cached terrain object
minitile *minicache::gettile(int id)
   {return(TERRAIN[id].tile);}

// get ray intersection test object
miniray *minicache::getray(int id)
   {return(TERRAIN[id].ray);}

// configuring
void minicache::configure_overlap(float overlap) {CONFIGURE_OVERLAP=overlap;}
void minicache::configure_minsize(int minsize) {CONFIGURE_MINSIZE=minsize;}
void minicache::configure_seatwosided(int seatwosided) {CONFIGURE_SEATWOSIDED=seatwosided;}
void minicache::configure_seaenabletex(int seaenabletex) {CONFIGURE_SEAENABLETEX=seaenabletex;}
void minicache::configure_zfight_sea(float zscale) {CONFIGURE_ZSCALE_SEA=zscale;}
void minicache::configure_zfight_prisms(float zscale) {CONFIGURE_ZSCALE_PRISMS=zscale;}
void minicache::configure_enableray(int enableray) {CONFIGURE_ENABLERAY=enableray;}
void minicache::configure_omitsea(int omitsea) {CONFIGURE_OMITSEA=omitsea;}
