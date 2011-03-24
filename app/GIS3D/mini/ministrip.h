// (c) by Stefan Roettger

#ifndef MINISTRIP_H
#define MINISTRIP_H

//! pre-defined snippet names
#define MINI_SNIPPET_VTX_BEGIN "snippet_vtx_begin"
#define MINI_SNIPPET_VTX_HEADER "snippet_vtx_header"
#define MINI_SNIPPET_VTX_BASIC "snippet_vtx_basic"
#define MINI_SNIPPET_VTX_NORMAL "snippet_vtx_normal"
#define MINI_SNIPPET_VTX_NORMAL_DIRECT "snippet_vtx_normal_direct"
#define MINI_SNIPPET_VTX_TEX "snippet_vtx_tex"
#define MINI_SNIPPET_VTX_FOG "snippet_vtx_fog"
#define MINI_SNIPPET_VTX_FOOTER "snippet_vtx_footer"
#define MINI_SNIPPET_VTX_END "snippet_vtx_end"
#define MINI_SNIPPET_FRG_BEGIN "snippet_frg_begin"
#define MINI_SNIPPET_FRG_HEADER "snippet_frg_header"
#define MINI_SNIPPET_FRG_BASIC "snippet_frg_basic"
#define MINI_SNIPPET_FRG_TEX "snippet_frg_tex"
#define MINI_SNIPPET_FRG_TEX2_DIRECT "snippet_frg_tex2_direct"
#define MINI_SNIPPET_FRG_SHADE "snippet_frg_shade"
#define MINI_SNIPPET_FRG_SHADE_DIRECT "snippet_frg_shade_direct"
#define MINI_SNIPPET_FRG_FOG "snippet_frg_fog"
#define MINI_SNIPPET_FRG_FOOTER "snippet_frg_footer"
#define MINI_SNIPPET_FRG_END "snippet_frg_end"

#include "database.h"

#include "miniOGL.h"

//! container class for triangle strips
class ministrip
   {
   public:

   //! maximum number of shader resources
   static const int SHADERMAX=100;
   static const int SNIPPETMAX=1000;
   static const int SHADERVTXPRMMAX=8;
   static const int SHADERFRGPRMMAX=8;
   static const int SHADERFRGTEXMAX=6;

   //! default constructor
   ministrip(int colcomps=0,int nrmcomps=0,int texcomps=0);

   //! destructor
   ~ministrip();

   //! clear strip
   void clear();

   //! begin next triangle strip
   void beginstrip();

   //! set color of next vertex
   void setcol(const float r,const float g,const float b,float a=1.0f);

   //! set normal of next vertex
   void setnrm(const float nx,const float ny,const float nz);

   //! set tex coords of next vertex
   void settex(const float tx,const float ty=0.0f,const float tz=0.0f,float tw=1.0f);

   //! add one vertex
   void addvtx(const float x,const float y,const float z);

   //! set scale
   void setscale(float scale);

   //! set matrix
   void setmatrix(double mtx[16]);

   //! set texture matrix
   void settexmatrix(double texmtx[16]);

   //! set Z-scaling
   void setZscale(float zscale);

   //! define shader snippet
   static void addsnippet(const char *snippetname,const char *snippet);

   //! get free shader slot
   static int getfreeslot();

   //! define vertex shader
   static void setvtxshader(int num,char *vtxprog);
   static void concatvtxshader(int num,const char *snippetname);
   static void setvtxshaderparams(int num,float p1=0.0f,float p2=0.0f,float p3=0.0f,float p4=0.0f,int n=0);

   //! define pixel shader
   static void setpixshader(int num,char *frgprog);
   static void concatpixshader(int num,const char *snippetname);
   static void setpixshaderparams(int num,float p1=0.0f,float p2=0.0f,float p3=0.0f,float p4=0.0f,int n=0);
   static void setpixshadertex(int num,unsigned char *image,int width,int height,int components=3,int mipmaps=1,int n=0);
   static void setpixshadertexRGB(int num,unsigned char *image,int width,int height,int mipmaps=1,int n=0);
   static void setpixshadertexRGBA(int num,unsigned char *image,int width,int height,int mipmaps=1,int n=0);
   static void setpixshadertexbuf(int num,databuf *buf,int mipmaps=1,int n=0);

   //! set direct shading parameters
   void setshadedirectparams(int num,
                             float lightdir[3],
                             float lightbias=0.5f,float lightoffset=0.5f);

   //! set direct texturing parameters
   void settexturedirectparams(int num,
                               float lightdir[3],
                               float transbias=4.0f,float transoffset=0.0f);

   //! set fog parameters
   void setfogparams(int num,
                     float fogstart,float fogend,
                     float fogdensity,
                     float fogcolor[3]);

   //! set actual shader
   void useshader(int num=0);

   //! get actual shader
   int getshader();

   //! render triangle strips
   void render();

   //! render triangle strips with multi-pass method for unordered semi-transparent geometry
   void rendermulti(int passes=4,
                    float ambient=0.1f,
                    float bordercontrol=1.0f,float centercontrol=1.0f,float colorcontrol=1.0f,
                    float bordercontrol2=1.0f,float centercontrol2=1.0f,float colorcontrol2=1.0f,
                    float stripewidth=1.0f,float stripeoffset=0.0f,
                    float stripedx=0.0f,float stripedy=0.0f,float stripedz=1.0f,
                    int correctz=0);

   //! get array size (number of vertices)
   int getsize() {return(SIZE);}

   //! get vertex array
   float *getvtx() {return(VTXARRAY);}

   //! get reference to vertex array
   float **getvtxref() {return(&VTXARRAY);}

   //! get vertex shader
   char *getvtxshader(int num);

   //! get pixel shader
   char *getpixshader(int num);

   protected:

   int SIZE;
   int MAXSIZE;

   float *VTXARRAY; // 3 components

   float *COLARRAY; // 3 or 4 components
   int COLCOMPS;

   float *NRMARRAY; // 3 components
   int NRMCOMPS;

   float *TEXARRAY; // 1-4 components
   int TEXCOMPS;

   float VTXX,VTXY,VTXZ;
   float COLR,COLG,COLB,COLA;
   float NRMX,NRMY,NRMZ;
   float TEXX,TEXY,TEXZ,TEXW;

   int COPYVTX;

   float SCALE;

   double MTX[16];
   double TEXMTX[16];

   float ZSCALE;

   void addvtx();

   void enablevtxshader(int num);
   void disablevtxshader(int num);

   void enablepixshader(int num);
   void disablepixshader(int num);

   static void initsnippets();
   static void freesnippets();

   static void initshader();

   private:

   struct SHADER_STRUCT
      {
      char *vtxprog;

      int vtxprogid;
      int vtxdirty;

      float vtxshaderpar1[SHADERVTXPRMMAX];
      float vtxshaderpar2[SHADERVTXPRMMAX];
      float vtxshaderpar3[SHADERVTXPRMMAX];
      float vtxshaderpar4[SHADERVTXPRMMAX];

      char *frgprog;

      int frgprogid;
      int frgdirty;

      float pixshaderpar1[SHADERFRGPRMMAX];
      float pixshaderpar2[SHADERFRGPRMMAX];
      float pixshaderpar3[SHADERFRGPRMMAX];
      float pixshaderpar4[SHADERFRGPRMMAX];

      int pixshadertexid[SHADERFRGTEXMAX];
      int pixshadertexw[SHADERFRGTEXMAX];
      int pixshadertexh[SHADERFRGTEXMAX];
      int pixshadertexmm[SHADERFRGTEXMAX];
      };

   typedef SHADER_STRUCT SHADER_TYPE;

   struct SNIPPET_STRUCT
      {
      char *snippetname;
      char *snippet;
      };

   typedef SNIPPET_STRUCT SNIPPET_TYPE;

   static int INSTANCES;

   static SHADER_TYPE SHADER[SHADERMAX];

   static SNIPPET_TYPE SNIPPET[SNIPPETMAX];
   static int SNIPPETS;

   int USESHADER;

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
