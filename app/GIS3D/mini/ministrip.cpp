// (c) by Stefan Roettger

#include "minibase.h"

#include "minibrick.h"

#include "ministrip.h"

int ministrip::INSTANCES=0;

ministrip::SHADER_TYPE ministrip::SHADER[SHADERMAX];

ministrip::SNIPPET_TYPE ministrip::SNIPPET[SNIPPETMAX];
int ministrip::SNIPPETS=0;

// initialize shader snippets
void ministrip::initsnippets()
   {
   addsnippet(MINI_SNIPPET_VTX_BEGIN,"!!ARBvp1.0\n");

   addsnippet(MINI_SNIPPET_VTX_HEADER,"\
      PARAM c0=program.env[0]; \n\
      PARAM c1=program.env[1]; \n\
      PARAM c2=program.env[2]; \n\
      PARAM c3=program.env[3]; \n\
      PARAM c4=program.env[4]; \n\
      PARAM c5=program.env[5]; \n\
      PARAM c6=program.env[6]; \n\
      PARAM c7=program.env[7]; \n\
      PARAM mat[4]={state.matrix.mvp}; \n\
      PARAM invtra[4]={state.matrix.modelview.invtrans}; \n\
      PARAM texmat[4]={state.matrix.texture[0]}; \n\
      TEMP vtx,col,pos; \n");

   addsnippet(MINI_SNIPPET_VTX_BASIC,"\
      ### fetch actual vertex \n\
      MOV vtx,vertex.position; \n\
      MOV col,vertex.color; \n\
      ### transform vertex with modelview \n\
      DP4 pos.x,mat[0],vtx; \n\
      DP4 pos.y,mat[1],vtx; \n\
      DP4 pos.z,mat[2],vtx; \n\
      DP4 pos.w,mat[3],vtx; \n");

   addsnippet(MINI_SNIPPET_VTX_NORMAL,"\
      ### fetch actual normal \n\
      TEMP nrm; \n\
      MOV nrm,vertex.normal; \n\
      ### transform normal with inverse transpose \n\
      TEMP vec; \n\
      DP4 vec.x,invtra[0],nrm; \n\
      DP4 vec.y,invtra[1],nrm; \n\
      DP4 vec.z,invtra[2],nrm; \n\
      DP4 vec.w,invtra[3],nrm; \n\
      ### write normal to tex coords \n\
      MOV result.texcoord[2],vec; \n");

   addsnippet(MINI_SNIPPET_VTX_NORMAL_DIRECT,"\
      ### directly write normal to tex coords\n\
      MOV result.texcoord[2],vertex.normal; \n");

   addsnippet(MINI_SNIPPET_VTX_TEX,"\
      ### fetch actual tex coords \n\
      TEMP texcrd; \n\
      MOV texcrd,vertex.texcoord[0]; \n\
      ### transform tex coords with texture matrix \n\
      TEMP crd; \n\
      DP4 crd.x,texmat[0],texcrd; \n\
      DP4 crd.y,texmat[1],texcrd; \n\
      DP4 crd.z,texmat[2],texcrd; \n\
      DP4 crd.w,texmat[3],texcrd; \n\
      ### write resulting tex coords \n\
      MOV result.texcoord[0],crd; \n");

   addsnippet(MINI_SNIPPET_VTX_FOG,"\
      ### calculate spherical fog coord \n\
      DP3 result.fogcoord.x,pos,pos; \n");

   addsnippet(MINI_SNIPPET_VTX_FOOTER,"\
      ### write resulting vertex \n\
      MOV result.position,pos; \n\
      MOV result.color,col; \n\
      ### write view position to tex coords \n\
      MOV result.texcoord[1],pos; \n");

   addsnippet(MINI_SNIPPET_VTX_END,"END\n");

   addsnippet(MINI_SNIPPET_FRG_BEGIN,"!!ARBfp1.0\n");

   addsnippet(MINI_SNIPPET_FRG_HEADER,"\
      PARAM c0=program.env[0]; \n\
      PARAM c1=program.env[1]; \n\
      PARAM c2=program.env[2]; \n\
      PARAM c3=program.env[3]; \n\
      PARAM c4=program.env[4]; \n\
      PARAM c5=program.env[5]; \n\
      PARAM c6=program.env[6]; \n\
      PARAM c7=program.env[7]; \n\
      TEMP col; \n");

   addsnippet(MINI_SNIPPET_FRG_BASIC,"\
      ### fetch fragment color \n\
      MOV col,fragment.color; \n");

   addsnippet(MINI_SNIPPET_FRG_TEX,"\
      ### fetch actual texel \n\
      TEMP tex; \n\
      TEX tex,fragment.texcoord[0],texture[0],2D; \n\
      ### modulate with texture color \n\
      MUL col,col,tex; \n");

   addsnippet(MINI_SNIPPET_FRG_TEX2_DIRECT,"\
      ### fetch two texels \n\
      TEMP tex1,tex2; \n\
      TEX tex1,fragment.texcoord[0],texture[0],2D; \n\
      TEX tex2,fragment.texcoord[0],texture[1],2D; \n\
      ### fetch fragment normal \n\
      TEMP nrm; \n\
      MOV nrm,fragment.texcoord[2]; \n\
      ### calculate directional light \n\
      TEMP len; \n\
      DP3 len.x,nrm,nrm; \n\
      RSQ len.x,len.x; \n\
      MUL nrm,nrm,len.x; \n\
      DP3 nrm.z,nrm,c5; \n\
      ### blend between texels \n\
      MAD nrm.z,nrm.z,c4.x,c4.y; \n\
      MAD_SAT nrm.z,nrm.z,c4.w,c4.w; \n\
      MUL nrm.z,nrm.z,c5.w; \n\
      SIN nrm.z,nrm.z; \n\
      LRP tex1,nrm.z,tex1,tex2; \n\
      ### modulate with texture color \n\
      MUL col,col,tex1; \n");

   addsnippet(MINI_SNIPPET_FRG_SHADE,"\
      ### fetch view position \n\
      TEMP pos; \n\
      MOV pos,fragment.texcoord[1]; \n\
      ### fetch fragment normal \n\
      TEMP nrm; \n\
      MOV nrm,fragment.texcoord[2]; \n\
      ### calculate head light \n\
      TEMP len; \n\
      DP3 len.x,nrm,nrm; \n\
      RSQ len.x,len.x; \n\
      MUL nrm,nrm,len.x; \n\
      DP3 len.x,pos,pos; \n\
      RSQ len.x,len.x; \n\
      MUL pos,pos,len.x; \n\
      DP3 nrm.z,nrm,pos; \n\
      ### modulate fragment color \n\
      ABS nrm.z,nrm.z; \n\
      MUL col.xyz,col,nrm.z; \n");

   addsnippet(MINI_SNIPPET_FRG_SHADE_DIRECT,"\
      ### fetch fragment normal \n\
      TEMP nrm; \n\
      MOV nrm,fragment.texcoord[2]; \n\
      ### calculate directional light \n\
      TEMP len; \n\
      DP3 len.x,nrm,nrm; \n\
      RSQ len.x,len.x; \n\
      MUL nrm,nrm,len.x; \n\
      DP3_SAT nrm.z,nrm,c5; \n\
      ### modulate fragment color \n\
      MAD nrm.z,nrm.z,c4.x,c4.y; \n\
      MUL col.xyz,col,nrm.z; \n");

   addsnippet(MINI_SNIPPET_FRG_FOG,"\
      ### fetch fog coord \n\
      TEMP fog; \n\
      MOV fog.x,fragment.fogcoord.x; \n\
      ### modulate with spherical fog \n\
      MAD_SAT fog.x,fog.x,c6.x,c6.y; \n\
      POW fog.x,fog.x,c6.z; \n\
      LRP col.xyz,fog.x,c7,col; \n");

   addsnippet(MINI_SNIPPET_FRG_FOOTER,"\
      ### write resulting pixel \n\
      MOV result.color,col; \n");

   addsnippet(MINI_SNIPPET_FRG_END,"END\n");
   }

// initialize default shader
void ministrip::initshader()
   {
   concatvtxshader(0,MINI_SNIPPET_VTX_BEGIN);
   concatvtxshader(0,MINI_SNIPPET_VTX_HEADER);
   concatvtxshader(0,MINI_SNIPPET_VTX_BASIC);
   concatvtxshader(0,MINI_SNIPPET_VTX_NORMAL);
   concatvtxshader(0,MINI_SNIPPET_VTX_FOOTER);
   concatvtxshader(0,MINI_SNIPPET_VTX_END);

   concatpixshader(0,MINI_SNIPPET_FRG_BEGIN);
   concatpixshader(0,MINI_SNIPPET_FRG_HEADER);
   concatpixshader(0,MINI_SNIPPET_FRG_BASIC);
   concatpixshader(0,MINI_SNIPPET_FRG_SHADE);
   concatpixshader(0,MINI_SNIPPET_FRG_FOOTER);
   concatpixshader(0,MINI_SNIPPET_FRG_END);
   }

// free shader snippets
void ministrip::freesnippets()
   {
   int n;

   for (n=0; n<SNIPPETS; n++)
      {
      if (SNIPPET[n].snippetname!=NULL) free(SNIPPET[n].snippetname);
      if (SNIPPET[n].snippet!=NULL) free(SNIPPET[n].snippet);
      }

   SNIPPETS=0;
   }

// add shader snippet
void ministrip::addsnippet(const char *snippetname,const char *snippet)
   {
   if (SNIPPETS>=SNIPPETMAX) ERRORMSG();

   SNIPPET[SNIPPETS].snippetname=strdup(snippetname);
   SNIPPET[SNIPPETS].snippet=strdup(snippet);

   SNIPPETS++;
   }

// get free shader slot
int ministrip::getfreeslot()
   {
   int n;

   for (n=0; n<SHADERMAX; n++)
      if (SHADER[n].vtxprog==NULL && SHADER[n].frgprog==NULL) return(n);

   return(-1);
   }

// default constructor
ministrip::ministrip(int colcomps,int nrmcomps,int texcomps)
   {
   int i,j;

   if (colcomps!=0 && colcomps!=3 && colcomps!=4) ERRORMSG();
   if (nrmcomps!=0 && nrmcomps!=3) ERRORMSG();
   if (texcomps<0 || texcomps>4) ERRORMSG();

   COLCOMPS=colcomps;
   NRMCOMPS=nrmcomps;
   TEXCOMPS=texcomps;

   MAXSIZE=1;

   if ((VTXARRAY=(float *)malloc(3*MAXSIZE*sizeof(float)))==NULL) ERRORMSG();

   if (COLCOMPS==0) COLARRAY=NULL;
   else
      if ((COLARRAY=(float *)malloc(COLCOMPS*MAXSIZE*sizeof(float)))==NULL) ERRORMSG();

   if (NRMCOMPS==0) NRMARRAY=NULL;
   else
      if ((NRMARRAY=(float *)malloc(NRMCOMPS*MAXSIZE*sizeof(float)))==NULL) ERRORMSG();

   if (TEXCOMPS==0) TEXARRAY=NULL;
   else
      if ((TEXARRAY=(float *)malloc(TEXCOMPS*MAXSIZE*sizeof(float)))==NULL) ERRORMSG();

   SIZE=0;

   COLR=0.0f;
   COLG=0.0f;
   COLB=0.0f;
   COLA=1.0f;

   NRMX=0.0f;
   NRMY=1.0f;
   NRMZ=0.0f;

   TEXX=0.0f;
   TEXY=0.0f;
   TEXZ=0.0f;
   TEXW=1.0f;

   COPYVTX=0;

   SCALE=1.0f;

   for (i=0; i<16; i++) MTX[i]=0.0;
   for (i=0; i<4; i++) MTX[i+4*i]=1.0;

   for (i=0; i<16; i++) TEXMTX[i]=0.0;
   for (i=0; i<4; i++) TEXMTX[i+4*i]=1.0;

   ZSCALE=1.0f;

   USESHADER=-1;

   if (INSTANCES==0)
      {
      for (i=0; i<SHADERMAX; i++)
         {
         SHADER[i].vtxprog=NULL;
         SHADER[i].vtxprogid=0;
         SHADER[i].vtxdirty=0;

         SHADER[i].frgprog=NULL;
         SHADER[i].frgprogid=0;
         SHADER[i].frgdirty=0;

         for (j=0; j<SHADERVTXPRMMAX; j++)
            {
            SHADER[i].vtxshaderpar1[j]=0.0f;
            SHADER[i].vtxshaderpar2[j]=0.0f;
            SHADER[i].vtxshaderpar3[j]=0.0f;
            SHADER[i].vtxshaderpar4[j]=0.0f;
            }

         for (j=0; j<SHADERFRGPRMMAX; j++)
            {
            SHADER[i].pixshaderpar1[j]=0.0f;
            SHADER[i].pixshaderpar2[j]=0.0f;
            SHADER[i].pixshaderpar3[j]=0.0f;
            SHADER[i].pixshaderpar4[j]=0.0f;
            }

         for (j=0; j<SHADERFRGTEXMAX; j++)
            {
            SHADER[i].pixshadertexid[j]=0;
            SHADER[i].pixshadertexw[j]=0;
            SHADER[i].pixshadertexh[j]=0;
            SHADER[i].pixshadertexmm[j]=0;
            }
         }

      initsnippets();
      }

   INSTANCES++;

   GLSETUP=0;
   WGLSETUP=0;

   GLEXT_MT=0;
   GLEXT_VP=0;
   GLEXT_FP=0;
   }

// destructor
ministrip::~ministrip()
   {
   int i,j;

   free(VTXARRAY);

   if (COLARRAY!=NULL) free(COLARRAY);
   if (NRMARRAY!=NULL) free(NRMARRAY);
   if (TEXARRAY!=NULL) free(TEXARRAY);

   INSTANCES--;

   if (INSTANCES==0)
      {
      for (i=0; i<SHADERMAX; i++)
         {
         if (SHADER[i].vtxprog!=NULL)
            {
            free(SHADER[i].vtxprog);
            SHADER[i].vtxprog=NULL;
            SHADER[i].vtxdirty=0;
            }

         if (SHADER[i].frgprog!=NULL)
            {
            free(SHADER[i].frgprog);
            SHADER[i].frgprog=NULL;
            SHADER[i].frgdirty=0;
            }

         for (j=0; j<SHADERFRGTEXMAX; j++)
            if (SHADER[i].pixshadertexid[j]!=0)
               {
               deletetexmap(SHADER[i].pixshadertexid[j]);
               SHADER[i].pixshadertexid[j]=0;

               SHADER[i].pixshadertexw[j]=0;
               SHADER[i].pixshadertexh[j]=0;
               SHADER[i].pixshadertexmm[j]=0;
               }
         }

      freesnippets();
      }

#if defined(GL_ARB_vertex_program) && defined(GL_ARB_fragment_program)

   GLuint progid;

   if (INSTANCES==0)
      if (GLEXT_VP!=0 && GLEXT_FP!=0)
         for (i=0; i<SHADERMAX; i++)
            {
            if (SHADER[i].vtxprogid!=0)
               {
               progid=SHADER[i].vtxprogid;
               glDeleteProgramsARB(1,&progid);
               SHADER[i].vtxprogid=0;
               }

            if (SHADER[i].frgprogid!=0)
               {
               progid=SHADER[i].frgprogid;
               glDeleteProgramsARB(1,&progid);
               SHADER[i].frgprogid=0;
               }
            }

#endif
   }

// clear strip
void ministrip::clear()
   {SIZE=0;}

// begin next triangle strip
void ministrip::beginstrip()
   {
   if (SIZE>0)
      {
      addvtx();
      COPYVTX=1;
      }
   }

// set color of next vertex
void ministrip::setcol(const float r,const float g,const float b,float a)
   {
   COLR=r;
   COLG=g;
   COLB=b;
   COLA=a;
   }

// set normal of next vertex
void ministrip::setnrm(const float nx,const float ny,const float nz)
   {
   NRMX=nx;
   NRMY=ny;
   NRMZ=nz;
   }

// set tex coords of next vertex
void ministrip::settex(const float tx,const float ty,const float tz,float tw)
   {
   TEXX=tx;
   TEXY=ty;
   TEXZ=tz;
   TEXW=tw;
   }

// add one vertex
void ministrip::addvtx(const float x,const float y,const float z)
   {
   VTXX=x;
   VTXY=y;
   VTXZ=z;

   addvtx();

   if (COPYVTX!=0)
      {
      addvtx();
      COPYVTX=0;
      }
   }

// add one vertex
void ministrip::addvtx()
   {
   float *ptr;

   if (SIZE>=MAXSIZE)
      {
      MAXSIZE*=2;

      if ((VTXARRAY=(float *)realloc(VTXARRAY,3*MAXSIZE*sizeof(float)))==NULL) ERRORMSG();

      if (COLARRAY!=NULL)
         if ((COLARRAY=(float *)realloc(COLARRAY,COLCOMPS*MAXSIZE*sizeof(float)))==NULL) ERRORMSG();

      if (NRMARRAY!=NULL)
         if ((NRMARRAY=(float *)realloc(NRMARRAY,NRMCOMPS*MAXSIZE*sizeof(float)))==NULL) ERRORMSG();

      if (TEXARRAY!=NULL)
         if ((TEXARRAY=(float *)realloc(TEXARRAY,TEXCOMPS*MAXSIZE*sizeof(float)))==NULL) ERRORMSG();
      }

   ptr=&VTXARRAY[3*SIZE];

   *ptr++=VTXX;
   *ptr++=VTXY;
   *ptr++=VTXZ;

   if (COLARRAY!=NULL)
      {
      ptr=&COLARRAY[COLCOMPS*SIZE];

      *ptr++=COLR;
      *ptr++=COLG;
      *ptr++=COLB;

      if (COLCOMPS==4) *ptr=COLA;
      }

   if (NRMARRAY!=NULL)
      {
      ptr=&NRMARRAY[NRMCOMPS*SIZE];

      *ptr++=NRMX;
      *ptr++=NRMY;
      *ptr++=NRMZ;
      }

   if (TEXARRAY!=NULL)
      {
      ptr=&TEXARRAY[TEXCOMPS*SIZE];

      *ptr++=TEXX;

      if (TEXCOMPS>1) *ptr++=TEXY;
      if (TEXCOMPS>2) *ptr++=TEXZ;
      if (TEXCOMPS>3) *ptr=TEXW;
      }

   SIZE++;
   }

// set scale
void ministrip::setscale(float scale)
   {SCALE=scale;}

// set matrix
void ministrip::setmatrix(double mtx[16])
   {
   int i;

   for (i=0; i<16; i++) MTX[i]=mtx[i];
   }

// set texture matrix
void ministrip::settexmatrix(double texmtx[16])
   {
   int i;

   for (i=0; i<16; i++) TEXMTX[i]=texmtx[i];
   }

// set Z-scaling
void ministrip::setZscale(float zscale)
   {ZSCALE=zscale;}

// set vertex shader
void ministrip::setvtxshader(int num,char *vtxprog)
   {
   if (num<0 || num>=SHADERMAX) ERRORMSG();

   if (SHADER[num].vtxprog!=NULL)
      {
      if (strcmp(vtxprog,SHADER[num].vtxprog)==0) return;
      free(SHADER[num].vtxprog);
      }

   SHADER[num].vtxprog=strdup(vtxprog);
   SHADER[num].vtxdirty=1;
   }

// concatenate vertex shader from snippets
void ministrip::concatvtxshader(int num,const char *snippetname)
   {
   int n;

   char *vtxprog;

   if (num<0 || num>=SHADERMAX) ERRORMSG();

   for (n=0; n<SNIPPETS; n++)
      if (strcmp(snippetname,SNIPPET[n].snippetname)==0)
         {
         vtxprog=strcct(SHADER[num].vtxprog,SNIPPET[n].snippet);
         if (SHADER[num].vtxprog!=NULL) free(SHADER[num].vtxprog);
         SHADER[num].vtxprog=vtxprog;
         SHADER[num].vtxdirty=1;

         break;
         }
   }

// set vertex shader parameter vector
void ministrip::setvtxshaderparams(int num,float p1,float p2,float p3,float p4,int n)
   {
   if (num<0 || num>=SHADERMAX) ERRORMSG();
   if (n<0 || n>=SHADERVTXPRMMAX) ERRORMSG();

   SHADER[num].vtxshaderpar1[n]=p1;
   SHADER[num].vtxshaderpar2[n]=p2;
   SHADER[num].vtxshaderpar3[n]=p3;
   SHADER[num].vtxshaderpar4[n]=p4;
   }

// enable vertex shader
void ministrip::enablevtxshader(int num)
   {
   if (num<0 || num>=SHADERMAX) ERRORMSG();

#ifndef NOOGL

#if defined(GL_ARB_vertex_program) && defined(GL_ARB_fragment_program)

   int i;

   GLuint vtxprogid;

   initglexts();
   initwglprocs();

   if (GLEXT_VP!=0 && GLEXT_FP!=0)
      if (SHADER[num].vtxprog!=NULL)
         {
         if (SHADER[num].vtxdirty!=0)
            {
            if (SHADER[num].vtxprogid!=0)
               {
               vtxprogid=SHADER[num].vtxprogid;
               glDeleteProgramsARB(1,&vtxprogid);
               }

            glGenProgramsARB(1,&vtxprogid);
            glBindProgramARB(GL_VERTEX_PROGRAM_ARB,vtxprogid);
            glProgramStringARB(GL_VERTEX_PROGRAM_ARB,GL_PROGRAM_FORMAT_ASCII_ARB,strlen(SHADER[num].vtxprog),SHADER[num].vtxprog);
            SHADER[num].vtxprogid=vtxprogid;

            SHADER[num].vtxdirty=0;
            }

         if (SHADER[num].vtxprogid!=0)
            {
            glBindProgramARB(GL_VERTEX_PROGRAM_ARB,SHADER[num].vtxprogid);
            glEnable(GL_VERTEX_PROGRAM_ARB);

            for (i=0; i<SHADERVTXPRMMAX; i++)
               glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB,i,SHADER[num].vtxshaderpar1[i],SHADER[num].vtxshaderpar2[i],SHADER[num].vtxshaderpar3[i],SHADER[num].vtxshaderpar4[i]);
            }
         }

#endif

#endif
   }

// disable vertex shader
void ministrip::disablevtxshader(int num)
   {
#ifndef NOOGL

#if defined(GL_ARB_vertex_program) && defined(GL_ARB_fragment_program)

   if (GLEXT_VP!=0 && GLEXT_FP!=0)
      if (SHADER[num].vtxprog!=NULL)
         if (SHADER[num].vtxprogid!=0)
            {
            glBindProgramARB(GL_VERTEX_PROGRAM_ARB,0);
            glDisable(GL_VERTEX_PROGRAM_ARB);
            }

#endif

#endif
   }

// set pixel shader
void ministrip::setpixshader(int num,char *frgprog)
   {
   if (num<0 || num>=SHADERMAX) ERRORMSG();

   if (SHADER[num].frgprog!=NULL)
      {
      if (strcmp(frgprog,SHADER[num].frgprog)==0) return;
      free(SHADER[num].frgprog);
      }

   SHADER[num].frgprog=strdup(frgprog);
   SHADER[num].frgdirty=1;
   }

// concatenate pixel shader from snippets
void ministrip::concatpixshader(int num,const char *snippetname)
   {
   int n;

   char *frgprog;

   if (num<0 || num>=SHADERMAX) ERRORMSG();

   for (n=0; n<SNIPPETS; n++)
      if (strcmp(snippetname,SNIPPET[n].snippetname)==0)
         {
         frgprog=strcct(SHADER[num].frgprog,SNIPPET[n].snippet);
         if (SHADER[num].frgprog!=NULL) free(SHADER[num].frgprog);
         SHADER[num].frgprog=frgprog;
         SHADER[num].frgdirty=1;

         break;
         }
   }

// set pixel shader parameter vector
void ministrip::setpixshaderparams(int num,float p1,float p2,float p3,float p4,int n)
   {
   if (num<0 || num>=SHADERMAX) ERRORMSG();
   if (n<0 || n>=SHADERFRGPRMMAX) ERRORMSG();

   SHADER[num].pixshaderpar1[n]=p1;
   SHADER[num].pixshaderpar2[n]=p2;
   SHADER[num].pixshaderpar3[n]=p3;
   SHADER[num].pixshaderpar4[n]=p4;
   }

// set pixel shader RGB texture map
void ministrip::setpixshadertexRGB(int num,unsigned char *image,int width,int height,int mipmaps,int n)
   {setpixshadertex(num,image,width,height,3,mipmaps,n);}

// set pixel shader RGBA texture map
void ministrip::setpixshadertexRGBA(int num,unsigned char *image,int width,int height,int mipmaps,int n)
   {setpixshadertex(num,image,width,height,4,mipmaps,n);}

// set pixel shader RGB[A] texture map
void ministrip::setpixshadertex(int num,unsigned char *image,int width,int height,int components,int mipmaps,int n)
   {
   if (num<0 || num>=SHADERMAX) ERRORMSG();
   if (n<0 || n>=SHADERFRGTEXMAX) ERRORMSG();

   if (width<2 || height<2) ERRORMSG();

   if (SHADER[num].pixshadertexid[n]!=0)
      {
      deletetexmap(SHADER[num].pixshadertexid[n]);
      SHADER[num].pixshadertexid[n]=0;

      SHADER[num].pixshadertexw[n]=0;
      SHADER[num].pixshadertexh[n]=0;
      SHADER[num].pixshadertexmm[n]=0;
      }

   if (image!=NULL)
      {
      if (components==1) SHADER[num].pixshadertexid[n]=buildLtexmap(image,&width,&height,mipmaps);
      else if (components==3) SHADER[num].pixshadertexid[n]=buildRGBtexmap(image,&width,&height,mipmaps);
      else if (components==4) SHADER[num].pixshadertexid[n]=buildRGBAtexmap(image,&width,&height,mipmaps);
      else ERRORMSG();

      SHADER[num].pixshadertexw[n]=width;
      SHADER[num].pixshadertexh[n]=height;
      SHADER[num].pixshadertexmm[n]=mipmaps;
      }
   }

// set pixel shader RGB[A] texture map from image buffer
void ministrip::setpixshadertexbuf(int num,databuf *buf,int mipmaps,int n)
   {
   int width,height;

   if (num<0 || num>=SHADERMAX) ERRORMSG();
   if (n<0 || n>=SHADERFRGTEXMAX) ERRORMSG();

   if (buf->missing()) ERRORMSG();

   if (buf->xsize<2 || buf->ysize<2 ||
       buf->zsize>1 || buf->tsteps>1) ERRORMSG();

   width=buf->xsize;
   height=buf->ysize;

   if (SHADER[num].pixshadertexid[n]!=0)
      {
      deletetexmap(SHADER[num].pixshadertexid[n]);
      SHADER[num].pixshadertexid[n]=0;

      SHADER[num].pixshadertexw[n]=0;
      SHADER[num].pixshadertexh[n]=0;
      SHADER[num].pixshadertexmm[n]=0;
      }

   if (buf->type==0) SHADER[num].pixshadertexid[n]=buildLtexmap((unsigned char *)buf->data,&width,&height,mipmaps);
   else if (buf->type==3) SHADER[num].pixshadertexid[n]=buildRGBtexmap((unsigned char *)buf->data,&width,&height,mipmaps);
   else if (buf->type==4) SHADER[num].pixshadertexid[n]=buildRGBAtexmap((unsigned char *)buf->data,&width,&height,mipmaps);
   else if (buf->type==5) SHADER[num].pixshadertexid[n]=buildRGBtexmap((unsigned char *)buf->data,&width,&height,mipmaps=0,1,buf->bytes);
   else if (buf->type==6) SHADER[num].pixshadertexid[n]=buildRGBAtexmap((unsigned char *)buf->data,&width,&height,mipmaps=0,1,buf->bytes);
   else if (buf->type==7) SHADER[num].pixshadertexid[n]=buildRGBtexmap((unsigned char *)buf->data,&width,&height,mipmaps=1,0,0,1);
   else if (buf->type==8) SHADER[num].pixshadertexid[n]=buildRGBAtexmap((unsigned char *)buf->data,&width,&height,mipmaps=1,0,0,1);
   else if (buf->type==9) SHADER[num].pixshadertexid[n]=buildRGBtexmap((unsigned char *)buf->data,&width,&height,mipmaps=1,1,buf->bytes,1);
   else if (buf->type==10) SHADER[num].pixshadertexid[n]=buildRGBAtexmap((unsigned char *)buf->data,&width,&height,mipmaps=1,1,buf->bytes,1);
   else ERRORMSG();

   SHADER[num].pixshadertexw[n]=width;
   SHADER[num].pixshadertexh[n]=height;
   SHADER[num].pixshadertexmm[n]=mipmaps;
   }

// enable pixel shader
void ministrip::enablepixshader(int num)
   {
   if (num<0 || num>=SHADERMAX) ERRORMSG();

#ifndef NOOGL

#if defined(GL_ARB_vertex_program) && defined(GL_ARB_fragment_program)

   int i;

   GLuint frgprogid;

   initglexts();
   initwglprocs();

   if (GLEXT_VP!=0 && GLEXT_FP!=0)
      if (SHADER[num].frgprog!=NULL)
         {
         if (SHADER[num].frgdirty!=0)
            {
            if (SHADER[num].frgprogid!=0)
               {
               frgprogid=SHADER[num].frgprogid;
               glDeleteProgramsARB(1,&frgprogid);
               }

            glGenProgramsARB(1,&frgprogid);
            glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,frgprogid);
            glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB,GL_PROGRAM_FORMAT_ASCII_ARB,strlen(SHADER[num].frgprog),SHADER[num].frgprog);
            SHADER[num].frgprogid=frgprogid;

            SHADER[num].frgdirty=0;
            }

         if (SHADER[num].frgprogid!=0)
            {
            glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,SHADER[num].frgprogid);
            glEnable(GL_FRAGMENT_PROGRAM_ARB);

            for (i=0; i<SHADERFRGPRMMAX; i++)
               glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,i,SHADER[num].pixshaderpar1[i],SHADER[num].pixshaderpar2[i],SHADER[num].pixshaderpar3[i],SHADER[num].pixshaderpar4[i]);

            if (GLEXT_MT!=0)
               {
#ifdef GL_ARB_multitexture
               for (i=0; i<SHADERFRGTEXMAX; i++)
                  if (SHADER[num].pixshadertexid[i]!=0)
                     {
                     glActiveTextureARB(GL_TEXTURE0_ARB+i);
                     bindtexmap(SHADER[num].pixshadertexid[i],0,0,0,SHADER[num].pixshadertexmm[i]);
                     }

               glActiveTextureARB(GL_TEXTURE0_ARB);
#endif
               }
            }
         }

#endif

#endif
   }

// disable pixel shader
void ministrip::disablepixshader(int num)
   {
#ifndef NOOGL

#if defined(GL_ARB_vertex_program) && defined(GL_ARB_fragment_program)

   int i;

   if (GLEXT_VP!=0 && GLEXT_FP!=0)
      if (SHADER[num].frgprog!=NULL)
         {
         glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,0);
         glDisable(GL_FRAGMENT_PROGRAM_ARB);

         if (GLEXT_MT!=0)
            {
#ifdef GL_ARB_multitexture
            for (i=0; i<SHADERFRGTEXMAX; i++)
               if (SHADER[num].pixshadertexid[i]!=0)
                  {
                  glActiveTextureARB(GL_TEXTURE0_ARB+i);
                  bindtexmap(0,0,0,0,0);
                  }

            glActiveTextureARB(GL_TEXTURE0_ARB);
#endif
            }
         }

#endif

#endif
   }

// set direct shading parameters
void ministrip::setshadedirectparams(int num,
                                     float lightdir[3],
                                     float lightbias,float lightoffset)
   {
   setpixshaderparams(num,lightdir[0],lightdir[1],lightdir[2],0.0f,5);
   setpixshaderparams(num,lightbias,lightoffset,0.0f,0.0f,4);
   }

// set direct texturing parameters
void ministrip::settexturedirectparams(int num,
                                       float lightdir[3],
                                       float transbias,float transoffset)
   {
   setpixshaderparams(num,lightdir[0],lightdir[1],lightdir[2],PI/2,5);
   setpixshaderparams(num,transbias,transoffset,0.0f,0.5f,4);
   }

// set fog parameters
void ministrip::setfogparams(int num,
                             float fogstart,float fogend,
                             float fogdensity,
                             float *fogcolor)
   {
   float fog_a,fog_b,fog_c;

   // calculate the fog parameters
   if (fogstart<fogend)
      {
      fog_a=fsqr(1.0f/fogend);
      fog_b=0.0f;
      fog_c=log(fmin(0.5f*fogdensity,1.0f))/(2.0f*log(fogstart/fogend/2.0f+0.5f));
      }
   else
      {
      fog_a=0.0f;
      fog_b=0.0f;
      fog_c=1.0f;
      }

   // pass the fog parameters
   setpixshaderparams(num,fog_a,fog_b,fog_c,0.0f,6);
   setpixshaderparams(num,fogcolor[0],fogcolor[1],fogcolor[2],0.0f,7);
   }

// set actual shader
void ministrip::useshader(int num)
   {USESHADER=num;}

// get actual shader
int ministrip::getshader()
   {return(USESHADER);}

// check for OpenGL extensions
void ministrip::initglexts()
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

// render triangle strips
void ministrip::render()
   {
#ifndef NOOGL

   if (COLARRAY==NULL) glColor4f(COLR,COLG,COLB,COLA);
   if (NRMARRAY==NULL) glNormal3f(NRMX,NRMY,NRMZ);

   glPushMatrix();
   glMultMatrixd(MTX);
   glScalef(SCALE,SCALE,SCALE);

   if (ZSCALE!=1.0f)
      {
      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
      glScalef(ZSCALE,ZSCALE,ZSCALE);
      glMatrixMode(GL_MODELVIEW);
      }

   if (USESHADER>=0)
      {
      enablevtxshader(USESHADER);
      enablepixshader(USESHADER);
      }

   glVertexPointer(3,GL_FLOAT,0,VTXARRAY);
   glEnableClientState(GL_VERTEX_ARRAY);

   if (COLARRAY!=NULL)
      {
      glColorPointer(COLCOMPS,GL_FLOAT,0,COLARRAY);
      glEnableClientState(GL_COLOR_ARRAY);
      }
   else glDisableClientState(GL_COLOR_ARRAY);

   if (NRMARRAY!=NULL)
      {
      glNormalPointer(GL_FLOAT,0,NRMARRAY);
      glEnableClientState(GL_NORMAL_ARRAY);
      }
   else glDisableClientState(GL_NORMAL_ARRAY);

   if (TEXARRAY!=NULL)
      {
      glMatrixMode(GL_TEXTURE);
      glPushMatrix();
      glLoadIdentity();
      glMultMatrixd(TEXMTX);
      glMatrixMode(GL_MODELVIEW);

      if (GLEXT_MT!=0)
         {
#ifdef GL_ARB_multitexture
         glClientActiveTextureARB(GL_TEXTURE0_ARB);
#endif
         }

      glTexCoordPointer(TEXCOMPS,GL_FLOAT,0,TEXARRAY);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      }
   else glDisableClientState(GL_TEXTURE_COORD_ARRAY);

   glDrawArrays(GL_TRIANGLE_STRIP,0,SIZE);

   glDisableClientState(GL_VERTEX_ARRAY);

   if (COLARRAY!=NULL) glDisableClientState(GL_COLOR_ARRAY);

   if (NRMARRAY!=NULL) glDisableClientState(GL_NORMAL_ARRAY);

   if (TEXARRAY!=NULL)
      {
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);

      glMatrixMode(GL_TEXTURE);
      glPopMatrix();
      glMatrixMode(GL_MODELVIEW);
      }

   if (USESHADER>=0)
      {
      disablevtxshader(USESHADER);
      disablepixshader(USESHADER);
      }

   if (ZSCALE!=1.0f)
      {
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      glMatrixMode(GL_MODELVIEW);
      }

   glPopMatrix();

#endif
   }

// render triangle strips with multi-pass method for unordered semi-transparent geometry
void ministrip::rendermulti(int passes,
                            float ambient,
                            float bordercontrol,float centercontrol,float colorcontrol,
                            float bordercontrol2,float centercontrol2,float colorcontrol2,
                            float stripewidth,float stripeoffset,
                            float stripedx,float stripedy,float stripedz,
                            int correctz)
   {
   int i;

   static minisurf surf;

   int shader;

   int dorender;

   if (passes<1 || passes>4) ERRORMSG();

   // disable regular shader
   shader=getshader();
   useshader(-1);

   // multi-pass rendering
   for (i=minisurf::FIRST_RENDER_PHASE; i<=minisurf::LAST_RENDER_PHASE; i++)
      {
      // enable external multi-pass shader
      dorender=surf.setextstate(1,
                                i,passes,
                                ambient,
                                bordercontrol,centercontrol,colorcontrol,
                                bordercontrol2,centercontrol2,colorcontrol2,
                                stripewidth,stripeoffset,stripedx,stripedy,stripedz,
                                correctz);

      // render strips with external multi-pass shader
      if (dorender!=0) render();

      // disable external multi-pass shader
      surf.setextstate(0,
                       i,passes,
                       ambient,
                       bordercontrol,centercontrol,colorcontrol,
                       bordercontrol2,centercontrol2,colorcontrol2,
                       stripewidth,stripeoffset,stripedx,stripedy,stripedz,
                       correctz);
      }

   // enable previous shader
   useshader(shader);
   }

// get vertex shader
char *ministrip::getvtxshader(int num)
   {
   if (num<0 || num>=SHADERMAX) ERRORMSG();

   return(SHADER[num].vtxprog);
   }

// get pixel shader
char *ministrip::getpixshader(int num)
   {
   if (num<0 || num>=SHADERMAX) ERRORMSG();

   return(SHADER[num].frgprog);
   }

// Windows OpenGL extension setup
void ministrip::initwglprocs()
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
