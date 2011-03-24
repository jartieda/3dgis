// (c) by Stefan Roettger

#ifndef MINIOGL_H
#define MINIOGL_H

// OpenGL dependent functions:

namespace miniOGL {

void initstate();
void exitstate();

void disableculling();
void enableBFculling();
void enableFFculling();

void disableAtest();
void enableAtest(float alpha=0.0f);

void disableZtest();
void enableZtest();

void disableZwriting();
void enableZwriting();

void enableAwriting();
void enableRGBwriting();
void enableRGBAwriting();
void disableRGBAwriting();

void enableblending();
void enableADDblending();
void enableATTblending();
void enableEMIblending();
void enablePRJblending();
void enableMIPblending();
void disableblending();

int buildLtexmap(unsigned char *image,int *width,int *height,int mipmaps=1,int mipmapped=0);
int buildLAtexmap(unsigned char *image,int *width,int *height,int mipmaps=1,int mipmapped=0);
int buildRGBtexmap(unsigned char *image,int *width,int *height,int mipmaps=1,int s3tc=0,int bytes=0,int mipmapped=0);
int buildRGBAtexmap(unsigned char *image,int *width,int *height,int mipmaps=1,int s3tc=0,int bytes=0,int mipmapped=0);
int buildRGBAtexmap(unsigned char *imageRGB,unsigned char *imageA,int *width,int *height);
int buildtexmap(unsigned char *image,int *width,int *height,int components,int depth,int mipmaps=1,int s3tc=0,int bytes=0,int mipmapped=0);
void bindtexmap(int texid,int width=0,int height=0,int size=0,int mipmaps=1);
int build3Dtexmap(unsigned char *volume,int *width,int *height,int *depth,int components);
void bind3Dtexmap(int texid);
void deletetexmap(int texid);

int compressRGBtexmap(unsigned char *image,int width,int height,
                      unsigned char **data,int *bytes);

void mtxmodel();
void mtxproj();
void mtxtex();

void mtxpush();
void mtxpop();

void mtxid();
void mtxscale(const float sx,const float sy,const float sz);
void mtxtranslate(const float tx,const float ty,const float tz);
void mtxrotate(const float angle,const float ax,const float ay,const float az);

void mtxmult(const float mtx[16]);
void mtxmult(const double mtx[16]);

void beginfans();
inline void beginfan();
inline void color(const float r,const float g,const float b,const float a=1.0f);
inline void normal(const float dx,const float dy,const float dz);
inline void texcoord(const float s,const float t,const float r=0.0f);
inline void fanvertex(const float x,const float y,const float z);
void endfans();

int getfancnt();
int getvtxcnt();

void linewidth(const int w);
void enablelinesmooth();
void disablelinesmooth();

inline void drawline(const float x1,const float y1,const float z1,const float x2,const float y2,const float z2);

unsigned char *readrgbpixels(int x,int y,int width,int height);
void writergbpixels(unsigned char *pixels,int width,int height,int winwidth,int winheight,int x,int y);

// configuring
void configure_generatemm(int generatemm=0); // enable hw-accelerated mipmap generation
void configure_compression(int compression=1); // enable texture compression (may result in texture seams)
int configure_depth(int depth=24); // bit depth of texture maps (may result in quantization artifacts)
void configure_gamma(float gamma=1.0f); // gamma correction of texture maps
void configure_level(float level=0.0f); // level adjustment of texture maps

// debugging
int get_unsupported_glexts(); // get number of unsupported OpenGL extensions
void print_unsupported_glexts(); // print unsupported OpenGL extensions

}

// OpenGL includes:

#ifndef NOOGL

#ifdef _WIN32
#   ifndef APIENTRY
#      define NOMINMAX
#      include <windows.h>
#   endif
#endif

#ifndef __APPLE__
#   ifndef _WIN32
#      define GL_GLEXT_PROTOTYPES
#   endif
#   include <GL/gl.h>
#   include <GL/glu.h>
#   ifndef __sgi
#      include "GL/glext.h"
#   endif
#else
#   include <OpenGL/gl.h>
#   include <OpenGL/glu.h>
#   include <OpenGL/glext.h>
#endif

// OpenGL 1.0 workaround:

#ifndef GL_VERSION_1_1
#   ifndef glGenTextures
#      define glGenTextures glGenTexturesEXT
#   endif
#   ifndef glBindTexture
#      define glBindTexture glBindTextureEXT
#   endif
#   ifndef glDeleteTextures
#      define glDeleteTextures glDeleteTexturesEXT
#   endif
#   ifndef glVertexPointer
#      define glVertexPointer glVertexPointerEXT
#   endif
#   ifndef glEnableClientState
#      define glEnableClientState glEnableClientStateEXT
#   endif
#   ifndef glDisableClientState
#      define glDisableClientState glDisableClientStateEXT
#   endif
#   ifndef glInterleavedArrays
#      define glInterleavedArrays glInterleavedArraysEXT
#   endif
#   ifndef glDrawArrays
#      define glDrawArrays glDrawArraysEXT
#   endif
#endif

// OpenGL 1.1 workaround:

#ifndef GL_VERSION_1_2
#   ifndef glBlendEquation
#      define glBlendEquation glBlendEquationEXT
#   endif
#   ifndef GL_TEXTURE_3D
#      define GL_TEXTURE_3D GL_TEXTURE_3D_EXT
#   endif
#   ifndef GL_TEXTURE_WRAP_R
#      define GL_TEXTURE_WRAP_R GL_TEXTURE_WRAP_R_EXT
#   endif
#   ifndef glTexImage3D
#      define glTexImage3D glTexImage3DEXT
#   endif
#endif

#endif

// OpenGL dependent inline functions:

namespace miniOGL {

#ifndef NOOGL
extern int fancnt,vtxcnt;
#endif

inline void beginfan()
   {
#ifndef NOOGL
   if (fancnt++>0) glEnd();
   glBegin(GL_TRIANGLE_FAN);
#endif
   }

inline void color(const float r,const float g,const float b,const float a)
   {
#ifndef NOOGL
   glColor4f(r,g,b,a);
#endif
   }

inline void normal(const float dx,const float dy,const float dz)
   {
#ifndef NOOGL
   glNormal3f(dx,dy,dz);
#endif
   }

inline void texcoord(const float s,const float t,const float r)
   {
#ifndef NOOGL
   glTexCoord3f(s,t,r);
#endif
   }

inline void fanvertex(const float x,const float y,const float z)
   {
#ifndef NOOGL
   glVertex3f(x,y,z);
   vtxcnt++;
#endif
   }

inline void drawline(const float x1,const float y1,const float z1,const float x2,const float y2,const float z2)
   {
#ifndef NOOGL
   glBegin(GL_LINES);
   glVertex3f(x1,y1,z1);
   glVertex3f(x2,y2,z2);
   glEnd();
#endif
   }

}

using namespace miniOGL;

#endif
