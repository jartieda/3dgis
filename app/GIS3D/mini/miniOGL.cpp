// (c) by Stefan Roettger

#include "miniOGLP.h"

namespace miniOGL {

// configuring
void configure_generatemm(int generatemm) {CONFIGURE_GENERATEMM=generatemm;}
void configure_compression(int compression) {CONFIGURE_COMPRESSION=compression;}
int configure_depth(int depth) {int prev=CONFIGURE_DEPTH; CONFIGURE_DEPTH=depth; return(prev);}
void configure_gamma(float gamma) {CONFIGURE_GAMMA=gamma;}
void configure_level(float level) {CONFIGURE_LEVEL=level;}

#ifndef NOOGL

static void initglexts()
   {
   char *GL_EXTs;

   static BOOLINT done=FALSE;

   if (!done)
      {
      glext_mm=FALSE;
      glext_tec=FALSE;
      glext_tfa=FALSE;
      glext_t3D=FALSE;
      glext_tc=FALSE;
      glext_ts3=FALSE;
      glext_tgm=FALSE;

      if ((GL_EXTs=(char *)glGetString(GL_EXTENSIONS))==NULL) ERRORMSG();

      if (strstr(GL_EXTs,"GL_EXT_blend_minmax")!=NULL) glext_mm=TRUE;
      if (strstr(GL_EXTs,"GL_SGIS_texture_edge_clamp")!=NULL) glext_tec=TRUE;
      if (strstr(GL_EXTs,"GL_EXT_texture_edge_clamp")!=NULL) glext_tec=TRUE;
      if (strstr(GL_EXTs,"GL_EXT_texture_filter_anisotropic")!=NULL) glext_tfa=TRUE;
      if (strstr(GL_EXTs,"GL_EXT_texture3D")!=NULL) glext_t3D=TRUE;
      if (strstr(GL_EXTs,"GL_ARB_texture_compression")!=NULL) glext_tc=TRUE;
      if (strstr(GL_EXTs,"GL_EXT_texture_compression_s3tc")!=NULL) glext_ts3=TRUE;
      if (strstr(GL_EXTs,"GL_SGIS_generate_mipmap")!=NULL) glext_tgm=TRUE;

      done=TRUE;
      }
   }

int get_unsupported_glexts()
   {
   int num;

   num=0;

   if (!glext_mm) num++;
   if (!glext_tec) num++;
   if (!glext_tfa) num++;
   if (!glext_t3D) num++;
   if (!glext_tc) num++;
   if (!glext_ts3) num++;
   if (!glext_tgm) num++;

   return(num);
   }

void print_unsupported_glexts()
   {
   initglexts();

   if (!glext_mm || !glext_tec || !glext_tfa || !glext_t3D || !glext_tc || !glext_ts3 || !glext_tgm)
      {
      printf("unsupported OpenGL extensions:");

      if (!glext_mm) printf(" EXT_blend_minmax");
      if (!glext_tec) printf(" SGIS_texture_edge_clamp");
      if (!glext_tfa) printf(" EXT_texture_filter_anisotropic");
      if (!glext_t3D) printf(" EXT_texture3D");
      if (!glext_tc) printf(" ARB_texture_compression");
      if (!glext_ts3) printf(" EXT_texture_compression_s3tc");
      if (!glext_tgm) printf(" SGIS_generate_mipmap");

      printf("\n");
      }
   }

#ifdef _WIN32

static void initwglprocs()
   {
   static BOOLINT done=FALSE;

   if (!done)
      {
#ifdef GL_EXT_blend_minmax
      if (glext_mm)
         if ((glBlendEquation=(PFNGLBLENDEQUATIONEXTPROC)wglGetProcAddress("glBlendEquationEXT"))==NULL) ERRORMSG();
#endif

#ifdef GL_EXT_texture3D
      if (glext_t3D)
         if ((glTexImage3D=(PFNGLTEXIMAGE3DEXTPROC)wglGetProcAddress("glTexImage3DEXT"))==NULL) ERRORMSG();
#endif

#ifdef GL_ARB_texture_compression
      if (glext_tc)
         {
         if ((glCompressedTexImage2DARB=(PFNGLCOMPRESSEDTEXIMAGE2DARBPROC)wglGetProcAddress("glCompressedTexImage2DARB"))==NULL) ERRORMSG();
         if ((glGetCompressedTexImageARB=(PFNGLGETCOMPRESSEDTEXIMAGEARBPROC)wglGetProcAddress("glGetCompressedTexImageARB"))==NULL) ERRORMSG();
         }
#endif

      done=TRUE;
      }
   }

#endif

#endif

void initstate()
   {
#ifndef NOOGL
   initglexts();

#ifdef _WIN32
   initwglprocs();
#endif

   glGetIntegerv(GL_DEPTH_FUNC,&depth_func);
   glGetBooleanv(GL_DEPTH_TEST,&depth_test);

   glDepthFunc(GL_LEQUAL);
   glEnable(GL_DEPTH_TEST);

   glGetIntegerv(GL_FRONT_FACE,&front_face);
   glGetIntegerv(GL_CULL_FACE_MODE,&cull_face_mode);
   glGetBooleanv(GL_CULL_FACE,&cull_face);

   glFrontFace(GL_CCW);
   glCullFace(GL_BACK);
   glEnable(GL_CULL_FACE);

   glGetBooleanv(GL_COLOR_WRITEMASK,color_writemask);
   glGetBooleanv(GL_DEPTH_WRITEMASK,&depth_writemask);

   glGetBooleanv(GL_BLEND,&blending);
   glGetIntegerv(GL_BLEND_SRC,&blend_src);
   glGetIntegerv(GL_BLEND_DST,&blend_dst);
   glDisable(GL_BLEND);

   glColor3f(1.0f,1.0f,1.0f);

   maxblendeq=FALSE;

   fancnt=vtxcnt=0;
#endif
   }

void exitstate()
   {
#ifndef NOOGL
   if (depth_func!=GL_LEQUAL) glDepthFunc(depth_func);
   if (!depth_test) glDisable(GL_DEPTH_TEST);

   if (front_face!=GL_CCW) glFrontFace(front_face);
   if (cull_face_mode!=GL_BACK) glCullFace(cull_face_mode);
   if (!cull_face) glDisable(GL_CULL_FACE);

   glColorMask(color_writemask[0],color_writemask[1],color_writemask[2],color_writemask[3]);
   glDepthMask(depth_writemask);

   if (blending)
      {
      glBlendFunc(blend_src,blend_dst);
      glEnable(GL_BLEND);
      }

   if (glGetError()!=0) WARNMSG();
#endif
   }

void disableculling()
   {
#ifndef NOOGL
   glDisable(GL_CULL_FACE);
#endif
   }

void enableBFculling()
   {
#ifndef NOOGL
   glCullFace(GL_BACK);
   glEnable(GL_CULL_FACE);
#endif
   }

void enableFFculling()
   {
#ifndef NOOGL
   glCullFace(GL_FRONT);
   glEnable(GL_CULL_FACE);
#endif
   }

void disableAtest()
   {
#ifndef NOOGL
   glDisable(GL_ALPHA_TEST);
#endif
   }

void enableAtest(float alpha)
   {
#ifndef NOOGL
   glAlphaFunc(GL_GREATER,alpha);
   glEnable(GL_ALPHA_TEST);
#endif
   }

void disableZtest()
   {
#ifndef NOOGL
   glDisable(GL_DEPTH_TEST);
#endif
   }

void enableZtest()
   {
#ifndef NOOGL
   glDepthFunc(GL_LEQUAL);
   glEnable(GL_DEPTH_TEST);
#endif
   }

void disableZwriting()
   {
#ifndef NOOGL
   glDepthMask(GL_FALSE);
#endif
   }

void enableZwriting()
   {
#ifndef NOOGL
   glDepthMask(GL_TRUE);
#endif
   }

void enableAwriting()
   {
#ifndef NOOGL
   glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_TRUE);
#endif
   }

void enableRGBwriting()
   {
#ifndef NOOGL
   glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_FALSE);
#endif
   }

void enableRGBAwriting()
   {
#ifndef NOOGL
   glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
#endif
   }

void disableRGBAwriting()
   {
#ifndef NOOGL
   glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
#endif
   }

void enableblending()
   {
#ifndef NOOGL
   glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
   glEnable(GL_BLEND);
#endif
   }

void enableADDblending()
   {
#ifndef NOOGL
   glBlendFunc(GL_ONE,GL_ONE);
   glEnable(GL_BLEND);
#endif
   }

void enableATTblending()
   {
#ifndef NOOGL
   glBlendFunc(GL_ZERO,GL_ONE_MINUS_SRC_ALPHA);
   glEnable(GL_BLEND);
#endif
   }

void enableEMIblending()
   {
#ifndef NOOGL
   glBlendFunc(GL_SRC_ALPHA,GL_ONE);
   glEnable(GL_BLEND);
#endif
   }

void enablePRJblending()
   {
#ifndef NOOGL
   glBlendFunc(GL_ONE_MINUS_DST_ALPHA,GL_ONE);
   glEnable(GL_BLEND);
#endif
   }

void enableMIPblending()
   {
#ifndef NOOGL
#ifdef GL_EXT_blend_minmax
   glBlendFunc(GL_ONE,GL_ONE);
   if (glext_mm) glBlendEquation(GL_MAX_EXT);
   glEnable(GL_BLEND);
   maxblendeq=TRUE;
#endif
#endif
   }

void disableblending()
   {
#ifndef NOOGL
#ifdef GL_EXT_blend_minmax
   if (maxblendeq)
      {
      if (glext_mm) glBlendEquation(GL_FUNC_ADD_EXT);
      maxblendeq=FALSE;
      }
#endif
   glDisable(GL_BLEND);
#endif
   }

int buildLtexmap(unsigned char *image,int *width,int *height,int mipmaps,int mipmapped)
   {return(buildtexmap(image,width,height,1,0,mipmaps,mipmapped));}

int buildLAtexmap(unsigned char *image,int *width,int *height,int mipmaps,int mipmapped)
   {return(buildtexmap(image,width,height,2,(CONFIGURE_DEPTH==24)?32:CONFIGURE_DEPTH,mipmaps,mipmapped));}

int buildRGBtexmap(unsigned char *image,int *width,int *height,int mipmaps,int s3tc,int bytes,int mipmapped)
   {return(buildtexmap(image,width,height,3,CONFIGURE_DEPTH,mipmaps,s3tc,bytes,mipmapped));}

int buildRGBAtexmap(unsigned char *image,int *width,int *height,int mipmaps,int s3tc,int bytes,int mipmapped)
   {return(buildtexmap(image,width,height,4,(CONFIGURE_DEPTH==24)?32:CONFIGURE_DEPTH,mipmaps,s3tc,bytes,mipmapped));}

int buildRGBAtexmap(unsigned char *imageRGB,unsigned char *imageA,int *width,int *height)
   {
   int i,c;

   int texid;

   unsigned char *image;

   if (width==NULL || height==NULL) ERRORMSG();
   if (*width<2 || *height<2) ERRORMSG();

   c=(*width)*(*height);

   if ((image=(unsigned char *)malloc(4*c))==NULL) ERRORMSG();

   for (i=0; i<c; i++)
      {
      image[4*i]=imageRGB[3*i];
      image[4*i+1]=imageRGB[3*i+1];
      image[4*i+2]=imageRGB[3*i+2];
      image[4*i+3]=255-imageA[i];
      }

   texid=buildRGBAtexmap(image,width,height);
   free(image);

   return(texid);
   }

int buildtexmap(unsigned char *image,int *width,int *height,int components,int depth,int mipmaps,int s3tc,int bytes,int mipmapped)
   {
#ifndef NOOGL
   int i,c;

   GLuint texid;
   GLint texsource,texformat;

   unsigned char *image2,*image3;
   unsigned char table[256];

   float intensity;
   int lumi,red,green,blue,alpha;

   GLint width2,height2;

   unsigned char *mipmap;
   int bytes2;
   int level;

   initglexts();

#ifdef _WIN32
   initwglprocs();
#endif

#ifdef GL_ARB_texture_compression
   if (glext_tc && CONFIGURE_COMPRESSION!=0)
      if (CONFIGURE_COMPRESSION==1) glHint(GL_TEXTURE_COMPRESSION_HINT_ARB,GL_FASTEST);
      else glHint(GL_TEXTURE_COMPRESSION_HINT_ARB,GL_NICEST);
#endif

   if (width==NULL || height==NULL) ERRORMSG();
   if (*width<2 || *height<2) ERRORMSG();

   c=(*width)*(*height);

   if (mipmapped!=0)
      {
      width2=(*width)/2;
      height2=(*height)/2;

      while (width2>0 && height2>0)
         {
         c+=width2*height2;
         width2/=2;
         height2/=2;
         }
      }

   switch (components)
      {
      case 1:
         texsource=GL_LUMINANCE;
         break;
      case 2:
         texsource=GL_LUMINANCE_ALPHA;
         break;
      case 3:
         if (s3tc==0) texsource=GL_RGB;
#ifdef GL_COMPRESSED_RGB_S3TC_DXT1_EXT
         else if (glext_ts3) texsource=GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
#endif
         else ERRORMSG();
         break;
      case 4:
         if (s3tc==0) texsource=GL_RGBA;
#ifdef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
         else if (glext_ts3) texsource=GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
#endif
         else ERRORMSG();
         break;
      default: ERRORMSG();
      }

   image2=image;

   if (s3tc==0)
      if (CONFIGURE_GAMMA!=1.0f ||
          CONFIGURE_LEVEL!=0.0f)
         {
         for (i=0; i<256; i++)
            {
            intensity=i/255.0f;

            if (CONFIGURE_GAMMA>0.0f) intensity=fpow(intensity,1.0f/CONFIGURE_GAMMA);

            if (CONFIGURE_LEVEL<0.0f && CONFIGURE_LEVEL>-1.0f) intensity=(intensity+CONFIGURE_LEVEL)/(1.0f+CONFIGURE_LEVEL);
            if (CONFIGURE_LEVEL>0.0f && CONFIGURE_LEVEL<1.0f) intensity/=(1.0f-CONFIGURE_LEVEL);

            intensity=fmin(fmax(intensity,0.0f),1.0f);

            table[i]=ftrc(255.0f*intensity+0.5f);
            }

         if ((image2=(unsigned char *)malloc(components*c))==NULL) ERRORMSG();

         if (components==1)
            for (i=0; i<c; i++) image2[i]=table[image[i]];
         else if (components==2)
            for (i=0; i<c; i++)
               {
               image2[2*i]=table[image[2*i]];
               image2[2*i+1]=image[2*i+1];
               }
         else if (components==3)
            for (i=0; i<3*c; i++) image2[i]=table[image[i]];
         else if (components==4)
            for (i=0; i<c; i++)
               {
               image2[4*i]=table[image[4*i]];
               image2[4*i+1]=table[image[4*i+1]];
               image2[4*i+2]=table[image[4*i+2]];
               image2[4*i+3]=image[4*i+3];
               }
         }

   image3=image2;

   switch (depth)
      {
      case 0:
         texformat=GL_LUMINANCE;
#ifdef GL_ARB_texture_compression
         if (glext_tc && CONFIGURE_COMPRESSION!=0) texformat=GL_COMPRESSED_LUMINANCE_ARB;
#endif
         break;
      case 8:
         texformat=GL_R3_G3_B2;
         break;
      case 16:
         texformat=GL_RGB5_A1;
         break;
      case 24:
      default:
         texformat=GL_RGB;
#ifdef GL_ARB_texture_compression
         if (glext_tc && (s3tc!=0 || CONFIGURE_COMPRESSION!=0)) texformat=GL_COMPRESSED_RGB_ARB;
#endif
         break;
      case 32:
         texformat=GL_RGBA;
#ifdef GL_ARB_texture_compression
         if (glext_tc && (s3tc!=0 || CONFIGURE_COMPRESSION!=0)) texformat=GL_COMPRESSED_RGBA_ARB;
#endif

         if (s3tc==0)
            if (components==1)
               {
               texsource=GL_LUMINANCE_ALPHA;

               if ((image3=(unsigned char *)malloc(2*c))==NULL) ERRORMSG();

               for (i=0; i<c; i++)
                  {
                  lumi=image2[i];

                  if (lumi==0) alpha=0;
                  else alpha=255;

                  image3[2*i]=lumi;
                  image3[2*i+1]=alpha;
                  }
               }
            else if (components==3)
               {
               texsource=GL_RGBA;

               if ((image3=(unsigned char *)malloc(4*c))==NULL) ERRORMSG();

               for (i=0; i<c; i++)
                  {
                  red=image2[3*i];
                  green=image2[3*i+1];
                  blue=image2[3*i+2];

                  if (red==0 && green==0 && blue==0) alpha=0;
                  else alpha=255;

                  image3[4*i]=red;
                  image3[4*i+1]=green;
                  image3[4*i+2]=blue;
                  image3[4*i+3]=alpha;
                  }
               }
         break;
      }

   glGenTextures(1,&texid);
   glBindTexture(GL_TEXTURE_2D,texid);

   glPixelStorei(GL_UNPACK_ALIGNMENT,1);

   if (s3tc!=0 && !glext_ts3) ERRORMSG();
#ifdef GL_ARB_texture_compression
   else if (s3tc!=0)
      if (mipmapped==0)
         {
         if ((((*width)-1)&(*width))!=0) WARNMSG();
         if ((((*height)-1)&(*height))!=0) WARNMSG();

         glCompressedTexImage2DARB(GL_TEXTURE_2D,0,texsource,*width,*height,0,bytes,image3);
         }
      else
         {
         width2=*width;
         height2=*height;

         mipmap=image3;
         bytes2=8*((width2+3)/4)*((height2+3)/4); // 8 bytes per 4x4 s3tc/dxt1 block
         level=0;

         while (width2>0 && height2>0)
            {
            if (mipmap>=image3+bytes) ERRORMSG();

            if (((width2-1)&width2)!=0) WARNMSG();
            if (((height2-1)&height2)!=0) WARNMSG();

            glCompressedTexImage2DARB(GL_TEXTURE_2D,level,texsource,width2,height2,0,bytes2,mipmap);

            width2/=2;
            height2/=2;

            if (width2>0 || height2>0)
               {
               if (width2==0) width2=1;
               if (height2==0) height2=1;

               mipmap+=bytes2;
               bytes2=8*((width2+3)/4)*((height2+3)/4); // 8 bytes per 4x4 s3tc/dxt1 block
               level++;
               }
            }
         }
#endif
   else if (mipmapped==0)
      if (mipmaps!=0)
         {
#ifndef GL_SGIS_generate_mipmap
         gluBuild2DMipmaps(GL_TEXTURE_2D,texformat,*width,*height,texsource,GL_UNSIGNED_BYTE,image3);
#else
         if (!glext_tgm || !CONFIGURE_GENERATEMM) gluBuild2DMipmaps(GL_TEXTURE_2D,texformat,*width,*height,texsource,GL_UNSIGNED_BYTE,image3);
         else
            {
            glTexParameteri(GL_TEXTURE_2D,GL_GENERATE_MIPMAP_SGIS,GL_TRUE);
            glTexImage2D(GL_TEXTURE_2D,0,texformat,*width,*height,0,texsource,GL_UNSIGNED_BYTE,image3);
            }
#endif
         }
      else
         {
         if ((((*width)-1)&(*width))!=0) WARNMSG();
         if ((((*height)-1)&(*height))!=0) WARNMSG();

         glTexImage2D(GL_TEXTURE_2D,0,texformat,*width,*height,0,texsource,GL_UNSIGNED_BYTE,image3);
         }
   else
      {
      width2=*width;
      height2=*height;

      mipmap=image3;
      bytes=width2*height2*components;
      level=0;

      while (width2>0 && height2>0)
         {
         if (((width2-1)&width2)!=0) WARNMSG();
         if (((height2-1)&height2)!=0) WARNMSG();

         glTexImage2D(GL_TEXTURE_2D,level,texformat,width2,height2,0,texsource,GL_UNSIGNED_BYTE,mipmap);

         width2/=2;
         height2/=2;

         if (width2>0 || height2>0)
            {
            if (width2==0) width2=1;
            if (height2==0) height2=1;

            mipmap+=bytes;
            bytes=width2*height2*components;
            level++;
            }
         }
      }

   glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_WIDTH,&width2);
   glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_HEIGHT,&height2);
   glBindTexture(GL_TEXTURE_2D,0);

   if (image3!=image2) free(image3);
   if (image2!=image) free(image2);

   *width=width2;
   *height=height2;

   return(texid);
#else
   return(0);
#endif
   }

void bindtexmap(int texid,int width,int height,int size,int mipmaps)
   {
#ifndef NOOGL
   GLfloat v1[]={1.0f,0.0f,0.0f,0.0f},
           v2[]={0.0f,0.0f,1.0f,1.0f};

   initglexts();

   if (texid>0)
      {
      glBindTexture(GL_TEXTURE_2D,texid);
      glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

      if (mipmaps!=0) glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
      else glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

      if (size>0 || width>0 || height>0)
         {
#if !defined(GL_CLAMP_TO_EDGE_SGIS) && !defined(GL_CLAMP_TO_EDGE_EXT)
         glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
         glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
#else
         if (glext_tec)
            {
#ifdef GL_CLAMP_TO_EDGE_SGIS
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE_SGIS);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE_SGIS);
#endif
#ifdef GL_CLAMP_TO_EDGE_EXT
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE_EXT);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE_EXT);
#endif
            }
         else
            {
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
            }
#endif
         }
      else
         {
         glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
         glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
         }

#ifdef GL_EXT_texture_filter_anisotropic
      if (glext_tfa)
         {
         GLfloat maxaniso=1.0f;
         glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT,&maxaniso);
         glTexParameterfv(GL_TEXTURE_2D,GL_TEXTURE_MAX_ANISOTROPY_EXT,&maxaniso);
         }
#endif

      if (size>1 && width>1 && height>1)
         {
         v1[0]=1.0f/(size-1)*(width-1)/width;
         v2[2]=-1.0f/(size-1)*(height-1)/height;

         v1[3]=0.5f/width;
         v2[3]=1.0f-0.5f/height;

         glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);
         glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);

         glTexGenfv(GL_S,GL_OBJECT_PLANE,v1);
         glTexGenfv(GL_T,GL_OBJECT_PLANE,v2);

         glEnable(GL_TEXTURE_GEN_S);
         glEnable(GL_TEXTURE_GEN_T);
         }

      glEnable(GL_TEXTURE_2D);
      }
   else
      {
      glBindTexture(GL_TEXTURE_2D,0);
      glDisable(GL_TEXTURE_2D);

      glDisable(GL_TEXTURE_GEN_S);
      glDisable(GL_TEXTURE_GEN_T);
      }
#endif
   }

int build3Dtexmap(unsigned char *volume,
                  int *width,int *height,int *depth,
                  int components)
   {
   if (volume==NULL) ERRORMSG();
   if (width==NULL || height==NULL || depth==NULL) ERRORMSG();
   if (components<1) ERRORMSG();

#ifndef NOOGL
#ifdef GL_EXT_texture3D
   GLuint texid;

   GLint width2,height2,depth2;

   initglexts();

   if (!glext_t3D) return(0);

   if (*width<2 || *height<2 || *depth<2) ERRORMSG();

   glGenTextures(1,&texid);
   glBindTexture(GL_TEXTURE_3D,texid);

   glPixelStorei(GL_UNPACK_ALIGNMENT,1);
   if (components==1)
      glTexImage3D(GL_TEXTURE_3D,0,GL_LUMINANCE,*width,*height,*depth,0,
                   GL_LUMINANCE,GL_UNSIGNED_BYTE,volume);
   else if (components==3)
      glTexImage3D(GL_TEXTURE_3D,0,GL_RGB,*width,*height,*depth,0,
                   GL_RGB,GL_UNSIGNED_BYTE,volume);
   else if (components==4)
      glTexImage3D(GL_TEXTURE_3D,0,GL_RGBA,*width,*height,*depth,0,
                   GL_RGBA,GL_UNSIGNED_BYTE,volume);
   else ERRORMSG();
   glGetTexLevelParameteriv(GL_TEXTURE_3D,0,GL_TEXTURE_WIDTH,&width2);
   glGetTexLevelParameteriv(GL_TEXTURE_3D,0,GL_TEXTURE_HEIGHT,&height2);
   glGetTexLevelParameteriv(GL_TEXTURE_3D,0,GL_TEXTURE_DEPTH,&depth2);
   glBindTexture(GL_TEXTURE_3D,0);

   *width=width2;
   *height=height2;
   *depth=depth2;

   return(texid);
#else
   return(0);
#endif
#else
   return(0);
#endif
   }

void bind3Dtexmap(int texid)
   {
   if (texid<0) ERRORMSG();

#ifndef NOOGL
#ifdef GL_EXT_texture3D
   if (texid>0)
      {
      glBindTexture(GL_TEXTURE_3D,texid);
      glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_S,GL_REPEAT);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_T,GL_REPEAT);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_R,GL_REPEAT);

      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
      glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

      glEnable(GL_TEXTURE_3D);
      }
   else glDisable(GL_TEXTURE_3D);
#endif
#endif
   }

void deletetexmap(int texid)
   {
#ifndef NOOGL
   GLuint GLtexid=texid;
   if (texid>0) glDeleteTextures(1,&GLtexid);
#endif
   }

int compressRGBtexmap(unsigned char *image,int width,int height,
                      unsigned char **data,int *bytes)
   {
#ifndef NOOGL
#if defined(GL_ARB_texture_compression) && defined(GL_COMPRESSED_RGB_S3TC_DXT1_EXT)
   GLint success,format,size;
   unsigned char *texture;

#ifdef _WIN32
   initwglprocs();
#endif

   // upload texture
   glTexImage2D(GL_TEXTURE_2D,0,GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
                width,height,0,GL_RGB,GL_UNSIGNED_BYTE,image);

   // check whether or not compression succeeded
   glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_COMPRESSED_ARB,&success);
   if (success==0) return(0);

   // check whether or not the internal format is ok
   glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_INTERNAL_FORMAT,&format);
   if (format!=GL_COMPRESSED_RGB_S3TC_DXT1_EXT) return(0);

   // get size of compressed data chunk
   glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB,&size);

   // allocate new texture buffer
   if ((texture=(unsigned char *)malloc(size))==NULL) ERRORMSG();

   // get compressed image into buffer
   glGetCompressedTexImageARB(GL_TEXTURE_2D,0,texture);

   *data=texture;
   *bytes=size;

   return(1);
#endif
#endif

   return(0);
   }

void mtxmodel()
   {
#ifndef NOOGL
   glMatrixMode(GL_MODELVIEW);
#endif
   }

void mtxproj()
   {
#ifndef NOOGL
   glMatrixMode(GL_PROJECTION);
#endif
   }

void mtxtex()
   {
#ifndef NOOGL
   glMatrixMode(GL_TEXTURE);
#endif
   }

void mtxpush()
   {
#ifndef NOOGL
   glPushMatrix();
#endif
   }

void mtxpop()
   {
#ifndef NOOGL
   glPopMatrix();
#endif
   }

void mtxid()
   {
#ifndef NOOGL
   glLoadIdentity();
#endif
   }

void mtxscale(const float sx,const float sy,const float sz)
   {
#ifndef NOOGL
   glScalef(sx,sy,sz);
#endif
   }

void mtxtranslate(const float tx,const float ty,const float tz)
   {
#ifndef NOOGL
   glTranslatef(tx,ty,tz);
#endif
   }

void mtxrotate(const float angle,const float ax,const float ay,const float az)
   {
#ifndef NOOGL
   glRotatef(angle,ax,ay,az);
#endif
   }

void mtxmult(const float mtx[16])
   {
#ifndef NOOGL
   glMultMatrixf(mtx);
#endif
   }

void mtxmult(const double mtx[16])
   {
#ifndef NOOGL
   glMultMatrixd(mtx);
#endif
   }

void beginfans()
   {
#ifndef NOOGL
   fancnt=vtxcnt=0;
#endif
   }

void endfans()
   {
#ifndef NOOGL
   if (fancnt>0) glEnd();
#endif
   }

int getfancnt()
   {
#ifndef NOOGL
   return(fancnt);
#else
   return(0);
#endif
   }

int getvtxcnt()
   {
#ifndef NOOGL
   return(vtxcnt);
#else
   return(0);
#endif
   }

void linewidth(const int w)
   {
#ifndef NOOGL
   glLineWidth(w);
#endif
   }

void enablelinesmooth()
   {
#ifndef NOOGL
   glEnable(GL_LINE_SMOOTH);
#endif
   }

void disablelinesmooth()
   {
#ifndef NOOGL
   glDisable(GL_LINE_SMOOTH);
#endif
   }

unsigned char *readrgbpixels(int x,int y,int width,int height)
   {
   unsigned char *pixels=NULL;

#ifndef NOOGL
   glFinish();

   if ((pixels=(unsigned char *)malloc(3*width*height))==NULL) ERRORMSG();

   glReadBuffer(GL_BACK);
   glReadPixels(x,y,width,height,GL_RGB,GL_UNSIGNED_BYTE,pixels);
#endif

   return(pixels);
   }

void writergbpixels(unsigned char *pixels,int width,int height,int winwidth,int winheight,int x,int y)
   {
#ifndef NOOGL
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluOrtho2D(0.0f,winwidth-1,0.0f,winheight-1);
   glMatrixMode(GL_MODELVIEW);

   glRasterPos2i(x,y);
   glDrawBuffer(GL_BACK);

   glPixelStorei(GL_UNPACK_ALIGNMENT,1);
   glDrawPixels(width,height,GL_RGB,GL_UNSIGNED_BYTE,pixels);
#endif
   }

}
