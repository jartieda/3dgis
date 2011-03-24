// (c) by Stefan Roettger

#include "minibase.h"

#include "miniv3d.h"

#include "minibrick.h"

// vertex arrays:

// default constructor
minivtxarray::minivtxarray()
   {
   MAXSIZE=1;
   if ((ARRAY=(float *)malloc(6*MAXSIZE*sizeof(float)))==NULL) ERRORMSG();
   SIZE=0;
   }

// destructor
minivtxarray::~minivtxarray()
   {free(ARRAY);}

// shrink to minimum
void minivtxarray::shrink()
   {
   if (MAXSIZE>1)
      {
      MAXSIZE=1;
      if ((ARRAY=(float *)realloc(ARRAY,6*MAXSIZE*sizeof(float)))==NULL) ERRORMSG();
      }

   SIZE=0;
   }

// reset array
void minivtxarray::reset()
   {SIZE=0;}

// add one vertex to the array (including normals)
void minivtxarray::addvtx(const float x,const float y,const float z,
                          const float nx,const float ny,const float nz)
   {
   float *ptr;

   if (SIZE>=MAXSIZE)
      {
      MAXSIZE*=2;
      if ((ARRAY=(float *)realloc(ARRAY,6*MAXSIZE*sizeof(float)))==NULL) ERRORMSG();
      }

   ptr=&ARRAY[6*SIZE++];

   *ptr++=nx;
   *ptr++=ny;
   *ptr++=nz;

   *ptr++=x;
   *ptr++=y;
   *ptr++=z;
   }

// iso spectrum:

// default constructor
minispect::minispect()
   {
   MINVAL=0.0f;
   MAXVAL=1.0f;

   N=0;
   NMAX=10;

   ISO=new float[NMAX];
   ISO2=new float[NMAX];

   RGBA=new float[4*NMAX];

   VTX1=new minivtxarray[NMAX];
   VTX2=new minivtxarray[NMAX];

   CSIZE=256;
   CULLMAP=NULL;

   UPDATED=0;

   WRITEBUF=0;
   READBUF=0;
   }

// destructor
minispect::~minispect()
   {
   delete[] ISO;
   delete[] ISO2;

   delete[] RGBA;

   delete[] VTX1;
   delete[] VTX2;

   if (CULLMAP!=NULL) delete[] CULLMAP;
   }

// determine range of iso values
void minispect::setrange(float minval,float maxval)
   {
   if (minval>maxval) ERRORMSG();

   if (minval==maxval) maxval++;

   if (minval==MINVAL && maxval==MAXVAL) return;

   MINVAL=minval;
   MAXVAL=maxval;

   UPDATED=1;
   }

// add iso value
void minispect::addiso(float iso,float R,float G,float B,float A)
   {
   int i;

   float *newiso,*newiso2;
   float *newrgba;

   minivtxarray *newvtx1,*newvtx2;

   for (i=0; i<N; i++)
      if (iso==ISO2[i])
         {
         RGBA[4*i]=R;
         RGBA[4*i+1]=G;
         RGBA[4*i+2]=B;
         RGBA[4*i+3]=A;

         return;
         }

   if (N>=NMAX)
      {
      NMAX*=2;

      newiso=new float[NMAX];
      newiso2=new float[NMAX];
      newrgba=new float[4*NMAX];

      newvtx1=new minivtxarray[NMAX];
      newvtx2=new minivtxarray[NMAX];

      for (i=0; i<N; i++)
         {
         newiso[i]=ISO[i];
         newiso2[i]=ISO2[i];

         newrgba[4*i]=RGBA[4*i];
         newrgba[4*i+1]=RGBA[4*i+1];
         newrgba[4*i+2]=RGBA[4*i+2];
         newrgba[4*i+3]=RGBA[4*i+3];

         newvtx1[i]=VTX1[i];
         newvtx1[i].reset();

         newvtx2[i]=VTX2[i];
         newvtx2[i].reset();
         }

      delete[] ISO;
      delete[] ISO2;

      delete[] RGBA;

      delete[] VTX1;
      delete[] VTX2;

      ISO=newiso;
      ISO2=newiso2;

      RGBA=newrgba;

      VTX1=newvtx1;
      VTX2=newvtx2;
      }

   ISO[N]=iso;
   ISO2[N]=iso;

   RGBA[4*N]=R;
   RGBA[4*N+1]=G;
   RGBA[4*N+2]=B;
   RGBA[4*N+3]=A;

   N++;

   UPDATED=1;
   }

// change iso value
void minispect::changeiso(float iso,float iso2)
   {
   int i;

   for (i=0; i<N; i++)
      if (iso==ISO2[i])
         {
         ISO2[i]=iso2;
         return;
         }
   }

// change iso color
void minispect::changeiso(float iso,float R,float G,float B,float A)
   {
   int i;

   for (i=0; i<N; i++)
      if (iso==ISO2[i])
         {
         RGBA[4*i]=R;
         RGBA[4*i+1]=G;
         RGBA[4*i+2]=B;
         RGBA[4*i+3]=A;

         return;
         }
   }

// delete iso value
void minispect::deliso(float iso)
   {
   int i;

   for (i=0; i<N; i++)
      if (iso==ISO2[i])
         {
         ISO[i]=ISO[N-1];
         ISO2[i]=ISO2[N-1];

         RGBA[4*i]=RGBA[4*(N-1)];
         RGBA[4*i+1]=RGBA[4*(N-1)+1];
         RGBA[4*i+2]=RGBA[4*(N-1)+2];
         RGBA[4*i+3]=RGBA[4*(N-1)+3];

         VTX1[N-1].shrink();
         VTX1[i].reset();

         VTX2[N-1].shrink();
         VTX2[i].reset();

         N--;

         UPDATED=1;

         break;
         }
   }

// check iso value
int minispect::chkiso(float iso)
   {
   int i;

   for (i=0; i<N; i++)
      if (iso==ISO2[i]) return(1);

   return(0);
   }

// synchronize iso spectrum
void minispect::sync()
   {
   int i;

   for (i=0; i<N; i++)
      if (ISO2[i]!=ISO[i])
         {
         ISO[i]=ISO2[i];
         UPDATED=1;
         }
   }

// reset iso spectrum
void minispect::reset()
   {
   N=0;
   UPDATED=1;
   }

// determine size of cullmap
void minispect::setcullmapsize(int size)
   {
   if (size<2) ERRORMSG();

   if (size==CSIZE) return;

   CSIZE=size;

   UPDATED=1;
   }

// get cull map
unsigned char *minispect::getcullmap()
   {
   int i,j,k;

   float imin,imax;

   if (CULLMAP==NULL || UPDATED!=0)
      {
      if (CULLMAP!=NULL) delete[] CULLMAP;

      CULLMAP=new unsigned char[CSIZE*CSIZE];

      for (i=0; i<CSIZE; i++)
         for (j=0; j<CSIZE; j++)
            {
            if (i<=j)
               {
               imin=(float)i/(CSIZE-1);
               imax=(float)j/(CSIZE-1);
               }
            else
               {
               imin=(float)j/(CSIZE-1);
               imax=(float)i/(CSIZE-1);
               }

            imin=MINVAL+imin*(MAXVAL-MINVAL);
            imax=MINVAL+imax*(MAXVAL-MINVAL);

            CULLMAP[i+j*CSIZE]=1;

            for (k=0; k<N; k++)
               if (ISO[k]>=imin && ISO[k]<=imax)
                  {
                  CULLMAP[i+j*CSIZE]=0;
                  break;
                  }
            }

      UPDATED=0;
      }

   return(CULLMAP);
   }

// evaluate cull map
int minispect::isvisible(float minval,float maxval)
   {
   int i,j;

   unsigned char *cullmap;

   cullmap=getcullmap();

   minval=(minval-MINVAL)/(MAXVAL-MINVAL);
   maxval=(maxval-MINVAL)/(MAXVAL-MINVAL);

   i=ftrc(ffloor((CSIZE-1)*minval));
   j=ftrc(fceil((CSIZE-1)*maxval));

   if (i<0 && j<0) return(0);
   if (i>CSIZE-1 && j>CSIZE-1) return(0);

   if (i<0) i=0;
   else if (i>CSIZE-1) i=CSIZE-1;

   if (j<0) j=0;
   else if (j>CSIZE-1) j=CSIZE-1;

   if (cullmap[i+j*CSIZE]!=0) return(0);

   return(1);
   }

// get number of opaque iso surfaces
int minispect::getopaqnum()
   {
   int i,n;

   for (i=n=0; i<N; i++)
      if (getA(i)==1.0f) n++;

   return(n);
   }

// get number of semi-transparent iso surfaces
int minispect::gettransnum()
   {
   int i,n;

   for (i=n=0; i<N; i++)
      if (getA(i)>0.0f && getA(i)<1.0f) n++;

   return(n);
   }

// shrink vertex buffers to minimum
void minispect::shrinkbufs()
   {
   int i;

   for (i=0; i<N; i++)
      {
      VTX1[i].shrink();
      VTX2[i].shrink();
      }
   }

// swap vertex array for writing
void minispect::swapwritebuf()
   {WRITEBUF=1-WRITEBUF;}

// swap vertex array for reading
void minispect::swapreadbuf()
   {READBUF=1-READBUF;}

// get vertex array for writing
minivtxarray *minispect::getwritebuf(int n)
   {
   if (WRITEBUF==0) return(&VTX1[n]);
   else return(&VTX2[n]);
   }

// get vertex array for reading
minivtxarray *minispect::getreadbuf(int n)
   {
   if (READBUF==0) return(&VTX1[n]);
   else return(&VTX2[n]);
   }

// iso surfaces:

int minisurf::INSTANCES=0;

int minisurf::VTXPROGID;
int minisurf::FRAGPROGID;

int minisurf::VTXPROGID2;
int minisurf::FRAGPROGID2;

// default constructor
minisurf::minisurf()
   {
   STRIPEWIDTH=1.0f;

   STRIPEDX=0.0f;
   STRIPEDY=0.0f;
   STRIPEDZ=1.0f;

   STRIPEOFFSET=0.0f;

   CORRECTZ=0;

   if (INSTANCES++==0)
      {
      VTXPROGID=0;
      FRAGPROGID=0;

      VTXPROGID2=0;
      FRAGPROGID2=0;
      }

   GLSETUP=0;
   WGLSETUP=0;

   GLEXT_VP=0;
   GLEXT_FP=0;
   }

// destructor
minisurf::~minisurf()
   {
#if defined(GL_ARB_vertex_program) && defined(GL_ARB_fragment_program)

   GLuint progid;

   if (--INSTANCES==0)
      if (GLEXT_VP!=0 && GLEXT_FP!=0)
         {
         if (VTXPROGID!=0)
            {
            progid=VTXPROGID;
            glDeleteProgramsARB(1,&progid);
            VTXPROGID=0;
            }

         if (FRAGPROGID!=0)
            {
            progid=FRAGPROGID;
            glDeleteProgramsARB(1,&progid);
            FRAGPROGID=0;
            }

         if (VTXPROGID2!=0)
            {
            progid=VTXPROGID2;
            glDeleteProgramsARB(1,&progid);
            VTXPROGID2=0;
            }

         if (FRAGPROGID2!=0)
            {
            progid=FRAGPROGID2;
            glDeleteProgramsARB(1,&progid);
            FRAGPROGID2=0;
            }
         }

#endif
   }

// reset surface
void minisurf::reset()
   {
   int i;

   for (i=0; i<SPECTRUM.getnum(); i++) SPECTRUM.getwritebuf(i)->reset();
   }

// check for OpenGL extensions
void minisurf::initglexts()
   {
#ifndef NOOGL

   char *gl_exts;

   if (GLSETUP==0)
      {
      GLEXT_VP=0;
      GLEXT_FP=0;

      if ((gl_exts=(char *)glGetString(GL_EXTENSIONS))==NULL) ERRORMSG();

      if (strstr(gl_exts,"GL_ARB_vertex_program")!=NULL) GLEXT_VP=1;
      if (strstr(gl_exts,"GL_ARB_fragment_program")!=NULL) GLEXT_FP=1;

      GLSETUP=1;
      }

#endif
   }

// enable torch light
void minisurf::enabletorch(int phase,
                           float ambient,
                           float bordercontrol,float centercontrol,float colorcontrol,
                           float fogstart,float fogend,
                           float fogdensity,
                           float fogcolor[3])
   {
#ifndef NOOGL

   static GLfloat ZROlight[]={0.0f,0.0f,0.0f,1.0f};
   static GLfloat lightdir[]={0.0f,0.0f,1.0f,0.0f};

   GLfloat AMBlight[]={ambient,ambient,ambient,1.0f};
   GLfloat DIRlight[]={1.0f-ambient,1.0f-ambient,1.0f-ambient,1.0f};

#if defined(GL_ARB_vertex_program) && defined(GL_ARB_fragment_program)

   float fog_a,fog_b,fog_c;

   static const char *vtxprog="!!ARBvp1.0 \n\
      OPTION ARB_position_invariant; \n\
      PARAM mod[4]={state.matrix.modelview}; \n\
      PARAM invtra[4]={state.matrix.modelview.invtrans}; \n\
      TEMP vtx,col,nrm,pos,vec; \n\
      ### fetch actual vertex \n\
      MOV vtx,vertex.position; \n\
      MOV col,vertex.color; \n\
      MOV nrm,vertex.normal; \n\
      ### transform vertex with modelview \n\
      DP4 pos.x,mod[0],vtx; \n\
      DP4 pos.y,mod[1],vtx; \n\
      DP4 pos.z,mod[2],vtx; \n\
      DP4 pos.w,mod[3],vtx; \n\
      ### transform normal with inverse transpose \n\
      DP4 vec.x,invtra[0],nrm; \n\
      DP4 vec.y,invtra[1],nrm; \n\
      DP4 vec.z,invtra[2],nrm; \n\
      DP4 vec.w,invtra[3],nrm; \n\
      ### write resulting color \n\
      MOV result.color,col; \n\
      ### write normal to tex coords \n\
      MOV result.texcoord,vec; \n\
      ### write view position to tex coords \n\
      MOV result.texcoord[1],pos; \n\
      ### calculate spherical fog coord \n\
      DP3 result.fogcoord.x,pos,pos; \n\
      END \n";

   static const char *fragprog1="!!ARBfp1.0 \n\
      PARAM c=program.env[0]; \n\
      PARAM d=program.env[1]; \n\
      TEMP col,nrm,pos,len; \n\
      ### fetch actual color \n\
      MOV col,fragment.color; \n\
      ### fetch fragment normal \n\
      MOV nrm,fragment.texcoord; \n\
      ### fetch view position \n\
      MOV pos,fragment.texcoord[1]; \n\
      ### apply head light to color and opacity \n\
      MUL nrm.z,nrm.z,c.z; \n\
      DP3 len.x,nrm,nrm; \n\
      RSQ len.x,len.x; \n\
      MUL nrm,nrm,len.x; \n\
      DP3 len.x,pos,pos; \n\
      RSQ len.x,len.x; \n\
      MUL pos,pos,len.x; \n\
      DP3 nrm.z,nrm,pos; \n\
      ABS nrm.z,nrm.z; \n\
      MAD_SAT nrm.x,nrm.z,d.x,d.y; \n\
      MAD_SAT nrm.y,nrm.z,d.z,d.w; \n\
      MUL col.xyz,col,nrm.x; \n\
      MUL col.w,col,nrm.y; \n\
      ### write resulting color \n\
      MOV result.color,col; \n\
      END \n";

   static const char *fragprog2="!!ARBfp1.0 \n\
      PARAM c=program.env[0]; \n\
      PARAM d=program.env[1]; \n\
      PARAM e=program.env[2]; \n\
      PARAM f=program.env[3]; \n\
      PARAM g=program.env[4]; \n\
      TEMP col,nrm,pos,len; \n\
      ### fetch actual color \n\
      MOV col,fragment.color; \n\
      ### fetch fragment normal \n\
      MOV nrm,fragment.texcoord; \n\
      ### fetch world position \n\
      MOV pos,fragment.texcoord[1]; \n\
      ### apply head light to color and opacity \n\
      MUL nrm.z,nrm.z,c.z; \n\
      DP3 len.x,nrm,nrm; \n\
      RSQ len.x,len.x; \n\
      MUL nrm,nrm,len.x; \n\
      DP3 len.x,pos,pos; \n\
      RSQ len.x,len.x; \n\
      MUL pos,pos,len.x; \n\
      DP3 nrm.z,nrm,pos; \n\
      ABS nrm.z,nrm.z; \n\
      MAD_SAT nrm.x,nrm.z,d.x,d.y; \n\
      MAD_SAT nrm.y,nrm.z,d.z,d.w; \n\
      MUL col.xyz,col,nrm.x; \n\
      MUL col.w,col,nrm.y; \n\
      ### modulate with spherical fog \n\
      MOV fog.x,fragment.fogcoord.x; \n\
      MAD_SAT fog.x,fog.x,f.x,f.y; \n\
      POW fog.x,fog.x,f.z; \n\
      LRP col.xyz,fog.x,g,col; \n\
      ### write resulting color \n\
      MOV result.color,col; \n\
      END \n";

   GLuint vtxprogid,fragprogid;

#endif

   // Phong alpha shading
   if (GLEXT_VP!=0 && GLEXT_FP!=0)
      {
#if defined(GL_ARB_vertex_program) && defined(GL_ARB_fragment_program)

      if (VTXPROGID==0)
         {
         glGenProgramsARB(1,&vtxprogid);
         glBindProgramARB(GL_VERTEX_PROGRAM_ARB,vtxprogid);
         glProgramStringARB(GL_VERTEX_PROGRAM_ARB,GL_PROGRAM_FORMAT_ASCII_ARB,strlen(vtxprog),vtxprog);
         VTXPROGID=vtxprogid;
         }

      if (FRAGPROGID==0)
         {
         glGenProgramsARB(1,&fragprogid);
         glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,fragprogid);
         if (fogstart<fogend && fogcolor!=NULL) glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB,GL_PROGRAM_FORMAT_ASCII_ARB,strlen(fragprog2),fragprog2);
         else glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB,GL_PROGRAM_FORMAT_ASCII_ARB,strlen(fragprog1),fragprog1);
         FRAGPROGID=fragprogid;
         }

      glBindProgramARB(GL_VERTEX_PROGRAM_ARB,VTXPROGID);
      glEnable(GL_VERTEX_PROGRAM_ARB);

      glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,FRAGPROGID);
      glEnable(GL_FRAGMENT_PROGRAM_ARB);

      glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,0,0.0f,0.0f,bordercontrol,0.0f);

      if (phase==FIRST_RENDER_PHASE) glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,1,1.0f-ambient,ambient,0.0f,1.0f);
      else glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,1,(1.0f-ambient)*colorcontrol,ambient*colorcontrol+1.0f-colorcontrol,-centercontrol,1.0f);

      // calculate the fog parameters
      if (fogstart<fogend && fogcolor!=NULL)
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
      glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,3,fog_a,fog_b,fog_c,0.0f);
      if (fogcolor!=NULL) glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,4,fogcolor[0],fogcolor[1],fogcolor[2],0.0f);

#endif
      }
   // Gouraud shading (results in popping artifacts)
   else if (phase==FIRST_RENDER_PHASE)
      {
      glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
      glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);
      glLightModelfv(GL_LIGHT_MODEL_AMBIENT,ZROlight);

      glLightfv(GL_LIGHT0,GL_AMBIENT,AMBlight);
      glLightfv(GL_LIGHT0,GL_DIFFUSE,DIRlight);

      glPushMatrix();
      glLoadIdentity();
      glLightfv(GL_LIGHT0,GL_POSITION,lightdir);
      glPopMatrix();

      glEnable(GL_COLOR_MATERIAL);
      glEnable(GL_NORMALIZE);

      glEnable(GL_LIGHT0);
      glEnable(GL_LIGHTING);
      }

#endif
   }

// disable torch light
void minisurf::disabletorch(int phase)
   {
#ifndef NOOGL

   if (GLEXT_VP!=0 && GLEXT_FP!=0)
      {
#if defined(GL_ARB_vertex_program) && defined(GL_ARB_fragment_program)

      glBindProgramARB(GL_VERTEX_PROGRAM_ARB,0);
      glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,0);

      glDisable(GL_VERTEX_PROGRAM_ARB);
      glDisable(GL_FRAGMENT_PROGRAM_ARB);

#endif
      }
   else if (phase==FIRST_RENDER_PHASE)
      {
      glDisable(GL_COLOR_MATERIAL);
      glDisable(GL_NORMALIZE);

      glDisable(GL_LIGHT0);
      glDisable(GL_LIGHTING);
      }

#endif
   }

// enable pattern
void minisurf::enablepattern(float ambient,
                             float bordercontrol,
                             float fogstart,float fogend,
                             float fogdensity,
                             float fogcolor[3])
   {
#ifndef NOOGL

#if defined(GL_ARB_vertex_program) && defined(GL_ARB_fragment_program)

   float fog_a,fog_b,fog_c;

   static const char *vtxprog="!!ARBvp1.0 \n\
      OPTION ARB_position_invariant; \n\
      PARAM mod[4]={state.matrix.modelview}; \n\
      PARAM invtra[4]={state.matrix.modelview.invtrans}; \n\
      ### fetch actual vertex \n\
      TEMP vtx,col,nrm,pos,vec; \n\
      MOV vtx,vertex.position; \n\
      MOV col,vertex.color; \n\
      MOV nrm,vertex.normal; \n\
      ### transform vertex with modelview \n\
      DP4 pos.x,mod[0],vtx; \n\
      DP4 pos.y,mod[1],vtx; \n\
      DP4 pos.z,mod[2],vtx; \n\
      DP4 pos.w,mod[3],vtx; \n\
      ### transform normal with inverse transpose \n\
      DP4 vec.x,invtra[0],nrm; \n\
      DP4 vec.y,invtra[1],nrm; \n\
      DP4 vec.z,invtra[2],nrm; \n\
      DP4 vec.w,invtra[3],nrm; \n\
      ### write resulting color \n\
      MOV result.color,col; \n\
      ### write normal to tex coords \n\
      MOV result.texcoord,vec; \n\
      ### write view position to tex coords \n\
      MOV result.texcoord[1],pos; \n\
      ### write world position to tex coords \n\
      MOV result.texcoord[2],vtx; \n\
      ### calculate spherical fog coord \n\
      DP3 result.fogcoord.x,pos,pos; \n\
      END \n";

   static const char *fragprog1="!!ARBfp1.0 \n\
      PARAM c=program.env[0]; \n\
      PARAM d=program.env[1]; \n\
      PARAM e=program.env[2]; \n\
      TEMP col,nrm,pos,crd,len; \n\
      ### fetch actual color \n\
      MOV col,fragment.color; \n\
      ### fetch fragment normal \n\
      MOV nrm,fragment.texcoord; \n\
      ### fetch view position \n\
      MOV pos,fragment.texcoord[1]; \n\
      ### fetch world position \n\
      MOV crd,fragment.texcoord[2]; \n\
      ### apply pattern to opacity \n\
      DP3 crd.w,crd,e; \n\
      SUB crd.w,crd.w,e.w; \n\
      FRC crd.w,crd.w; \n\
      SUB_SAT col.w,col.w,crd.w; \n\
      ### apply head light to color \n\
      MUL nrm.z,nrm.z,c.z; \n\
      DP3 len.x,nrm,nrm; \n\
      RSQ len.x,len.x; \n\
      MUL nrm,nrm,len.x; \n\
      DP3 len.x,pos,pos; \n\
      RSQ len.x,len.x; \n\
      MUL pos,pos,len.x; \n\
      DP3 nrm.z,nrm,pos; \n\
      ABS nrm.z,nrm.z; \n\
      MAD_SAT nrm.x,nrm.z,d.x,d.y; \n\
      MUL col.xyz,col,nrm.x; \n\
      ### write resulting color \n\
      MOV result.color,col; \n\
      END \n";

   static const char *fragprog2="!!ARBfp1.0 \n\
      PARAM c=program.env[0]; \n\
      PARAM d=program.env[1]; \n\
      PARAM e=program.env[2]; \n\
      PARAM f=program.env[3]; \n\
      PARAM g=program.env[4]; \n\
      TEMP col,nrm,pos,crd,len; \n\
      ### fetch actual color \n\
      MOV col,fragment.color; \n\
      ### fetch fragment normal \n\
      MOV nrm,fragment.texcoord; \n\
      ### fetch view position \n\
      MOV pos,fragment.texcoord[1]; \n\
      ### fetch world position \n\
      MOV crd,fragment.texcoord[2]; \n\
      ### apply pattern to opacity \n\
      DP3 crd.w,crd,e; \n\
      SUB crd.w,crd.w,e.w; \n\
      FRC crd.w,crd.w; \n\
      SUB_SAT col.w,col.w,crd.w; \n\
      ### apply head light to color \n\
      MUL nrm.z,nrm.z,c.z; \n\
      DP3 len.x,nrm,nrm; \n\
      RSQ len.x,len.x; \n\
      MUL nrm,nrm,len.x; \n\
      DP3 len.x,pos,pos; \n\
      RSQ len.x,len.x; \n\
      MUL pos,pos,len.x; \n\
      DP3 nrm.z,nrm,pos; \n\
      ABS nrm.z,nrm.z; \n\
      MAD_SAT nrm.x,nrm.z,d.x,d.y; \n\
      MUL col.xyz,col,nrm.x; \n\
      ### modulate with spherical fog \n\
      MOV fog.x,fragment.fogcoord.x; \n\
      MAD_SAT fog.x,fog.x,f.x,f.y; \n\
      POW fog.x,fog.x,f.z; \n\
      LRP col.xyz,fog.x,g,col; \n\
      ### write resulting color \n\
      MOV result.color,col; \n\
      END \n";

   GLuint vtxprogid,fragprogid;

#endif

   if (GLEXT_VP!=0 && GLEXT_FP!=0)
      {
#if defined(GL_ARB_vertex_program) && defined(GL_ARB_fragment_program)

      if (VTXPROGID2==0)
         {
         glGenProgramsARB(1,&vtxprogid);
         glBindProgramARB(GL_VERTEX_PROGRAM_ARB,vtxprogid);
         glProgramStringARB(GL_VERTEX_PROGRAM_ARB,GL_PROGRAM_FORMAT_ASCII_ARB,strlen(vtxprog),vtxprog);
         VTXPROGID2=vtxprogid;
         }

      if (FRAGPROGID2==0)
         {
         glGenProgramsARB(1,&fragprogid);
         glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,fragprogid);
         if (fogstart<fogend && fogcolor!=NULL) glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB,GL_PROGRAM_FORMAT_ASCII_ARB,strlen(fragprog2),fragprog2);
         else glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB,GL_PROGRAM_FORMAT_ASCII_ARB,strlen(fragprog1),fragprog1);
         FRAGPROGID2=fragprogid;
         }

      glBindProgramARB(GL_VERTEX_PROGRAM_ARB,VTXPROGID2);
      glEnable(GL_VERTEX_PROGRAM_ARB);

      glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,FRAGPROGID2);
      glEnable(GL_FRAGMENT_PROGRAM_ARB);

      glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,0,0.0f,0.0f,bordercontrol,0.0f);
      glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,1,1.0f-ambient,ambient,0.0f,0.0f);
      glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,2,STRIPEWIDTH*STRIPEDX,STRIPEWIDTH*STRIPEDY,STRIPEWIDTH*STRIPEDZ,STRIPEOFFSET);

      // calculate the fog parameters
      if (fogstart<fogend && fogcolor!=NULL)
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
      glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,3,fog_a,fog_b,fog_c,0.0f);
      if (fogcolor!=NULL) glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,4,fogcolor[0],fogcolor[1],fogcolor[2],0.0f);

#endif
      }

#endif
   }

// disable pattern
void minisurf::disablepattern()
   {
#ifndef NOOGL

   if (GLEXT_VP!=0 && GLEXT_FP!=0)
      {
#if defined(GL_ARB_vertex_program) && defined(GL_ARB_fragment_program)

      glBindProgramARB(GL_VERTEX_PROGRAM_ARB,0);
      glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,0);

      glDisable(GL_VERTEX_PROGRAM_ARB);
      glDisable(GL_FRAGMENT_PROGRAM_ARB);

#endif
      }

#endif
   }

// render triangles
void minisurf::renderarrays(int phase)
   {
#ifndef NOOGL

   int n;

   for (n=0; n<SPECTRUM.getnum(); n++)
      if ((phase==FIRST_RENDER_PHASE && SPECTRUM.getA(n)==1.0f) ||
          (phase!=FIRST_RENDER_PHASE && SPECTRUM.getA(n)>0.0f && SPECTRUM.getA(n)<1.0f))
         if (SPECTRUM.getreadbuf(n)->getsize()>0)
            {
            glColor4f(SPECTRUM.getR(n),
                      SPECTRUM.getG(n),
                      SPECTRUM.getB(n),
                      SPECTRUM.getA(n));

            glDisableClientState(GL_COLOR_ARRAY);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);

            glEnableClientState(GL_NORMAL_ARRAY);
            glEnableClientState(GL_VERTEX_ARRAY);

            glInterleavedArrays(GL_N3F_V3F,0,SPECTRUM.getreadbuf(n)->getarray());
            glDrawArrays(GL_TRIANGLES,0,SPECTRUM.getreadbuf(n)->getsize());

            glDisableClientState(GL_NORMAL_ARRAY);
            glDisableClientState(GL_VERTEX_ARRAY);
            }

#endif
   }

// initialize state for each phase
void minisurf::setstate(int enable,
                        int phase,
                        int passes,
                        float ambient,
                        float bordercontrol,float centercontrol,float colorcontrol,
                        float bordercontrol2,float centercontrol2,float colorcontrol2)
   {
#ifndef NOOGL

   initglexts();
   initwglprocs();

   // first pass for opaque triangles
   if (phase==ONE_RENDER_PHASE || phase==FIRST_RENDER_PHASE)
      if (enable!=0) enabletorch(FIRST_RENDER_PHASE,ambient,bordercontrol,centercontrol,colorcontrol);
      else disabletorch(FIRST_RENDER_PHASE);

   // set common OpenGL states
   if (phase!=FIRST_RENDER_PHASE)
      if (enable!=0)
         {
         enableAtest();
         disableculling();
         }

   // pseudo single-pass rendering
   // iso surfaces exhibit a stripe pattern
   if (passes==1)
      {
      // second pass for striped triangles
      if (phase==ONE_RENDER_PHASE || phase==SECOND_RENDER_PHASE)
         if (enable!=0) enablepattern(ambient,bordercontrol);
         else disablepattern();
      }
   // fast 2-pass rendering
   // iso surfaces are assumed to have equal emission (and opacity)
   // otherwise discontinuity artifacts will occur
   else if (passes==2)
      {
      // second pass for semi-transparent triangles
      if (phase==ONE_RENDER_PHASE || phase==SECOND_RENDER_PHASE)
         if (enable!=0)
            {
            enableblending();
            disableZwriting();

            enabletorch(SECOND_RENDER_PHASE,ambient,bordercontrol,centercontrol,colorcontrol);
            }
         else
            {
            disabletorch(SECOND_RENDER_PHASE);

            enableZwriting();
            disableblending();
            }

      // optional third pass for correct Z
      if (CORRECTZ!=0)
         if (phase==ONE_RENDER_PHASE || phase==LAST_RENDER_PHASE)
            if (enable!=0) disableRGBAwriting();
            else enableRGBAwriting();
      }
   // 3-pass rendering with separate attenuation and emission
   // good compromise between performance and appearance
   // without discontinuity artifacts but also with reduced depth perception
   else if (passes==3)
      {
      // second pass for attenuation
      if (phase==ONE_RENDER_PHASE || phase==SECOND_RENDER_PHASE)
         if (enable!=0)
            {
            enableATTblending();
            disableZwriting();

            enabletorch(SECOND_RENDER_PHASE,ambient,bordercontrol,centercontrol,colorcontrol);
            }
         else
            {
            disabletorch(SECOND_RENDER_PHASE);

            enableZwriting();
            disableblending();
            }

      // third pass for emission
      if (phase==ONE_RENDER_PHASE || phase==THIRD_RENDER_PHASE)
         if (enable!=0)
            {
            enableEMIblending();
            disableZwriting();

            enabletorch(THIRD_RENDER_PHASE,ambient,bordercontrol*bordercontrol2,centercontrol*centercontrol2,colorcontrol*colorcontrol2);
            }
         else
            {
            disabletorch(THIRD_RENDER_PHASE);

            enableZwriting();
            disableblending();
            }

      // optional fourth pass for correct Z
      if (CORRECTZ!=0)
         if (phase==ONE_RENDER_PHASE || phase==LAST_RENDER_PHASE)
            if (enable!=0) disableRGBAwriting();
            else enableRGBAwriting();
      }
   // 4-pass rendering with emission being summed up until the first backface is encountered
   // nice appearance with good depth perception at reasonable speed
   // the front-most backface is emphasized by suppressing emission from behind
   else if (passes==4)
      {
      // second pass for attenuation
      if (phase==ONE_RENDER_PHASE || phase==SECOND_RENDER_PHASE)
         if (enable!=0)
            {
            enableATTblending();
            disableZwriting();

            enabletorch(SECOND_RENDER_PHASE,ambient,bordercontrol,centercontrol,colorcontrol);
            }
         else
            {
            disabletorch(SECOND_RENDER_PHASE);

            enableZwriting();
            disableblending();
            }

      // third pass for Z
      if (phase==ONE_RENDER_PHASE || phase==THIRD_RENDER_PHASE)
         if (enable!=0)
            {
            enableFFculling();
            disableRGBAwriting();
            }
         else
            {
            enableRGBAwriting();
            disableculling();
            }

      // fourth pass for emission
      if (phase==ONE_RENDER_PHASE || phase==FOURTH_RENDER_PHASE)
         if (enable!=0)
            {
            enableEMIblending();
            disableZwriting();

            enabletorch(FOURTH_RENDER_PHASE,ambient,bordercontrol*bordercontrol2,centercontrol*centercontrol2,colorcontrol*colorcontrol2);
            }
         else
            {
            disabletorch(FOURTH_RENDER_PHASE);

            enableZwriting();
            disableblending();
            }

      // optional fifth pass for correct Z
      if (CORRECTZ!=0)
         if (phase==ONE_RENDER_PHASE || phase==LAST_RENDER_PHASE)
            if (enable!=0) disableRGBAwriting();
            else enableRGBAwriting();
      }

   // unset common OpenGL states
   if (phase!=FIRST_RENDER_PHASE)
      if (enable==0)
         {
         enableBFculling();
         disableAtest();
         }

#endif
   }

// render pass
void minisurf::renderpass(int phase,
                          int passes,
                          float ambient,
                          float bordercontrol,float centercontrol,float colorcontrol,
                          float bordercontrol2,float centercontrol2,float colorcontrol2)
   {
   setstate(1,phase,passes,
            ambient,bordercontrol,centercontrol,colorcontrol,
            bordercontrol2,centercontrol2,colorcontrol2);

   renderarrays(phase);

   setstate(0,phase,passes,
            ambient,bordercontrol,centercontrol,colorcontrol,
            bordercontrol2,centercontrol2,colorcontrol2);
   }

// render surfaces
void minisurf::render(int phase,
                      int passes,
                      float ambient,
                      float bordercontrol,float centercontrol,float colorcontrol,
                      float bordercontrol2,float centercontrol2,float colorcontrol2)
   {
#ifndef NOOGL

   // single pass for opaque triangles
   if (SPECTRUM.getopaqnum()!=0)
      if (phase==ONE_RENDER_PHASE || phase==FIRST_RENDER_PHASE)
         renderpass(FIRST_RENDER_PHASE,passes,ambient,bordercontrol,centercontrol,colorcontrol,bordercontrol2,centercontrol2,colorcontrol2);

   // multiple passes for semi-transparent triangles
   if (SPECTRUM.gettransnum()!=0)
      if (passes==1)
         {
         if (phase==ONE_RENDER_PHASE || phase==SECOND_RENDER_PHASE)
            renderpass(SECOND_RENDER_PHASE,passes,ambient,bordercontrol,centercontrol,colorcontrol,bordercontrol2,centercontrol2,colorcontrol2);
         }
      else if (passes==2)
         {
         if (phase==ONE_RENDER_PHASE || phase==SECOND_RENDER_PHASE)
            renderpass(SECOND_RENDER_PHASE,passes,ambient,bordercontrol,centercontrol,colorcontrol,bordercontrol2,centercontrol2,colorcontrol2);

         if (CORRECTZ!=0)
            if (phase==ONE_RENDER_PHASE || phase==LAST_RENDER_PHASE)
               renderpass(LAST_RENDER_PHASE,passes,ambient,bordercontrol,centercontrol,colorcontrol,bordercontrol2,centercontrol2,colorcontrol2);
         }
      else if (passes==3)
         {
         if (phase==ONE_RENDER_PHASE || phase==SECOND_RENDER_PHASE)
            renderpass(SECOND_RENDER_PHASE,passes,ambient,bordercontrol,centercontrol,colorcontrol,bordercontrol2,centercontrol2,colorcontrol2);

         if (phase==ONE_RENDER_PHASE || phase==THIRD_RENDER_PHASE)
            renderpass(THIRD_RENDER_PHASE,passes,ambient,bordercontrol,centercontrol,colorcontrol,bordercontrol2,centercontrol2,colorcontrol2);

         if (CORRECTZ!=0)
            if (phase==ONE_RENDER_PHASE || phase==LAST_RENDER_PHASE)
               renderpass(LAST_RENDER_PHASE,passes,ambient,bordercontrol,centercontrol,colorcontrol,bordercontrol2,centercontrol2,colorcontrol2);
         }
      else if (passes==4)
         {
         if (phase==ONE_RENDER_PHASE || phase==SECOND_RENDER_PHASE)
            renderpass(SECOND_RENDER_PHASE,passes,ambient,bordercontrol,centercontrol,colorcontrol,bordercontrol2,centercontrol2,colorcontrol2);

         if (phase==ONE_RENDER_PHASE || phase==THIRD_RENDER_PHASE)
            renderpass(THIRD_RENDER_PHASE,passes,ambient,bordercontrol,centercontrol,colorcontrol,bordercontrol2,centercontrol2,colorcontrol2);

         if (phase==ONE_RENDER_PHASE || phase==FOURTH_RENDER_PHASE)
            renderpass(FOURTH_RENDER_PHASE,passes,ambient,bordercontrol,centercontrol,colorcontrol,bordercontrol2,centercontrol2,colorcontrol2);

         if (CORRECTZ!=0)
            if (phase==ONE_RENDER_PHASE || phase==LAST_RENDER_PHASE)
               renderpass(LAST_RENDER_PHASE,passes,ambient,bordercontrol,centercontrol,colorcontrol,bordercontrol2,centercontrol2,colorcontrol2);
         }

#endif
   }

// initialize state for each phase
int minisurf::setextstate(int enable,
                          int phase,
                          int passes,
                          float ambient,
                          float bordercontrol,float centercontrol,float colorcontrol,
                          float bordercontrol2,float centercontrol2,float colorcontrol2,
                          float stripewidth,float stripeoffset,
                          float stripedx,float stripedy,float stripedz,
                          int correctz)
   {
   if (phase==ONE_RENDER_PHASE) ERRORMSG();

   if (phase==FIRST_RENDER_PHASE) return(0);
   if (passes==1 && phase>SECOND_RENDER_PHASE) return(0);
   if (passes==2 && phase>SECOND_RENDER_PHASE && (correctz==0 || phase!=LAST_RENDER_PHASE)) return(0);
   if (passes==3 && phase>THIRD_RENDER_PHASE && (correctz==0 || phase!=LAST_RENDER_PHASE)) return(0);
   if (passes==4 && phase>FOURTH_RENDER_PHASE && (correctz==0 || phase!=LAST_RENDER_PHASE)) return(0);

   // set stripe pattern
   setstripewidth(stripewidth);
   setstripedir(stripedx,stripedy,stripedz);
   setstripeoffset(stripeoffset);

   // enable correct Z
   setcorrectz(correctz);

   // pass rendering state
   setstate(enable,phase,passes,
            ambient,bordercontrol,centercontrol,colorcontrol,
            bordercontrol2,centercontrol2,colorcontrol2);

   return(1);
   }

// cache one data block
void minisurf::cacheblock(const unsigned int i,const unsigned int j,const unsigned int k,const unsigned int s,
                          const float vc1,const float vc2,const float vc3,const float vc4,const float vc5,const float vc6,const float vc7,const float vc8,
                          const mininorm &nc1,const mininorm &nc2,const mininorm &nc3,const mininorm &nc4,const mininorm &nc5,const mininorm &nc6,const mininorm &nc7,const mininorm &nc8)
   {
   int n;

   float minval,maxval;

   float iso;
   minivtxarray *buf;

   minval=maxval=vc1;

   if (vc2<minval) minval=vc2;
   else if (vc2>maxval) maxval=vc2;

   if (vc3<minval) minval=vc3;
   else if (vc3>maxval) maxval=vc3;

   if (vc4<minval) minval=vc4;
   else if (vc4>maxval) maxval=vc4;

   if (vc5<minval) minval=vc5;
   else if (vc5>maxval) maxval=vc5;

   if (vc6<minval) minval=vc6;
   else if (vc6>maxval) maxval=vc6;

   if (vc7<minval) minval=vc7;
   else if (vc7>maxval) maxval=vc7;

   if (vc8<minval) minval=vc8;
   else if (vc8>maxval) maxval=vc8;

   if ((((i+j+k)/s)%2)==0)
      for (n=0; n<SPECTRUM.getnum(); n++)
         {
         iso=SPECTRUM.getval(n);
         buf=SPECTRUM.getwritebuf(n);

         if (iso<minval || iso>maxval) continue;

         extractiso(iso,
                    i,j,k,vc1,nc1,
                    i+s,j+s,k,vc4,nc4,
                    i+s,j,k+s,vc6,nc6,
                    i,j+s,k+s,vc7,nc7,
                    buf);

         extractiso(iso,
                    i+s,j,k,vc2,nc2,
                    i,j,k,vc1,nc1,
                    i+s,j+s,k,vc4,nc4,
                    i+s,j,k+s,vc6,nc6,
                    buf);

         extractiso(iso,
                    i,j+s,k,vc3,nc3,
                    i,j,k,vc1,nc1,
                    i,j+s,k+s,vc7,nc7,
                    i+s,j+s,k,vc4,nc4,
                    buf);

         extractiso(iso,
                    i,j,k+s,vc5,nc5,
                    i,j,k,vc1,nc1,
                    i+s,j,k+s,vc6,nc6,
                    i,j+s,k+s,vc7,nc7,
                    buf);

         extractiso(iso,
                    i+s,j+s,k+s,vc8,nc8,
                    i+s,j+s,k,vc4,nc4,
                    i,j+s,k+s,vc7,nc7,
                    i+s,j,k+s,vc6,nc6,
                    buf);
         }
   else
      for (n=0; n<SPECTRUM.getnum(); n++)
         {
         iso=SPECTRUM.getval(n);
         buf=SPECTRUM.getwritebuf(n);

         if (iso<minval || iso>maxval) continue;

         extractiso(iso,
                    i+s,j,k,vc2,nc2,
                    i,j+s,k,vc3,nc3,
                    i+s,j+s,k+s,vc8,nc8,
                    i,j,k+s,vc5,nc5,
                    buf);

         extractiso(iso,
                    i,j,k,vc1,nc1,
                    i+s,j,k,vc2,nc2,
                    i,j,k+s,vc5,nc5,
                    i,j+s,k,vc3,nc3,
                    buf);

         extractiso(iso,
                    i+s,j+s,k,vc4,nc4,
                    i+s,j,k,vc2,nc2,
                    i,j+s,k,vc3,nc3,
                    i+s,j+s,k+s,vc8,nc8,
                    buf);

         extractiso(iso,
                    i+s,j,k+s,vc6,nc6,
                    i+s,j,k,vc2,nc2,
                    i+s,j+s,k+s,vc8,nc8,
                    i,j,k+s,vc5,nc5,
                    buf);

         extractiso(iso,
                    i,j+s,k+s,vc7,nc7,
                    i,j+s,k,vc3,nc3,
                    i,j,k+s,vc5,nc5,
                    i+s,j+s,k+s,vc8,nc8,
                    buf);
         }
   }

// marching tetrahedra
void minisurf::extractiso(const float isoval,
                          const int x1,const int y1,const int z1,const float c1,const mininorm &n1,
                          const int x2,const int y2,const int z2,const float c2,const mininorm &n2,
                          const int x3,const int y3,const int z3,const float c3,const mininorm &n3,
                          const int x4,const int y4,const int z4,const float c4,const mininorm &n4,
                          minivtxarray *buf)
   {
   int flag=0;

   float d1,d2,d3,d4;

   d1=c1-isoval;
   d2=c2-isoval;
   d3=c3-isoval;
   d4=c4-isoval;

   if (d1<0.0f) flag|=1;
   if (d2<0.0f) flag|=2;
   if (d3<0.0f) flag|=4;
   if (d4<0.0f) flag|=8;

   switch (flag)
      {
      // 1st case: isoval<c for one and isoval>=c for other three vertices
      // 2nd case: isoval>=c for one and isoval<c for other three vertices
      case 1:
         extractiso1(x1,y1,z1,n1,fabs(d1),
                     x2,y2,z2,n2,fabs(d2),
                     x3,y3,z3,n3,fabs(d3),
                     x4,y4,z4,n4,fabs(d4),buf); break;
      case 2+4+8:
         extractiso1(x1,y1,z1,n1,fabs(d1),
                     x2,y2,z2,n2,fabs(d2),
                     x4,y4,z4,n4,fabs(d4),
                     x3,y3,z3,n3,fabs(d3),buf); break;
      case 2:
         extractiso1(x2,y2,z2,n2,fabs(d2),
                     x1,y1,z1,n1,fabs(d1),
                     x4,y4,z4,n4,fabs(d4),
                     x3,y3,z3,n3,fabs(d3),buf); break;
      case 1+4+8:
         extractiso1(x2,y2,z2,n2,fabs(d2),
                     x1,y1,z1,n1,fabs(d1),
                     x3,y3,z3,n3,fabs(d3),
                     x4,y4,z4,n4,fabs(d4),buf); break;
      case 4:
         extractiso1(x3,y3,z3,n3,fabs(d3),
                     x1,y1,z1,n1,fabs(d1),
                     x2,y2,z2,n2,fabs(d2),
                     x4,y4,z4,n4,fabs(d4),buf); break;
      case 1+2+8:
         extractiso1(x3,y3,z3,n3,fabs(d3),
                     x1,y1,z1,n1,fabs(d1),
                     x4,y4,z4,n4,fabs(d4),
                     x2,y2,z2,n2,fabs(d2),buf); break;
      case 8:
         extractiso1(x4,y4,z4,n4,fabs(d4),
                     x1,y1,z1,n1,fabs(d1),
                     x3,y3,z3,n3,fabs(d3),
                     x2,y2,z2,n2,fabs(d2),buf); break;
      case 1+2+4:
         extractiso1(x4,y4,z4,n4,fabs(d4),
                     x1,y1,z1,n1,fabs(d1),
                     x2,y2,z2,n2,fabs(d2),
                     x3,y3,z3,n3,fabs(d3),buf); break;

      // 1st case: isoval<c for two and isoval>=c for other two vertices
      // 2nd case: isoval>=c for two and isoval<c for other two vertices
      case 1+2:
         extractiso2(x1,y1,z1,n1,fabs(d1),
                     x2,y2,z2,n2,fabs(d2),
                     x3,y3,z3,n3,fabs(d3),
                     x4,y4,z4,n4,fabs(d4),buf); break;
      case 4+8:
         extractiso2(x1,y1,z1,n1,fabs(d1),
                     x2,y2,z2,n2,fabs(d2),
                     x4,y4,z4,n4,fabs(d4),
                     x3,y3,z3,n3,fabs(d3),buf); break;
      case 1+4:
         extractiso2(x1,y1,z1,n1,fabs(d1),
                     x3,y3,z3,n3,fabs(d3),
                     x4,y4,z4,n4,fabs(d4),
                     x2,y2,z2,n2,fabs(d2),buf); break;
      case 2+8:
         extractiso2(x1,y1,z1,n1,fabs(d1),
                     x3,y3,z3,n3,fabs(d3),
                     x2,y2,z2,n2,fabs(d2),
                     x4,y4,z4,n4,fabs(d4),buf); break;
      case 2+4:
         extractiso2(x2,y2,z2,n2,fabs(d2),
                     x3,y3,z3,n3,fabs(d3),
                     x1,y1,z1,n1,fabs(d1),
                     x4,y4,z4,n4,fabs(d4),buf); break;
      case 1+8:
         extractiso2(x2,y2,z2,n2,fabs(d2),
                     x3,y3,z3,n3,fabs(d3),
                     x4,y4,z4,n4,fabs(d4),
                     x1,y1,z1,n1,fabs(d1),buf); break;
      }
   }

// marching tetrahedra subcase #1
void minisurf::extractiso1(const int x1,const int y1,const int z1,const mininorm &n1,const float d1,
                           const int x2,const int y2,const int z2,const mininorm &n2,const float d2,
                           const int x3,const int y3,const int z3,const mininorm &n3,const float d3,
                           const int x4,const int y4,const int z4,const mininorm &n4,const float d4,
                           minivtxarray *buf)
   {
   float f1,f2,f3;

   float px1,py1,pz1,
         px2,py2,pz2,
         px3,py3,pz3;

   mininorm pn1,pn2,pn3;

   f1=1.0f/(d1+d2);
   f2=1.0f/(d1+d3);
   f3=1.0f/(d1+d4);

   px1=f1*(d2*x1+d1*x2);
   py1=f1*(d2*y1+d1*y2);
   pz1=f1*(d2*z1+d1*z2);
   pn1=f1*(d2*n1+d1*n2);

   px2=f2*(d3*x1+d1*x3);
   py2=f2*(d3*y1+d1*y3);
   pz2=f2*(d3*z1+d1*z3);
   pn2=f2*(d3*n1+d1*n3);

   px3=f3*(d4*x1+d1*x4);
   py3=f3*(d4*y1+d1*y4);
   pz3=f3*(d4*z1+d1*z4);
   pn3=f3*(d4*n1+d1*n4);

   buf->addvtx(px1,py1,pz1,pn1.x,pn1.y,pn1.z);
   buf->addvtx(px2,py2,pz2,pn2.x,pn2.y,pn2.z);
   buf->addvtx(px3,py3,pz3,pn3.x,pn3.y,pn3.z);
   }

// marching tetrahedra subcase #2
void minisurf::extractiso2(const int x1,const int y1,const int z1,const mininorm &n1,const float d1,
                           const int x2,const int y2,const int z2,const mininorm &n2,const float d2,
                           const int x3,const int y3,const int z3,const mininorm &n3,const float d3,
                           const int x4,const int y4,const int z4,const mininorm &n4,const float d4,
                           minivtxarray *buf)
   {
   float f1,f2,f3,f4;

   float px1,py1,pz1,
         px2,py2,pz2,
         px3,py3,pz3,
         px4,py4,pz4;

   mininorm pn1,pn2,pn3,pn4;

   f1=1.0f/(d1+d3);
   f2=1.0f/(d1+d4);
   f3=1.0f/(d2+d3);
   f4=1.0f/(d2+d4);

   px1=f1*(d3*x1+d1*x3);
   py1=f1*(d3*y1+d1*y3);
   pz1=f1*(d3*z1+d1*z3);
   pn1=f1*(d3*n1+d1*n3);

   px2=f2*(d4*x1+d1*x4);
   py2=f2*(d4*y1+d1*y4);
   pz2=f2*(d4*z1+d1*z4);
   pn2=f2*(d4*n1+d1*n4);

   px3=f3*(d3*x2+d2*x3);
   py3=f3*(d3*y2+d2*y3);
   pz3=f3*(d3*z2+d2*z3);
   pn3=f3*(d3*n2+d2*n3);

   px4=f4*(d4*x2+d2*x4);
   py4=f4*(d4*y2+d2*y4);
   pz4=f4*(d4*z2+d2*z4);
   pn4=f4*(d4*n2+d2*n4);

   buf->addvtx(px1,py1,pz1,pn1.x,pn1.y,pn1.z);
   buf->addvtx(px2,py2,pz2,pn2.x,pn2.y,pn2.z);
   buf->addvtx(px4,py4,pz4,pn4.x,pn4.y,pn4.z);

   buf->addvtx(px1,py1,pz1,pn1.x,pn1.y,pn1.z);
   buf->addvtx(px4,py4,pz4,pn4.x,pn4.y,pn4.z);
   buf->addvtx(px3,py3,pz3,pn3.x,pn3.y,pn3.z);
   }

// Windows OpenGL extension setup
void minisurf::initwglprocs()
   {
#ifndef NOOGL

#ifdef _WIN32

   if (WGLSETUP==0)
      {
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

// brick volume:

// default constructor
minibrick::minibrick(int cols,int rows)
   {
   int i;

   COLS=cols;
   ROWS=rows;

   BRICKS=new databuf[COLS*ROWS];
   MINMAX=new databuf[COLS*ROWS];
   NORMAL=new databuf[COLS*ROWS];

   MINVAL=new float[COLS*ROWS];
   MAXVAL=new float[COLS*ROWS];

   MAXLOD=new int[COLS*ROWS];
   for (i=0; i<COLS*ROWS; i++) MAXLOD[i]=-2;

   LOD=new int[COLS*ROWS];
   for (i=0; i<COLS*ROWS; i++) LOD[i]=-1;

   SURFACE=new minisurf *[COLS*ROWS];
   for (i=0; i<COLS*ROWS; i++) SURFACE[i]=new minisurf;

   LRU=new int[COLS*ROWS];
   for (i=0; i<COLS*ROWS; i++) LRU[i]=0;

   FRAME=0;

   START_CALLBACK=NULL;

   ISRUNNING=0;
   ISREADY=1;

   UPDATED_SPECT=0;
   UPDATED_COORD=0;
   UPDATED_LOD=1;

   DX1=DY1=DZ1=0.0f;
   DX2=DY2=DZ2=0.0f;
   DX3=DY3=DZ3=0.0f;

   for (i=0; i<6; i++) CLIP_ON[i]=0;

   OFF=0.0f;
   DIST=0.0f;

   FREEZE=0;

   RAY=new miniray();

   CONFIGURE_REVERSE=1;
   CONFIGURE_SEMIOPEN=0;
   CONFIGURE_LOMEM=1;

   CONFIGURE_RENDERPASSES=4;
   CONFIGURE_AMBIENT=0.1f;
   CONFIGURE_BRDRCTRL=1.0f;
   CONFIGURE_CNTRCTRL=1.0f;
   CONFIGURE_COLRCTRL=1.0f;
   CONFIGURE_BRDRCTRL2=1.0f;
   CONFIGURE_CNTRCTRL2=1.0f;
   CONFIGURE_COLRCTRL2=1.0f;

   CONFIGURE_STRIPEWIDTH=1.0f;
   CONFIGURE_STRIPEDX=0.0f;
   CONFIGURE_STRIPEDY=0.0f;
   CONFIGURE_STRIPEDZ=1.0f;
   CONFIGURE_STRIPEOFFSET=0.0f;

   CONFIGURE_CORRECTZ=0;
   }

// destructor
minibrick::~minibrick()
   {
   int i;

   stopthread();

   for (i=0; i<COLS*ROWS; i++)
      {
      if (!BRICKS[i].missing()) BRICKS[i].release();
      if (!MINMAX[i].missing()) MINMAX[i].release();
      if (!NORMAL[i].missing()) NORMAL[i].release();
      }

   delete[] BRICKS;
   delete[] MINMAX;
   delete[] NORMAL;

   delete[] MINVAL;
   delete[] MAXVAL;

   delete[] MAXLOD;
   delete[] LOD;

   for (i=0; i<COLS*ROWS; i++) delete SURFACE[i];
   delete[] SURFACE;

   delete[] LRU;

   delete RAY;
   }

// set paging callbacks
void minibrick::setloader(int (*isavailable)(int col,int row,int lod,void *data),void *data,
                          void (*loadvolume)(int col,int row,int lod,databuf *volume,void *data),
                          float offsetlat,float offsetlon,
                          float scalex,float scaley,float scaleelev,
                          int paging,
                          float safety,
                          int expire)
   {
   AVAILABLE_CALLBACK=isavailable;
   LOAD_CALLBACK=loadvolume;
   LOAD_DATA=data;

   OFFSETLAT=offsetlat;
   OFFSETLON=offsetlon;

   SCALEX=scalex;
   SCALEY=scaley;
   SCALEELEV=scaleelev;

   PAGING=paging;
   SAFETY=fmax(safety,1.0f);
   EXPIRE=expire;
   }

// set the brick orientation
void minibrick::setorientation(float dx1,float dy1,float dz1,
                               float dx2,float dy2,float dz2,
                               float dx3,float dy3,float dz3)
   {
   if (COLS==1 && ROWS==1)
      {
      DX1=dx1;
      DY1=dy1;
      DZ1=dz1;

      DX2=dx2;
      DY2=dy2;
      DZ2=dz2;

      DX3=dx3;
      DY3=dy3;
      DZ3=dz3;
      }
   }

// reset the coordinate system
void minibrick::resetcoords(float offsetlat,float offsetlon,
                            float scalex,float scaley,float scaleelev)
   {
   OFFSETLAT=offsetlat;
   OFFSETLON=offsetlon;

   SCALEX=scalex;
   SCALEY=scaley;
   SCALEELEV=scaleelev;

   UPDATED_COORD=1;
   }

// reset the brick position
void minibrick::resetpos(float midx,float midy,float basez,
                         float dx,float dy,float dz,
                         int col,int row)
   {
   databuf *ptr;

   ptr=&BRICKS[col+row*COLS];

   ptr->swx=midx-dx/2.0f;
   ptr->swy=midy-dy/2.0f;
   ptr->nwx=midx-dx/2.0f;
   ptr->nwy=midy+dy/2.0f;
   ptr->nex=midx+dx/2.0f;
   ptr->ney=midy+dy/2.0f;
   ptr->sex=midx+dx/2.0f;
   ptr->sey=midy-dy/2.0f;
   ptr->h0=basez;
   ptr->dh=dz;

   UPDATED_COORD=1;
   }

// set optional working thread
void minibrick::setthread(void (*start)(void *(*thread)(void *brick),void *brick,void *data),void *data,
                          void (*join)(void *data),
                          void (*lock_cs1)(void *data),
                          void (*unlock_cs1)(void *data),
                          void (*lock_cs2)(void *data),
                          void (*unlock_cs2)(void *data))
   {
   if (start==NULL || join==NULL ||
       lock_cs1==NULL || unlock_cs1==NULL ||
       lock_cs2==NULL || unlock_cs2==NULL) ERRORMSG();

   stopthread();

   START_CALLBACK=start;
   JOIN_CALLBACK=join;
   LOCK1_CALLBACK=lock_cs1;
   UNLOCK1_CALLBACK=unlock_cs1;
   LOCK2_CALLBACK=lock_cs2;
   UNLOCK2_CALLBACK=unlock_cs2;
   START_DATA=data;
   }

// stop working thread
void minibrick::stopthread()
   {
   if (ISRUNNING!=0)
      {
      JOIN_CALLBACK(START_DATA);

      START_CALLBACK=NULL;

      ISRUNNING=0;
      ISREADY=1;
      }
   }

// add iso value
void minibrick::addiso(float iso,float R,float G,float B,float A)
   {
   int i,j;

   // lock critical section
   if (ISRUNNING!=0) LOCK2_CALLBACK(START_DATA);

   // update iso spectrum
   for (i=0; i<COLS; i++)
      for (j=0; j<ROWS; j++) SURFACE[i+j*COLS]->getspectrum()->addiso(iso,R,G,B,A);

   // remember modification
   UPDATED_SPECT=1;

   // unlock critical section
   if (ISRUNNING!=0) UNLOCK2_CALLBACK(START_DATA);
   }

// change iso value
void minibrick::changeiso(float iso,float iso2)
   {
   int i,j;

   // update iso spectrum
   for (i=0; i<COLS; i++)
      for (j=0; j<ROWS; j++) SURFACE[i+j*COLS]->getspectrum()->changeiso(iso,iso2);

   // remember modification
   UPDATED_SPECT=1;
   }

// change iso color
void minibrick::changeiso(float iso,float R,float G,float B,float A)
   {
   int i,j;

   // change iso spectrum
   for (i=0; i<COLS; i++)
      for (j=0; j<ROWS; j++) SURFACE[i+j*COLS]->getspectrum()->changeiso(iso,R,G,B,A);
   }

// delete iso value
void minibrick::deliso(float iso)
   {
   int i,j;

   // lock critical section
   if (ISRUNNING!=0) LOCK2_CALLBACK(START_DATA);

   // update iso spectrum
   for (i=0; i<COLS; i++)
      for (j=0; j<ROWS; j++) SURFACE[i+j*COLS]->getspectrum()->deliso(iso);

   // remember modification
   UPDATED_SPECT=1;

   // unlock critical section
   if (ISRUNNING!=0) UNLOCK2_CALLBACK(START_DATA);
   }

// check iso value
int minibrick::chkiso(float iso)
   {
   int i,j;

   // check iso spectrum
   for (i=0; i<COLS; i++)
      for (j=0; j<ROWS; j++) if (SURFACE[i+j*COLS]->getspectrum()->chkiso(iso)!=0) return(1);

   return(0);
   }

// reset iso spectrum
void minibrick::resetiso()
   {
   int i,j;

   // lock critical section
   if (ISRUNNING!=0) LOCK2_CALLBACK(START_DATA);

   // update iso spectrum
   for (i=0; i<COLS; i++)
      for (j=0; j<ROWS; j++) SURFACE[i+j*COLS]->getspectrum()->reset();

   // remember modification
   UPDATED_SPECT=1;

   // unlock critical section
   if (ISRUNNING!=0) UNLOCK2_CALLBACK(START_DATA);
   }

// determine size of cullmap
void minibrick::setcullmapsize(int size)
   {
   int i,j;

   // lock critical section
   if (ISRUNNING!=0) LOCK2_CALLBACK(START_DATA);

   // update iso spectrum
   for (i=0; i<COLS; i++)
      for (j=0; j<ROWS; j++) SURFACE[i+j*COLS]->getspectrum()->setcullmapsize(size);

   // unlock critical section
   if (ISRUNNING!=0) UNLOCK2_CALLBACK(START_DATA);
   }

// add clipping plane
void minibrick::addclip(int num,float ox,float oy,float oz,float nx,float ny,float nz)
   {
   if (num<0 || num>=6) return;

   CLIP_OX[num]=ox;
   CLIP_OY[num]=oy;
   CLIP_OZ[num]=oz;

   CLIP_NX[num]=nx;
   CLIP_NY[num]=ny;
   CLIP_NZ[num]=nz;

   CLIP_ON[num]=1;
   }

// delete clipping plane
void minibrick::delclip(int num)
   {
   if (num<0 || num>=6) return;

   CLIP_ON[num]=0;
   }

// reset clipping planes
void minibrick::resetclip()
   {
   int i;

   for (i=0; i<6; i++) CLIP_ON[i]=0;
   }

// check for visibility
int minibrick::isvisible(databuf *brick,float ex,float ey,float ez,float rad,float dist,float farp,float fovy,float aspect)
   {
   float px,pz,py;
   float mindist;

   px=((brick->swx+brick->nwx+brick->nex+brick->sex)/4.0f+OFFSETLON)*SCALEX;
   py=(brick->h0+brick->dh/2.0f)*SCALEELEV;
   pz=((brick->swy+brick->nwy+brick->ney+brick->sey)/4.0f+OFFSETLAT)*SCALEY;

   if (rad>=0.0f || (rad<0.0f && dist<=0.0f)) dist=fsqrt(fsqr(px-ex)+fsqr(py-ey)+fsqr(pz-ez));

   mindist=fsqrt(fmax(fsqr((brick->nex-brick->swx)*SCALEX)+fsqr((brick->ney-brick->swy)*SCALEY),
                      fsqr((brick->sex-brick->nwx)*SCALEX)+fsqr((brick->sey-brick->nwy)*SCALEY))+
                 fsqr(brick->dh*SCALEELEV))/2.0f;

   farp*=fsqrt(1.0f+fsqr(ftan(fovy*PI/360.0f))*(1.0f+aspect*aspect));

   if (dist-mindist>farp) return(0);

   return(1);
   }

// calculate initial LOD (minimum available resolution)
int minibrick::getlod0(int col,int row)
   {
   int lod0;

   if (MAXLOD[col+row*COLS]<-1)
      {
      lod0=0;

      if (AVAILABLE_CALLBACK!=NULL && LOAD_CALLBACK!=NULL)
         while (AVAILABLE_CALLBACK(col,row,lod0,LOAD_DATA)!=0) lod0++;

      MAXLOD[col+row*COLS]=lod0-1;
      }

   return(MAXLOD[col+row*COLS]);
   }

// calculate minimum LOD (maximum necessary resolution)
int minibrick::getlod(databuf *brick,int lod,int maxlod,float ex,float ey,float ez,float rad,float off,float dist)
   {
   float px,pz,py;
   float mindist;

   int size;

   px=((brick->swx+brick->nwx+brick->nex+brick->sex)/4.0f+OFFSETLON)*SCALEX;
   py=(brick->h0+brick->dh/2.0f)*SCALEELEV;
   pz=((brick->swy+brick->nwy+brick->ney+brick->sey)/4.0f+OFFSETLAT)*SCALEY;

   mindist=fsqrt(fmax(fsqr((brick->nex-brick->swx)*SCALEX)+fsqr((brick->ney-brick->swy)*SCALEY),
                      fsqr((brick->sex-brick->nwx)*SCALEX)+fsqr((brick->sey-brick->nwy)*SCALEY))+
                 fsqr(brick->dh*SCALEELEV))/2.0f;

   if (rad>0.0f) dist=fsqrt(fsqr(px-ex)+fsqr(py-ey)+fsqr(pz-ez))+off-mindist;
   else if (rad<0.0f)
      {
      rad=-rad;
      if (dist<=0.0f) dist=fsqrt(fsqr(px-ex)+fsqr(py-ey)+fsqr(pz-ez));
      }
   else dist=0.0f;

   size=min(brick->xsize,min(brick->ysize,brick->zsize));

   while (lod-->0) size=2*size-1;

   for (lod=0; size>3 && dist>2.0f*rad*SAFETY && lod<maxlod; size=size/2+1,rad*=2.0f) lod++;

   return(lod);
   }

// page brick tiles in and out
void minibrick::pagedata(float ex,float ey,float ez,
                         float rad,float off,float dist,float farp,
                         float fovy,float aspect,
                         float t)
   {
   int i,j,k;

   int lod;

   databuf brick,minmax,normal;
   float minval,maxval;
   minisurf *surface;

   // page in and out
   for (i=0; i<COLS; i++)
      for (j=0; j<ROWS; j++)
         {
         lod=getlod0(i,j);

         if (lod>=0)
            if (PAGING==0) lod=0;
            else if (LOD[i+j*COLS]>=0) lod=getlod(&BRICKS[i+j*COLS],LOD[i+j*COLS],MAXLOD[i+j*COLS],ex,ey,ez,rad,off,dist);
            else lod=MAXLOD[i+j*COLS];

         if (lod!=LOD[i+j*COLS])
            {
            if (PAGING!=0)
               if (LOD[i+j*COLS]>=0)
                  if (isvisible(&BRICKS[i+j*COLS],ex,ey,ez,rad,dist,farp*SAFETY,fovy,aspect)==0) continue;

            if (ISRUNNING==0)
               {
               if (!BRICKS[i+j*COLS].missing())
                  {
                  BRICKS[i+j*COLS].release();
                  MINMAX[i+j*COLS].release();
                  NORMAL[i+j*COLS].release();
                  }

               LOAD_CALLBACK(i,j,lod,&BRICKS[i+j*COLS],LOAD_DATA);
               checkdata(&BRICKS[i+j*COLS],&MINMAX[i+j*COLS],&NORMAL[i+j*COLS],&MINVAL[i+j*COLS],&MAXVAL[i+j*COLS],SURFACE[i+j*COLS]);
               }
            else
               {
               // reset new data
               brick.reset();
               normal.reset();
               minmax.reset();

               // create new surface
               surface=new minisurf;

               // load new data
               LOAD_CALLBACK(i,j,lod,&brick,LOAD_DATA);
               checkdata(&brick,&minmax,&normal,&minval,&maxval,surface);

               // initialize spectrum of new surface
               for (k=0; k<SURFACE[i+j*COLS]->getspectrum()->getnum(); k++)
                  surface->getspectrum()->addiso(SURFACE[i+j*COLS]->getspectrum()->getval(k),
                                                 SURFACE[i+j*COLS]->getspectrum()->getR(k),
                                                 SURFACE[i+j*COLS]->getspectrum()->getG(k),
                                                 SURFACE[i+j*COLS]->getspectrum()->getB(k),
                                                 SURFACE[i+j*COLS]->getspectrum()->getA(k));

               // configure new surface
               surface->getspectrum()->setcullmapsize(SURFACE[i+j*COLS]->getspectrum()->getcullmapsize());

               // initialize active surface
               updatedata(lod,ex,ey,ez,rad,off,dist,t,
                          &brick,&minmax,&normal,
                          &minval,&maxval,
                          surface);

               // enable inactive surface
               surface->getspectrum()->swapwritebuf();

               // lock critical section
               LOCK1_CALLBACK(START_DATA);

               // release old data
               if (!BRICKS[i+j*COLS].missing())
                  {
                  BRICKS[i+j*COLS].release();
                  MINMAX[i+j*COLS].release();
                  NORMAL[i+j*COLS].release();
                  }

               // copy new data
               BRICKS[i+j*COLS]=brick;
               MINMAX[i+j*COLS]=minmax;
               NORMAL[i+j*COLS]=normal;

               // store new range of values
               MINVAL[i+j*COLS]=minval;
               MAXVAL[i+j*COLS]=maxval;

               // delete old surface
               delete SURFACE[i+j*COLS];

               // copy new surface
               SURFACE[i+j*COLS]=surface;

               // unlock critical section
               UNLOCK1_CALLBACK(START_DATA);
               }

            // update LOD info
            LOD[i+j*COLS]=lod;
            LRU[i+j*COLS]=FRAME;
            }

         if (PAGING!=0 && EXPIRE>0)
            if (FRAME-LRU[i+j*COLS]>EXPIRE)
               {
               // lock critical section
               if (ISRUNNING!=0) LOCK1_CALLBACK(START_DATA);

               // expire
               BRICKS[i+j*COLS].release();
               MINMAX[i+j*COLS].release();
               NORMAL[i+j*COLS].release();

               // update LOD info
               LOD[i+j*COLS]=-1;

               // shrink vertex buffers to minimum
               SURFACE[i+j*COLS]->getspectrum()->shrinkbufs();

               // unlock critical section
               if (ISRUNNING!=0) UNLOCK1_CALLBACK(START_DATA);
               }
         }
   }

// check the brick tile size to be 2^n+1 and initialize brick tiles
void minibrick::checkdata(databuf *brick,databuf *minmax,databuf *normal,
                          float *minval,float *maxval,
                          minisurf *surface)
   {
   unsigned int xs,ys,zs;

   if (brick->missing()) ERRORMSG();

   if (brick->xsize<2 || brick->ysize<2 || brick->zsize<2 || brick->tsteps<1) ERRORMSG();

   // always convert to float
   brick->convertdata(databuf::DATABUF_TYPE_FLOAT);

   for (xs=3; xs<brick->xsize; xs=2*xs-1);
   for (ys=3; ys<brick->ysize; ys=2*ys-1);
   for (zs=3; zs<brick->zsize; zs=2*zs-1);

   // resample to 2^n+1 (n>=1)
   if (xs!=brick->xsize || ys!=brick->ysize || zs!=brick->zsize) brick->resampledata(xs,ys,zs);

   if (!minmax->missing()) minmax->release();
   if (!normal->missing()) normal->release();

   LOMEM=CONFIGURE_LOMEM;

   if (LOMEM==0) minmax->alloc(brick->xsize,brick->ysize,brick->zsize,brick->tsteps,1);
   else minmax->alloc(brick->xsize,brick->ysize,brick->zsize,brick->tsteps,0);

   normal->alloc(brick->xsize,brick->ysize,brick->zsize,brick->tsteps,3);

   initdata(brick,minmax,normal,minval,maxval,surface);
   }

// initialize a brick tile
void minibrick::initdata(databuf *brick,databuf *minmax,databuf *normal,
                         float *minval,float *maxval,
                         minisurf *surface)
   {
   unsigned int i,j,k,t;

   float *data; // reference corner

   float vc1,vc2,vc3,vc4,vc5,vc6,vc7,vc8; // scalar values of corners

   float minv,maxv; // range of scalar values

   float extx,exty,extz,maxext; // extent of a cell
   float celx,cely,celz; // relative size of a cell

   int cdo1x,cdo1y,cdo1z,cdo2x,cdo2y,cdo2z; // precalculated offsets
   float cdfx,cdfy,cdfz; // precalculated factors

   float cdx,cdy,cdz; // central differences
   float cdnorm; // norm of central differences

   DATA=(float *)brick->data;

   if (LOMEM==0) MM=(unsigned short int *)minmax->data;
   else MM2=(unsigned char *)minmax->data;

   NRML=(unsigned char *)normal->data;

   XSIZE=brick->xsize;
   YSIZE=brick->ysize;
   ZSIZE=brick->zsize;

   MINSIZE=XSIZE;
   if (YSIZE<MINSIZE) MINSIZE=YSIZE;
   if (ZSIZE<MINSIZE) MINSIZE=ZSIZE;

   MINV=MAXV=DATA[0];

   // calculate minimum and maximum scalar values of the tile
   for (t=0; t<brick->tsteps; t++)
      for (i=0; i<XSIZE; i++)
         for (j=0; j<YSIZE; j++)
            for (k=0; k<ZSIZE; k++)
               {
               data=&DATA[i+(j+(k+t*ZSIZE)*YSIZE)*XSIZE];

               if (*data<MINV) MINV=*data;
               else if (*data>MAXV) MAXV=*data;
               }

   // check for singular value
   if (MAXV==MINV) MAXV++;

   // calculate minimum and maximum scalar values for each block
   for (t=0; t<brick->tsteps; t++)
      for (i=0; i<XSIZE-1; i+=MINSIZE-1)
         for (j=0; j<YSIZE-1; j+=MINSIZE-1)
            for (k=0; k<ZSIZE-1; k+=MINSIZE-1)
               {
               data=&DATA[i+(j+(k+t*ZSIZE)*YSIZE)*XSIZE];

               vc1=data[0];
               vc2=data[MINSIZE-1];
               vc3=data[(MINSIZE-1)*XSIZE];
               vc4=data[MINSIZE-1+(MINSIZE-1)*XSIZE];
               vc5=data[(MINSIZE-1)*YSIZE*XSIZE];
               vc6=data[MINSIZE-1+(MINSIZE-1)*YSIZE*XSIZE];
               vc7=data[(MINSIZE-1+(MINSIZE-1)*YSIZE)*XSIZE];
               vc8=data[MINSIZE-1+(MINSIZE-1+(MINSIZE-1)*YSIZE)*XSIZE];

               initblock(i,j,k,MINSIZE-1,t,
                         vc1,vc2,vc3,vc4,vc5,vc6,vc7,vc8,
                         &minv,&maxv);
               }

   // store range of scalar values
   *minval=MINV;
   *maxval=MAXV;

   // pass range of scalar values to iso spectrum
   surface->getspectrum()->setrange(MINV,MAXV);

   // calculate extent of a cell
   extx=fsqrt(fsqr((brick->sex-brick->swx)*SCALEX)+fsqr((brick->sey-brick->swy)*SCALEY))/(brick->xsize-1);
   exty=fsqrt(fsqr((brick->nwx-brick->swx)*SCALEX)+fsqr((brick->nwy-brick->swy)*SCALEY))/(brick->ysize-1);
   extz=brick->dh*SCALEELEV/(brick->zsize-1);

   // check for zero extent
   if (extx==0.0f) extx=1.0f;
   if (exty==0.0f) exty=1.0f;
   if (extz==0.0f) extz=1.0f;

   // calculate relative size of a cell
   maxext=fmax(extx,fmax(exty,extz));
   celx=extx/maxext;
   cely=exty/maxext;
   celz=extz/maxext;

   // calculate normals
   for (t=0; t<brick->tsteps; t++)
      for (i=0; i<XSIZE; i++)
         for (j=0; j<YSIZE; j++)
            for (k=0; k<ZSIZE; k++)
               {
               cdo1x=1;
               cdo1y=XSIZE;
               cdo1z=XSIZE*YSIZE;

               cdo2x=1;
               cdo2y=XSIZE;
               cdo2z=XSIZE*YSIZE;

               cdfx=cdfy=cdfz=0.5f;

               if (i==0) {cdo1x=0; cdfx=1.0f;}
               else if (i==XSIZE-1) {cdo2x=0; cdfx=1.0f;}

               if (j==0) {cdo1y=0; cdfy=1.0f;}
               else if (j==YSIZE-1) {cdo2y=0; cdfy=1.0f;}

               if (k==0) {cdo1z=0; cdfz=1.0f;}
               else if (k==ZSIZE-1) {cdo2z=0; cdfz=1.0f;}

               data=&DATA[i+(j+(k+t*ZSIZE)*YSIZE)*XSIZE];

               cdx=cdfx*(data[cdo2x]-data[-cdo1x])/celx;
               cdy=cdfy*(data[cdo2y]-data[-cdo1y])/cely;
               cdz=cdfz*(data[cdo2z]-data[-cdo1z])/celz;

               cdnorm=fsqrt(cdx*cdx+cdy*cdy+cdz*cdz);

               // normalize
               if (cdnorm>0.0f)
                  {
                  cdx*=celx/cdnorm;
                  cdy*=cely/cdnorm;
                  cdz*=celz/cdnorm;
                  }

               // reverse normals
               if (CONFIGURE_REVERSE!=0)
                  {
                  cdx=-cdx;
                  cdy=-cdy;
                  cdz=-cdz;
                  }

               cdx=(cdx+1.0f)/2.0f;
               cdy=(cdy+1.0f)/2.0f;
               cdz=(cdz+1.0f)/2.0f;

               NRML[3*(i+(j+(k+t*ZSIZE)*YSIZE)*XSIZE)]=ftrc(255.0f*cdx+0.5f);
               NRML[3*(i+(j+(k+t*ZSIZE)*YSIZE)*XSIZE)+1]=ftrc(255.0f*cdy+0.5f);
               NRML[3*(i+(j+(k+t*ZSIZE)*YSIZE)*XSIZE)+2]=ftrc(255.0f*cdz+0.5f);
               }
   }

// initialize a block of the brick volume
void minibrick::initblock(const unsigned int i,const unsigned int j,const unsigned int k,const unsigned int s,const unsigned int t,
                          const float vc1,const float vc2,const float vc3,const float vc4,const float vc5,const float vc6,const float vc7,const float vc8,
                          float *minval,float *maxval)
   {
   float minv,maxv;

   float *data; // reference corner for scalar values
   unsigned short int *mm; // reference corner for minmax values
   unsigned char *mm2; // reference corner for lomem minmax values

   unsigned int s2; // half edge length
   unsigned int sxs,s2xs; // precalculated offsets

   float vm; // midpoint of block
   float vf1,vf2,vf3,vf4,vf5,vf6; // midpoints of faces
   float ve1,ve2,ve3,ve4,ve5,ve6,ve7,ve8,ve9,ve10,ve11,ve12; // midpoints of edges

   if (s>=2)
      {
      s2=s/2;

      sxs=s*XSIZE;
      s2xs=s2*XSIZE;

      // bottom layer of block:

      data=&DATA[i+(j+(k+t*ZSIZE)*YSIZE)*XSIZE];

      ve9=data[s2];
      ve5=data[s2xs];
      vf5=data[s2+s2xs];
      ve6=data[s+s2xs];
      ve10=data[s2+sxs];

      // top layer of block:

      data=&DATA[i+(j+(k+s+t*ZSIZE)*YSIZE)*XSIZE];

      ve11=data[s2];
      ve7=data[s2xs];
      vf6=data[s2+s2xs];
      ve8=data[s+s2xs];
      ve12=data[s2+sxs];

      // middle layer of block:

      data=&DATA[i+(j+(k+s2+t*ZSIZE)*YSIZE)*XSIZE];

      ve1=data[0];
      vf3=data[s2];
      ve2=data[s];
      vf1=data[s2xs];
      vm=data[s2+s2xs];
      vf2=data[s+s2xs];
      ve3=data[sxs];
      vf4=data[s2+sxs];
      ve4=data[s+sxs];

      // call recursively:

      initblock(i,j,k,s2,t,vc1,ve9,ve5,vf5,ve1,vf3,vf1,vm,&minv,&maxv);

      *minval=minv;
      *maxval=maxv;

      initblock(i+s2,j,k,s2,t,ve9,vc2,vf5,ve6,vf3,ve2,vm,vf2,&minv,&maxv);

      if (minv<*minval) *minval=minv;
      if (maxv>*maxval) *maxval=maxv;

      initblock(i,j+s2,k,s2,t,ve5,vf5,vc3,ve10,vf1,vm,ve3,vf4,&minv,&maxv);

      if (minv<*minval) *minval=minv;
      if (maxv>*maxval) *maxval=maxv;

      initblock(i+s2,j+s2,k,s2,t,vf5,ve6,ve10,vc4,vm,vf2,vf4,ve4,&minv,&maxv);

      if (minv<*minval) *minval=minv;
      if (maxv>*maxval) *maxval=maxv;

      initblock(i,j,k+s2,s2,t,ve1,vf3,vf1,vm,vc5,ve11,ve7,vf6,&minv,&maxv);

      if (minv<*minval) *minval=minv;
      if (maxv>*maxval) *maxval=maxv;

      initblock(i+s2,j,k+s2,s2,t,vf3,ve2,vm,vf2,ve11,vc6,vf6,ve8,&minv,&maxv);

      if (minv<*minval) *minval=minv;
      if (maxv>*maxval) *maxval=maxv;

      initblock(i,j+s2,k+s2,s2,t,vf1,vm,ve3,vf4,ve7,vf6,vc7,ve12,&minv,&maxv);

      if (minv<*minval) *minval=minv;
      if (maxv>*maxval) *maxval=maxv;

      initblock(i+s2,j+s2,k+s2,s2,t,vm,vf2,vf4,ve4,vf6,ve8,ve12,vc8,&minv,&maxv);

      if (minv<*minval) *minval=minv;
      if (maxv>*maxval) *maxval=maxv;

      if (LOMEM==0)
         {
         mm=&MM[i+(j+(k+t*ZSIZE)*YSIZE)*XSIZE];

         mm[s2]=max(ftrc(ffloor(65535.0f*(*minval-MINV)/(MAXV-MINV))),0);
         mm[s2xs]=min(ftrc(fceil(65535.0f*(*maxval-MINV)/(MAXV-MINV))),65535);
         }
      else
         {
         mm2=&MM2[i+(j+(k+t*ZSIZE)*YSIZE)*XSIZE];

         mm2[s2]=max(ftrc(ffloor(255.0f*(*minval-MINV)/(MAXV-MINV))),0);
         mm2[s2xs]=min(ftrc(fceil(255.0f*(*maxval-MINV)/(MAXV-MINV))),255);
         }
      }
   else
      {
      minv=maxv=vc1;

      if (vc2<minv) minv=vc2;
      else if (vc2>maxv) maxv=vc2;

      if (vc3<minv) minv=vc3;
      else if (vc3>maxv) maxv=vc3;

      if (vc4<minv) minv=vc4;
      else if (vc4>maxv) maxv=vc4;

      if (vc5<minv) minv=vc5;
      else if (vc5>maxv) maxv=vc5;

      if (vc6<minv) minv=vc6;
      else if (vc6>maxv) maxv=vc6;

      if (vc7<minv) minv=vc7;
      else if (vc7>maxv) maxv=vc7;

      if (vc8<minv) minv=vc8;
      else if (vc8>maxv) maxv=vc8;

      *minval=minv;
      *maxval=maxv;
      }
   }

// extract triangle mesh from the brick volume
void minibrick::extract(float ex,float ey,float ez,
                        float rad,float farp,
                        float fovy,float aspect,
                        float t)
   {
   int i,j;

   if (farp<=0.0f) ERRORMSG();

   if (FREEZE==0)
      {
      // update vertex arrays
      if (START_CALLBACK==NULL)
         if (check4update(ex,ey,ez,rad,OFF,DIST,farp,fovy,aspect,t)!=0)
            {
            sync();

            update(ex,ey,ez,
                   rad,OFF,DIST,farp,
                   fovy,aspect,
                   t);
            }

      // start data cycle
      if (START_CALLBACK!=NULL && ISREADY!=0)
         {
         // initialize vertex arrays
         if (FRAME==0)
            {
            sync();

            update(ex,ey,ez,
                   rad,OFF,DIST,farp,
                   fovy,aspect,
                   t);
            }

         // wait for the previously started thread to finish completely
         if (ISRUNNING!=0) JOIN_CALLBACK(START_DATA);

         // memorize thread to be joined
         ISRUNNING=0;

         // check for update
         if (check4update(ex,ey,ez,rad,OFF,DIST,farp,fovy,aspect,t)!=0)
            {
            // synchronize
            sync();

            // set parameters for next cycle
            CYCLE_EX=ex;
            CYCLE_EY=ey;
            CYCLE_EZ=ez;
            CYCLE_RAD=rad;
            CYCLE_OFF=OFF;
            CYCLE_DIST=DIST;
            CYCLE_FARP=farp;
            CYCLE_FOVY=fovy;
            CYCLE_ASPECT=aspect;
            CYCLE_T=t;

            // swap write buffer
            for (i=0; i<COLS; i++)
               for (j=0; j<ROWS; j++) SURFACE[i+j*COLS]->getspectrum()->swapwritebuf();

            // memorize thread to be running
            ISRUNNING=1;

            // memorize thread to be working
            ISREADY=0;

            // start next thread
            START_CALLBACK(startcycle,this,START_DATA);
            }
         }
      }
   }

// render the brick volume
void minibrick::render(float ex,float ey,float ez,
                       float rad,float farp,
                       float fovy,float aspect,
                       float t,
                       int phase)
   {
#ifndef NOOGL

   int i,j;

   GLdouble equ[4];

   if (farp<=0.0f) ERRORMSG();

   if (phase==minisurf::ONE_RENDER_PHASE ||
       phase==minisurf::FIRST_RENDER_PHASE)
      {
      extract(ex,ey,ez,
              rad,farp,
              fovy,aspect,
              t);

      // lock critical OpenGL section
      if (ISRUNNING!=0) LOCK1_CALLBACK(START_DATA);
      }

   initstate();

   glPushMatrix();

   // rotate in world space
   if (FREEZE!=0)
      if (DX1!=0.0f || DY1!=0.0f || DZ1!=0.0f ||
          DX2!=0.0f || DY2!=0.0f || DZ2!=0.0f ||
          DX3!=0.0f || DY3!=0.0f || DZ3!=0.0f)
         {
         GLfloat mtx[16]={DX1,DY1,DZ1,0.0f,
                          DX2,DY2,DZ2,0.0f,
                          -DX3,-DY3,-DZ3,0.0f,
                          0.0f,0.0f,0.0f,1.0f};

         glTranslatef(((BRICKS->swx+BRICKS->nwx+BRICKS->nex+BRICKS->sex)/4.0f+OFFSETLON)*SCALEX,
                      (BRICKS->h0+BRICKS->dh/2.0f)*SCALEELEV,
                      -((BRICKS->swy+BRICKS->nwy+BRICKS->ney+BRICKS->sey)/4.0f+OFFSETLAT)*SCALEY);

         glMultMatrixf(mtx);

         glTranslatef(-((BRICKS->swx+BRICKS->nwx+BRICKS->nex+BRICKS->sex)/4.0f+OFFSETLON)*SCALEX,
                      -(BRICKS->h0+BRICKS->dh/2.0f)*SCALEELEV,
                      ((BRICKS->swy+BRICKS->nwy+BRICKS->ney+BRICKS->sey)/4.0f+OFFSETLAT)*SCALEY);
         }

   // transform from object to world space
   glScalef(SCALEX,SCALEELEV,-SCALEY);
   glTranslatef(OFFSETLON,0.0f,OFFSETLAT);

   // enable clipping planes
   for (i=0; i<6; i++)
      if (CLIP_ON[i]!=0)
         {
         equ[0]=CLIP_NX[i];
         equ[1]=CLIP_NY[i];
         equ[2]=CLIP_NZ[i];

         equ[3]=-CLIP_NX[i]*CLIP_OX[i]-CLIP_NY[i]*CLIP_OY[i]-CLIP_NZ[i]*CLIP_OZ[i];

         glClipPlane(GL_CLIP_PLANE0+i,equ);

         glEnable(GL_CLIP_PLANE0+i);
         }

   // render vertex arrays
   for (i=0; i<COLS; i++)
      for (j=0; j<ROWS; j++)
         if (MAXLOD[i+j*COLS]>=0)
            {
            if (LOD[i+j*COLS]>=0)
               if (isvisible(&BRICKS[i+j*COLS],ex,ey,ez,rad,DIST,farp,fovy,aspect)==0)
                  if (isvisible(&BRICKS[i+j*COLS],ex,ey,ez,rad,DIST,farp*SAFETY,fovy,aspect)==0) continue;
                  else
                     {
                     LRU[i+j*COLS]=FRAME;
                     continue;
                     }

            drawdata(&BRICKS[i+j*COLS],SURFACE[i+j*COLS],phase);
            LRU[i+j*COLS]=FRAME;
            }

   // disable clipping planes
   for (i=0; i<6; i++)
      if (CLIP_ON[i]!=0) glDisable(GL_CLIP_PLANE0+i);

   glPopMatrix();

   exitstate();

   if (phase==minisurf::ONE_RENDER_PHASE ||
       phase==minisurf::LAST_RENDER_PHASE)
      {
      // unlock critical OpenGL section
      if (ISRUNNING!=0) UNLOCK1_CALLBACK(START_DATA);

      FRAME++;
      }

#endif
   }

// release all buffers
void minibrick::release()
   {
   int i;

   stopthread();

   freeze();

   for (i=0; i<COLS*ROWS; i++)
      {
      if (!BRICKS[i].missing()) BRICKS[i].release();
      if (!MINMAX[i].missing()) MINMAX[i].release();
      if (!NORMAL[i].missing()) NORMAL[i].release();

      LOD[i]=-1;
      }

   UPDATED_LOD=1;
   }

// start update cycle in new thread created by application
void *minibrick::startcycle(void *brick)
   {
   ((minibrick *)brick)->cycle();
   return(NULL);
   }

// the update cycle
void minibrick::cycle()
   {
   int i,j;

   // lock critical section while data is being updated
   LOCK2_CALLBACK(START_DATA);

   // update vertex arrays
   update(CYCLE_EX,CYCLE_EY,CYCLE_EZ,
          CYCLE_RAD,CYCLE_OFF,CYCLE_DIST,CYCLE_FARP,
          CYCLE_FOVY,CYCLE_ASPECT,
          CYCLE_T);

   // unlock critical section after data has been updated
   UNLOCK2_CALLBACK(START_DATA);

   // lock critical section
   LOCK1_CALLBACK(START_DATA);

   // swap read buffer
   for (i=0; i<COLS; i++)
      for (j=0; j<ROWS; j++) SURFACE[i+j*COLS]->getspectrum()->swapreadbuf();

   // memorize thread to be ready
   ISREADY=1;

   // unlock critical section
   UNLOCK1_CALLBACK(START_DATA);
   }

// check for update
int minibrick::check4update(float ex,float ey,float ez,
                            float rad,float off,float dist,float farp,
                            float fovy,float aspect,
                            float t)
   {
   if (// viewing parameters did not change
       farp==LAST_FARP &&
       fovy==LAST_FOVY &&
       aspect==LAST_ASPECT &&
       // resolution did not change
       ((rad==0.0f &&
         rad==LAST_RAD) ||
        // view point did not change
        (rad>0.0f &&
         rad==LAST_RAD &&
         ex==LAST_EX && ey==LAST_EY && ez==LAST_EZ &&
         off==LAST_OFF) ||
        // viewing distance did not change
        (rad<0.0f &&
         rad==LAST_RAD &&
         ex==LAST_EX && ey==LAST_EY && ez==LAST_EZ &&
         dist==LAST_DIST)) &&
       // visualization time did not change
       t==LAST_T &&
       // iso spectrum did not change
       UPDATED_SPECT==0 &&
       // coordinate system (or brick position) did not change
       (UPDATED_COORD==0 ||
        (rad==0.0f &&
         rad==LAST_RAD)) &&
       // brick lod did not change
       UPDATED_LOD==0) return(0);

   return(1);
   }

// sync iso spectrum
void minibrick::sync()
   {
   int i,j;

   // change iso spectrum
   for (i=0; i<COLS; i++)
      for (j=0; j<ROWS; j++) SURFACE[i+j*COLS]->getspectrum()->sync();

   // clear modification
   UPDATED_SPECT=0;
   }

// update vertex arrays
void minibrick::update(float ex,float ey,float ez,
                       float rad,float off,float dist,float farp,
                       float fovy,float aspect,
                       float t)
   {
   int i,j;

   // page
   pagedata(ex,ey,ez,
            rad,off,dist,farp,
            fovy,aspect,
            t);

   // calculate
   for (i=0; i<COLS; i++)
      for (j=0; j<ROWS; j++)
         if (LOD[i+j*COLS]>=0)
            if (isvisible(&BRICKS[i+j*COLS],ex,ey,ez,rad,dist,farp*SAFETY,fovy,aspect)!=0)
               updatedata(LOD[i+j*COLS],ex,ey,ez,rad,off,dist,t,
                          &BRICKS[i+j*COLS],&MINMAX[i+j*COLS],&NORMAL[i+j*COLS],
                          &MINVAL[i+j*COLS],&MAXVAL[i+j*COLS],
                          SURFACE[i+j*COLS]);

   // remember parameters of last cycle
   LAST_EX=ex;
   LAST_EY=ey;
   LAST_EZ=ez;
   LAST_RAD=rad;
   LAST_OFF=off;
   LAST_DIST=dist;
   LAST_FARP=farp;
   LAST_FOVY=fovy;
   LAST_ASPECT=aspect;
   LAST_T=t;

   // clear update
   UPDATED_COORD=0;
   UPDATED_LOD=0;
   }

// update vertex arrays of a brick tile
void minibrick::updatedata(int lod,float ex,float ey,float ez,float rad,float off,float dist,float t,
                           databuf *brick,databuf *minmax,databuf *normal,
                           float *minval,float *maxval,
                           minisurf *surface)
   {
   unsigned int i,j,k;

   float lt;
   unsigned int ts;

   float dx,dy;

   float px,py,pz;

   float *data; // reference corner for scalar values
   unsigned char *nrml; // reference corner for normal vectors

   float vc1,vc2,vc3,vc4,vc5,vc6,vc7,vc8; // scalar values of corners
   mininorm nc1,nc2,nc3,nc4,nc5,nc6,nc7,nc8; // normal vectors of corners

   surface->reset();

   if (brick->tsteps==1 || brick->dt==0.0f) lt=0.0f;
   else lt=(t-brick->t0)/brick->dt;

   // closed time interval
   if (CONFIGURE_SEMIOPEN==0)
      if (lt<0.0f || lt>brick->tsteps-1) return;

   // semi-open time interval
   if (CONFIGURE_SEMIOPEN!=0 && brick->tsteps>1)
      if (lt<0.0f || lt>=brick->tsteps-1) return;

   ts=ftrc(lt);

   BRICK=brick;
   SURF=surface;

   DATA=&((float *)brick->data)[ts*XSIZE*YSIZE*ZSIZE];

   if (LOMEM==0) MM=&((unsigned short int *)minmax->data)[ts*XSIZE*YSIZE*ZSIZE];
   else MM2=&((unsigned char *)minmax->data)[ts*XSIZE*YSIZE*ZSIZE];

   NRML=&((unsigned char *)normal->data)[ts*3*XSIZE*YSIZE*ZSIZE];

   XSIZE=brick->xsize;
   YSIZE=brick->ysize;
   ZSIZE=brick->zsize;

   MINSIZE=XSIZE;
   if (YSIZE<MINSIZE) MINSIZE=YSIZE;
   if (ZSIZE<MINSIZE) MINSIZE=ZSIZE;

   MINV=*minval;
   MAXV=*maxval;

   if (LOMEM==0) MMRANGE=(MAXV-MINV)/65535.0f;
   else MMRANGE=(MAXV-MINV)/255.0f;

   if (rad>0.0f) // C-LOD
      {
      RES=2.0f*(rad+1.0f)*(1<<lod);
      SLOD=-MAXFLOAT;

      ex=ex/SCALEX-OFFSETLON;
      ey=ey/SCALEELEV;
      ez=ez/SCALEY-OFFSETLAT;

      dx=brick->sex-brick->swx;
      dy=brick->sey-brick->swy;

      EX=((ex-brick->swx)*dx+(ez-brick->swy)*dy)*(XSIZE-1)/(dx*dx+dy*dy);
      CX=fsqrt(dx*dx+dy*dy)*SCALEX/(XSIZE-1);

      dx=brick->nwx-brick->swx;
      dy=brick->nwy-brick->swy;

      EY=((ex-brick->swx)*dx+(ez-brick->swy)*dy)*(YSIZE-1)/(dx*dx+dy*dy);
      CY=fsqrt(dx*dx+dy*dy)*SCALEY/(YSIZE-1);

      EZ=(ey-brick->h0)*(ZSIZE-1)/brick->dh;
      CZ=brick->dh*SCALEELEV/(ZSIZE-1);

      DOFF=off;
      }
   else if (rad<0.0f) // S-LOD
      {
      if (COLS>1 || ROWS>1) ERRORMSG();

      RES=2.0f*(1.0f-rad)*(1<<lod);

      if (dist<=0.0f)
         {
         px=((brick->swx+brick->nwx+brick->nex+brick->sex)/4.0f+OFFSETLON)*SCALEX;
         py=(brick->h0+brick->dh/2.0f)*SCALEELEV;
         pz=((brick->swy+brick->nwy+brick->ney+brick->sey)/4.0f+OFFSETLAT)*SCALEY;

         dist=fsqrt(fsqr(px-ex)+fsqr(py-ey)+fsqr(pz-ez));
         }

      SLOD=fsqr(dist);
      }
   else RES=SLOD=-MAXFLOAT; // maximum resolution

   THETA=lt-ts;
   OMTHETA=1.0f-THETA;

   // traverse the octree
   for (i=0; i<XSIZE-1; i+=MINSIZE-1)
      for (j=0; j<YSIZE-1; j+=MINSIZE-1)
         for (k=0; k<ZSIZE-1; k+=MINSIZE-1)
            {
            data=&DATA[i+(j+k*YSIZE)*XSIZE];
            nrml=&NRML[3*(i+(j+k*YSIZE)*XSIZE)];

            if (THETA==0.0f)
               {
               vc1=data[0];
               vc2=data[MINSIZE-1];
               vc3=data[(MINSIZE-1)*XSIZE];
               vc4=data[MINSIZE-1+(MINSIZE-1)*XSIZE];
               vc5=data[(MINSIZE-1)*YSIZE*XSIZE];
               vc6=data[MINSIZE-1+(MINSIZE-1)*YSIZE*XSIZE];
               vc7=data[(MINSIZE-1+(MINSIZE-1)*YSIZE)*XSIZE];
               vc8=data[MINSIZE-1+(MINSIZE-1+(MINSIZE-1)*YSIZE)*XSIZE];

               nc1=mininorm(&nrml[0]);
               nc2=mininorm(&nrml[3*(MINSIZE-1)]);
               nc3=mininorm(&nrml[3*(MINSIZE-1)*XSIZE]);
               nc4=mininorm(&nrml[3*(MINSIZE-1+(MINSIZE-1)*XSIZE)]);
               nc5=mininorm(&nrml[3*(MINSIZE-1)*YSIZE*XSIZE]);
               nc6=mininorm(&nrml[3*(MINSIZE-1+(MINSIZE-1)*YSIZE*XSIZE)]);
               nc7=mininorm(&nrml[3*(MINSIZE-1+(MINSIZE-1)*YSIZE)*XSIZE]);
               nc8=mininorm(&nrml[3*(MINSIZE-1+(MINSIZE-1+(MINSIZE-1)*YSIZE)*XSIZE)]);
               }
            else
               {
               vc1=OMTHETA*data[0]+THETA*data[XSIZE*YSIZE*ZSIZE];
               vc2=OMTHETA*data[MINSIZE-1]+THETA*data[MINSIZE-1+XSIZE*YSIZE*ZSIZE];
               vc3=OMTHETA*data[(MINSIZE-1)*XSIZE]+THETA*data[(MINSIZE-1)*XSIZE+XSIZE*YSIZE*ZSIZE];
               vc4=OMTHETA*data[MINSIZE-1+(MINSIZE-1)*XSIZE]+THETA*data[MINSIZE-1+(MINSIZE-1)*XSIZE+XSIZE*YSIZE*ZSIZE];
               vc5=OMTHETA*data[(MINSIZE-1)*YSIZE*XSIZE]+THETA*data[(MINSIZE-1)*YSIZE*XSIZE+XSIZE*YSIZE*ZSIZE];
               vc6=OMTHETA*data[MINSIZE-1+(MINSIZE-1)*YSIZE*XSIZE]+THETA*data[MINSIZE-1+(MINSIZE-1)*YSIZE*XSIZE+XSIZE*YSIZE*ZSIZE];
               vc7=OMTHETA*data[(MINSIZE-1+(MINSIZE-1)*YSIZE)*XSIZE]+THETA*data[(MINSIZE-1+(MINSIZE-1)*YSIZE)*XSIZE+XSIZE*YSIZE*ZSIZE];
               vc8=OMTHETA*data[MINSIZE-1+(MINSIZE-1+(MINSIZE-1)*YSIZE)*XSIZE]+THETA*data[MINSIZE-1+(MINSIZE-1+(MINSIZE-1)*YSIZE)*XSIZE+XSIZE*YSIZE*ZSIZE];

               nc1=OMTHETA*mininorm(&nrml[0])+THETA*mininorm(&nrml[3*XSIZE*YSIZE*ZSIZE]);
               nc2=OMTHETA*mininorm(&nrml[3*(MINSIZE-1)])+THETA*mininorm(&nrml[3*(MINSIZE-1+XSIZE*YSIZE*ZSIZE)]);
               nc3=OMTHETA*mininorm(&nrml[3*(MINSIZE-1)*XSIZE])+THETA*mininorm(&nrml[3*((MINSIZE-1)*XSIZE+XSIZE*YSIZE*ZSIZE)]);
               nc4=OMTHETA*mininorm(&nrml[3*(MINSIZE-1+(MINSIZE-1)*XSIZE)])+THETA*mininorm(&nrml[3*(MINSIZE-1+(MINSIZE-1)*XSIZE+XSIZE*YSIZE*ZSIZE)]);
               nc5=OMTHETA*mininorm(&nrml[3*(MINSIZE-1)*YSIZE*XSIZE])+THETA*mininorm(&nrml[3*((MINSIZE-1)*YSIZE*XSIZE+XSIZE*YSIZE*ZSIZE)]);
               nc6=OMTHETA*mininorm(&nrml[3*(MINSIZE-1+(MINSIZE-1)*YSIZE*XSIZE)])+THETA*mininorm(&nrml[3*(MINSIZE-1+(MINSIZE-1)*YSIZE*XSIZE+XSIZE*YSIZE*ZSIZE)]);
               nc7=OMTHETA*mininorm(&nrml[3*(MINSIZE-1+(MINSIZE-1)*YSIZE*XSIZE)])+THETA*mininorm(&nrml[3*((MINSIZE-1+(MINSIZE-1)*YSIZE)*XSIZE+XSIZE*YSIZE*ZSIZE)]);
               nc8=OMTHETA*mininorm(&nrml[3*(MINSIZE-1+(MINSIZE-1+(MINSIZE-1)*YSIZE)*XSIZE)])+THETA*mininorm(&nrml[3*(MINSIZE-1+(MINSIZE-1+(MINSIZE-1)*YSIZE)*XSIZE+XSIZE*YSIZE*ZSIZE)]);
               }

            calcblock(i,j,k,MINSIZE-1,
                      vc1,vc2,vc3,vc4,vc5,vc6,vc7,vc8,
                      nc1,nc2,nc3,nc4,nc5,nc6,nc7,nc8);
            }

   // additional processing
   processarrays(brick,surface);
   }

// calculate the actual blending factor
float minibrick::bf(const unsigned int i,const unsigned int j,const unsigned int k,const unsigned int s2)
   {
   if (SLOD<0.0f) return(1.0f-fmin(fmax((fsqr((i-EX)*CX)+fsqr((j-EY)*CY)+fsqr((k-EZ)*CZ)+DOFF)/fsqr(s2*RES),0.0f),1.0f)); // C-LOD
   else if (RES>0.0f) return(1.0f-fmin(fmax(SLOD/fsqr(s2*RES),0.0f),1.0f)); // S-LOD
   else return(1.0f); // maximum resolution
   }

// calculate the actual octree
void minibrick::calcblock(const unsigned int i,const unsigned int j,const unsigned int k,const unsigned int s, // index of front left bottom block corner and block size
                          const float vc1,const float vc2,const float vc3,const float vc4,const float vc5,const float vc6,const float vc7,const float vc8, // corner values
                          const mininorm &nc1,const mininorm &nc2,const mininorm &nc3,const mininorm &nc4,const mininorm &nc5,const mininorm &nc6,const mininorm &nc7,const mininorm &nc8) // corner normals
   {
   /* layout of vertices:

       c7---e12--c8
      /    /    /|
     e7---f6---e8|
    /    /    /  |
   c5---e11--c6  |
   |         |   |
   |   e3---f4---e4
   |  /    / |  /|
   | f1---m----f2|
   |/    /   |/  |
   e1---f3---e2  |
   |         |   |
   |   c3---e10--c4
   |  /    / |  /
   | e5---f5---e6
   |/    /   |/
   c1---e9---c2

   v = scalar value
   n = normal vector */

   float b; // actual blending factor

   float *data; // reference corner for scalar values
   unsigned short int *mm; // reference corner for minmax values
   unsigned char *mm2; // reference corner for lomem minmax values
   unsigned char *nrml; // reference corner for normal vectors

   unsigned int s2; // half edge length
   unsigned int mi,mj,mk; // index of midpoint
   unsigned int s3,s23,sxs,sxs3,s2xs,s2xs3,nts,nts3; // precalculated offsets

   // scalar values
   float vm; // midpoint of block
   float vf1,vf2,vf3,vf4,vf5,vf6; // midpoints of faces
   float ve1,ve2,ve3,ve4,ve5,ve6,ve7,ve8,ve9,ve10,ve11,ve12; // midpoints of edges

   // normal vectors
   mininorm nm; // midpoint of block
   mininorm nf1,nf2,nf3,nf4,nf5,nf6; // midpoints of faces
   mininorm ne1,ne2,ne3,ne4,ne5,ne6,ne7,ne8,ne9,ne10,ne11,ne12; // midpoints of edges

   float minval,maxval; // minimum and maximum scalar values of corners
   float l[18],lambda,omlambda; // LOD interpolation factors

   if (s>=2)
      {
      s2=s/2;

      mi=i+s2;
      mj=j+s2;
      mk=k+s2;

      // get blending factor of actual node
      b=bf(mi,mj,mk,s2);

      // check for subdivision
      if (b>0.0f)
         {
         s3=3*s;
         s23=3*s2;

         sxs=s*XSIZE;
         sxs3=3*sxs;

         s2xs=s2*XSIZE;
         s2xs3=3*s2xs;

         minval=maxval=vc1;

         if (vc2<minval) minval=vc2;
         else if (vc2>maxval) maxval=vc2;

         if (vc3<minval) minval=vc3;
         else if (vc3>maxval) maxval=vc3;

         if (vc4<minval) minval=vc4;
         else if (vc4>maxval) maxval=vc4;

         if (vc5<minval) minval=vc5;
         else if (vc5>maxval) maxval=vc5;

         if (vc6<minval) minval=vc6;
         else if (vc6>maxval) maxval=vc6;

         if (vc7<minval) minval=vc7;
         else if (vc7>maxval) maxval=vc7;

         if (vc8<minval) minval=vc8;
         else if (vc8>maxval) maxval=vc8;

         if (THETA==0.0f)
            {
            // evaluate cullmap
            if (LOMEM==0)
               {
               mm=&MM[i+(j+k*YSIZE)*XSIZE];
               minval=fmin(MMRANGE*mm[s2]+MINV,minval);
               maxval=fmax(MMRANGE*mm[s2xs]+MINV,maxval);
               if (SURF->getspectrum()->isvisible(minval,maxval)==0) return;
               }
            else
               {
               mm2=&MM2[i+(j+k*YSIZE)*XSIZE];
               minval=fmin(MMRANGE*mm2[s2]+MINV,minval);
               maxval=fmax(MMRANGE*mm2[s2xs]+MINV,maxval);
               if (SURF->getspectrum()->isvisible(minval,maxval)==0) return;
               }

            // bottom layer of block:

            data=&DATA[i+(j+k*YSIZE)*XSIZE];

            ve9=data[s2];
            ve5=data[s2xs];
            vf5=data[s2+s2xs];
            ve6=data[s+s2xs];
            ve10=data[s2+sxs];

            nrml=&NRML[3*(i+(j+k*YSIZE)*XSIZE)];

            ne9=mininorm(&nrml[s23]);
            ne5=mininorm(&nrml[s2xs3]);
            nf5=mininorm(&nrml[s23+s2xs3]);
            ne6=mininorm(&nrml[s3+s2xs3]);
            ne10=mininorm(&nrml[s23+sxs3]);

            // top layer of block:

            data=&DATA[i+(j+(k+s)*YSIZE)*XSIZE];

            ve11=data[s2];
            ve7=data[s2xs];
            vf6=data[s2+s2xs];
            ve8=data[s+s2xs];
            ve12=data[s2+sxs];

            nrml=&NRML[3*(i+(j+(k+s)*YSIZE)*XSIZE)];

            ne11=mininorm(&nrml[s23]);
            ne7=mininorm(&nrml[s2xs3]);
            nf6=mininorm(&nrml[s23+s2xs3]);
            ne8=mininorm(&nrml[s3+s2xs3]);
            ne12=mininorm(&nrml[s23+sxs3]);

            // middle layer of block:

            data=&DATA[i+(j+(k+s2)*YSIZE)*XSIZE];

            ve1=data[0];
            vf3=data[s2];
            ve2=data[s];
            vf1=data[s2xs];
            vm=data[s2+s2xs];
            vf2=data[s+s2xs];
            ve3=data[sxs];
            vf4=data[s2+sxs];
            ve4=data[s+sxs];

            nrml=&NRML[3*(i+(j+(k+s2)*YSIZE)*XSIZE)];

            ne1=mininorm(&nrml[0]);
            nf3=mininorm(&nrml[s23]);
            ne2=mininorm(&nrml[s3]);
            nf1=mininorm(&nrml[s2xs3]);
            nm=mininorm(&nrml[s23+s2xs3]);
            nf2=mininorm(&nrml[s3+s2xs3]);
            ne3=mininorm(&nrml[sxs3]);
            nf4=mininorm(&nrml[s23+sxs3]);
            ne4=mininorm(&nrml[s3+sxs3]);
            }
         else
            {
            nts=XSIZE*YSIZE*ZSIZE;
            nts3=3*nts;

            // evaluate cullmap
            if (LOMEM==0)
               {
               mm=&MM[i+(j+k*YSIZE)*XSIZE];
               minval=fmin(MMRANGE*(OMTHETA*mm[s2]+THETA*mm[s2+nts])+MINV,minval);
               maxval=fmax(MMRANGE*(OMTHETA*mm[s2xs]+THETA*mm[s2xs+nts])+MINV,maxval);
               if (SURF->getspectrum()->isvisible(minval,maxval)==0) return;
               }
            else
               {
               mm2=&MM2[i+(j+k*YSIZE)*XSIZE];
               minval=fmin(MMRANGE*(OMTHETA*mm2[s2]+THETA*mm2[s2+nts])+MINV,minval);
               maxval=fmax(MMRANGE*(OMTHETA*mm2[s2xs]+THETA*mm2[s2xs+nts])+MINV,maxval);
               if (SURF->getspectrum()->isvisible(minval,maxval)==0) return;
               }

            // bottom layer of block:

            data=&DATA[i+(j+k*YSIZE)*XSIZE];

            ve9=OMTHETA*data[s2]+THETA*data[s2+nts];
            ve5=OMTHETA*data[s2xs]+THETA*data[s2xs+nts];
            vf5=OMTHETA*data[s2+s2xs]+THETA*data[s2+s2xs+nts];
            ve6=OMTHETA*data[s+s2xs]+THETA*data[s+s2xs+nts];
            ve10=OMTHETA*data[s2+sxs]+THETA*data[s2+sxs+nts];

            nrml=&NRML[3*(i+(j+k*YSIZE)*XSIZE)];

            ne9=OMTHETA*mininorm(&nrml[s23])+THETA*mininorm(&nrml[s23+nts3]);
            ne5=OMTHETA*mininorm(&nrml[s2xs3])+THETA*mininorm(&nrml[s2xs3+nts3]);
            nf5=OMTHETA*mininorm(&nrml[s23+s2xs3])+THETA*mininorm(&nrml[s23+s2xs3+nts3]);
            ne6=OMTHETA*mininorm(&nrml[s3+s2xs3])+THETA*mininorm(&nrml[s3+s2xs3+nts3]);
            ne10=OMTHETA*mininorm(&nrml[s23+sxs3])+THETA*mininorm(&nrml[s23+sxs3+nts3]);

            // top layer of block:

            data=&DATA[i+(j+(k+s)*YSIZE)*XSIZE];

            ve11=OMTHETA*data[s2]+THETA*data[s2+nts];
            ve7=OMTHETA*data[s2xs]+THETA*data[s2xs+nts];
            vf6=OMTHETA*data[s2+s2xs]+THETA*data[s2+s2xs+nts];
            ve8=OMTHETA*data[s+s2xs]+THETA*data[s+s2xs+nts];
            ve12=OMTHETA*data[s2+sxs]+THETA*data[s2+sxs+nts];

            nrml=&NRML[3*(i+(j+(k+s)*YSIZE)*XSIZE)];

            ne11=OMTHETA*mininorm(&nrml[s23])+THETA*mininorm(&nrml[s23+nts3]);
            ne7=OMTHETA*mininorm(&nrml[s2xs3])+THETA*mininorm(&nrml[s2xs3+nts3]);
            nf6=OMTHETA*mininorm(&nrml[s23+s2xs3])+THETA*mininorm(&nrml[s23+s2xs3+nts3]);
            ne8=OMTHETA*mininorm(&nrml[s3+s2xs3])+THETA*mininorm(&nrml[s3+s2xs3+nts3]);
            ne12=OMTHETA*mininorm(&nrml[s23+sxs3])+THETA*mininorm(&nrml[s23+sxs3+nts3]);

            // middle layer of block:

            data=&DATA[i+(j+(k+s2)*YSIZE)*XSIZE];

            ve1=OMTHETA*data[0]+THETA*data[nts];
            vf3=OMTHETA*data[s2]+THETA*data[s2+nts];
            ve2=OMTHETA*data[s]+THETA*data[s+nts];
            vf1=OMTHETA*data[s2xs]+THETA*data[s2xs+nts];
            vm=OMTHETA*data[s2+s2xs]+THETA*data[s2+s2xs+nts];
            vf2=OMTHETA*data[s+s2xs]+THETA*data[s+s2xs+nts];
            ve3=OMTHETA*data[sxs]+THETA*data[sxs+nts];
            vf4=OMTHETA*data[s2+sxs]+THETA*data[s2+sxs+nts];
            ve4=OMTHETA*data[s+sxs]+THETA*data[s+sxs+nts];

            nrml=&NRML[3*(i+(j+(k+s2)*YSIZE)*XSIZE)];

            ne1=OMTHETA*mininorm(&nrml[0])+THETA*mininorm(&nrml[nts3]);
            nf3=OMTHETA*mininorm(&nrml[s23])+THETA*mininorm(&nrml[s23+nts3]);
            ne2=OMTHETA*mininorm(&nrml[s3])+THETA*mininorm(&nrml[s3+nts3]);
            nf1=OMTHETA*mininorm(&nrml[s2xs3])+THETA*mininorm(&nrml[s2xs3+nts3]);
            nm=OMTHETA*mininorm(&nrml[s23+s2xs3])+THETA*mininorm(&nrml[s23+s2xs3+nts3]);
            nf2=OMTHETA*mininorm(&nrml[s3+s2xs3])+THETA*mininorm(&nrml[s3+s2xs3+nts3]);
            ne3=OMTHETA*mininorm(&nrml[sxs3])+THETA*mininorm(&nrml[sxs3+nts3]);
            nf4=OMTHETA*mininorm(&nrml[s23+sxs3])+THETA*mininorm(&nrml[s23+sxs3+nts3]);
            ne4=OMTHETA*mininorm(&nrml[s3+sxs3])+THETA*mininorm(&nrml[s3+sxs3+nts3]);
            }

         l[0]=fmin(bf(mi,mj,mk-s,s2),b);
         l[1]=bf(mi-s,mj,mk-s,s2);
         l[2]=bf(mi+s,mj,mk-s,s2);
         l[3]=bf(mi,mj-s,mk-s,s2);
         l[4]=bf(mi,mj+s,mk-s,s2);

         l[5]=fmin(bf(mi,mj,mk+s,s2),b);
         l[6]=bf(mi-s,mj,mk+s,s2);
         l[7]=bf(mi+s,mj,mk+s,s2);
         l[8]=bf(mi,mj-s,mk+s,s2);
         l[9]=bf(mi,mj+s,mk+s,s2);

         l[10]=fmin(bf(mi-s,mj,mk,s2),b);
         l[11]=fmin(bf(mi+s,mj,mk,s2),b);
         l[12]=fmin(bf(mi,mj-s,mk,s2),b);
         l[13]=fmin(bf(mi,mj+s,mk,s2),b);
         l[14]=bf(mi-s,mj-s,mk,s2);
         l[15]=bf(mi+s,mj-s,mk,s2);
         l[16]=bf(mi-s,mj+s,mk,s2);
         l[17]=bf(mi+s,mj+s,mk,s2);

         if ((((i+j+k)/s)%2)==0)
            {
            // interpolate bottom layer of block:

            if ((lambda=fmin(l[0],fmin(l[3],l[12])))<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               ve9=omlambda*(vc1+vc2)+lambda*ve9;
               ne9=omlambda*(nc1+nc2)+lambda*ne9;
               }

            if ((lambda=fmin(l[0],fmin(l[1],l[10])))<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               ve5=omlambda*(vc1+vc3)+lambda*ve5;
               ne5=omlambda*(nc1+nc3)+lambda*ne5;
               }

            if ((lambda=l[0])<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               vf5=omlambda*(vc1+vc4)+lambda*vf5;
               nf5=omlambda*(nc1+nc4)+lambda*nf5;
               }

            if ((lambda=fmin(l[0],fmin(l[2],l[11])))<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               ve6=omlambda*(vc2+vc4)+lambda*ve6;
               ne6=omlambda*(nc2+nc4)+lambda*ne6;
               }

            if ((lambda=fmin(l[0],fmin(l[4],l[13])))<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               ve10=omlambda*(vc3+vc4)+lambda*ve10;
               ne10=omlambda*(nc3+nc4)+lambda*ne10;
               }

            // interpolate top layer of block:

            if ((lambda=fmin(l[5],fmin(l[8],l[12])))<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               ve11=omlambda*(vc5+vc6)+lambda*ve11;
               ne11=omlambda*(nc5+nc6)+lambda*ne11;
               }

            if ((lambda=fmin(l[5],fmin(l[6],l[10])))<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               ve7=omlambda*(vc5+vc7)+lambda*ve7;
               ne7=omlambda*(nc5+nc7)+lambda*ne7;
               }

            if ((lambda=l[5])<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               vf6=omlambda*(vc6+vc7)+lambda*vf6;
               nf6=omlambda*(nc6+nc7)+lambda*nf6;
               }

            if ((lambda=fmin(l[5],fmin(l[7],l[11])))<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               ve8=omlambda*(vc6+vc8)+lambda*ve8;
               ne8=omlambda*(nc6+nc8)+lambda*ne8;
               }

            if ((lambda=fmin(l[5],fmin(l[9],l[13])))<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               ve12=omlambda*(vc7+vc8)+lambda*ve12;
               ne12=omlambda*(nc7+nc8)+lambda*ne12;
               }

            // interpolate middle layer of block:

            if ((lambda=fmin(l[10],fmin(l[14],l[12])))<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               ve1=omlambda*(vc1+vc5)+lambda*ve1;
               ne1=omlambda*(nc1+nc5)+lambda*ne1;
               }

            if ((lambda=l[12])<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               vf3=omlambda*(vc1+vc6)+lambda*vf3;
               nf3=omlambda*(nc1+nc6)+lambda*nf3;
               }

            if ((lambda=fmin(l[11],fmin(l[15],l[12])))<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               ve2=omlambda*(vc2+vc6)+lambda*ve2;
               ne2=omlambda*(nc2+nc6)+lambda*ne2;
               }

            if ((lambda=l[10])<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               vf1=omlambda*(vc1+vc7)+lambda*vf1;
               nf1=omlambda*(nc1+nc7)+lambda*nf1;
               }

            if ((lambda=b)<1.0f)
               {
               omlambda=(1.0f-lambda)/4.0f;
               vm=omlambda*(vc1+vc4+vc6+vc7)+lambda*vm;
               nm=omlambda*(nc1+nc4+nc6+nc7)+lambda*nm;
               }

            if ((lambda=l[11])<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               vf2=omlambda*(vc4+vc6)+lambda*vf2;
               nf2=omlambda*(nc4+nc6)+lambda*nf2;
               }

            if ((lambda=fmin(l[10],fmin(l[16],l[13])))<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               ve3=omlambda*(vc3+vc7)+lambda*ve3;
               ne3=omlambda*(nc3+nc7)+lambda*ne3;
               }

            if ((lambda=l[13])<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               vf4=omlambda*(vc4+vc7)+lambda*vf4;
               nf4=omlambda*(nc4+nc7)+lambda*nf4;
               }

            if ((lambda=fmin(l[11],fmin(l[17],l[13])))<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               ve4=omlambda*(vc4+vc8)+lambda*ve4;
               ne4=omlambda*(nc4+nc8)+lambda*ne4;
               }
            }
         else
            {
            // interpolate bottom layer of block:

            if ((lambda=fmin(l[0],fmin(l[3],l[12])))<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               ve9=omlambda*(vc1+vc2)+lambda*ve9;
               ne9=omlambda*(nc1+nc2)+lambda*ne9;
               }

            if ((lambda=fmin(l[0],fmin(l[1],l[10])))<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               ve5=omlambda*(vc1+vc3)+lambda*ve5;
               ne5=omlambda*(nc1+nc3)+lambda*ne5;
               }

            if ((lambda=l[0])<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               vf5=omlambda*(vc2+vc3)+lambda*vf5;
               nf5=omlambda*(nc2+nc3)+lambda*nf5;
               }

            if ((lambda=fmin(l[0],fmin(l[2],l[11])))<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               ve6=omlambda*(vc2+vc4)+lambda*ve6;
               ne6=omlambda*(nc2+nc4)+lambda*ne6;
               }

            if ((lambda=fmin(l[0],fmin(l[4],l[13])))<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               ve10=omlambda*(vc3+vc4)+lambda*ve10;
               ne10=omlambda*(nc3+nc4)+lambda*ne10;
               }

            // interpolate top layer of block:

            if ((lambda=fmin(l[5],fmin(l[8],l[12])))<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               ve11=omlambda*(vc5+vc6)+lambda*ve11;
               ne11=omlambda*(nc5+nc6)+lambda*ne11;
               }

            if ((lambda=fmin(l[5],fmin(l[6],l[10])))<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               ve7=omlambda*(vc5+vc7)+lambda*ve7;
               ne7=omlambda*(nc5+nc7)+lambda*ne7;
               }

            if ((lambda=l[5])<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               vf6=omlambda*(vc5+vc8)+lambda*vf6;
               nf6=omlambda*(nc5+nc8)+lambda*nf6;
               }

            if ((lambda=fmin(l[5],fmin(l[7],l[11])))<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               ve8=omlambda*(vc6+vc8)+lambda*ve8;
               ne8=omlambda*(nc6+nc8)+lambda*ne8;
               }

            if ((lambda=fmin(l[5],fmin(l[9],l[13])))<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               ve12=omlambda*(vc7+vc8)+lambda*ve12;
               ne12=omlambda*(nc7+nc8)+lambda*ne12;
               }

            // interpolate middle layer of block:

            if ((lambda=fmin(l[10],fmin(l[14],l[12])))<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               ve1=omlambda*(vc1+vc5)+lambda*ve1;
               ne1=omlambda*(nc1+nc5)+lambda*ne1;
               }

            if ((lambda=l[12])<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               vf3=omlambda*(vc2+vc5)+lambda*vf3;
               nf3=omlambda*(nc2+nc5)+lambda*nf3;
               }

            if ((lambda=fmin(l[11],fmin(l[15],l[12])))<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               ve2=omlambda*(vc2+vc6)+lambda*ve2;
               ne2=omlambda*(nc2+nc6)+lambda*ne2;
               }

            if ((lambda=l[10])<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               vf1=omlambda*(vc3+vc5)+lambda*vf1;
               nf1=omlambda*(nc3+nc5)+lambda*nf1;
               }

            if ((lambda=b)<1.0f)
               {
               omlambda=(1.0f-lambda)/4.0f;
               vm=omlambda*(vc2+vc3+vc5+vc8)+lambda*vm;
               nm=omlambda*(nc2+nc3+nc5+nc8)+lambda*nm;
               }

            if ((lambda=l[11])<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               vf2=omlambda*(vc2+vc8)+lambda*vf2;
               nf2=omlambda*(nc2+nc8)+lambda*nf2;
               }

            if ((lambda=fmin(l[10],fmin(l[16],l[13])))<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               ve3=omlambda*(vc3+vc7)+lambda*ve3;
               ne3=omlambda*(nc3+nc7)+lambda*ne3;
               }

            if ((lambda=l[13])<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               vf4=omlambda*(vc3+vc8)+lambda*vf4;
               nf4=omlambda*(nc3+nc8)+lambda*nf4;
               }

            if ((lambda=fmin(l[11],fmin(l[17],l[13])))<1.0f)
               {
               omlambda=(1.0f-lambda)/2.0f;
               ve4=omlambda*(vc4+vc8)+lambda*ve4;
               ne4=omlambda*(nc4+nc8)+lambda*ne4;
               }
            }

         // call recursively
         calcblock(i,j,k,s2,vc1,ve9,ve5,vf5,ve1,vf3,vf1,vm,nc1,ne9,ne5,nf5,ne1,nf3,nf1,nm);
         calcblock(i+s2,j,k,s2,ve9,vc2,vf5,ve6,vf3,ve2,vm,vf2,ne9,nc2,nf5,ne6,nf3,ne2,nm,nf2);
         calcblock(i,j+s2,k,s2,ve5,vf5,vc3,ve10,vf1,vm,ve3,vf4,ne5,nf5,nc3,ne10,nf1,nm,ne3,nf4);
         calcblock(i+s2,j+s2,k,s2,vf5,ve6,ve10,vc4,vm,vf2,vf4,ve4,nf5,ne6,ne10,nc4,nm,nf2,nf4,ne4);
         calcblock(i,j,k+s2,s2,ve1,vf3,vf1,vm,vc5,ve11,ve7,vf6,ne1,nf3,nf1,nm,nc5,ne11,ne7,nf6);
         calcblock(i+s2,j,k+s2,s2,vf3,ve2,vm,vf2,ve11,vc6,vf6,ve8,nf3,ne2,nm,nf2,ne11,nc6,nf6,ne8);
         calcblock(i,j+s2,k+s2,s2,vf1,vm,ve3,vf4,ve7,vf6,vc7,ve12,nf1,nm,ne3,nf4,ne7,nf6,nc7,ne12);
         calcblock(i+s2,j+s2,k+s2,s2,vm,vf2,vf4,ve4,vf6,ve8,ve12,vc8,nm,nf2,nf4,ne4,nf6,ne8,ne12,nc8);

         return;
         }
      }

   SURF->cacheblock(i,j,k,s,
                    vc1,vc2,vc3,vc4,vc5,vc6,vc7,vc8,
                    nc1,nc2,nc3,nc4,nc5,nc6,nc7,nc8);
   }

// render a brick tile
void minibrick::drawdata(databuf *brick,minisurf *surface,int phase)
   {
#ifndef NOOGL

   GLfloat mtx[16]={brick->sex-brick->swx,0.0f,brick->nwx-brick->swx,0.0f,
                    brick->sey-brick->swy,0.0f,brick->nwy-brick->swy,0.0f,
                    0.0f,brick->dh,0.0f,0.0f,
                    0.0f,0.0f,0.0f,1.0f};

   glPushMatrix();

   // warp tile from computational to object space
   // assuming that the tile can be approximated by a rectangle
   // with the origin being the south-west corner
   glTranslatef(brick->swx,brick->h0,brick->swy);
   glMultMatrixf(mtx);
   glScalef(1.0f/(brick->xsize-1),1.0f/(brick->ysize-1),1.0f/(brick->zsize-1));

   // set stripe pattern
   surface->setstripewidth(CONFIGURE_STRIPEWIDTH);
   surface->setstripedir(CONFIGURE_STRIPEDX,CONFIGURE_STRIPEDY,CONFIGURE_STRIPEDZ);
   surface->setstripeoffset(CONFIGURE_STRIPEOFFSET);

   // enable correct Z
   surface->setcorrectz(CONFIGURE_CORRECTZ);

   // push vertex arrays
   surface->render(phase,
                   CONFIGURE_RENDERPASSES,
                   CONFIGURE_AMBIENT,
                   CONFIGURE_BRDRCTRL,CONFIGURE_CNTRCTRL,CONFIGURE_COLRCTRL,
                   CONFIGURE_BRDRCTRL2,CONFIGURE_CNTRCTRL2,CONFIGURE_COLRCTRL2);

   glPopMatrix();

#endif
   }

// process triangles
void minibrick::processarrays(databuf *brick,minisurf *surface)
   {
   int n;

   float **array;
   int num;

   miniv3d s,o;

   BOOLINT valid;

   RAY->clearbuffer();

   valid=TRUE;

   if (brick->nwx!=brick->swx ||
       brick->sey!=brick->swy) valid=false;

   if (FREEZE!=0)
      if (DX1!=0.0f || DY1!=0.0f || DZ1!=0.0f ||
          DX2!=0.0f || DY2!=0.0f || DZ2!=0.0f ||
          DX3!=0.0f || DY3!=0.0f || DZ3!=0.0f) valid=false;

   if (valid)
      for (n=0; n<surface->getspectrum()->getnum(); n++)
         {
         array=surface->getspectrum()->getreadbuf(n)->getarrayref();
         num=surface->getspectrum()->getreadbuf(n)->getsize()/3;

         // assuming an axis-aligned brick
         s.x=(brick->sex-brick->swx)/(brick->xsize-1)*SCALEX;
         s.y=(brick->dh)/(brick->zsize-1)*SCALEELEV;
         s.z=-(brick->nwy-brick->swy)/(brick->ysize-1)*SCALEY;

         // translate to left bottom front corner
         o.x=(brick->swx+OFFSETLON)*SCALEX;
         o.y=brick->h0*SCALEELEV;
         o.z=-(brick->swy+OFFSETLAT)*SCALEY;

         RAY->addtriangles(array,3,num,3,&s,&o,1);
         }

   RAY->swapbuffer();
   }
