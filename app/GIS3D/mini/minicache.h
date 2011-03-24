// (c) by Stefan Roettger

#ifndef MINICACHE_H
#define MINICACHE_H

#include "miniv3d.h"
#include "miniv4d.h"

#include "miniOGL.h"

#include "minitile.h"
#include "miniwarp.h"

#include "miniray.h"

class minicache
   {
   public:

   //! default constructor
   minicache();

   //! destructor
   ~minicache();

   //! attach a tileset for scene double buffering
   void attach(minitile *terrain,
               void (*prismedge)(float x,float y,float yf,float z,void *data)=0,
               void (*prismcache)(int phase,float scale,float ex,float ey,float ez,void *data)=0,
               int (*prismrender)(float *cache,int cnt,float lambda,miniwarp *warp,void *data)=0,
               int (*prismtrigger)(int phase,void *data)=0,
               void (*prismsync)(int id,void *data)=0,
               void *data=0);

   //! detach a tileset
   void detach(minitile *terrain);

   //! determine whether or not a tileset is displayed
   void display(minitile *terrain,int yes=1);

   //! specify per-tileset lighting
   void setlight(minitile *terrain,float lx,float ly,float lz,float ls,float lo);

   //! make cache current
   //! required for multiple rendering windows/contexts
   //! this should be called before updating the vertex buffers of the corresponding window
   void makecurrent();

   //! render back buffer of the cache
   int rendercache();

   //! set culling mode
   void setculling(int on=1);

   //! define triangle mesh opacity
   void setopacity(float alpha=1.0f);

   //! define alpha test threshold
   void setalphatest(float alpha=0.0f);

   //! define color of semi-transparent sea
   void setseacolor(float r=0.0f,float g=0.5f,float b=1.0f,float a=1.0f);

   //! define color of prism boundary
   void setprismcolor(float prismR=1.0f,float prismG=1.0f,float prismB=1.0f,float prismA=0.9f);

   //! define optional vertex shader
   void setvtxshader(const char *vtxprog=0);
   void setvtxshaderparams(float p1=0.0f,float p2=0.0f,float p3=0.0f,float p4=0.0f,int n=0);
   void usevtxshader(int on=1);

   //! define optional pixel shader
   void setpixshader(const char *fragprog=0);
   void setpixshaderparams(float p1=0.0f,float p2=0.0f,float p3=0.0f,float p4=0.0f,int n=0);
   void setpixshadertex(unsigned char *image,int width,int height,int components=3);
   void setpixshadertexRGB(unsigned char *image,int width,int height);
   void setpixshadertexRGBA(unsigned char *image,int width,int height);
   void usepixshader(int on=1);

   //! define optional sea shader
   void setseashader(const char *seaprog=0);
   void setseashaderparams(float p1=0.0f,float p2=0.0f,float p3=0.0f,float p4=0.0f,int n=0);
   void setseashadertex(unsigned char *image,int width,int height,int components=3);
   void setseashadertexRGB(unsigned char *image,int width,int height);
   void setseashadertexRGBA(unsigned char *image,int width,int height);
   void useseashader(int on=0);

   //! define optional sea callbacks
   void setseacb(void (*preseacb)(void *data)=0,
                 void (*postseacb)(void *data)=0,
                 void *data=0);

   //! get number of rendered primitives
   int getfancnt(); // rendered triangle fans
   int getvtxcnt(); // rendered vertices

   //! get cached terrain object
   minitile *gettile(int id);

   //! get ray intersection test object
   miniray *getray(int id);

   //! configuring
   void configure_overlap(float overlap=0.02f); // overlap of tile borders (prevents pixel flickering)
   void configure_minsize(int minsize=33); // disable overlap for mesh size less than minsize
   void configure_seatwosided(int seatwosided=1); // disable back face culling for the sea surface
   void configure_seaenabletex(int seaenabletex=0); // enable texture mappping for the sea surface
   void configure_zfight_sea(float zscale=0.99f); // shift sea towards the viewer to avoid Z-fighting
   void configure_zfight_prisms(float zscale=0.95f); // shift prisms towards the viewer to avoid Z-fighting
   void configure_enableray(int enableray=0); // enable ray intersection test
   void configure_omitsea(int omitsea=0); // omit sea level ray tests

   protected:

   enum
      {
      BEGINFAN_OP=1,
      FANVERTEX_OP=2,
      TEXMAP_OP=3,
      TRIGGER_OP=4
      };

   struct CACHE_STRUCT
      {
      unsigned char *op;
      float *arg;
      int size,maxsize;

      int fancnt,vtxcnt;

      float *prism_cache;
      int prism_size,prism_maxsize;
      };

   typedef CACHE_STRUCT CACHE_TYPE;

   struct TERRAIN_STRUCT
      {
      minitile *tile;
      float scale,lambda;

      int cache_num;
      int cache_phase;

      CACHE_TYPE cache[2];

      int last_beginfan;

      miniray *ray;

      int first_fancnt;
      int first_beginfan;
      float first_scale;
      int first_col;
      int first_row;
      int first_size;

      int render_phase;
      int render_count;

      int isvisible;

      float lx,ly,lz;
      float ls,lo;
      };

   typedef TERRAIN_STRUCT TERRAIN_TYPE;

   int CACHE_ID;
   int RENDER_ID;

   static minicache *CACHE;

   TERRAIN_TYPE *TERRAIN;
   int NUMTERRAIN,MAXTERRAIN;

   static void cache_beginfan();
   static void cache_fanvertex(const float i,const float y,const float j);
   static void cache_texmap(const int m,const int n,const int S);
   static void cache_prismedge(const float x,const float y,const float yf,const float z);
   static void cache_trigger(const int phase,const float scale,const float ex,const float ey,const float ez);
   static void cache_sync(const int id);

   void cache(const int op,const float arg1=0.0f,const float arg2=0.0f,const float arg3=0.0f);
   void cacheprismedge(const float x,const float y,const float yf,const float z);
   void cachetrigger(const int phase,const float scale,const float ex,const float ey,const float ez);
   void cachesync(const int id);

   inline int rendercache(int id,int phase);

   inline void rendertexmap(int m,int n,int S);

   inline int rendertrigger(int phase);
   inline int rendertrigger(int phase,float scale);
   inline int rendertrigger();

   int CULLMODE;

   float OPACITY,ALPHATEST;
   float SEA_R,SEA_G,SEA_B,SEA_A;

   float PRISM_R,PRISM_G,PRISM_B,PRISM_A;

   char *VTXPROG;
   int VTXDIRTY;

   int VTXPROGID;
   int USEVTXSHADER;

   float VTXSHADERPAR1[8];
   float VTXSHADERPAR2[8];
   float VTXSHADERPAR3[8];
   float VTXSHADERPAR4[8];

   char *FRAGPROG;
   int FRAGDIRTY;

   int FRAGPROGID;
   int USEPIXSHADER;

   float PIXSHADERPAR1[8];
   float PIXSHADERPAR2[8];
   float PIXSHADERPAR3[8];
   float PIXSHADERPAR4[8];

   int PIXSHADERTEXID;
   int PIXSHADERTEXWIDTH;
   int PIXSHADERTEXHEIGHT;

   char *SEAPROG;
   int SEADIRTY;

   int SEAPROGID;
   int USESEASHADER;

   float SEASHADERPAR1[8];
   float SEASHADERPAR2[8];
   float SEASHADERPAR3[8];
   float SEASHADERPAR4[8];

   int SEASHADERTEXID;
   int SEASHADERTEXWIDTH;
   int SEASHADERTEXHEIGHT;

   int PRISMCACHE_VTXPROGID;
   int PRISMCACHE_FRAGPROGID;

   void (*PRISMEDGE_CALLBACK)(float x,float y,float yf,float z,void *data);
   void (*PRISMCACHE_CALLBACK)(int phase,float scale,float ex,float ey,float ez,void *data);
   int (*PRISMRENDER_CALLBACK)(float *cache,int cnt,float lambda,miniwarp *warp,void *data);
   int (*PRISMTRIGGER_CALLBACK)(int phase,void *data);
   void (*PRISMSYNC_CALLBACK)(int id,void *data);
   void *CALLBACK_DATA;

   void (*PRESEA_CB)(void *data);
   void (*POSTSEA_CB)(void *data);
   void *CB_DATA;

   private:

   float CONFIGURE_OVERLAP;
   int CONFIGURE_MINSIZE;
   int CONFIGURE_SEATWOSIDED;
   int CONFIGURE_SEAENABLETEX;
   float CONFIGURE_ZSCALE_SEA;
   float CONFIGURE_ZSCALE_PRISMS;
   float CONFIGURE_ENABLERAY;
   float CONFIGURE_OMITSEA;

   void initterrain(TERRAIN_TYPE *terrain);
   void freeterrain(TERRAIN_TYPE *terrain);

   int renderprisms(float *cache,int cnt,float lambda,miniwarp *warp,
                    float pr=1.0f,float pg=1.0f,float pb=1.0f,float pa=0.9f,
                    float lx=0.0f,float ly=0.0f,float lz=0.0f,
                    float ls=0.0f,float lo=1.0f);

   void enablevtxshader();
   void setvtxshadertexprm(float s1,float s2,float o1,float o2,float scale);
   void disablevtxshader();

   void enablepixshader();
   void setpixshadertexprm(float s,float o,float lx=0.0f,float ly=0.0f,float lz=0.0f,float ls=0.0f,float lo=1.0f);
   void disablepixshader();

   void enableseashader();
   void disableseashader();

   int getfancnt(int id);
   int getvtxcnt(int id);

   // OpenGL extensions:

   int GLSETUP;

   int GLEXT_MT;
   int GLEXT_VP;
   int GLEXT_FP;

   void initglexts();

   // Windows OpenGL extensions:

   int WGLSETUP;

#ifndef NOOGL

#ifdef _WIN32

#ifdef GL_ARB_multitexture
   PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;
   PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB;
#endif

#if defined(GL_ARB_vertex_program) && defined(GL_ARB_fragment_program)
   PFNGLGENPROGRAMSARBPROC glGenProgramsARB;
   PFNGLBINDPROGRAMARBPROC glBindProgramARB;
   PFNGLPROGRAMSTRINGARBPROC glProgramStringARB;
   PFNGLPROGRAMENVPARAMETER4FARBPROC glProgramEnvParameter4fARB;
   PFNGLDELETEPROGRAMSARBPROC glDeleteProgramsARB;
#endif

#endif

#endif

   void initwglprocs();
   };

#endif
