// (c) by Stefan Roettger

#include "miniP.h"
#include <iostream>
// set the global error handler for the mini library
void setminierrorhandler(void (*handler)(const char *file,int line,int fatal))
   {minibase::minierrorhandler=handler;}

namespace mini {

// set fine tuning parameters
void setparams(float minr,
               float maxd,
               float sead,
               float mino,
               int maxc)
   {
   if (minr<1.0f || maxd<=0.0f || sead<0.0f || mino<0.0f || maxc<0) ERRORMSG();

   minres=minr;
   maxd2=maxd;
   sead2=sead;
   minoff=mino;
   maxcull=maxc;
   }

// scale the height field
void scalemap(short int *image,int size)
   {
   int i,j,
       mi,mj;

   float pi,pj,
         ri,rj;

   short int *ptr1,*ptr2;

   if (size==S)
      for (i=0; i<S; i++)
         for (ptr1=&image[(S-1)*S+i],ptr2=y[i]; ptr1>=image; ptr1-=S) *ptr2++=*ptr1;
   else
      for (i=0; i<S; i++)
         for (j=0; j<S; j++)
            {
            if (i<S-1)
               {
               pi=(float)i/(S-1)*(size-1);
               mi=ftrc(pi);
               ri=pi-mi;
               }
            else
               {
               mi=size-2;
               ri=1.0f;
               }

            if (j>0)
               {
               pj=(float)(S-1-j)/(S-1)*(size-1);
               mj=ftrc(pj);
               rj=pj-mj;
               }
            else
               {
               mj=size-2;
               rj=1.0f;
               }

            y[i][j]=ftrc((1.0f-rj)*((1.0f-ri)*image[mj*size+mi]+
                                           ri*image[mj*size+mi+1])+
                                rj*((1.0f-ri)*image[(mj+1)*size+mi]+
                                           ri*image[(mj+1)*size+mi+1])+0.5f);
            }
   }

// calculate the height differences
void calcDH(int mins=1,int maxs=0,int ds=1)
   {
   int i,j,s,
       m,n;

   if (maxs==0) maxs=S-1;

   DH=y[S];
   for (i=0; i<S; i++) DH[i]=(short int)65535;

   for (s=maxs; s>mins; s/=2)
      {
      DH[s]=0;

      for (i=s/2; i<S; i+=s)
         for (j=s/2; j<S; j+=s)
            if (ds<s)
               for (m=-s/2; m<=s/2; m+=ds)
                  for (n=-s/2; n<=s/2; n+=ds)
                     DH[s]=max((unsigned short int)DH[s],abs(y[i+m][j+n]-y[i][j]));
            else
               for (m=-s/2; m<=s/2; m++)
                  for (n=-s/2; n<=s/2; n++)
                     DH[s]=max((unsigned short int)DH[s],abs(y[i+m][j+n]-y[i][j]));
      }
   }

// store a d2-value
inline void store(const float fc,const int i,const int j,const int s2)
   {
   bc[i-s2][j]=cpr(fc)%256;
   bc[i][j-s2]=cpr(fc)/256;
   }

// increase a d2-value
inline void increase(const float fc,const int i,const int j,const int s2)
   {
   float fc1;

   fc1=fmin(fc,1.0f);

   if (fc1>dcpr(i,j,s2))
      {
      store(fc1,i,j,s2);

      while (dcpr(i,j,s2)<fc1)
         if (++bc[i-s2][j]==0)
            if (++bc[i][j-s2]==0)
               {
               bc[i-s2][j]=bc[i][j-s2]=255;
               break;
               }
      }
   }

// propagate a local d2-value to the next higher level
inline void propagate(const int i,const int j,const int s,const int i0,const int j0)
   {
   float l1,l2;

   if (i0<0 || i0>=S || j0<0 || j0>=S) return;

   l1=2.0f*s*D*minres;
   l2=l1-fsqrt(fsqr(X(i)-X(i0))+fsqr(Z(j)-Z(j0)));
   if (l2<=0.0f) ERRORMSG();

   increase(dcpr(i,j,s/2)*l1/l2/2.0f,i0,j0,s);
   }

// calculate d2-value
inline float d2value(const float a,const float b,const float m,const int s)
   {
   float d2,dh;

   d2=fabs(a+b-2.0f*m);
   dh=fmax(fabs(m-a),fabs(m-b));

   if (m-dh<SEALEVELMAX && m+dh>SEALEVELMIN) d2=fmax(d2,sead2*s*D);

   return(d2);
   }

// calculate the d2-values
void calcD2(int mins=2,float avgd2=0.1f)
   {
   int i,j,s,s2;

   float fc;

   // initialize the d2-values
   for (i=0; i<S-1; i++) memset(bc[i],0,S-1);

   // compute an approximate d2-value
   fc=avgd2*DH[S-1]*SCALE/(S-1)/D/maxd2;

   // approximate the least-significant d2-values
   if (mins>2)
      for (s=2; s<=mins; s*=2)
         {
         s2=s/2;
         for (i=s2; i<S; i+=s)
            for (j=s2; j<S; j+=s) store(fc,i,j,s2);
         }

   // propagate the d2-values up the tree
   for (s=mins; s<S; s*=2)
      {
      s2=s/2;
      for (i=s2; i<S; i+=s)
         for (j=s2; j<S; j+=s)
            {
            // calculate the local d2-value
            if ((i/s+j/s)%2==0) fc=d2value(Y(i-s2,j-s2),Y(i+s2,j+s2),Y(i,j),s);
            else fc=d2value(Y(i-s2,j+s2),Y(i+s2,j-s2),Y(i,j),s);

            // calculate the local d2-value more accurately
            if (s>mins || mins==2)
               {
               fc=fmax(fc,d2value(Y(i-s2,j-s2),Y(i+s2,j-s2),Y(i,j-s2),s));
               fc=fmax(fc,d2value(Y(i-s2,j+s2),Y(i+s2,j+s2),Y(i,j+s2),s));
               fc=fmax(fc,d2value(Y(i-s2,j-s2),Y(i-s2,j+s2),Y(i-s2,j),s));
               fc=fmax(fc,d2value(Y(i+s2,j-s2),Y(i+s2,j+s2),Y(i+s2,j),s));
               }

            // store the local d2-value
            increase(fc/s/D/maxd2,i,j,s2);

            // propagate the local d2-value
            if (s<S-1)
               switch ((i/s)%2+2*((j/s)%2))
                  {
                  case 0:
                     propagate(i,j,s,i+s2,j+s2);
                     propagate(i,j,s,i-s-s2,j+s2);
                     propagate(i,j,s,i+s2,j-s-s2);
                     break;
                  case 1:
                     propagate(i,j,s,i-s2,j+s2);
                     propagate(i,j,s,i-s2,j-s-s2);
                     propagate(i,j,s,i+s+s2,j+s2);
                     break;
                  case 2:
                     propagate(i,j,s,i+s2,j-s2);
                     propagate(i,j,s,i+s2,j+s+s2);
                     propagate(i,j,s,i-s-s2,j-s2);
                     break;
                  case 3:
                     propagate(i,j,s,i-s2,j-s2);
                     propagate(i,j,s,i+s+s2,j-s2);
                     propagate(i,j,s,i-s2,j+s+s2);
                     break;
                  }
            }
      }
   }

// preprocess the height field
void *initmap(short int *image,void **d2map,
              int *size,float *dim,float scale,
              float cellaspect,
              short int (*getelevation)(int i,int j,int S,void *data),
              void *objref,
              int fast,float avgd2)
   {
   int i,j;

   if (*size<2 || *dim<=0.0f || scale<0.0f) ERRORMSG();

   for (S=3; S<*size; S=2*S-1);

   Dx=(*dim)*(*size-1)/(S-1);
   Dz=Dx*cellaspect;
   D=(Dx+Dz)/2.0f;

   SCALE=scale;

   if ((y=(short int **)malloc((S+1)*sizeof(short int *)))==NULL) ERRORMSG();
   for (i=0; i<=S; i++)
      if ((y[i]=(short int *)malloc(S*sizeof(short int)))==NULL) ERRORMSG();

   if (image!=NULL) scalemap(image,*size);
   else if (*size==S)
      {
      if (getelevation==NULL) ERRORMSG();

      for (j=0; j<S; j++)
         for (i=0; i<S; i++) y[i][S-1-j]=getelevation(i,j,S,objref);
      }
   else
      {
      if (getelevation==NULL) ERRORMSG();

      if ((image=(short int *)malloc((*size)*(*size)*sizeof(short int)))==NULL) ERRORMSG();

      for (j=0; j<*size; j++)
         for (i=0; i<*size; i++) image[i+j*(*size)]=getelevation(i,j,*size,objref);

      scalemap(image,*size);

      free(image);
      }

   if ((bc=(unsigned char **)malloc((S-1)*sizeof(unsigned char *)))==NULL) ERRORMSG();
   for (i=0; i<S-1; i++)
      if ((bc[i]=(unsigned char *)malloc(S-1))==NULL) ERRORMSG();

   tid=0;
   yf=NULL;

   updatemaps(fast,avgd2);

   OX=OY=OZ=0.0f;
   for (i=0; i<4; i++) bc2[i]=NULL;

   *d2map=(void *)bc;

   *size=S;
   *dim=Dx;

   return((void *)y);
   }

// calculate the texture map
int inittexmap(unsigned char *image,int *width,int *height,
               int mipmaps,int s3tc,int rgba,int bytes,int mipmapped)
   {
   if (S==0) ERRORMSG();

   if (tid!=0) deletetexmap(tid);

   if (image==NULL) return(tid=0);

   if (*width<2 || *height<2) ERRORMSG();

   if (rgba==0) tid=buildRGBtexmap(image,width,height,mipmaps,s3tc,bytes,mipmapped);
   else tid=buildRGBAtexmap(image,width,height,mipmaps,s3tc,bytes,mipmapped);

   twidth=*width;
   theight=*height;

   tmipmaps=mipmaps;

   return(tid);
   }

// scale the ground fog map
void scalefog(unsigned char *image,int size)
   {
   int i,j,
       mi,mj;

   float pi,pj,
         ri,rj;

   unsigned char *ptr1,*ptr2;

   if (size==S)
      for (i=0; i<S; i++)
         for (ptr1=&image[(S-1)*S+i],ptr2=yf[i]; ptr1>=image; ptr1-=S) *ptr2++=*ptr1;
   else
      for (i=0; i<S; i++)
         for (j=0; j<S; j++)
            {
            if (i<S-1)
               {
               pi=(float)i/(S-1)*(size-1);
               mi=ftrc(pi);
               ri=pi-mi;
               }
            else
               {
               mi=size-2;
               ri=1.0f;
               }

            if (j>0)
               {
               pj=(float)(S-1-j)/(S-1)*(size-1);
               mj=ftrc(pj);
               rj=pj-mj;
               }
            else
               {
               mj=size-2;
               rj=1.0f;
               }

            yf[i][j]=ftrc((1.0f-rj)*((1.0f-ri)*image[mj*size+mi]+
                                            ri*image[mj*size+mi+1])+
                                 rj*((1.0f-ri)*image[(mj+1)*size+mi]+
                                            ri*image[(mj+1)*size+mi+1])+0.5f);
            }
   }

// recalculate the height differences
void recalcDH(float lambda,
              int mins=1,int maxs=0)
   {
   int s;

   float h;

   if (SCALE==0.0f) ERRORMSG();

   if (maxs==0) maxs=S-1;

   for (s=maxs; s>mins; s/=2)
      {
      h=(unsigned short int)DH[s]+lambda/SCALE;
      if (h>65535.0f) h=65535.0f;
      DH[s]=ftrc(fceil(h));
      }
   }

// recalculate the d2-values
void recalcD2(float fogatt,int mins=2)
   {
   int i,j,s,s2;

   float fc;

   // propagate the d2-values up the tree
   for (s=mins; s<S; s*=2)
      {
      s2=s/2;
      for (i=s2; i<S; i+=s)
         for (j=s2; j<S; j+=s)
            {
            // calculate the local d2-value
            if ((i/s+j/s)%2==0) fc=d2value(YF(i-s2,j-s2),YF(i+s2,j+s2),YF(i,j),s);
            else fc=d2value(YF(i-s2,j+s2),YF(i+s2,j-s2),YF(i,j),s);

            // calculate the local d2-value more accurately
            if (s>mins || mins==2)
               {
               fc=fmax(fc,d2value(YF(i-s2,j-s2),YF(i+s2,j-s2),YF(i,j-s2),s));
               fc=fmax(fc,d2value(YF(i-s2,j+s2),YF(i+s2,j+s2),YF(i,j+s2),s));
               fc=fmax(fc,d2value(YF(i-s2,j-s2),YF(i-s2,j+s2),YF(i-s2,j),s));
               fc=fmax(fc,d2value(YF(i+s2,j-s2),YF(i+s2,j+s2),YF(i+s2,j),s));
               }

            // store the local d2-value
            increase(fc*fogatt/s/D/maxd2,i,j,s2);

            // propagate the local d2-value
            if (s<S-1)
               switch ((i/s)%2+2*((j/s)%2))
                  {
                  case 0:
                     propagate(i,j,s,i+s2,j+s2);
                     propagate(i,j,s,i-s-s2,j+s2);
                     propagate(i,j,s,i+s2,j-s-s2);
                     break;
                  case 1:
                     propagate(i,j,s,i-s2,j+s2);
                     propagate(i,j,s,i-s2,j-s-s2);
                     propagate(i,j,s,i+s+s2,j+s2);
                     break;
                  case 2:
                     propagate(i,j,s,i+s2,j-s2);
                     propagate(i,j,s,i+s2,j+s+s2);
                     propagate(i,j,s,i-s-s2,j-s2);
                     break;
                  case 3:
                     propagate(i,j,s,i-s2,j-s2);
                     propagate(i,j,s,i+s+s2,j-s2);
                     propagate(i,j,s,i-s2,j+s+s2);
                     break;
                  }
            }
      }
   }

// calculate the ground fog map
void *initfogmap(unsigned char *image,int size,
                 float lambda,float displace,float emission,
                 float fogatt,float fogR,float fogG,float fogB,
                 int fast)
   {
   int i;

   if (S==0) ERRORMSG();

   if (image==NULL) return(NULL);

   if (size<2) ERRORMSG();
   if (lambda<=0.0f || displace<0.0f || emission<0.0f) ERRORMSG();
   if (fogatt<0.0f) ERRORMSG();

   if (yf!=NULL) ERRORMSG();
   if ((yf=(unsigned char **)malloc(S*sizeof(unsigned char *)))==NULL) ERRORMSG();
   for (i=0; i<S; i++)
      if ((yf[i]=(unsigned char *)malloc(S))==NULL) ERRORMSG();

   scalefog(image,size);

   LAMBDA=lambda;
   DISPLACE=displace;
   EMISSION=emission;
   FOGATT=fogatt;

   FR=fogR;
   FG=fogG;
   FB=fogB;

   updatemaps(fast,0.0f,1);

   return((void *)yf);
   }

// set the height field and all associated maps
void setmaps(void *map,void *d2map,
             int size,float dim,float scale,
             int texid,int width,int height,int mipmaps,
             float cellaspect,
             float ox,float oy,float oz,
             void **d2map2,int *size2,
             void *fogmap,float lambda,float displace,
             float emission,float fogatt,float fogR,float fogG,float fogB)
   {
   int i;

   S=size;

   Dx=dim;
   Dz=Dx*cellaspect;
   D=(Dx+Dz)/2.0f;

   SCALE=scale;

   y=(short int **)map;
   DH=y[S];

   bc=(unsigned char **)d2map;

   tid=texid;
   twidth=width;
   theight=height;
   tmipmaps=mipmaps;

   yf=(unsigned char **)fogmap;

   LAMBDA=lambda;
   DISPLACE=displace;
   EMISSION=emission;
   FOGATT=fogatt;

   FR=fogR;
   FG=fogG;
   FB=fogB;

   OX=ox;
   OY=oy;
   OZ=oz;

   for (i=0; i<4; i++)
      if (d2map2==NULL) bc2[i]=NULL;
      else
         {
         bc2[i]=(unsigned char **)d2map2[i];
         S2[i]=size2[i];
         }
   }

// set the sea level
void setsea(float level)
   {
   if (level==-MAXFLOAT) SEALEVEL=level;
   else SEALEVEL=level/SCALE;
   }

// set the sea level range
void setsearange(float seamin,float seamax)
   {
   SEALEVELMIN=seamin;
   SEALEVELMAX=seamax;
   }

// undo the previous triangulation
void undomap(const int i,const int j,const int s2)
   {
   int s4;

   if (bc[i][j]!=0)
      {
      bc[i][j]=0;

      if ((s4=s2/2)>0)
         {
         undomap(i+s4,j+s4,s4);
         undomap(i-s4,j+s4,s4);
         undomap(i-s4,j-s4,s4);
         undomap(i+s4,j-s4,s4);
         }
      }
   }

// triangulate the height field
void calcmap(const int i,const int j,const int s)
   {
   int s2,s4;

   float dx,dy,dz;

   float dh;

   float f;

   float l,d;

   s2=s/2;
   s4=s/4;

   dx=X(i)-FX;
   dz=Z(j)-FZ;

   // evaluate the subdivision variable
   if (!ORTHO) f=(dx*dx+DF2+dz*dz)/fsqr(s*D*fmax(c*dcpr(i,j,s2),minres));
   else f=fsqr(S-1)/fsqr(s*fmax(c*dcpr(i,j,s2),minres));

   // check subdivision condition
   if (f<1.0f)
      {
      // blending factor
      if (f<=0.25f) bc[i][j]=255;
      else bc[i][j]=340-ftrc(340.0f*f);

      if (s4>0)
         {
         // view frustum culling
         if (CULLING && s>(S>>maxcull) && f<0.25f)
            {
            dy=Y(i,j)-FY;

            dx+=FX-EX;
            dy+=FY-EY;
            dz+=FZ-EZ;

            dh=(unsigned short int)DH[s];

            l=DX*dx+DY*dy+DZ*dz;
            d=3.0f*(k1*s2+k2*dh);

            if (l<NEARP-d || l>FARP+d) return;

            if (!ORTHO)
               {
               if (nx1*dx+ny1*dy+nz1*dz>3.0f*(k11*s2+k12*dh)) return;
               if (nx2*dx+ny2*dy+nz2*dz>3.0f*(k21*s2+k22*dh)) return;
               if (nx3*dx+ny3*dy+nz3*dz>3.0f*(k31*s2+k32*dh)) return;
               if (nx4*dx+ny4*dy+nz4*dz>3.0f*(k41*s2+k42*dh)) return;
               }
            else
               {
               if (fabs(RX*dx+RY*dy+RZ*dz)>3.0f*(k11*s2+k12*dh)+k31) return;
               if (fabs(UX*dx+UY*dy+UZ*dz)>3.0f*(k21*s2+k22*dh)+k32) return;
               }
            }

         // subdivision
         calcmap(i+s4,j+s4,s2);
         calcmap(i-s4,j+s4,s2);
         calcmap(i-s4,j-s4,s2);
         calcmap(i+s4,j-s4,s2);
         }
      }
   else undomap(i,j,s2);
   }

// blending functions:

inline float blendE(const int bc,const float v0,const float v1,const float v2)
   {return((bc==255)?v0:(2*bc*v0+(255-bc)*(v1+v2))/510.0f);}

inline float blendM(const int bc,const float v0,const float v1,const float v2)
   {return((bc==0)?-MAXFLOAT:blendE(bc,v0,v1,v2));}

inline int maxbc(const int w,int i,int j,const int s2)
   {
   int s;

   if (bc2[w]==NULL) return(255);

   if (S2[w]<S)
      {
      if ((s=(S-1)/(S2[w]-1))>s2) return(0);
      i/=s;
      j/=s;
      }
   else
      {
      s=(S2[w]-1)/(S-1);
      i*=s;
      j*=s;
      }

   return(bc2[w][i][j]);
   }

inline float blendD(const int i,const int j,const float y1,const float y2)
   {return(blendE(bc[i][j],y[i][j],y1,y2));}

inline float blendV(const int i,const int j,const int s2,const float y1,const float y2)
   {return(blendM(min((i>0)?bc[i-s2][j]:maxbc(0,S-1-s2,j,s2),
                      (i<S-1)?bc[i+s2][j]:maxbc(1,s2,j,s2)),y[i][j],y1,y2));}

inline float blendH(const int i,const int j,const int s2,const float y1,const float y2)
   {return(blendM(min((j>0)?bc[i][j-s2]:maxbc(2,i,S-1-s2,s2),
                      (j<S-1)?bc[i][j+s2]:maxbc(3,i,s2,s2)),y[i][j],y1,y2));}

// geomorph the triangulation
void drawmap(const int i,const int j,const int s,
             const float e1,const float e2,const float e3,const float e4)
   {
   int s2,s4;

   float dx,dy,dz;

   float dh;

   float l,d;

   float m0,m1,m2,m3,m4;

   int bc1,bc2,bc3,bc4;

   s2=s/2;
   s4=s/4;

   // blend the center vertex
   if (((i+j)/s)%2==1) m0=blendD(i,j,e1,e3);
   else m0=blendD(i,j,e2,e4);

   // view frustum culling
   if (CULLING && s>(S>>maxcull))
      {
      dx=X(i)-EX;
      dy=m0*SCALE-EY;
      dz=Z(j)-EZ;

      dh=(unsigned short int)DH[s];

      l=DX*dx+DY*dy+DZ*dz;
      d=k1*s2+k2*dh;

      if (l<NEARP-d || l>FARP+d) return;

      if (!ORTHO)
         {
         if (nx1*dx+ny1*dy+nz1*dz>k11*s2+k12*dh) return;
         if (nx2*dx+ny2*dy+nz2*dz>k21*s2+k22*dh) return;
         if (nx3*dx+ny3*dy+nz3*dz>k31*s2+k32*dh) return;
         if (nx4*dx+ny4*dy+nz4*dz>k41*s2+k42*dh) return;
         }
      else
         {
         if (fabs(RX*dx+RY*dy+RZ*dz)>k11*s2+k12*dh+k31) return;
         if (fabs(UX*dx+UY*dy+UZ*dz)>k21*s2+k22*dh+k32) return;
         }
      }

   // blend the edge vertices
   m1=blendV(i+s2,j,s2,e4,e1);
   m2=blendH(i,j+s2,s2,e1,e2);
   m3=blendV(i-s2,j,s2,e2,e3);
   m4=blendH(i,j-s2,s2,e3,e4);

   // subdivision factors
   if (s4>0)
      {
      bc1=bc[i+s4][j+s4];
      bc2=bc[i-s4][j+s4];
      bc3=bc[i-s4][j-s4];
      bc4=bc[i+s4][j-s4];
      }
   else bc1=bc2=bc3=bc4=0;

   // first triangle fan quarter
   if (bc1!=0)
      {
      if (m1==-MAXFLOAT) m1=(e4+e1)/2.0f;
      if (m2==-MAXFLOAT) m2=(e1+e2)/2.0f;
      drawmap(i+s4,j+s4,s2,e1,m2,m0,m1);
      }
   else
      {
      beginfan();
      fanvertex(i,m0,j);
      if (bc4!=0)
         {
         if (m1==-MAXFLOAT) m1=(e4+e1)/2.0f;
         fanvertex(i+s2,m1,j);
         }
      else if (m1!=-MAXFLOAT) fanvertex(i+s2,m1,j);
      fanvertex(i+s2,e1,j+s2);
      }

   // second triangle fan quarter
   if (bc2!=0)
      {
      if (bc1==0)
         {
         if (m2==-MAXFLOAT) m2=(e1+e2)/2.0f;
         fanvertex(i,m2,j+s2);
         }
      if (m3==-MAXFLOAT) m3=(e2+e3)/2.0f;
      drawmap(i-s4,j+s4,s2,m2,e2,m3,m0);
      }
   else
      {
      if (bc1!=0)
         {
         beginfan();
         fanvertex(i,m0,j);
         fanvertex(i,m2,j+s2);
         }
      else if (m2!=-MAXFLOAT) fanvertex(i,m2,j+s2);
      fanvertex(i-s2,e2,j+s2);
      }

   // third triangle fan quarter
   if (bc3!=0)
      {
      if (bc2==0)
         {
         if (m3==-MAXFLOAT) m3=(e2+e3)/2.0f;
         fanvertex(i-s2,m3,j);
         }
      if (m4==-MAXFLOAT) m4=(e3+e4)/2.0f;
      drawmap(i-s4,j-s4,s2,m0,m3,e3,m4);
      }
   else
      {
      if (bc2!=0)
         {
         beginfan();
         fanvertex(i,m0,j);
         fanvertex(i-s2,m3,j);
         }
      else if (m3!=-MAXFLOAT) fanvertex(i-s2,m3,j);
      fanvertex(i-s2,e3,j-s2);
      }

   // fourth triangle fan quarter
   if (bc4!=0)
      {
      if (bc3==0)
         {
         if (m4==-MAXFLOAT) m4=(e3+e4)/2.0f;
         fanvertex(i,m4,j-s2);
         }
      drawmap(i+s4,j-s4,s2,m1,m0,m4,e4);
      }
   else
      {
      if (bc3!=0)
         {
         beginfan();
         fanvertex(i,m0,j);
         fanvertex(i,m4,j-s2);
         }
      else if (m4!=-MAXFLOAT) fanvertex(i,m4,j-s2);
      fanvertex(i+s2,e4,j-s2);
      if (bc1==0)
         if (m1!=-MAXFLOAT) fanvertex(i+s2,m1,j);
         else fanvertex(i+s2,e1,j+s2);
      else fanvertex(i+s2,m1,j);
      }
   }

// prism caching functions:

inline void cacheprism(const int i0,const float y0,const float yf0,const int j0,
                       const int i1,const float y1,const float yf1,const int j1,
                       const int i2,const float y2,const float yf2,const int j2)
   {
   float offx,offy,offz;

   float p1x,p1y,p1yf,p1z;
   float p2x,p2y,p2yf,p2z;
   float p3x,p3y,p3yf,p3z;

   float *prismptr;

   offx=-S/2*Dx+OX;
   offy=DISPLACE+OY;
   offz=S/2*Dz+OZ;

   p1x=i0*Dx+offx;
   p1y=y0*SCALE+offy;
   p1yf=yf0*LAMBDA;
   p1z=offz-j0*Dz;
   p2x=i1*Dx+offx;
   p2y=y1*SCALE+offy;
   p2yf=yf1*LAMBDA;
   p2z=offz-j1*Dz;
   p3x=i2*Dx+offx;
   p3y=y2*SCALE+offy;
   p3yf=yf2*LAMBDA;
   p3z=offz-j2*Dz;

   if (prismedge_callback!=NULL)
      {
      prismedge_callback(p1x,p1y+p1yf,p1yf,p1z);
      prismedge_callback(p2x,p2y+p2yf,p2yf,p2z);
      prismedge_callback(p3x,p3y+p3yf,p3yf,p3z);
      }
   else
      {
      if (PRISMCACHE==NULL)
         {
         if ((PRISMCACHE=(float *)malloc(12*PRISMMAX*sizeof(float)))==NULL) ERRORMSG();
         PRISMCNT=0;
         }

      if (PRISMCNT>=PRISMMAX)
         {
         PRISMMAX*=2;
         if ((PRISMCACHE=(float *)realloc(PRISMCACHE,12*PRISMMAX*sizeof(float)))==NULL) ERRORMSG();
         }

      prismptr=&PRISMCACHE[12*PRISMCNT];

      *prismptr++=p1x;
      *prismptr++=p1y;
      *prismptr++=p1y+p1yf;
      *prismptr++=p1z;
      *prismptr++=p2x;
      *prismptr++=p2y;
      *prismptr++=p2y+p2yf;
      *prismptr++=p2z;
      *prismptr++=p3x;
      *prismptr++=p3y;
      *prismptr++=p3y+p3yf;
      *prismptr=p3z;

      PRISMCNT++;
      }
   }

inline void BEGINFAN()
   {
   FANSTATE=0;
   beginfan();
   }

inline void FANVERTEX(const int i,const float y,const float yf,const int j)
   {
   static int i0,i1;
   static float y0,y1;
   static float yf0,yf1;
   static int j0,j1;

   if (tid!=0)
      switch (FANSTATE)
         {
         case 0:
            fanvertex(i,y,j);
            i0=i; y0=y; yf0=yf; j0=j;
            FANSTATE++;
            break;
         case 1:
            fanvertex(i,y,j);
            i1=i; y1=y; yf1=yf; j1=j;
            FANSTATE++;
            break;
         default:
            fanvertex(i,y,j);
            if (yf0>0.0f || yf1>0.0f || yf>0.0f) cacheprism(i0,y0,yf0,j0,i1,y1,yf1,j1,i,y,yf,j);
            i1=i; y1=y; yf1=yf; j1=j;
            break;
         }
   else
      switch (FANSTATE)
         {
         case 0:
            i0=i; y0=y; yf0=yf; j0=j;
            FANSTATE++;
            break;
         case 1:
            i1=i; y1=y; yf1=yf; j1=j;
            FANSTATE++;
            break;
         default:
            if (yf0>0.0f || yf1>0.0f || yf>0.0f) cacheprism(i0,y0,yf0,j0,i1,y1,yf1,j1,i,y,yf,j);
            i1=i; y1=y; yf1=yf; j1=j;
            break;
         }
   }

// prism clipping functions:

inline void cliptetra1(const float p1x,const float p1y,const float p1z,const float d1,
                       const float p2x,const float p2y,const float p2z,const float d2,
                       const float p3x,const float p3y,const float p3z,const float d3,
                       const float p4x,const float p4y,const float p4z,const float d4)
   {
   float pp1x,pp1y,pp1z,
         pp2x,pp2y,pp2z,
         pp3x,pp3y,pp3z;

   pp1x=(d2*p1x+d1*p2x)/(d1+d2);
   pp1y=(d2*p1y+d1*p2y)/(d1+d2);
   pp1z=(d2*p1z+d1*p2z)/(d1+d2);
   pp2x=(d3*p1x+d1*p3x)/(d1+d3);
   pp2y=(d3*p1y+d1*p3y)/(d1+d3);
   pp2z=(d3*p1z+d1*p3z)/(d1+d3);
   pp3x=(d4*p1x+d1*p4x)/(d1+d4);
   pp3y=(d4*p1y+d1*p4y)/(d1+d4);
   pp3z=(d4*p1z+d1*p4z)/(d1+d4);

   beginfan();
   fanvertex(pp1x,pp1y,pp1z);
   fanvertex(pp2x,pp2y,pp2z);
   fanvertex(pp3x,pp3y,pp3z);
   }

inline void cliptetra2(const float p1x,const float p1y,const float p1z,const float d1,
                       const float p2x,const float p2y,const float p2z,const float d2,
                       const float p3x,const float p3y,const float p3z,const float d3,
                       const float p4x,const float p4y,const float p4z,const float d4)
   {
   float pp1x,pp1y,pp1z,
         pp2x,pp2y,pp2z,
         pp3x,pp3y,pp3z,
         pp4x,pp4y,pp4z;

   pp1x=(d3*p1x+d1*p3x)/(d1+d3);
   pp1y=(d3*p1y+d1*p3y)/(d1+d3);
   pp1z=(d3*p1z+d1*p3z)/(d1+d3);
   pp2x=(d3*p2x+d2*p3x)/(d2+d3);
   pp2y=(d3*p2y+d2*p3y)/(d2+d3);
   pp2z=(d3*p2z+d2*p3z)/(d2+d3);
   pp3x=(d4*p1x+d1*p4x)/(d1+d4);
   pp3y=(d4*p1y+d1*p4y)/(d1+d4);
   pp3z=(d4*p1z+d1*p4z)/(d1+d4);
   pp4x=(d4*p2x+d2*p4x)/(d2+d4);
   pp4y=(d4*p2y+d2*p4y)/(d2+d4);
   pp4z=(d4*p2z+d2*p4z)/(d2+d4);

   beginfan();
   fanvertex(pp1x,pp1y,pp1z);
   fanvertex(pp2x,pp2y,pp2z);
   fanvertex(pp4x,pp4y,pp4z);
   fanvertex(pp3x,pp3y,pp3z);
   }

inline void cliptetra(const float p1x,const float p1y,const float p1z,
                      const float p2x,const float p2y,const float p2z,
                      const float p3x,const float p3y,const float p3z,
                      const float p4x,const float p4y,const float p4z)
   {
   int flag=0;

   float d1,d2,d3,d4;

   float nearp=1.0001f*NEARP;

   d1=(p1x-EX)*DX+(p1y-EY)*DY+(p1z-EZ)*DZ-nearp;
   d2=(p2x-EX)*DX+(p2y-EY)*DY+(p2z-EZ)*DZ-nearp;
   d3=(p3x-EX)*DX+(p3y-EY)*DY+(p3z-EZ)*DZ-nearp;
   d4=(p4x-EX)*DX+(p4y-EY)*DY+(p4z-EZ)*DZ-nearp;

   if (d1<0.0f) flag|=1;
   if (d2<0.0f) flag|=2;
   if (d3<0.0f) flag|=4;
   if (d4<0.0f) flag|=8;

   switch (flag)
      {
      case 1: case 14: cliptetra1(p1x,p1y,p1z,fabs(d1),p2x,p2y,p2z,fabs(d2),p3x,p3y,p3z,fabs(d3),p4x,p4y,p4z,fabs(d4)); break;
      case 2: case 13: cliptetra1(p2x,p2y,p2z,fabs(d2),p1x,p1y,p1z,fabs(d1),p3x,p3y,p3z,fabs(d3),p4x,p4y,p4z,fabs(d4)); break;
      case 4: case 11: cliptetra1(p3x,p3y,p3z,fabs(d3),p1x,p1y,p1z,fabs(d1),p2x,p2y,p2z,fabs(d2),p4x,p4y,p4z,fabs(d4)); break;
      case 8: case 7: cliptetra1(p4x,p4y,p4z,fabs(d4),p1x,p1y,p1z,fabs(d1),p2x,p2y,p2z,fabs(d2),p3x,p3y,p3z,fabs(d3)); break;

      case 3: cliptetra2(p1x,p1y,p1z,fabs(d1),p2x,p2y,p2z,fabs(d2),p3x,p3y,p3z,fabs(d3),p4x,p4y,p4z,fabs(d4)); break;
      case 5: cliptetra2(p1x,p1y,p1z,fabs(d1),p3x,p3y,p3z,fabs(d3),p2x,p2y,p2z,fabs(d2),p4x,p4y,p4z,fabs(d4)); break;
      case 6: cliptetra2(p2x,p2y,p2z,fabs(d2),p3x,p3y,p3z,fabs(d3),p1x,p1y,p1z,fabs(d1),p4x,p4y,p4z,fabs(d4)); break;
      case 9: cliptetra2(p1x,p1y,p1z,fabs(d1),p4x,p4y,p4z,fabs(d4),p2x,p2y,p2z,fabs(d2),p3x,p3y,p3z,fabs(d3)); break;
      case 10: cliptetra2(p2x,p2y,p2z,fabs(d2),p4x,p4y,p4z,fabs(d4),p1x,p1y,p1z,fabs(d1),p3x,p3y,p3z,fabs(d3)); break;
      case 12: cliptetra2(p3x,p3y,p3z,fabs(d3),p4x,p4y,p4z,fabs(d4),p1x,p1y,p1z,fabs(d1),p2x,p2y,p2z,fabs(d2)); break;
      }
   }

void drawbase(const float p1x,const float p1y,const float p1z,
              const float p2x,const float p2y,const float p2z,
              const float p3x,const float p3y,const float p3z,
              const float p4x,const float p4y,const float p4z,
              const float p5x,const float p5y,const float p5z,
              const float p6x,const float p6y,const float p6z,
              const float alpha)
   {
   disableculling();

   beginfans();
   color(0.0f,0.0f,0.0f,alpha);
   cliptetra(p1x,p1y,p1z,p2x,p2y,p2z,p3x,p3y,p3z,p4x,p4y,p4z);
   cliptetra(p2x,p2y,p2z,p3x,p3y,p3z,p4x,p4y,p4z,p5x,p5y,p5z);
   cliptetra(p3x,p3y,p3z,p4x,p4y,p4z,p5x,p5y,p5z,p6x,p6y,p6z);
   endfans();

   enableBFculling();
   }

// render the cached prisms using an emissive optical model
// requires an alpha channel in the frame buffer
void drawfog(float *prismcache,int prismcnt)
   {
   int i;

   float *prismptr;

   float p1x,p2x,p3x,p4x,p5x,p6x;
   float p1y,p2y,p3y,p4y,p5y,p6y;
   float p1z,p2z,p3z,p4z,p5z,p6z;

   float d1,d2,d3,d4,d5,d6,mind,maxd;
   float emission;

   for (i=0; i<prismcnt; i++)
      {
      enableAwriting();

      prismptr=&prismcache[12*i];

      // fetch vertices
      p1x=p4x=*prismptr++;
      p1y=*prismptr++;
      p4y=*prismptr++;
      p1z=p4z=*prismptr++;
      p2x=p5x=*prismptr++;
      p2y=*prismptr++;
      p5y=*prismptr++;
      p2z=p5z=*prismptr++;
      p3x=p6x=*prismptr++;
      p3y=*prismptr++;
      p6y=*prismptr++;
      p3z=p6z=*prismptr;

      // calculate distances to base plane
      d1=(p1x-EX)*DX+(p1y-EY)*DY+(p1z-EZ)*DZ;
      d2=(p2x-EX)*DX+(p2y-EY)*DY+(p2z-EZ)*DZ;
      d3=(p3x-EX)*DX+(p3y-EY)*DY+(p3z-EZ)*DZ;
      d4=(p4x-EX)*DX+(p4y-EY)*DY+(p4z-EZ)*DZ;
      d5=(p5x-EX)*DX+(p5y-EY)*DY+(p5z-EZ)*DZ;
      d6=(p6x-EX)*DX+(p6y-EY)*DY+(p6z-EZ)*DZ;

      // compute distance range
      mind=maxd=d1;
      if (d2<mind) mind=d2;
      else if (d2>maxd) maxd=d2;
      if (d3<mind) mind=d3;
      else if (d3>maxd) maxd=d3;
      if (d4<mind) mind=d4;
      else if (d4>maxd) maxd=d4;
      if (d5<mind) mind=d5;
      else if (d5>maxd) maxd=d5;
      if (d6<mind) mind=d6;
      else if (d6>maxd) maxd=d6;

      if (maxd<=NEARP) continue; // near plane culling

      // normalize distances
      maxd-=mind;
      d1=(d1-mind)/maxd;
      d2=(d2-mind)/maxd;
      d3=(d3-mind)/maxd;
      d4=(d4-mind)/maxd;
      d5=(d5-mind)/maxd;
      d6=(d6-mind)/maxd;

      // render back faces
      // -> dst_alpha=1-distance
      beginfans();
      beginfan();
      color(0.0f,0.0f,0.0f,1.0f-d1);
      fanvertex(p1x,p1y,p1z);
      color(0.0f,0.0f,0.0f,1.0f-d3);
      fanvertex(p3x,p3y,p3z);
      color(0.0f,0.0f,0.0f,1.0f-d4);
      fanvertex(p4x,p4y,p4z);
      color(0.0f,0.0f,0.0f,1.0f-d5);
      fanvertex(p5x,p5y,p5z);
      color(0.0f,0.0f,0.0f,1.0f-d2);
      fanvertex(p2x,p2y,p2z);
      color(0.0f,0.0f,0.0f,1.0f-d3);
      fanvertex(p3x,p3y,p3z);
      beginfan();
      color(0.0f,0.0f,0.0f,1.0f-d6);
      fanvertex(p6x,p6y,p6z);
      color(0.0f,0.0f,0.0f,1.0f-d3);
      fanvertex(p3x,p3y,p3z);
      color(0.0f,0.0f,0.0f,1.0f-d2);
      fanvertex(p2x,p2y,p2z);
      color(0.0f,0.0f,0.0f,1.0f-d5);
      fanvertex(p5x,p5y,p5z);
      color(0.0f,0.0f,0.0f,1.0f-d4);
      fanvertex(p4x,p4y,p4z);
      color(0.0f,0.0f,0.0f,1.0f-d3);
      fanvertex(p3x,p3y,p3z);
      endfans();

      enableADDblending();

      // blend front faces
      // -> dst_alpha=1-segment_length
      beginfans();
      beginfan();
      color(0.0f,0.0f,0.0f,d1);
      fanvertex(p1x,p1y,p1z);
      color(0.0f,0.0f,0.0f,d3);
      fanvertex(p3x,p3y,p3z);
      color(0.0f,0.0f,0.0f,d2);
      fanvertex(p2x,p2y,p2z);
      color(0.0f,0.0f,0.0f,d5);
      fanvertex(p5x,p5y,p5z);
      color(0.0f,0.0f,0.0f,d4);
      fanvertex(p4x,p4y,p4z);
      color(0.0f,0.0f,0.0f,d3);
      fanvertex(p3x,p3y,p3z);
      beginfan();
      color(0.0f,0.0f,0.0f,d6);
      fanvertex(p6x,p6y,p6z);
      color(0.0f,0.0f,0.0f,d3);
      fanvertex(p3x,p3y,p3z);
      color(0.0f,0.0f,0.0f,d4);
      fanvertex(p4x,p4y,p4z);
      color(0.0f,0.0f,0.0f,d5);
      fanvertex(p5x,p5y,p5z);
      color(0.0f,0.0f,0.0f,d2);
      fanvertex(p2x,p2y,p2z);
      color(0.0f,0.0f,0.0f,d3);
      fanvertex(p3x,p3y,p3z);
      endfans();

      // treat clipped parts behind the near plane
      if (mind<NEARP) drawbase(p1x,p1y,p1z,p2x,p2y,p2z,p3x,p3y,p3z,
                               p4x,p4y,p4z,p5x,p5y,p5z,p6x,p6y,p6z,
                               (NEARP-mind)/maxd);

      enablePRJblending();
      enableRGBwriting();

      emission=EMISSION*maxd;

      // blend back faces
      // -> dst_color+=segment_length*emission*fogcolor
      beginfans();
      color(emission*FR,emission*FG,emission*FB);
      beginfan();
      fanvertex(p1x,p1y,p1z);
      fanvertex(p3x,p3y,p3z);
      fanvertex(p4x,p4y,p4z);
      fanvertex(p5x,p5y,p5z);
      fanvertex(p2x,p2y,p2z);
      fanvertex(p3x,p3y,p3z);
      beginfan();
      fanvertex(p6x,p6y,p6z);
      fanvertex(p3x,p3y,p3z);
      fanvertex(p2x,p2y,p2z);
      fanvertex(p5x,p5y,p5z);
      fanvertex(p4x,p4y,p4z);
      fanvertex(p3x,p3y,p3z);
      endfans();

      disableblending();
      }
   }

// prism clipping functions:

inline void cliptetra1(const float p1x,const float p1y,const float p1z,const float d1,const float c1,
                       const float p2x,const float p2y,const float p2z,const float d2,const float c2,
                       const float p3x,const float p3y,const float p3z,const float d3,const float c3,
                       const float p4x,const float p4y,const float p4z,const float d4,const float c4)
   {
   float pp1x,pp1y,pp1z,pc1,
         pp2x,pp2y,pp2z,pc2,
         pp3x,pp3y,pp3z,pc3;

   pp1x=(d2*p1x+d1*p2x)/(d1+d2);
   pp1y=(d2*p1y+d1*p2y)/(d1+d2);
   pp1z=(d2*p1z+d1*p2z)/(d1+d2);
   pc1=(d2*c1+d1*c2)/(d1+d2);
   pp2x=(d3*p1x+d1*p3x)/(d1+d3);
   pp2y=(d3*p1y+d1*p3y)/(d1+d3);
   pp2z=(d3*p1z+d1*p3z)/(d1+d3);
   pc2=(d3*c1+d1*c3)/(d1+d3);
   pp3x=(d4*p1x+d1*p4x)/(d1+d4);
   pp3y=(d4*p1y+d1*p4y)/(d1+d4);
   pp3z=(d4*p1z+d1*p4z)/(d1+d4);
   pc3=(d4*c1+d1*c4)/(d1+d4);

   beginfan();
   color(pc1*FR,pc1*FG,pc1*FB);
   fanvertex(pp1x,pp1y,pp1z);
   color(pc2*FR,pc2*FG,pc2*FB);
   fanvertex(pp2x,pp2y,pp2z);
   color(pc3*FR,pc3*FG,pc3*FB);
   fanvertex(pp3x,pp3y,pp3z);
   beginfan();
   color(pc1*FR,pc1*FG,pc1*FB);
   fanvertex(pp1x,pp1y,pp1z);
   color(pc3*FR,pc3*FG,pc3*FB);
   fanvertex(pp3x,pp3y,pp3z);
   color(pc2*FR,pc2*FG,pc2*FB);
   fanvertex(pp2x,pp2y,pp2z);
   }

inline void cliptetra2(const float p1x,const float p1y,const float p1z,const float d1,const float c1,
                       const float p2x,const float p2y,const float p2z,const float d2,const float c2,
                       const float p3x,const float p3y,const float p3z,const float d3,const float c3,
                       const float p4x,const float p4y,const float p4z,const float d4,const float c4)
   {
   float pp1x,pp1y,pp1z,pc1,
         pp2x,pp2y,pp2z,pc2,
         pp3x,pp3y,pp3z,pc3,
         pp4x,pp4y,pp4z,pc4;

   pp1x=(d3*p1x+d1*p3x)/(d1+d3);
   pp1y=(d3*p1y+d1*p3y)/(d1+d3);
   pp1z=(d3*p1z+d1*p3z)/(d1+d3);
   pc1=(d3*c1+d1*c3)/(d1+d3);
   pp2x=(d3*p2x+d2*p3x)/(d2+d3);
   pp2y=(d3*p2y+d2*p3y)/(d2+d3);
   pp2z=(d3*p2z+d2*p3z)/(d2+d3);
   pc2=(d3*c2+d2*c3)/(d2+d3);
   pp3x=(d4*p1x+d1*p4x)/(d1+d4);
   pp3y=(d4*p1y+d1*p4y)/(d1+d4);
   pp3z=(d4*p1z+d1*p4z)/(d1+d4);
   pc3=(d4*c1+d1*c4)/(d1+d4);
   pp4x=(d4*p2x+d2*p4x)/(d2+d4);
   pp4y=(d4*p2y+d2*p4y)/(d2+d4);
   pp4z=(d4*p2z+d2*p4z)/(d2+d4);
   pc4=(d4*c2+d2*c4)/(d2+d4);

   beginfan();
   color(pc1*FR,pc1*FG,pc1*FB);
   fanvertex(pp1x,pp1y,pp1z);
   color(pc2*FR,pc2*FG,pc2*FB);
   fanvertex(pp2x,pp2y,pp2z);
   color(pc4*FR,pc4*FG,pc4*FB);
   fanvertex(pp4x,pp4y,pp4z);
   color(pc3*FR,pc3*FG,pc3*FB);
   fanvertex(pp3x,pp3y,pp3z);
   beginfan();
   color(pc1*FR,pc1*FG,pc1*FB);
   fanvertex(pp1x,pp1y,pp1z);
   color(pc3*FR,pc3*FG,pc3*FB);
   fanvertex(pp3x,pp3y,pp3z);
   color(pc4*FR,pc4*FG,pc4*FB);
   fanvertex(pp4x,pp4y,pp4z);
   color(pc2*FR,pc2*FG,pc2*FB);
   fanvertex(pp2x,pp2y,pp2z);
   }

inline void cliptetra(const float p1x,const float p1y,const float p1z,const float c1,
                      const float p2x,const float p2y,const float p2z,const float c2,
                      const float p3x,const float p3y,const float p3z,const float c3,
                      const float p4x,const float p4y,const float p4z,const float c4)
   {
   int flag=0;

   float d1,d2,d3,d4;

   float nearp=1.0001f*NEARP;

   d1=(p1x-EX)*DX+(p1y-EY)*DY+(p1z-EZ)*DZ-nearp;
   d2=(p2x-EX)*DX+(p2y-EY)*DY+(p2z-EZ)*DZ-nearp;
   d3=(p3x-EX)*DX+(p3y-EY)*DY+(p3z-EZ)*DZ-nearp;
   d4=(p4x-EX)*DX+(p4y-EY)*DY+(p4z-EZ)*DZ-nearp;

   if (d1<0.0f) flag|=1;
   if (d2<0.0f) flag|=2;
   if (d3<0.0f) flag|=4;
   if (d4<0.0f) flag|=8;

   switch (flag)
      {
      case 1: case 14: cliptetra1(p1x,p1y,p1z,fabs(d1),c1,p2x,p2y,p2z,fabs(d2),c2,p3x,p3y,p3z,fabs(d3),c3,p4x,p4y,p4z,fabs(d4),c4); break;
      case 2: case 13: cliptetra1(p2x,p2y,p2z,fabs(d2),c2,p1x,p1y,p1z,fabs(d1),c1,p3x,p3y,p3z,fabs(d3),c3,p4x,p4y,p4z,fabs(d4),c4); break;
      case 4: case 11: cliptetra1(p3x,p3y,p3z,fabs(d3),c3,p1x,p1y,p1z,fabs(d1),c1,p2x,p2y,p2z,fabs(d2),c2,p4x,p4y,p4z,fabs(d4),c4); break;
      case 8: case 7: cliptetra1(p4x,p4y,p4z,fabs(d4),c4,p1x,p1y,p1z,fabs(d1),c1,p2x,p2y,p2z,fabs(d2),c2,p3x,p3y,p3z,fabs(d3),c3); break;

      case 3: cliptetra2(p1x,p1y,p1z,fabs(d1),c1,p2x,p2y,p2z,fabs(d2),c2,p3x,p3y,p3z,fabs(d3),c3,p4x,p4y,p4z,fabs(d4),c4); break;
      case 5: cliptetra2(p1x,p1y,p1z,fabs(d1),c1,p3x,p3y,p3z,fabs(d3),c3,p2x,p2y,p2z,fabs(d2),c2,p4x,p4y,p4z,fabs(d4),c4); break;
      case 6: cliptetra2(p2x,p2y,p2z,fabs(d2),c2,p3x,p3y,p3z,fabs(d3),c3,p1x,p1y,p1z,fabs(d1),c1,p4x,p4y,p4z,fabs(d4),c4); break;
      case 9: cliptetra2(p1x,p1y,p1z,fabs(d1),c1,p4x,p4y,p4z,fabs(d4),c4,p2x,p2y,p2z,fabs(d2),c2,p3x,p3y,p3z,fabs(d3),c3); break;
      case 10: cliptetra2(p2x,p2y,p2z,fabs(d2),c2,p4x,p4y,p4z,fabs(d4),c4,p1x,p1y,p1z,fabs(d1),c1,p3x,p3y,p3z,fabs(d3),c3); break;
      case 12: cliptetra2(p3x,p3y,p3z,fabs(d3),c3,p4x,p4y,p4z,fabs(d4),c4,p1x,p1y,p1z,fabs(d1),c1,p2x,p2y,p2z,fabs(d2),c2); break;
      }
   }

// render the cached prisms using maximum intensity projection
void mipfog(float *prismcache,int prismcnt)
   {
   int i;

   float *prismptr;

   float p1x,p2x,p3x,p4x,p5x,p6x;
   float p1y,p2y,p3y,p4y,p5y,p6y;
   float p1z,p2z,p3z,p4z,p5z,p6z;

   float c1,c2,c3,c4,c5,c6;

   enableMIPblending();

   beginfans();
   for (i=0; i<prismcnt; i++)
      {
      prismptr=&prismcache[12*i];

      // fetch vertices
      p1x=p4x=*prismptr++;
      p1y=*prismptr++;
      p4y=*prismptr++;
      p1z=p4z=*prismptr++;
      p2x=p5x=*prismptr++;
      p2y=*prismptr++;
      p5y=*prismptr++;
      p2z=p5z=*prismptr++;
      p3x=p6x=*prismptr++;
      p3y=*prismptr++;
      p6y=*prismptr++;
      p3z=p6z=*prismptr;

      // calculate vertex colors
      if (tid!=0)
         {
         c1=(p4y-p1y)/(255.0f*LAMBDA);
         c2=(p5y-p2y)/(255.0f*LAMBDA);
         c3=(p6y-p3y)/(255.0f*LAMBDA);
         c4=c5=c6=0.0f;
         }
      else
         {
         c1=c2=c3=0.0f;
         c4=(p4y-p1y)/(255.0f*LAMBDA);
         c5=(p5y-p2y)/(255.0f*LAMBDA);
         c6=(p6y-p3y)/(255.0f*LAMBDA);
         }

      // render faces
      beginfan();
      color(c1*FR,c1*FG,c1*FB);
      fanvertex(p1x,p1y,p1z);
      color(c3*FR,c3*FG,c3*FB);
      fanvertex(p3x,p3y,p3z);
      color(c4*FR,c4*FG,c4*FB);
      fanvertex(p4x,p4y,p4z);
      color(c5*FR,c5*FG,c5*FB);
      fanvertex(p5x,p5y,p5z);
      color(c2*FR,c2*FG,c2*FB);
      fanvertex(p2x,p2y,p2z);
      color(c3*FR,c3*FG,c3*FB);
      fanvertex(p3x,p3y,p3z);
      color(c2*FR,c2*FG,c2*FB);
      fanvertex(p2x,p2y,p2z);
      beginfan();
      color(c6*FR,c6*FG,c6*FB);
      fanvertex(p6x,p6y,p6z);
      color(c4*FR,c4*FG,c4*FB);
      fanvertex(p4x,p4y,p4z);
      color(c3*FR,c3*FG,c3*FB);
      fanvertex(p3x,p3y,p3z);
      color(c2*FR,c2*FG,c2*FB);
      fanvertex(p2x,p2y,p2z);
      color(c5*FR,c5*FG,c5*FB);
      fanvertex(p5x,p5y,p5z);
      color(c4*FR,c4*FG,c4*FB);
      fanvertex(p4x,p4y,p4z);
      color(c5*FR,c5*FG,c5*FB);
      fanvertex(p5x,p5y,p5z);

      cliptetra(p1x,p1y,p1z,c1,p2x,p2y,p2z,c2,p3x,p3y,p3z,c3,p4x,p4y,p4z,c4);
      cliptetra(p2x,p2y,p2z,c2,p3x,p3y,p3z,c3,p4x,p4y,p4z,c4,p5x,p5y,p5z,c5);
      cliptetra(p3x,p3y,p3z,c3,p4x,p4y,p4z,c4,p5x,p5y,p5z,c5,p6x,p6y,p6z,c6);
      }
   endfans();

   disableblending();
   }

// fog blending functions:

inline float blendFD(const int i,const int j,const float yf1,const float yf2)
   {return(blendE(bc[i][j],yf[i][j],yf1,yf2));}

inline float blendFV(const int i,const int j,const int s2,const float yf1,const float yf2)
   {return(blendM(min((i>0)?bc[i-s2][j]:maxbc(0,S-1-s2,j,s2),
                      (i<S-1)?bc[i+s2][j]:maxbc(1,s2,j,s2)),yf[i][j],yf1,yf2));}

inline float blendFH(const int i,const int j,const int s2,const float yf1,const float yf2)
   {return(blendM(min((j>0)?bc[i][j-s2]:maxbc(2,i,S-1-s2,s2),
                      (j<S-1)?bc[i][j+s2]:maxbc(3,i,s2,s2)),yf[i][j],yf1,yf2));}

// geomorph the triangulation and the prisms
void drawmap(const int i,const int j,const int s,
             const float e1,const float e2,const float e3,const float e4,
             const float f1,const float f2,const float f3,const float f4)
   {
   int s2,s4;

   float dx,dy,dz;

   float dh;

   float l,d;

   float m0,m1,m2,m3,m4,
         mf0,mf1,mf2,mf3,mf4;

   int bc1,bc2,bc3,bc4;

   s2=s/2;
   s4=s/4;

   // blend the center vertex
   if (((i+j)/s)%2==1) {m0=blendD(i,j,e1,e3); mf0=blendFD(i,j,f1,f3);}
   else {m0=blendD(i,j,e2,e4); mf0=blendFD(i,j,f2,f4);}

   // view frustum culling
   if (CULLING && s>(S>>maxcull))
      {
      dx=X(i)-EX;
      dy=m0*SCALE-EY;
      dz=Z(j)-EZ;

      dh=(unsigned short int)DH[s];

      l=DX*dx+DY*dy+DZ*dz;
      d=k1*s2+k2*dh;

      if (l<NEARP-d || l>FARP+d) return;

      if (!ORTHO)
         {
         if (nx1*dx+ny1*dy+nz1*dz>k11*s2+k12*dh) return;
         if (nx2*dx+ny2*dy+nz2*dz>k21*s2+k22*dh) return;
         if (nx3*dx+ny3*dy+nz3*dz>k31*s2+k32*dh) return;
         if (nx4*dx+ny4*dy+nz4*dz>k41*s2+k42*dh) return;
         }
      else
         {
         if (fabs(RX*dx+RY*dy+RZ*dz)>k11*s2+k12*dh+k31) return;
         if (fabs(UX*dx+UY*dy+UZ*dz)>k21*s2+k22*dh+k32) return;
         }
      }

   // blend the edge vertices
   m1=blendV(i+s2,j,s2,e4,e1);
   mf1=blendFV(i+s2,j,s2,f4,f1);
   m2=blendH(i,j+s2,s2,e1,e2);
   mf2=blendFH(i,j+s2,s2,f1,f2);
   m3=blendV(i-s2,j,s2,e2,e3);
   mf3=blendFV(i-s2,j,s2,f2,f3);
   m4=blendH(i,j-s2,s2,e3,e4);
   mf4=blendFH(i,j-s2,s2,f3,f4);

   // subdivision factors
   if (s4>0)
      {
      bc1=bc[i+s4][j+s4];
      bc2=bc[i-s4][j+s4];
      bc3=bc[i-s4][j-s4];
      bc4=bc[i+s4][j-s4];
      }
   else bc1=bc2=bc3=bc4=0;

   // first triangle fan quarter
   if (bc1!=0)
      {
      if (m1==-MAXFLOAT) {m1=(e4+e1)/2.0f; mf1=(f4+f1)/2.0f;}
      if (m2==-MAXFLOAT) {m2=(e1+e2)/2.0f; mf2=(f1+f2)/2.0f;}
      drawmap(i+s4,j+s4,s2,e1,m2,m0,m1,f1,mf2,mf0,mf1);
      }
   else
      {
      BEGINFAN();
      FANVERTEX(i,m0,mf0,j);
      if (bc4!=0)
         {
         if (m1==-MAXFLOAT) {m1=(e4+e1)/2.0f; mf1=(f4+f1)/2.0f;}
         FANVERTEX(i+s2,m1,mf1,j);
         }
      else if (m1!=-MAXFLOAT) FANVERTEX(i+s2,m1,mf1,j);
      FANVERTEX(i+s2,e1,f1,j+s2);
      }

   // second triangle fan quarter
   if (bc2!=0)
      {
      if (bc1==0)
         {
         if (m2==-MAXFLOAT) {m2=(e1+e2)/2.0f; mf2=(f1+f2)/2.0f;}
         FANVERTEX(i,m2,mf2,j+s2);
         }
      if (m3==-MAXFLOAT) {m3=(e2+e3)/2.0f; mf3=(f2+f3)/2.0f;}
      drawmap(i-s4,j+s4,s2,m2,e2,m3,m0,mf2,f2,mf3,mf0);
      }
   else
      {
      if (bc1!=0)
         {
         BEGINFAN();
         FANVERTEX(i,m0,mf0,j);
         FANVERTEX(i,m2,mf2,j+s2);
         }
      else if (m2!=-MAXFLOAT) FANVERTEX(i,m2,mf2,j+s2);
      FANVERTEX(i-s2,e2,f2,j+s2);
      }

   // third triangle fan quarter
   if (bc3!=0)
      {
      if (bc2==0)
         {
         if (m3==-MAXFLOAT) {m3=(e2+e3)/2.0f; mf3=(f2+f3)/2.0f;}
         FANVERTEX(i-s2,m3,mf3,j);
         }
      if (m4==-MAXFLOAT) {m4=(e3+e4)/2.0f; mf4=(f3+f4)/2.0f;}
      drawmap(i-s4,j-s4,s2,m0,m3,e3,m4,mf0,mf3,f3,mf4);
      }
   else
      {
      if (bc2!=0)
         {
         BEGINFAN();
         FANVERTEX(i,m0,mf0,j);
         FANVERTEX(i-s2,m3,mf3,j);
         }
      else if (m3!=-MAXFLOAT) FANVERTEX(i-s2,m3,mf3,j);
      FANVERTEX(i-s2,e3,f3,j-s2);
      }

   // fourth triangle fan quarter
   if (bc4!=0)
      {
      if (bc3==0)
         {
         if (m4==-MAXFLOAT) {m4=(e3+e4)/2.0f; mf4=(f3+f4)/2.0f;}
         FANVERTEX(i,m4,mf4,j-s2);
         }
      drawmap(i+s4,j-s4,s2,m1,m0,m4,e4,mf1,mf0,mf4,f4);
      }
   else
      {
      if (bc3!=0)
         {
         BEGINFAN();
         FANVERTEX(i,m0,mf0,j);
         FANVERTEX(i,m4,mf4,j-s2);
         }
      else if (m4!=-MAXFLOAT) FANVERTEX(i,m4,mf4,j-s2);
      FANVERTEX(i+s2,e4,f4,j-s2);
      if (bc1==0)
         if (m1!=-MAXFLOAT) FANVERTEX(i+s2,m1,mf1,j);
         else FANVERTEX(i+s2,e1,f1,j+s2);
      else FANVERTEX(i+s2,m1,mf1,j);
      }
   }

// geomorph and pipe the triangulation
void pipemap(const int i,const int j,const int s,
             const float e1,const float e2,const float e3,const float e4)
   {
   int s2,s4;

   float dx,dy,dz;

   float dh;

   float l,d;

   float m0,m1,m2,m3,m4;

   int bc1,bc2,bc3,bc4;

   s2=s/2;
   s4=s/4;

   // blend the center vertex
   if (((i+j)/s)%2==1) m0=blendD(i,j,e1,e3);
   else m0=blendD(i,j,e2,e4);

   // view frustum culling
   if (CULLING && s>(S>>maxcull))
      {
      dx=X(i)-EX;
      dy=m0*SCALE-EY;
      dz=Z(j)-EZ;

      dh=(unsigned short int)DH[s];

      l=DX*dx+DY*dy+DZ*dz;
      d=k1*s2+k2*dh;

      if (l<NEARP-d || l>FARP+d) return;

      if (!ORTHO)
         {
         if (nx1*dx+ny1*dy+nz1*dz>k11*s2+k12*dh) return;
         if (nx2*dx+ny2*dy+nz2*dz>k21*s2+k22*dh) return;
         if (nx3*dx+ny3*dy+nz3*dz>k31*s2+k32*dh) return;
         if (nx4*dx+ny4*dy+nz4*dz>k41*s2+k42*dh) return;
         }
      else
         {
         if (fabs(RX*dx+RY*dy+RZ*dz)>k11*s2+k12*dh+k31) return;
         if (fabs(UX*dx+UY*dy+UZ*dz)>k21*s2+k22*dh+k32) return;
         }
      }

   // trigger the notify callback
   if (notify_callback!=NULL) notify_callback(i,j,s);

   // blend the edge vertices
   m1=blendV(i+s2,j,s2,e4,e1);
   m2=blendH(i,j+s2,s2,e1,e2);
   m3=blendV(i-s2,j,s2,e2,e3);
   m4=blendH(i,j-s2,s2,e3,e4);

   // subdivision factors
   if (s4>0)
      {
      bc1=bc[i+s4][j+s4];
      bc2=bc[i-s4][j+s4];
      bc3=bc[i-s4][j-s4];
      bc4=bc[i+s4][j-s4];
      }
   else bc1=bc2=bc3=bc4=0;

   // first triangle fan quarter
   if (bc1!=0)
      {
      if (m1==-MAXFLOAT) m1=(e4+e1)/2.0f;
      if (m2==-MAXFLOAT) m2=(e1+e2)/2.0f;
      pipemap(i+s4,j+s4,s2,e1,m2,m0,m1);
      }
   else
      {
      beginfan_callback();
      fanvertex_callback(i,m0,j);
      if (bc4!=0)
         {
         if (m1==-MAXFLOAT) m1=(e4+e1)/2.0f;
         fanvertex_callback(i+s2,m1,j);
         }
      else if (m1!=-MAXFLOAT) fanvertex_callback(i+s2,m1,j);
      fanvertex_callback(i+s2,e1,j+s2);
      }

   // second triangle fan quarter
   if (bc2!=0)
      {
      if (bc1==0)
         {
         if (m2==-MAXFLOAT) m2=(e1+e2)/2.0f;
         fanvertex_callback(i,m2,j+s2);
         }
      if (m3==-MAXFLOAT) m3=(e2+e3)/2.0f;
      pipemap(i-s4,j+s4,s2,m2,e2,m3,m0);
      }
   else
      {
      if (bc1!=0)
         {
         beginfan_callback();
         fanvertex_callback(i,m0,j);
         fanvertex_callback(i,m2,j+s2);
         }
      else if (m2!=-MAXFLOAT) fanvertex_callback(i,m2,j+s2);
      fanvertex_callback(i-s2,e2,j+s2);
      }

   // third triangle fan quarter
   if (bc3!=0)
      {
      if (bc2==0)
         {
         if (m3==-MAXFLOAT) m3=(e2+e3)/2.0f;
         fanvertex_callback(i-s2,m3,j);
         }
      if (m4==-MAXFLOAT) m4=(e3+e4)/2.0f;
      pipemap(i-s4,j-s4,s2,m0,m3,e3,m4);
      }
   else
      {
      if (bc2!=0)
         {
         beginfan_callback();
         fanvertex_callback(i,m0,j);
         fanvertex_callback(i-s2,m3,j);
         }
      else if (m3!=-MAXFLOAT) fanvertex_callback(i-s2,m3,j);
      fanvertex_callback(i-s2,e3,j-s2);
      }

   // fourth triangle fan quarter
   if (bc4!=0)
      {
      if (bc3==0)
         {
         if (m4==-MAXFLOAT) m4=(e3+e4)/2.0f;
         fanvertex_callback(i,m4,j-s2);
         }
      pipemap(i+s4,j-s4,s2,m1,m0,m4,e4);
      }
   else
      {
      if (bc3!=0)
         {
         beginfan_callback();
         fanvertex_callback(i,m0,j);
         fanvertex_callback(i,m4,j-s2);
         }
      else if (m4!=-MAXFLOAT) fanvertex_callback(i,m4,j-s2);
      fanvertex_callback(i+s2,e4,j-s2);
      if (bc1==0)
         if (m1!=-MAXFLOAT) fanvertex_callback(i+s2,m1,j);
         else fanvertex_callback(i+s2,e1,j+s2);
      else fanvertex_callback(i+s2,m1,j);
      }
   }

// sea piping functions:

inline void extractshore(const float i1,const float y1,const float j1,
                         const float i2,const float y2,const float j2,
                         const float i3,const float y3,const float j3)
   {
   float d1,d2;

   float im1,jm1,im2,jm2;

   d1=(y1-SEALEVEL)/(y1-y2);
   d2=(y1-SEALEVEL)/(y1-y3);

   im1=i1*(1.0f-d1)+i2*d1;
   jm1=j1*(1.0f-d1)+j2*d1;

   im2=i1*(1.0f-d2)+i3*d2;
   jm2=j1*(1.0f-d2)+j3*d2;

   if (y1<SEALEVEL)
      {
      beginfan_callback();
      fanvertex_callback(i1,SEALEVEL,j1);
      fanvertex_callback(im1,SEALEVEL,jm1);
      fanvertex_callback(im2,SEALEVEL,jm2);
      }
   else
      {
      beginfan_callback();
      fanvertex_callback(im2,SEALEVEL,jm2);
      fanvertex_callback(im1,SEALEVEL,jm1);
      fanvertex_callback(i2,SEALEVEL,j2);
      fanvertex_callback(i3,SEALEVEL,j3);
      }
   }

inline void rendershore(const float i1,const float y1,const float j1,
                        const float i2,const float y2,const float j2,
                        const float i3,const float y3,const float j3)
   {
   int flag=0;

   if (y1<SEALEVEL) flag|=1;
   if (y2<SEALEVEL) flag|=2;
   if (y3<SEALEVEL) flag|=4;

   switch (flag)
      {
      case 1:
      case 2+4:
         extractshore(i1,y1,j1,i2,y2,j2,i3,y3,j3);
         break;
      case 2:
      case 4+1:
         extractshore(i2,y2,j2,i3,y3,j3,i1,y1,j1);
         break;
      case 4:
      case 1+2:
         extractshore(i3,y3,j3,i1,y1,j1,i2,y2,j2);
         break;
      case 1+2+4:
         beginfan_callback();
         fanvertex_callback(i1,SEALEVEL,j1);
         fanvertex_callback(i2,SEALEVEL,j2);
         fanvertex_callback(i3,SEALEVEL,j3);
         break;
      }
   }

inline void BEGINFAN_SEA()
   {SEASTATE=0;}

inline void FANVERTEX_SEA(const int i,const float y,const int j)
   {
   static int i0,i1;
   static float y0,y1;
   static int j0,j1;

   switch (SEASTATE)
      {
      case 0:
         i0=i; y0=y; j0=j;
         SEASTATE++;
         break;
      case 1:
         i1=i; y1=y; j1=j;
         SEASTATE++;
         break;
      default:
         rendershore(i0,y0,j0,i1,y1,j1,i,y,j);
         i1=i; y1=y; j1=j;
         break;
      }
   }

// calculate and pipe sea level
void pipesea(const int i,const int j,const int s,
             const float e1,const float e2,const float e3,const float e4)
   {
   int s2,s4;

   float dx,dy,dz;

   float dh;

   float l,d;

   float m0,m1,m2,m3,m4;

   int bc1,bc2,bc3,bc4;

   BOOLINT below,above;

   s2=s/2;
   s4=s/4;

   // blend the center vertex
   if (((i+j)/s)%2==1) m0=blendD(i,j,e1,e3);
   else m0=blendD(i,j,e2,e4);

   // view frustum culling
   if (CULLING && s>(S>>maxcull))
      {
      dx=X(i)-EX;
      dy=SEALEVEL*SCALE-EY;
      dz=Z(j)-EZ;

      l=DX*dx+DY*dy+DZ*dz;
      d=k1*s2;

      if (l<NEARP-d || l>FARP+d) return;

      if (!ORTHO)
         {
         if (nx1*dx+ny1*dy+nz1*dz>k11*s2) return;
         if (nx2*dx+ny2*dy+nz2*dz>k21*s2) return;
         if (nx3*dx+ny3*dy+nz3*dz>k31*s2) return;
         if (nx4*dx+ny4*dy+nz4*dz>k41*s2) return;
         }
      else
         {
         if (fabs(RX*dx+RY*dy+RZ*dz)>k11*s2+k31) return;
         if (fabs(UX*dx+UY*dy+UZ*dz)>k21*s2+k32) return;
         }
      }

   // trigger the notify callback
   if (notify_callback!=NULL) notify_callback(i,j,s);

   // blend the edge vertices
   m1=blendV(i+s2,j,s2,e4,e1);
   m2=blendH(i,j+s2,s2,e1,e2);
   m3=blendV(i-s2,j,s2,e2,e3);
   m4=blendH(i,j-s2,s2,e3,e4);

   // subdivision factors
   if (s4>0)
      {
      bc1=bc[i+s4][j+s4];
      bc2=bc[i-s4][j+s4];
      bc3=bc[i-s4][j-s4];
      bc4=bc[i+s4][j-s4];
      }
   else bc1=bc2=bc3=bc4=0;

   dh=(unsigned short int)DH[s];

   below=above=FALSE;

   // check if triangles are below or above sea level
   if (m0+dh<SEALEVEL)
      {
      below=TRUE;
      bc1=bc2=bc3=bc4=0;
      }
   else if (m0-dh>SEALEVEL)
      {
      above=TRUE;
      bc1=bc2=bc3=bc4=0;
      }
   else
      if (e1<SEALEVEL && e2<SEALEVEL && e3<SEALEVEL && e4<SEALEVEL &&
          (m0<SEALEVEL || m0==-MAXFLOAT) &&
          (m1<SEALEVEL || m1==-MAXFLOAT) &&
          (m2<SEALEVEL || m2==-MAXFLOAT) &&
          (m3<SEALEVEL || m3==-MAXFLOAT) &&
          (m4<SEALEVEL || m4==-MAXFLOAT)) below=TRUE;
      else
         if (e1>SEALEVEL && e2>SEALEVEL && e3>SEALEVEL && e4>SEALEVEL &&
             (m0>SEALEVEL || m0==-MAXFLOAT) &&
             (m1>SEALEVEL || m1==-MAXFLOAT) &&
             (m2>SEALEVEL || m2==-MAXFLOAT) &&
             (m3>SEALEVEL || m3==-MAXFLOAT) &&
             (m4>SEALEVEL || m4==-MAXFLOAT)) above=TRUE;

   if (below)
      {
      // first triangle fan quarter
      if (bc1!=0)
         {
         if (m1==-MAXFLOAT) m1=(e4+e1)/2.0f;
         if (m2==-MAXFLOAT) m2=(e1+e2)/2.0f;
         pipesea(i+s4,j+s4,s2,e1,m2,m0,m1);
         }
      else
         {
         beginfan_callback();
         fanvertex_callback(i,SEALEVEL,j);
         if (bc4!=0)
            {
            if (m1==-MAXFLOAT) m1=(e4+e1)/2.0f;
            fanvertex_callback(i+s2,SEALEVEL,j);
            }
         else if (m1!=-MAXFLOAT) fanvertex_callback(i+s2,SEALEVEL,j);
         fanvertex_callback(i+s2,SEALEVEL,j+s2);
         }

      // second triangle fan quarter
      if (bc2!=0)
         {
         if (bc1==0)
            {
            if (m2==-MAXFLOAT) m2=(e1+e2)/2.0f;
            fanvertex_callback(i,SEALEVEL,j+s2);
            }
         if (m3==-MAXFLOAT) m3=(e2+e3)/2.0f;
         pipesea(i-s4,j+s4,s2,m2,e2,m3,m0);
         }
      else
         {
         if (bc1!=0)
            {
            beginfan_callback();
            fanvertex_callback(i,SEALEVEL,j);
            fanvertex_callback(i,SEALEVEL,j+s2);
            }
         else if (m2!=-MAXFLOAT) fanvertex_callback(i,SEALEVEL,j+s2);
         fanvertex_callback(i-s2,SEALEVEL,j+s2);
         }

      // third triangle fan quarter
      if (bc3!=0)
         {
         if (bc2==0)
            {
            if (m3==-MAXFLOAT) m3=(e2+e3)/2.0f;
            fanvertex_callback(i-s2,SEALEVEL,j);
            }
         if (m4==-MAXFLOAT) m4=(e3+e4)/2.0f;
         pipesea(i-s4,j-s4,s2,m0,m3,e3,m4);
         }
      else
         {
         if (bc2!=0)
            {
            beginfan_callback();
            fanvertex_callback(i,SEALEVEL,j);
            fanvertex_callback(i-s2,SEALEVEL,j);
            }
         else if (m3!=-MAXFLOAT) fanvertex_callback(i-s2,SEALEVEL,j);
         fanvertex_callback(i-s2,SEALEVEL,j-s2);
         }

      // fourth triangle fan quarter
      if (bc4!=0)
         {
         if (bc3==0)
            {
            if (m4==-MAXFLOAT) m4=(e3+e4)/2.0f;
            fanvertex_callback(i,SEALEVEL,j-s2);
            }
         pipesea(i+s4,j-s4,s2,m1,m0,m4,e4);
         }
      else
         {
         if (bc3!=0)
            {
            beginfan_callback();
            fanvertex_callback(i,SEALEVEL,j);
            fanvertex_callback(i,SEALEVEL,j-s2);
            }
         else if (m4!=-MAXFLOAT) fanvertex_callback(i,SEALEVEL,j-s2);
         fanvertex_callback(i+s2,SEALEVEL,j-s2);
         if (bc1==0)
            if (m1!=-MAXFLOAT) fanvertex_callback(i+s2,SEALEVEL,j);
            else fanvertex_callback(i+s2,SEALEVEL,j+s2);
         else fanvertex_callback(i+s2,SEALEVEL,j);
         }
      }
   else
      if (!above)
         {
         // first triangle fan quarter
         if (bc1!=0)
            {
            if (m1==-MAXFLOAT) m1=(e4+e1)/2.0f;
            if (m2==-MAXFLOAT) m2=(e1+e2)/2.0f;
            pipesea(i+s4,j+s4,s2,e1,m2,m0,m1);
            }
         else
            {
            BEGINFAN_SEA();
            FANVERTEX_SEA(i,m0,j);
            if (bc4!=0)
               {
               if (m1==-MAXFLOAT) m1=(e4+e1)/2.0f;
               FANVERTEX_SEA(i+s2,m1,j);
               }
            else if (m1!=-MAXFLOAT) FANVERTEX_SEA(i+s2,m1,j);
            FANVERTEX_SEA(i+s2,e1,j+s2);
            }

         // second triangle fan quarter
         if (bc2!=0)
            {
            if (bc1==0)
               {
               if (m2==-MAXFLOAT) m2=(e1+e2)/2.0f;
               FANVERTEX_SEA(i,m2,j+s2);
               }
            if (m3==-MAXFLOAT) m3=(e2+e3)/2.0f;
            pipesea(i-s4,j+s4,s2,m2,e2,m3,m0);
            }
         else
            {
            if (bc1!=0)
               {
               BEGINFAN_SEA();
               FANVERTEX_SEA(i,m0,j);
               FANVERTEX_SEA(i,m2,j+s2);
               }
            else if (m2!=-MAXFLOAT) FANVERTEX_SEA(i,m2,j+s2);
            FANVERTEX_SEA(i-s2,e2,j+s2);
            }

         // third triangle fan quarter
         if (bc3!=0)
            {
            if (bc2==0)
               {
               if (m3==-MAXFLOAT) m3=(e2+e3)/2.0f;
               FANVERTEX_SEA(i-s2,m3,j);
               }
            if (m4==-MAXFLOAT) m4=(e3+e4)/2.0f;
            pipesea(i-s4,j-s4,s2,m0,m3,e3,m4);
            }
         else
            {
            if (bc2!=0)
               {
               BEGINFAN_SEA();
               FANVERTEX_SEA(i,m0,j);
               FANVERTEX_SEA(i-s2,m3,j);
               }
            else if (m3!=-MAXFLOAT) FANVERTEX_SEA(i-s2,m3,j);
            FANVERTEX_SEA(i-s2,e3,j-s2);
            }

         // fourth triangle fan quarter
         if (bc4!=0)
            {
            if (bc3==0)
               {
               if (m4==-MAXFLOAT) m4=(e3+e4)/2.0f;
               FANVERTEX_SEA(i,m4,j-s2);
               }
            pipesea(i+s4,j-s4,s2,m1,m0,m4,e4);
            }
         else
            {
            if (bc3!=0)
               {
               BEGINFAN_SEA();
               FANVERTEX_SEA(i,m0,j);
               FANVERTEX_SEA(i,m4,j-s2);
               }
            else if (m4!=-MAXFLOAT) FANVERTEX_SEA(i,m4,j-s2);
            FANVERTEX_SEA(i+s2,e4,j-s2);
            if (bc1==0)
               if (m1!=-MAXFLOAT) FANVERTEX_SEA(i+s2,m1,j);
               else FANVERTEX_SEA(i+s2,e1,j+s2);
            else FANVERTEX_SEA(i+s2,m1,j);
            }
         }
      else
         {
         // first quarter
         if (bc1!=0)
            {
            if (m1==-MAXFLOAT) m1=(e4+e1)/2.0f;
            if (m2==-MAXFLOAT) m2=(e1+e2)/2.0f;
            pipesea(i+s4,j+s4,s2,e1,m2,m0,m1);
            }

         // second quarter
         if (bc2!=0)
            {
            if (m2==-MAXFLOAT) m2=(e1+e2)/2.0f;
            if (m3==-MAXFLOAT) m3=(e2+e3)/2.0f;
            pipesea(i-s4,j+s4,s2,m2,e2,m3,m0);
            }

         // third quarter
         if (bc3!=0)
            {
            if (m3==-MAXFLOAT) m3=(e2+e3)/2.0f;
            if (m4==-MAXFLOAT) m4=(e3+e4)/2.0f;
            pipesea(i-s4,j-s4,s2,m0,m3,e3,m4);
            }

         // fourth quarter
         if (bc4!=0)
            {
            if (m4==-MAXFLOAT) m4=(e3+e4)/2.0f;
            if (m1==-MAXFLOAT) m1=(e4+e1)/2.0f;
            pipesea(i+s4,j-s4,s2,m1,m0,m4,e4);
            }
         }
   }

// prism piping functions:

inline void BEGINFAN_PRISMS()
   {
   FANSTATE=0;
   beginfan_callback();
   }

inline void FANVERTEX_PRISMS(const int i,const float y,const float yf,const int j)
   {
   static int i0,i1;
   static float y0,y1;
   static float yf0,yf1;
   static int j0,j1;

   switch (FANSTATE)
      {
      case 0:
         fanvertex_callback(i,y,j);
         i0=i; y0=y; yf0=yf; j0=j;
         FANSTATE++;
         break;
      case 1:
         fanvertex_callback(i,y,j);
         i1=i; y1=y; yf1=yf; j1=j;
         FANSTATE++;
         break;
      default:
         fanvertex_callback(i,y,j);
         if (yf0>0.0f || yf1>0.0f || yf>0.0f) cacheprism(i0,y0,yf0,j0,i1,y1,yf1,j1,i,y,yf,j);
         i1=i; y1=y; yf1=yf; j1=j;
         break;
      }
   }

// geomorph and pipe the triangulation and the prisms
void pipemap(const int i,const int j,const int s,
             const float e1,const float e2,const float e3,const float e4,
             const float f1,const float f2,const float f3,const float f4)
   {
   int s2,s4;

   float dx,dy,dz;

   float dh;

   float l,d;

   float m0,m1,m2,m3,m4,
         mf0,mf1,mf2,mf3,mf4;

   int bc1,bc2,bc3,bc4;

   s2=s/2;
   s4=s/4;

   // blend the center vertex
   if (((i+j)/s)%2==1) {m0=blendD(i,j,e1,e3); mf0=blendFD(i,j,f1,f3);}
   else {m0=blendD(i,j,e2,e4); mf0=blendFD(i,j,f2,f4);}

   // view frustum culling
   if (CULLING && s>(S>>maxcull))
      {
      dx=X(i)-EX;
      dy=m0*SCALE-EY;
      dz=Z(j)-EZ;

      dh=(unsigned short int)DH[s];

      l=DX*dx+DY*dy+DZ*dz;
      d=k1*s2+k2*dh;

      if (l<NEARP-d || l>FARP+d) return;

      if (!ORTHO)
         {
         if (nx1*dx+ny1*dy+nz1*dz>k11*s2+k12*dh) return;
         if (nx2*dx+ny2*dy+nz2*dz>k21*s2+k22*dh) return;
         if (nx3*dx+ny3*dy+nz3*dz>k31*s2+k32*dh) return;
         if (nx4*dx+ny4*dy+nz4*dz>k41*s2+k42*dh) return;
         }
      else
         {
         if (fabs(RX*dx+RY*dy+RZ*dz)>k11*s2+k12*dh+k31) return;
         if (fabs(UX*dx+UY*dy+UZ*dz)>k21*s2+k22*dh+k32) return;
         }
      }

   // trigger the notify callback
   if (notify_callback!=NULL) notify_callback(i,j,s);

   // blend the edge vertices
   m1=blendV(i+s2,j,s2,e4,e1);
   mf1=blendFV(i+s2,j,s2,f4,f1);
   m2=blendH(i,j+s2,s2,e1,e2);
   mf2=blendFH(i,j+s2,s2,f1,f2);
   m3=blendV(i-s2,j,s2,e2,e3);
   mf3=blendFV(i-s2,j,s2,f2,f3);
   m4=blendH(i,j-s2,s2,e3,e4);
   mf4=blendFH(i,j-s2,s2,f3,f4);

   // subdivision factors
   if (s4>0)
      {
      bc1=bc[i+s4][j+s4];
      bc2=bc[i-s4][j+s4];
      bc3=bc[i-s4][j-s4];
      bc4=bc[i+s4][j-s4];
      }
   else bc1=bc2=bc3=bc4=0;

   // first triangle fan quarter
   if (bc1!=0)
      {
      if (m1==-MAXFLOAT) {m1=(e4+e1)/2.0f; mf1=(f4+f1)/2.0f;}
      if (m2==-MAXFLOAT) {m2=(e1+e2)/2.0f; mf2=(f1+f2)/2.0f;}
      pipemap(i+s4,j+s4,s2,e1,m2,m0,m1,f1,mf2,mf0,mf1);
      }
   else
      {
      BEGINFAN_PRISMS();
      FANVERTEX_PRISMS(i,m0,mf0,j);
      if (bc4!=0)
         {
         if (m1==-MAXFLOAT) {m1=(e4+e1)/2.0f; mf1=(f4+f1)/2.0f;}
         FANVERTEX_PRISMS(i+s2,m1,mf1,j);
         }
      else if (m1!=-MAXFLOAT) FANVERTEX_PRISMS(i+s2,m1,mf1,j);
      FANVERTEX_PRISMS(i+s2,e1,f1,j+s2);
      }

   // second triangle fan quarter
   if (bc2!=0)
      {
      if (bc1==0)
         {
         if (m2==-MAXFLOAT) {m2=(e1+e2)/2.0f; mf2=(f1+f2)/2.0f;}
         FANVERTEX_PRISMS(i,m2,mf2,j+s2);
         }
      if (m3==-MAXFLOAT) {m3=(e2+e3)/2.0f; mf3=(f2+f3)/2.0f;}
      pipemap(i-s4,j+s4,s2,m2,e2,m3,m0,mf2,f2,mf3,mf0);
      }
   else
      {
      if (bc1!=0)
         {
         BEGINFAN_PRISMS();
         FANVERTEX_PRISMS(i,m0,mf0,j);
         FANVERTEX_PRISMS(i,m2,mf2,j+s2);
         }
      else if (m2!=-MAXFLOAT) FANVERTEX_PRISMS(i,m2,mf2,j+s2);
      FANVERTEX_PRISMS(i-s2,e2,f2,j+s2);
      }

   // third triangle fan quarter
   if (bc3!=0)
      {
      if (bc2==0)
         {
         if (m3==-MAXFLOAT) {m3=(e2+e3)/2.0f; mf3=(f2+f3)/2.0f;}
         FANVERTEX_PRISMS(i-s2,m3,mf3,j);
         }
      if (m4==-MAXFLOAT) {m4=(e3+e4)/2.0f; mf4=(f3+f4)/2.0f;}
      pipemap(i-s4,j-s4,s2,m0,m3,e3,m4,mf0,mf3,f3,mf4);
      }
   else
      {
      if (bc2!=0)
         {
         BEGINFAN_PRISMS();
         FANVERTEX_PRISMS(i,m0,mf0,j);
         FANVERTEX_PRISMS(i-s2,m3,mf3,j);
         }
      else if (m3!=-MAXFLOAT) FANVERTEX_PRISMS(i-s2,m3,mf3,j);
      FANVERTEX_PRISMS(i-s2,e3,f3,j-s2);
      }

   // fourth triangle fan quarter
   if (bc4!=0)
      {
      if (bc3==0)
         {
         if (m4==-MAXFLOAT) {m4=(e3+e4)/2.0f; mf4=(f3+f4)/2.0f;}
         FANVERTEX_PRISMS(i,m4,mf4,j-s2);
         }
      pipemap(i+s4,j-s4,s2,m1,m0,m4,e4,mf1,mf0,mf4,f4);
      }
   else
      {
      if (bc3!=0)
         {
         BEGINFAN_PRISMS();
         FANVERTEX_PRISMS(i,m0,mf0,j);
         FANVERTEX_PRISMS(i,m4,mf4,j-s2);
         }
      else if (m4!=-MAXFLOAT) FANVERTEX_PRISMS(i,m4,mf4,j-s2);
      FANVERTEX_PRISMS(i+s2,e4,f4,j-s2);
      if (bc1==0)
         if (m1!=-MAXFLOAT) FANVERTEX_PRISMS(i+s2,m1,mf1,j);
         else FANVERTEX_PRISMS(i+s2,e1,f1,j+s2);
      else FANVERTEX_PRISMS(i+s2,m1,mf1,j);
      }
   }

// calculate the base elevation at position (x,z)
float getbaseheight(float x,float z)
   {
   if (x<X(0)) x=X(0);
   else if (x>X(S-1)) x=X(S-1);

   if (z<Z(S-1)) z=Z(S-1);
   else if (z>Z(0)) z=Z(0);

   return(getheight(x+OX,z+OZ)-OY);
   }

// triangulate and draw the landscape
void drawlandscape(float res,
                   float ex,float ey,float ez,
                   float fx,float fy,float fz,
                   float dx,float dy,float dz,
                   float ux,float uy,float uz,
                   float fovy,float aspect,
                   float nearp,float farp,
                   void (*beginfan)(),
                   void (*fanvertex)(float i,float y,float j),
                   void (*notify)(int i,int j,int s),
                   void (*prismedge)(float x,float y,float yf,float z),
                   int state)
   {
       std::cout<<"drawlandscape1"<<std::endl;
   float length;

   float rx,ry,rz;

   float baseoff;

   if (S==0) ERRORMSG();

   if (fovy>=180.0f || aspect<=0.0f ||
       nearp<0.0f || farp<=nearp) ERRORMSG();

   if (state!=SINGLEPHASE &&
       state!=CALCMAPPHASE &&
       state!=DRAWMAPPHASE &&
       state!=DRAWSEAPHASE) return;

   if ((length=fsqrt(dx*dx+dy*dy+dz*dz))==0.0f) ERRORMSG();
   dx/=length;
   dy/=length;
   dz/=length;

   c=fsqrt(fmax(res,0.0f))*maxd2;

   EX=ex-OX;
   EY=ey-OY;
   EZ=ez-OZ;

   FX=fx-OX;
   FY=fy-OY;
   FZ=fz-OZ;

   DX=dx;
   DY=dy;
   DZ=dz;

   NEARP=nearp;
   FARP=farp;

   k1=fabs(dx)*Dx+fabs(dz)*Dz;
   k2=fabs(dy)*SCALE;

   rx=uy*dz-dy*uz;
   ry=uz*dx-dz*ux;
   rz=ux*dy-dx*uy;

   ux=ry*dz-dy*rz;
   uy=rz*dx-dz*rx;
   uz=rx*dy-dx*ry;

   ORTHO=fovy<0.0f;
   CULLING=fovy!=0.0f;

   if (!ORTHO)
      {
      baseoff=fabs(FY-getbaseheight(FX,FZ));

      if (minoff>0.0f) baseoff=fmax(baseoff-minoff*minres*D,0.0f);

      DF2=fsqr(baseoff);

      if (CULLING)
         {
         if ((length=fsqrt(rx*rx+ry*ry+rz*rz)/ftan(fovy/2.0f*RAD)/aspect)==0.0f) ERRORMSG();
         rx/=length;
         ry/=length;
         rz/=length;

         if ((length=fsqrt(ux*ux+uy*uy+uz*uz)/ftan(fovy/2.0f*RAD))==0.0f) ERRORMSG();
         ux/=length;
         uy/=length;
         uz/=length;

         nx1=(dy+ry-uy)*(dz+rz+uz)-(dy+ry+uy)*(dz+rz-uz);
         ny1=(dz+rz-uz)*(dx+rx+ux)-(dz+rz+uz)*(dx+rx-ux);
         nz1=(dx+rx-ux)*(dy+ry+uy)-(dx+rx+ux)*(dy+ry-uy);

         if ((length=fsqrt(nx1*nx1+ny1*ny1+nz1*nz1))==0.0f) ERRORMSG();
         nx1/=length;
         ny1/=length;
         nz1/=length;

         k11=fabs(nx1)*Dx+fabs(nz1)*Dz;
         k12=fabs(ny1)*SCALE;

         nx2=(dy-ry+uy)*(dz-rz-uz)-(dy-ry-uy)*(dz-rz+uz);
         ny2=(dz-rz+uz)*(dx-rx-ux)-(dz-rz-uz)*(dx-rx+ux);
         nz2=(dx-rx+ux)*(dy-ry-uy)-(dx-rx-ux)*(dy-ry+uy);

         if ((length=fsqrt(nx2*nx2+ny2*ny2+nz2*nz2))==0.0f) ERRORMSG();
         nx2/=length;
         ny2/=length;
         nz2/=length;

         k21=fabs(nx2)*Dx+fabs(nz2)*Dz;
         k22=fabs(ny2)*SCALE;

         nx3=(dy+ry+uy)*(dz-rz+uz)-(dy-ry+uy)*(dz+rz+uz);
         ny3=(dz+rz+uz)*(dx-rx+ux)-(dz-rz+uz)*(dx+rx+ux);
         nz3=(dx+rx+ux)*(dy-ry+uy)-(dx-rx+ux)*(dy+ry+uy);

         if ((length=fsqrt(nx3*nx3+ny3*ny3+nz3*nz3))==0.0f) ERRORMSG();
         nx3/=length;
         ny3/=length;
         nz3/=length;

         k31=fabs(nx3)*Dx+fabs(nz3)*Dz;
         k32=fabs(ny3)*SCALE;

         nx4=(dy-ry-uy)*(dz+rz-uz)-(dy+ry-uy)*(dz-rz-uz);
         ny4=(dz-rz-uz)*(dx+rx-ux)-(dz+rz-uz)*(dx-rx-ux);
         nz4=(dx-rx-ux)*(dy+ry-uy)-(dx+rx-ux)*(dy-ry-uy);

         if ((length=fsqrt(nx4*nx4+ny4*ny4+nz4*nz4))==0.0f) ERRORMSG();
         nx4/=length;
         ny4/=length;
         nz4/=length;

         k41=fabs(nx4)*Dx+fabs(nz4)*Dz;
         k42=fabs(ny4)*SCALE;
         }
      }
   else
      {
      if ((length=fsqrt(rx*rx+ry*ry+rz*rz))==0.0f) ERRORMSG();
      rx/=length;
      ry/=length;
      rz/=length;

      if ((length=fsqrt(ux*ux+uy*uy+uz*uz))==0.0f) ERRORMSG();
      ux/=length;
      uy/=length;
      uz/=length;

      k11=fabs(rx)*Dx+fabs(rz)*Dz;
      k12=fabs(ry)*SCALE;

      k21=fabs(ux)*Dx+fabs(uz)*Dz;
      k22=fabs(uy)*SCALE;

      k31=fabs(fovy)/2.0f*aspect;
      k32=fabs(fovy)/2.0f;

      RX=rx;
      RY=ry;
      RZ=rz;

      UX=ux;
      UY=uy;
      UZ=uz;
      }

   if (state==SINGLEPHASE || state==CALCMAPPHASE)
      {
      if (CULLING || SEALEVEL!=-MAXFLOAT)
         if (DH[0]==0)
            {
            calcDH(1,S/2);
            if (yf!=NULL) recalcDH(255.0f*LAMBDA+DISPLACE,1,S/2);
            }

      calcmap(S/2,S/2,S-1);
      }

   if (state==SINGLEPHASE || state==DRAWMAPPHASE)
      if (beginfan==NULL || fanvertex==NULL)
         {
         initstate();

         color(1.0f,1.0f,1.0f);
         bindtexmap(tid,twidth,theight,S,tmipmaps);

         mtxpush();
         mtxtranslate(OX,OY,OZ);

         mtxscale(Dx,SCALE,-Dz);
         mtxtranslate(-S/2,0.0f,-S/2);

         beginfans();
         if (yf==NULL)
            drawmap(S/2,S/2,S-1,
                    y[S-1][S-1],y[0][S-1],y[0][0],y[S-1][0]);
         else
            drawmap(S/2,S/2,S-1,
                    y[S-1][S-1],y[0][S-1],y[0][0],y[S-1][0],
                    yf[S-1][S-1],yf[0][S-1],yf[0][0],yf[S-1][0]);
         endfans();

         mtxpop();

         bindtexmap(0,0,0,0,0);

         exitstate();
         }
      else
         {
         beginfan_callback=beginfan;
         fanvertex_callback=fanvertex;
         notify_callback=notify;
         prismedge_callback=prismedge;

         if (yf==NULL)
            pipemap(S/2,S/2,S-1,
                    y[S-1][S-1],y[0][S-1],y[0][0],y[S-1][0]);
         else
            pipemap(S/2,S/2,S-1,
                    y[S-1][S-1],y[0][S-1],y[0][0],y[S-1][0],
                    yf[S-1][S-1],yf[0][S-1],yf[0][0],yf[S-1][0]);
         }

   if (state==SINGLEPHASE || state==DRAWSEAPHASE)
      if (beginfan!=NULL && fanvertex!=NULL)
         if (SEALEVEL!=-MAXFLOAT)
            {
            beginfan_callback=beginfan;
            fanvertex_callback=fanvertex;
            notify_callback=notify;

            pipesea(S/2,S/2,S-1,
                    y[S-1][S-1],y[0][S-1],y[0][0],y[S-1][0]);
            }

   if (state==SINGLEPHASE)
      drawprismcache(ex,ey,ez,
                     dx,dy,dz,
                     nearp,farp,
                     EMISSION,FR,FG,FB);
   }

// check the visibility of the landscape
int checklandscape(float ex,float ey,float ez,
                   float dx,float dy,float dz,
                   float ux,float uy,float uz,
                   float fovy,float aspect,
                   float nearp,float farp)
   {
   float length;

   float rx,ry,rz;

   float dh;

   float l,d;

   if (S==0) ERRORMSG();

   if (fovy==0.0f || fovy>=180.0f || aspect<=0.0f ||
       nearp<0.0f || farp<=nearp) ERRORMSG();

   if ((length=fsqrt(dx*dx+dy*dy+dz*dz))==0.0f) ERRORMSG();
   dx/=length;
   dy/=length;
   dz/=length;

   EX=ex-OX;
   EY=ey-OY;
   EZ=ez-OZ;

   DX=dx;
   DY=dy;
   DZ=dz;

   NEARP=nearp;
   FARP=farp;

   k1=fabs(dx)*Dx+fabs(dz)*Dz;
   k2=fabs(dy)*SCALE;

   rx=uy*dz-dy*uz;
   ry=uz*dx-dz*ux;
   rz=ux*dy-dx*uy;

   ux=ry*dz-dy*rz;
   uy=rz*dx-dz*rx;
   uz=rx*dy-dx*ry;

   ORTHO=fovy<0.0f;

   if (!ORTHO)
      {
      if ((length=fsqrt(rx*rx+ry*ry+rz*rz)/ftan(fovy/2.0f*RAD)/aspect)==0.0f) ERRORMSG();
      rx/=length;
      ry/=length;
      rz/=length;

      if ((length=fsqrt(ux*ux+uy*uy+uz*uz)/ftan(fovy/2.0f*RAD))==0.0f) ERRORMSG();
      ux/=length;
      uy/=length;
      uz/=length;

      nx1=(dy+ry-uy)*(dz+rz+uz)-(dy+ry+uy)*(dz+rz-uz);
      ny1=(dz+rz-uz)*(dx+rx+ux)-(dz+rz+uz)*(dx+rx-ux);
      nz1=(dx+rx-ux)*(dy+ry+uy)-(dx+rx+ux)*(dy+ry-uy);

      if ((length=fsqrt(nx1*nx1+ny1*ny1+nz1*nz1))==0.0f) ERRORMSG();
      nx1/=length;
      ny1/=length;
      nz1/=length;

      k11=fabs(nx1)*Dx+fabs(nz1)*Dz;
      k12=fabs(ny1)*SCALE;

      nx2=(dy-ry+uy)*(dz-rz-uz)-(dy-ry-uy)*(dz-rz+uz);
      ny2=(dz-rz+uz)*(dx-rx-ux)-(dz-rz-uz)*(dx-rx+ux);
      nz2=(dx-rx+ux)*(dy-ry-uy)-(dx-rx-ux)*(dy-ry+uy);

      if ((length=fsqrt(nx2*nx2+ny2*ny2+nz2*nz2))==0.0f) ERRORMSG();
      nx2/=length;
      ny2/=length;
      nz2/=length;

      k21=fabs(nx2)*Dx+fabs(nz2)*Dz;
      k22=fabs(ny2)*SCALE;

      nx3=(dy+ry+uy)*(dz-rz+uz)-(dy-ry+uy)*(dz+rz+uz);
      ny3=(dz+rz+uz)*(dx-rx+ux)-(dz-rz+uz)*(dx+rx+ux);
      nz3=(dx+rx+ux)*(dy-ry+uy)-(dx-rx+ux)*(dy+ry+uy);

      if ((length=fsqrt(nx3*nx3+ny3*ny3+nz3*nz3))==0.0f) ERRORMSG();
      nx3/=length;
      ny3/=length;
      nz3/=length;

      k31=fabs(nx3)*Dx+fabs(nz3)*Dz;
      k32=fabs(ny3)*SCALE;

      nx4=(dy-ry-uy)*(dz+rz-uz)-(dy+ry-uy)*(dz-rz-uz);
      ny4=(dz-rz-uz)*(dx+rx-ux)-(dz+rz-uz)*(dx-rx-ux);
      nz4=(dx-rx-ux)*(dy+ry-uy)-(dx+rx-ux)*(dy-ry-uy);

      if ((length=fsqrt(nx4*nx4+ny4*ny4+nz4*nz4))==0.0f) ERRORMSG();
      nx4/=length;
      ny4/=length;
      nz4/=length;

      k41=fabs(nx4)*Dx+fabs(nz4)*Dz;
      k42=fabs(ny4)*SCALE;
      }
   else
      {
      if ((length=fsqrt(rx*rx+ry*ry+rz*rz))==0.0f) ERRORMSG();
      rx/=length;
      ry/=length;
      rz/=length;

      if ((length=fsqrt(ux*ux+uy*uy+uz*uz))==0.0f) ERRORMSG();
      ux/=length;
      uy/=length;
      uz/=length;

      k11=fabs(rx)*Dx+fabs(rz)*Dz;
      k12=fabs(ry)*SCALE;

      k21=fabs(ux)*Dx+fabs(uz)*Dz;
      k22=fabs(uy)*SCALE;

      k31=fabs(fovy)/2.0f*aspect;
      k32=fabs(fovy)/2.0f;

      RX=rx;
      RY=ry;
      RZ=rz;

      UX=ux;
      UY=uy;
      UZ=uz;
      }

   dx=X(S/2)-EX;
   dy=Y(S/2,S/2)-EY;
   dz=Z(S/2)-EZ;

   dh=(unsigned short int)DH[S-1];

   if (SEALEVEL!=-MAXFLOAT)
      if (Y(S/2,S/2)-SEALEVEL>dh) dh=Y(S/2,S/2)-SEALEVEL;

   l=DX*dx+DY*dy+DZ*dz;
   d=k1*S/2+k2*dh;

   if (l<NEARP-d || l>FARP+d) return(0);

   if (!ORTHO)
      {
      if (nx1*dx+ny1*dy+nz1*dz>k11*S/2+k12*dh) return(0);
      if (nx2*dx+ny2*dy+nz2*dz>k21*S/2+k22*dh) return(0);
      if (nx3*dx+ny3*dy+nz3*dz>k31*S/2+k32*dh) return(0);
      if (nx4*dx+ny4*dy+nz4*dz>k41*S/2+k42*dh) return(0);
      }
   else
      {
      if (fabs(RX*dx+RY*dy+RZ*dz)>k11*S/2+k12*dh+k31) return(0);
      if (fabs(UX*dx+UY*dy+UZ*dz)>k21*S/2+k22*dh+k32) return(0);
      }

   return(1);
   }

// draw the cached prisms
void drawprismcache(float ex,float ey,float ez,
                    float dx,float dy,float dz,
                    float nearp,float farp,
                    float emission,float fogR,float fogG,float fogB,
                    float *prismcache,int prismcnt)
   {
   float length;

   if (nearp<0.0f || farp<=nearp) ERRORMSG();

   if (prismcache==NULL)
      {
      prismcache=PRISMCACHE;
      prismcnt=PRISMCNT;
      PRISMCNT=0;
      }

   if (prismcnt>0)
      {
      if ((length=fsqrt(dx*dx+dy*dy+dz*dz))==0.0f) ERRORMSG();
      dx/=length;
      dy/=length;
      dz/=length;

      EX=ex;
      EY=ey;
      EZ=ez;

      DX=dx;
      DY=dy;
      DZ=dz;

      NEARP=nearp;
      FARP=farp;

      EMISSION=emission;

      FR=fogR;
      FG=fogG;
      FB=fogB;

      initstate();
      disableZwriting();

      if (emission>0.0f) drawfog(prismcache,prismcnt);
      else mipfog(prismcache,prismcnt);

      enableZwriting();
      exitstate();
      }
   }

// return the elevation at grid position (i,j)
float getheight(int i,int j)
   {
   if (S==0) ERRORMSG();

   if (i<0 || i>=S || j<0 || j>=S) ERRORMSG();

   return(Y(i,j)+OY);
   }

// calculate the elevation at coordinates (s,t)
void getheight(float s,float t,float *height)
   {
   float x,z;

   if (S==0) ERRORMSG();

   x=s*(S-1)*Dx;
   z=(1.0f-t)*(S-1)*Dz;

   x+=OX+X(0);
   z+=OZ+Z(S-1);

   *height=getheight(x,z);
   }

// calculate the elevation at position (x,z)
float getheight(float x,float z)
   {
   int mi,mj;

   float ri,rj;

   if (S==0) ERRORMSG();

   x-=OX+X(0);
   z-=OZ+Z(S-1);

   if (x<0.0f && x>-SAFETYZONE*Dx) x=0.0f;
   if (x>(S-1)*Dx && x<(S-1+SAFETYZONE)*Dx) x=(S-1)*Dx;

   if (z<0.0f && z>-SAFETYZONE*Dz) z=0.0f;
   if (z>(S-1)*Dz && z<(S-1+SAFETYZONE)*Dz) z=(S-1)*Dz;

   if (x<0.0f || x>(S-1)*Dx || z<0.0f || z>(S-1)*Dz) return(-MAXFLOAT);

   mi=ftrc(x/Dx);
   mj=ftrc(z/Dz);

   ri=x/Dx-mi;
   rj=z/Dz-mj;

   if (mi==S-1)
      {
      mi=S-2;
      ri=1.0f;
      }

   if (mj==S-1)
      {
      mj=S-2;
      rj=1.0f;
      }

   if ((mi+mj)%2==0)
      if (ri>rj)
         return(Y(mi,S-1-mj)+
                ri*(Y(mi+1,S-1-mj)-Y(mi,S-1-mj))+
                rj*(Y(mi+1,S-1-mj-1)-Y(mi+1,S-1-mj))+OY);
      else
         return(Y(mi,S-1-mj)+
                rj*(Y(mi,S-1-mj-1)-Y(mi,S-1-mj))+
                ri*(Y(mi+1,S-1-mj-1)-Y(mi,S-1-mj-1))+OY);
   else
      if (ri+rj<1.0)
         return(Y(mi,S-1-mj)+
                ri*(Y(mi+1,S-1-mj)-Y(mi,S-1-mj))+
                rj*(Y(mi,S-1-mj-1)-Y(mi,S-1-mj))+OY);
      else
         return(Y(mi+1,S-1-mj-1)+
                (1.0-ri)*(Y(mi,S-1-mj-1)-Y(mi+1,S-1-mj-1))+
                (1.0-rj)*(Y(mi+1,S-1-mj)-Y(mi+1,S-1-mj-1))+OY);
   }

// calculate the height of the fog layer at position (x,z)
float getfogheight(float x,float z)
   {
   int mi,mj;

   float ri,rj;

   if (S==0) ERRORMSG();

   if (yf==NULL) return(0.0f);

   x-=OX+X(0);
   z-=OZ+Z(S-1);

   if (x<0.0f && x>-SAFETYZONE*Dx) x=0.0f;
   if (x>(S-1)*Dx && x<(S-1+SAFETYZONE)*Dx) x=(S-1)*Dx;

   if (z<0.0f && z>-SAFETYZONE*Dz) z=0.0f;
   if (z>(S-1)*Dz && z<(S-1+SAFETYZONE)*Dz) z=(S-1)*Dz;

   if (x<0.0f || x>(S-1)*Dx || z<0.0f || z>(S-1)*Dz) return(0.0f);

   mi=ftrc(x/Dx);
   mj=ftrc(z/Dz);

   ri=x/Dx-mi;
   rj=z/Dz-mj;

   if (mi==S-1)
      {
      mi=S-2;
      ri=1.0f;
      }

   if (mj==S-1)
      {
      mj=S-2;
      rj=1.0f;
      }

   if ((mi+mj)%2==0)
      if (ri>rj)
         return(YF(mi,S-1-mj)+
                ri*(YF(mi+1,S-1-mj)-YF(mi,S-1-mj))+
                rj*(YF(mi+1,S-1-mj-1)-YF(mi+1,S-1-mj)));
      else
         return(YF(mi,S-1-mj)+
                rj*(YF(mi,S-1-mj-1)-YF(mi,S-1-mj))+
                ri*(YF(mi+1,S-1-mj-1)-YF(mi,S-1-mj-1)));
   else
      if (ri+rj<1.0)
         return(YF(mi,S-1-mj)+
                ri*(YF(mi+1,S-1-mj)-YF(mi,S-1-mj))+
                rj*(YF(mi,S-1-mj-1)-YF(mi,S-1-mj)));
      else
         return(YF(mi+1,S-1-mj-1)+
                (1.0-ri)*(YF(mi,S-1-mj-1)-YF(mi+1,S-1-mj-1))+
                (1.0-rj)*(YF(mi+1,S-1-mj)-YF(mi+1,S-1-mj-1)));
   }

// calculate the normal vector at coordinates (s,t)
void getnormal(float s,float t,float *nx,float *nz)
   {
   float x,z,ny;

   if (S==0) ERRORMSG();

   x=s*(S-1)*Dx;
   z=(1.0f-t)*(S-1)*Dz;

   x+=OX+X(0);
   z+=OZ+Z(S-1);

   getnormal(x,z,nx,&ny,nz);
   }

// calculate the normal vector at position (x,z)
void getnormal(float x,float z,float *nx,float *ny,float *nz)
   {
   int mi,mj;

   float ri,rj;

   float dx1,dy1,dz1,
         dx2,dy2,dz2;

   float length;

   if (S==0) ERRORMSG();

   *nx=*ny=*nz=0.0f;

   x-=OX+X(0);
   z-=OZ+Z(S-1);

   if (x<0.0f && x>-SAFETYZONE*Dx) x=0.0f;
   if (x>(S-1)*Dx && x<(S-1+SAFETYZONE)*Dx) x=(S-1)*Dx;

   if (z<0.0f && z>-SAFETYZONE*Dz) z=0.0f;
   if (z>(S-1)*Dz && z<(S-1+SAFETYZONE)*Dz) z=(S-1)*Dz;

   if (x<0.0f || x>(S-1)*Dx || z<0.0f || z>(S-1)*Dz) return;

   mi=ftrc(x/Dx);
   mj=ftrc(z/Dz);

   ri=x/Dx-mi;
   rj=z/Dz-mj;

   if (mi==S-1)
      {
      mi=S-2;
      ri=1.0f;
      }

   if (mj==S-1)
      {
      mj=S-2;
      rj=1.0f;
      }

   if ((mi+mj)%2==0)
      if (ri>rj)
         {
         dx1=Dx;
         dy1=Y(mi+1,S-1-mj)-Y(mi,S-1-mj);
         dz1=0.0f;
         dx2=Dx;
         dy2=Y(mi+1,S-1-mj-1)-Y(mi,S-1-mj);
         dz2=-Dz;
         }
      else
         {
         dx1=Dx;
         dy1=Y(mi+1,S-1-mj-1)-Y(mi,S-1-mj);
         dz1=-Dz;
         dx2=0.0f;
         dy2=Y(mi,S-1-mj-1)-Y(mi,S-1-mj);
         dz2=-Dz;
         }
   else
      if (ri+rj<1.0)
         {
         dx1=0.0f;
         dy1=Y(mi,S-1-mj)-Y(mi,S-1-mj-1);
         dz1=Dz;
         dx2=Dx;
         dy2=Y(mi+1,S-1-mj)-Y(mi,S-1-mj-1);
         dz2=Dz;
         }
      else
         {
         dx1=Dx;
         dy1=Y(mi+1,S-1-mj)-Y(mi,S-1-mj-1);
         dz1=Dz;
         dx2=Dx;
         dy2=Y(mi+1,S-1-mj-1)-Y(mi,S-1-mj-1);
         dz2=0.0f;
         }

   *nx=dy1*dz2-dz1*dy2;
   *ny=dz1*dx2-dx1*dz2;
   *nz=dy1*dx2-dx1*dy2;

   if ((length=fsqrt((*nx)*(*nx)+(*ny)*(*ny)+(*nz)*(*nz)))==0.0f) ERRORMSG();
   *nx/=length;
   *ny/=length;
   *nz/=length;
   }

// calculate the maximum size of the triangulation
int getmaxsize(float res,float fx,float fy,float fz,float fovy)
   {
   float f;

   float dx,dz;

   float baseoff;

   int lvl,size;

   if (S==0) ERRORMSG();

   c=fsqrt(fmax(res,0.0f))*maxd2;

   ORTHO=fovy<0.0f;

   if (!ORTHO)
      {
      FX=fx-OX;
      FY=fy-OY;
      FZ=fz-OZ;

      if (FX<X(0)) dx=X(0)-FX;
      else if (FX>X(S-1)) dx=FX-X(S-1);
      else dx=0.0f;

      if (FZ<Z(S-1)) dz=Z(S-1)-FZ;
      else if (FZ>Z(0)) dz=FZ-Z(0);
      else dz=0.0f;

      baseoff=fabs(FY-getbaseheight(FX,FZ));

      if (minoff>0.0f) baseoff=fmax(baseoff-minoff*minres*D,0.0f);

      DF2=fsqr(baseoff);

      f=(dx*dx+DF2+dz*dz)/fsqr((S-1)*D*fmax(c*dcpr(S/2,S/2,S/2),minres));
      }
   else f=1.0f/fsqr(fmax(c*dcpr(S/2,S/2,S/2),minres));

   if (f==0.0f) size=S;
   else
      {
      lvl=max(ftrc(fceil(-flog(f)/flog(4.0f)))+1,1);
      size=ftrc(fmin(fpow(2,lvl)+1,S)+0.5f);
      }

   return(size);
   }

// set the elevation at grid position (i,j)
void setheight(int i,int j,float h)
   {
   if (S==0) ERRORMSG();

   if (i<0 || i>=S || j<0 || j>=S) ERRORMSG();

   if (h<-32768.0f) h=-32768.0f;
   else if (h>32767.0f) h=32767.0f;

   y[i][j]=ftrc(h+0.5f);
   }

// set the elevation at position (x,z)
int setheight(float x,float z,float h)
   {
   int mi,mj;

   float ri,rj;

   if (S==0) ERRORMSG();

   x-=OX+X(0);
   z-=OZ+Z(S-1);

   if (x<0.0f && x>-SAFETYZONE*Dx) x=0.0f;
   if (x>(S-1)*Dx && x<(S-1+SAFETYZONE)*Dx) x=(S-1)*Dx;

   if (z<0.0f && z>-SAFETYZONE*Dz) z=0.0f;
   if (z>(S-1)*Dz && z<(S-1+SAFETYZONE)*Dz) z=(S-1)*Dz;

   if (x<0.0f || x>(S-1)*Dx || z<0.0f || z>(S-1)*Dz) return(0);

   mi=ftrc(x/Dx);
   mj=ftrc(z/Dz);

   ri=x/Dx-mi;
   rj=z/Dz-mj;

   if (mi==S-1)
      {
      mi=S-2;
      ri=1.0f;
      }

   if (mj==S-1)
      {
      mj=S-2;
      rj=1.0f;
      }

   setheight(mi,mj,y[mi][mj]*ri*rj+h*(1.0f-ri)*(1.0f-rj));
   setheight(mi+1,mj,y[mi+1][mj]*(1.0f-ri)*rj+h*ri*(1.0f-rj));
   setheight(mi,mj+1,y[mi][mj+1]*ri*(1.0f-rj)+h*(1.0f-ri)*rj);
   setheight(mi+1,mj+1,y[mi+1][mj+1]*(1.0f-ri)*(1.0f-rj)+h*ri*rj);

   return(1);
   }

// set the real elevation at grid position (i,j)
void setrealheight(int i,int j,float h)
   {
   if (SCALE==0.0f) ERRORMSG();
   setheight(i,j,(h-OY)/SCALE);
   }

// set the real elevation at position (x,z)
int setrealheight(float x,float z,float h)
   {
   if (SCALE==0.0f) ERRORMSG();
   return(setheight(x,z,(h-OY)/SCALE));
   }

// update d2- and dh-values after modification of height map
void updatemaps(int fast,float avgd2,int recalc)
   {
   int i,s;

   if (S==0) ERRORMSG();

   for (i=0,s=2; i<fast && s<S-1; i++) s*=2;

   if (recalc==0)
      if (fast==0)
         {
         calcDH();
         calcD2();
         }
      else
         {
         if (SEALEVEL!=-MAXFLOAT) calcDH();
         else
            {
            calcDH(S/2,S-1,s/2);
            DH[0]=0;
            }

         calcD2(s,avgd2);
         }

   if (yf!=NULL)
      {
      recalcDH(255.0f*LAMBDA+DISPLACE);
      recalcD2(FOGATT,s);
      }
   }

// delete the height field and all associated maps
void deletemaps()
   {
   int i;

   if (S==0) ERRORMSG();

   for (i=0; i<=S; i++) free(y[i]);
   for (i=0; i<S-1; i++) free(bc[i]);

   free(y);
   free(bc);

   if (yf!=NULL)
      {
      for (i=0; i<S; i++) free(yf[i]);
      free(yf);
      }

   if (tid!=0) deletetexmap(tid);

   if (PRISMCACHE!=NULL && PRISMCNT==0)
      {
      free(PRISMCACHE);
      PRISMCACHE=NULL;
      }

   S=0;
   }

}

namespace Mini {

// set fine tuning parameters
void setparams(float minr,
               float maxd,
               float sead,
               float mino,
               int maxc)
   {
   if (minr<1.0f || maxd<=0.0f || sead<0.0f || mino<0.0f || maxc<0) ERRORMSG();

   minres=minr;
   maxd2=maxd;
   sead2=sead;
   minoff=mino;
   maxcull=maxc;
   }

// scale the height field
void scalemap(float *image,int size)
   {
   int i,j,
       mi,mj;

   float pi,pj,
         ri,rj;

   float *ptr1,*ptr2;

   if (size==S)
      for (i=0; i<S; i++)
         for (ptr1=&image[(S-1)*S+i],ptr2=y[i]; ptr1>=image; ptr1-=S) *ptr2++=*ptr1;
   else
      for (i=0; i<S; i++)
         for (j=0; j<S; j++)
            {
            if (i<S-1)
               {
               pi=(float)i/(S-1)*(size-1);
               mi=ftrc(pi);
               ri=pi-mi;
               }
            else
               {
               mi=size-2;
               ri=1.0f;
               }

            if (j>0)
               {
               pj=(float)(S-1-j)/(S-1)*(size-1);
               mj=ftrc(pj);
               rj=pj-mj;
               }
            else
               {
               mj=size-2;
               rj=1.0f;
               }

            y[i][j]=ftrc((1.0f-rj)*((1.0f-ri)*image[mj*size+mi]+
                                           ri*image[mj*size+mi+1])+
                                rj*((1.0f-ri)*image[(mj+1)*size+mi]+
                                           ri*image[(mj+1)*size+mi+1])+0.5f);
            }
   }

// calculate the height differences
void calcDH(int mins=1,int maxs=0,int ds=1)
   {
   int i,j,s,
       m,n;

   if (maxs==0) maxs=S-1;

   DH=y[S];
   for (i=0; i<S; i++) DH[i]=MAXFLOAT;

   for (s=maxs; s>mins; s/=2)
      {
      DH[s]=0.0f;

      for (i=s/2; i<S; i+=s)
         for (j=s/2; j<S; j+=s)
            if (ds<s)
               for (m=-s/2; m<=s/2; m+=ds)
                  for (n=-s/2; n<=s/2; n+=ds)
                     DH[s]=fmax(DH[s],fabs(y[i+m][j+n]-y[i][j]));
            else
               for (m=-s/2; m<=s/2; m++)
                  for (n=-s/2; n<=s/2; n++)
                     DH[s]=fmax(DH[s],fabs(y[i+m][j+n]-y[i][j]));
      }
   }

// store a d2-value
inline void store(const float fc,const int i,const int j,const int s2)
   {
   bc[i-s2][j]=cpr(fc)%256;
   bc[i][j-s2]=cpr(fc)/256;
   }

// increase a d2-value
inline void increase(const float fc,const int i,const int j,const int s2)
   {
   float fc1;

   fc1=fmin(fc,1.0f);

   if (fc1>dcpr(i,j,s2))
      {
      store(fc1,i,j,s2);

      while (dcpr(i,j,s2)<fc1)
         if (++bc[i-s2][j]==0)
            if (++bc[i][j-s2]==0)
               {
               bc[i-s2][j]=bc[i][j-s2]=255;
               break;
               }
      }
   }

// propagate a local d2-value to the next higher level
inline void propagate(const int i,const int j,const int s,const int i0,const int j0)
   {
   float l1,l2;

   if (i0<0 || i0>=S || j0<0 || j0>=S) return;

   l1=2.0f*s*D*minres;
   l2=l1-fsqrt(fsqr(X(i)-X(i0))+fsqr(Z(j)-Z(j0)));
   if (l2<=0.0f) ERRORMSG();

   increase(dcpr(i,j,s/2)*l1/l2/2.0f,i0,j0,s);
   }

// calculate d2-value
inline float d2value(const float a,const float b,const float m,const int s)
   {
   float d2,dh;

   d2=fabs(a+b-2.0f*m);
   dh=fmax(fabs(m-a),fabs(m-b));

   if (m-dh<SEALEVELMAX && m+dh>SEALEVELMIN) d2=fmax(d2,sead2*s*D);

   return(d2);
   }

// calculate the d2-values
void calcD2(int mins=2,float avgd2=0.1f)
   {
   int i,j,s,s2;

   float fc;

   // initialize the d2-values
   for (i=0; i<S-1; i++) memset(bc[i],0,S-1);

   // compute an approximate d2-value
   fc=avgd2*DH[S-1]*SCALE/(S-1)/D/maxd2;

   // approximate the least-significant d2-values
   if (mins>2)
      for (s=2; s<=mins; s*=2)
         {
         s2=s/2;
         for (i=s2; i<S; i+=s)
            for (j=s2; j<S; j+=s) store(fc,i,j,s2);
         }

   // propagate the d2-values up the tree
   for (s=mins; s<S; s*=2)
      {
      s2=s/2;
      for (i=s2; i<S; i+=s)
         for (j=s2; j<S; j+=s)
            {
            // calculate the local d2-value
            if ((i/s+j/s)%2==0) fc=d2value(Y(i-s2,j-s2),Y(i+s2,j+s2),Y(i,j),s);
            else fc=d2value(Y(i-s2,j+s2),Y(i+s2,j-s2),Y(i,j),s);

            // calculate the local d2-value more accurately
            if (s>mins || mins==2)
               {
               fc=fmax(fc,d2value(Y(i-s2,j-s2),Y(i+s2,j-s2),Y(i,j-s2),s));
               fc=fmax(fc,d2value(Y(i-s2,j+s2),Y(i+s2,j+s2),Y(i,j+s2),s));
               fc=fmax(fc,d2value(Y(i-s2,j-s2),Y(i-s2,j+s2),Y(i-s2,j),s));
               fc=fmax(fc,d2value(Y(i+s2,j-s2),Y(i+s2,j+s2),Y(i+s2,j),s));
               }

            // store the local d2-value
            increase(fc/s/D/maxd2,i,j,s2);

            // propagate the local d2-value
            if (s<S-1)
               switch ((i/s)%2+2*((j/s)%2))
                  {
                  case 0:
                     propagate(i,j,s,i+s2,j+s2);
                     propagate(i,j,s,i-s-s2,j+s2);
                     propagate(i,j,s,i+s2,j-s-s2);
                     break;
                  case 1:
                     propagate(i,j,s,i-s2,j+s2);
                     propagate(i,j,s,i-s2,j-s-s2);
                     propagate(i,j,s,i+s+s2,j+s2);
                     break;
                  case 2:
                     propagate(i,j,s,i+s2,j-s2);
                     propagate(i,j,s,i+s2,j+s+s2);
                     propagate(i,j,s,i-s-s2,j-s2);
                     break;
                  case 3:
                     propagate(i,j,s,i-s2,j-s2);
                     propagate(i,j,s,i+s+s2,j-s2);
                     propagate(i,j,s,i-s2,j+s+s2);
                     break;
                  }
            }
      }
   }

// preprocess the height field
void *initmap(float *image,void **d2map,
              int *size,float *dim,float scale,
              float cellaspect,
              float (*getelevation)(int i,int j,int S,void *data),
              void *objref,
              int fast,float avgd2)
   {
   int i,j;

   if (*size<2 || *dim<=0.0f || scale<0.0f) ERRORMSG();

   for (S=3; S<*size; S=2*S-1);

   Dx=(*dim)*(*size-1)/(S-1);
   Dz=Dx*cellaspect;
   D=(Dx+Dz)/2.0f;

   SCALE=scale;

   if ((y=(float **)malloc((S+1)*sizeof(float *)))==NULL) ERRORMSG();
   for (i=0; i<=S; i++)
      if ((y[i]=(float *)malloc(S*sizeof(float)))==NULL) ERRORMSG();

   if (image!=NULL) scalemap(image,*size);
   else if (*size==S)
      {
      if (getelevation==NULL) ERRORMSG();

      for (j=0; j<S; j++)
         for (i=0; i<S; i++) y[i][S-1-j]=getelevation(i,j,S,objref);
      }
   else
      {
      if (getelevation==NULL) ERRORMSG();

      if ((image=(float *)malloc((*size)*(*size)*sizeof(float)))==NULL) ERRORMSG();

      for (j=0; j<*size; j++)
         for (i=0; i<*size; i++) image[i+j*(*size)]=getelevation(i,j,*size,objref);

      scalemap(image,*size);

      free(image);
      }

   if ((bc=(unsigned char **)malloc((S-1)*sizeof(unsigned char *)))==NULL) ERRORMSG();
   for (i=0; i<S-1; i++)
      if ((bc[i]=(unsigned char *)malloc(S-1))==NULL) ERRORMSG();

   tid=0;
   yf=NULL;

   updatemaps(fast,avgd2);

   OX=OY=OZ=0.0f;
   for (i=0; i<4; i++) bc2[i]=NULL;

   *d2map=(void *)bc;

   *size=S;
   *dim=Dx;

   return((void *)y);
   }

// calculate the texture map
int inittexmap(unsigned char *image,int *width,int *height,
               int mipmaps,int s3tc,int rgba,int bytes,int mipmapped)
   {
   if (S==0) ERRORMSG();

   if (tid!=0) deletetexmap(tid);

   if (image==NULL) return(tid=0);

   if (*width<2 || *height<2) ERRORMSG();

   if (rgba==0) tid=buildRGBtexmap(image,width,height,mipmaps,s3tc,bytes,mipmapped);
   else tid=buildRGBAtexmap(image,width,height,mipmaps,s3tc,bytes,mipmapped);

   twidth=*width;
   theight=*height;

   tmipmaps=mipmaps;

   return(tid);
   }

// scale the ground fog map
void scalefog(unsigned char *image,int size)
   {
   int i,j,
       mi,mj;

   float pi,pj,
         ri,rj;

   unsigned char *ptr1,*ptr2;

   if (size==S)
      for (i=0; i<S; i++)
         for (ptr1=&image[(S-1)*S+i],ptr2=yf[i]; ptr1>=image; ptr1-=S) *ptr2++=*ptr1;
   else
      for (i=0; i<S; i++)
         for (j=0; j<S; j++)
            {
            if (i<S-1)
               {
               pi=(float)i/(S-1)*(size-1);
               mi=ftrc(pi);
               ri=pi-mi;
               }
            else
               {
               mi=size-2;
               ri=1.0f;
               }

            if (j>0)
               {
               pj=(float)(S-1-j)/(S-1)*(size-1);
               mj=ftrc(pj);
               rj=pj-mj;
               }
            else
               {
               mj=size-2;
               rj=1.0f;
               }

            yf[i][j]=ftrc((1.0f-rj)*((1.0f-ri)*image[mj*size+mi]+
                                            ri*image[mj*size+mi+1])+
                                 rj*((1.0f-ri)*image[(mj+1)*size+mi]+
                                            ri*image[(mj+1)*size+mi+1])+0.5f);
            }
   }

// recalculate the height differences
void recalcDH(float lambda,
              int mins=1,int maxs=0)
   {
   int s;

   float h;

   if (SCALE==0.0f) ERRORMSG();

   if (maxs==0) maxs=S-1;

   for (s=maxs; s>mins; s/=2)
      {
      h=DH[s]+lambda/SCALE;
      if (h>MAXFLOAT) h=MAXFLOAT;
      DH[s]=h;
      }
   }

// recalculate the d2-values
void recalcD2(float fogatt,int mins=2)
   {
   int i,j,s,s2;

   float fc;

   // propagate the d2-values up the tree
   for (s=mins; s<S; s*=2)
      {
      s2=s/2;
      for (i=s2; i<S; i+=s)
         for (j=s2; j<S; j+=s)
            {
            // calculate the local d2-value
            if ((i/s+j/s)%2==0) fc=d2value(YF(i-s2,j-s2),YF(i+s2,j+s2),YF(i,j),s);
            else fc=d2value(YF(i-s2,j+s2),YF(i+s2,j-s2),YF(i,j),s);

            // calculate the local d2-value more accurately
            if (s>mins || mins==2)
               {
               fc=fmax(fc,d2value(YF(i-s2,j-s2),YF(i+s2,j-s2),YF(i,j-s2),s));
               fc=fmax(fc,d2value(YF(i-s2,j+s2),YF(i+s2,j+s2),YF(i,j+s2),s));
               fc=fmax(fc,d2value(YF(i-s2,j-s2),YF(i-s2,j+s2),YF(i-s2,j),s));
               fc=fmax(fc,d2value(YF(i+s2,j-s2),YF(i+s2,j+s2),YF(i+s2,j),s));
               }

            // store the local d2-value
            increase(fc*fogatt/s/D/maxd2,i,j,s2);

            // propagate the local d2-value
            if (s<S-1)
               switch ((i/s)%2+2*((j/s)%2))
                  {
                  case 0:
                     propagate(i,j,s,i+s2,j+s2);
                     propagate(i,j,s,i-s-s2,j+s2);
                     propagate(i,j,s,i+s2,j-s-s2);
                     break;
                  case 1:
                     propagate(i,j,s,i-s2,j+s2);
                     propagate(i,j,s,i-s2,j-s-s2);
                     propagate(i,j,s,i+s+s2,j+s2);
                     break;
                  case 2:
                     propagate(i,j,s,i+s2,j-s2);
                     propagate(i,j,s,i+s2,j+s+s2);
                     propagate(i,j,s,i-s-s2,j-s2);
                     break;
                  case 3:
                     propagate(i,j,s,i-s2,j-s2);
                     propagate(i,j,s,i+s+s2,j-s2);
                     propagate(i,j,s,i-s2,j+s+s2);
                     break;
                  }
            }
      }
   }

// calculate the ground fog map
void *initfogmap(unsigned char *image,int size,
                 float lambda,float displace,float emission,
                 float fogatt,float fogR,float fogG,float fogB,
                 int fast)
   {
   int i;

   if (S==0) ERRORMSG();

   if (image==NULL) return(NULL);

   if (size<2) ERRORMSG();
   if (lambda<=0.0f || displace<0.0f || emission<0.0f) ERRORMSG();
   if (fogatt<0.0f) ERRORMSG();

   if (yf!=NULL) ERRORMSG();
   if ((yf=(unsigned char **)malloc(S*sizeof(unsigned char *)))==NULL) ERRORMSG();
   for (i=0; i<S; i++)
      if ((yf[i]=(unsigned char *)malloc(S))==NULL) ERRORMSG();

   scalefog(image,size);

   LAMBDA=lambda;
   DISPLACE=displace;
   EMISSION=emission;
   FOGATT=fogatt;

   FR=fogR;
   FG=fogG;
   FB=fogB;

   updatemaps(fast,0.0f,1);

   return((void *)yf);
   }

// set the height field and all associated maps
void setmaps(void *map,void *d2map,
             int size,float dim,float scale,
             int texid,int width,int height,int mipmaps,
             float cellaspect,
             float ox,float oy,float oz,
             void **d2map2,int *size2,
             void *fogmap,float lambda,float displace,
             float emission,float fogatt,float fogR,float fogG,float fogB)
   {
   int i;

   S=size;

   Dx=dim;
   Dz=Dx*cellaspect;
   D=(Dx+Dz)/2.0f;

   SCALE=scale;

   y=(float **)map;
   DH=y[S];

   bc=(unsigned char **)d2map;

   tid=texid;
   twidth=width;
   theight=height;
   tmipmaps=mipmaps;

   yf=(unsigned char **)fogmap;

   LAMBDA=lambda;
   DISPLACE=displace;
   EMISSION=emission;
   FOGATT=fogatt;

   FR=fogR;
   FG=fogG;
   FB=fogB;

   OX=ox;
   OY=oy;
   OZ=oz;

   for (i=0; i<4; i++)
      if (d2map2==NULL) bc2[i]=NULL;
      else
         {
         bc2[i]=(unsigned char **)d2map2[i];
         S2[i]=size2[i];
         }
   }

// set the sea level
void setsea(float level)
   {
   if (level==-MAXFLOAT) SEALEVEL=level;
   else SEALEVEL=level/SCALE;
   }

// set the sea level range
void setsearange(float seamin,float seamax)
   {
   SEALEVELMIN=seamin;
   SEALEVELMAX=seamax;
   }

// undo the previous triangulation
void undomap(const int i,const int j,const int s2)
   {
   int s4;

   if (bc[i][j]!=0)
      {
      bc[i][j]=0;

      if ((s4=s2/2)>0)
         {
         undomap(i+s4,j+s4,s4);
         undomap(i-s4,j+s4,s4);
         undomap(i-s4,j-s4,s4);
         undomap(i+s4,j-s4,s4);
         }
      }
   }

// triangulate the height field
void calcmap(const int i,const int j,const int s)
   {
   int s2,s4;

   float dx,dy,dz;

   float dh;

   float f;

   float l,d;

   s2=s/2;
   s4=s/4;

   dx=X(i)-FX;
   dz=Z(j)-FZ;

   // evaluate the subdivision variable
   if (!ORTHO) f=(dx*dx+DF2+dz*dz)/fsqr(s*D*fmax(c*dcpr(i,j,s2),minres));
   else f=fsqr(S-1)/fsqr(s*fmax(c*dcpr(i,j,s2),minres));

   // check subdivision condition
   if (f<1.0f)
      {
      // blending factor
      if (f<=0.25f) bc[i][j]=255;
      else bc[i][j]=340-ftrc(340.0f*f);

      if (s4>0)
         {
         // view frustum culling
         if (CULLING && s>(S>>maxcull) && f<0.25f)
            {
            dy=Y(i,j)-FY;

            dx+=FX-EX;
            dy+=FY-EY;
            dz+=FZ-EZ;

            dh=DH[s];

            l=DX*dx+DY*dy+DZ*dz;
            d=3.0f*(k1*s2+k2*dh);

            if (l<NEARP-d || l>FARP+d) return;

            if (!ORTHO)
               {
               if (nx1*dx+ny1*dy+nz1*dz>3.0f*(k11*s2+k12*dh)) return;
               if (nx2*dx+ny2*dy+nz2*dz>3.0f*(k21*s2+k22*dh)) return;
               if (nx3*dx+ny3*dy+nz3*dz>3.0f*(k31*s2+k32*dh)) return;
               if (nx4*dx+ny4*dy+nz4*dz>3.0f*(k41*s2+k42*dh)) return;
               }
            else
               {
               if (fabs(RX*dx+RY*dy+RZ*dz)>3.0f*(k11*s2+k12*dh)+k31) return;
               if (fabs(UX*dx+UY*dy+UZ*dz)>3.0f*(k21*s2+k22*dh)+k32) return;
               }
            }

         // subdivision
         calcmap(i+s4,j+s4,s2);
         calcmap(i-s4,j+s4,s2);
         calcmap(i-s4,j-s4,s2);
         calcmap(i+s4,j-s4,s2);
         }
      }
   else undomap(i,j,s2);
   }

// blending functions:

inline float blendE(const int bc,const float v0,const float v1,const float v2)
   {return((bc==255)?v0:(2*bc*v0+(255-bc)*(v1+v2))/510.0f);}

inline float blendM(const int bc,const float v0,const float v1,const float v2)
   {return((bc==0)?-MAXFLOAT:blendE(bc,v0,v1,v2));}

inline int maxbc(const int w,int i,int j,const int s2)
   {
   int s;

   if (bc2[w]==NULL) return(255);

   if (S2[w]<S)
      {
      if ((s=(S-1)/(S2[w]-1))>s2) return(0);
      i/=s;
      j/=s;
      }
   else
      {
      s=(S2[w]-1)/(S-1);
      i*=s;
      j*=s;
      }

   return(bc2[w][i][j]);
   }

inline float blendD(const int i,const int j,const float y1,const float y2)
   {return(blendE(bc[i][j],y[i][j],y1,y2));}

inline float blendV(const int i,const int j,const int s2,const float y1,const float y2)
   {return(blendM(min((i>0)?bc[i-s2][j]:maxbc(0,S-1-s2,j,s2),
                      (i<S-1)?bc[i+s2][j]:maxbc(1,s2,j,s2)),y[i][j],y1,y2));}

inline float blendH(const int i,const int j,const int s2,const float y1,const float y2)
   {return(blendM(min((j>0)?bc[i][j-s2]:maxbc(2,i,S-1-s2,s2),
                      (j<S-1)?bc[i][j+s2]:maxbc(3,i,s2,s2)),y[i][j],y1,y2));}

// geomorph the triangulation
void drawmap(const int i,const int j,const int s,
             const float e1,const float e2,const float e3,const float e4)
   {
   int s2,s4;

   float dx,dy,dz;

   float dh;

   float l,d;

   float m0,m1,m2,m3,m4;

   int bc1,bc2,bc3,bc4;

   s2=s/2;
   s4=s/4;

   // blend the center vertex
   if (((i+j)/s)%2==1) m0=blendD(i,j,e1,e3);
   else m0=blendD(i,j,e2,e4);

   // view frustum culling
   if (CULLING && s>(S>>maxcull))
      {
      dx=X(i)-EX;
      dy=m0*SCALE-EY;
      dz=Z(j)-EZ;

      dh=DH[s];

      l=DX*dx+DY*dy+DZ*dz;
      d=k1*s2+k2*dh;

      if (l<NEARP-d || l>FARP+d) return;

      if (!ORTHO)
         {
         if (nx1*dx+ny1*dy+nz1*dz>k11*s2+k12*dh) return;
         if (nx2*dx+ny2*dy+nz2*dz>k21*s2+k22*dh) return;
         if (nx3*dx+ny3*dy+nz3*dz>k31*s2+k32*dh) return;
         if (nx4*dx+ny4*dy+nz4*dz>k41*s2+k42*dh) return;
         }
      else
         {
         if (fabs(RX*dx+RY*dy+RZ*dz)>k11*s2+k12*dh+k31) return;
         if (fabs(UX*dx+UY*dy+UZ*dz)>k21*s2+k22*dh+k32) return;
         }
      }

   // blend the edge vertices
   m1=blendV(i+s2,j,s2,e4,e1);
   m2=blendH(i,j+s2,s2,e1,e2);
   m3=blendV(i-s2,j,s2,e2,e3);
   m4=blendH(i,j-s2,s2,e3,e4);

   // subdivision factors
   if (s4>0)
      {
      bc1=bc[i+s4][j+s4];
      bc2=bc[i-s4][j+s4];
      bc3=bc[i-s4][j-s4];
      bc4=bc[i+s4][j-s4];
      }
   else bc1=bc2=bc3=bc4=0;

   // first triangle fan quarter
   if (bc1!=0)
      {
      if (m1==-MAXFLOAT) m1=(e4+e1)/2.0f;
      if (m2==-MAXFLOAT) m2=(e1+e2)/2.0f;
      drawmap(i+s4,j+s4,s2,e1,m2,m0,m1);
      }
   else
      {
      beginfan();
      fanvertex(i,m0,j);
      if (bc4!=0)
         {
         if (m1==-MAXFLOAT) m1=(e4+e1)/2.0f;
         fanvertex(i+s2,m1,j);
         }
      else if (m1!=-MAXFLOAT) fanvertex(i+s2,m1,j);
      fanvertex(i+s2,e1,j+s2);
      }

   // second triangle fan quarter
   if (bc2!=0)
      {
      if (bc1==0)
         {
         if (m2==-MAXFLOAT) m2=(e1+e2)/2.0f;
         fanvertex(i,m2,j+s2);
         }
      if (m3==-MAXFLOAT) m3=(e2+e3)/2.0f;
      drawmap(i-s4,j+s4,s2,m2,e2,m3,m0);
      }
   else
      {
      if (bc1!=0)
         {
         beginfan();
         fanvertex(i,m0,j);
         fanvertex(i,m2,j+s2);
         }
      else if (m2!=-MAXFLOAT) fanvertex(i,m2,j+s2);
      fanvertex(i-s2,e2,j+s2);
      }

   // third triangle fan quarter
   if (bc3!=0)
      {
      if (bc2==0)
         {
         if (m3==-MAXFLOAT) m3=(e2+e3)/2.0f;
         fanvertex(i-s2,m3,j);
         }
      if (m4==-MAXFLOAT) m4=(e3+e4)/2.0f;
      drawmap(i-s4,j-s4,s2,m0,m3,e3,m4);
      }
   else
      {
      if (bc2!=0)
         {
         beginfan();
         fanvertex(i,m0,j);
         fanvertex(i-s2,m3,j);
         }
      else if (m3!=-MAXFLOAT) fanvertex(i-s2,m3,j);
      fanvertex(i-s2,e3,j-s2);
      }

   // fourth triangle fan quarter
   if (bc4!=0)
      {
      if (bc3==0)
         {
         if (m4==-MAXFLOAT) m4=(e3+e4)/2.0f;
         fanvertex(i,m4,j-s2);
         }
      drawmap(i+s4,j-s4,s2,m1,m0,m4,e4);
      }
   else
      {
      if (bc3!=0)
         {
         beginfan();
         fanvertex(i,m0,j);
         fanvertex(i,m4,j-s2);
         }
      else if (m4!=-MAXFLOAT) fanvertex(i,m4,j-s2);
      fanvertex(i+s2,e4,j-s2);
      if (bc1==0)
         if (m1!=-MAXFLOAT) fanvertex(i+s2,m1,j);
         else fanvertex(i+s2,e1,j+s2);
      else fanvertex(i+s2,m1,j);
      }
   }

// prism caching functions:

inline void cacheprism(const int i0,const float y0,const float yf0,const int j0,
                       const int i1,const float y1,const float yf1,const int j1,
                       const int i2,const float y2,const float yf2,const int j2)
   {
   float offx,offy,offz;

   float p1x,p1y,p1yf,p1z;
   float p2x,p2y,p2yf,p2z;
   float p3x,p3y,p3yf,p3z;

   float *prismptr;

   offx=-S/2*Dx+OX;
   offy=DISPLACE+OY;
   offz=S/2*Dz+OZ;

   p1x=i0*Dx+offx;
   p1y=y0*SCALE+offy;
   p1yf=yf0*LAMBDA;
   p1z=offz-j0*Dz;
   p2x=i1*Dx+offx;
   p2y=y1*SCALE+offy;
   p2yf=yf1*LAMBDA;
   p2z=offz-j1*Dz;
   p3x=i2*Dx+offx;
   p3y=y2*SCALE+offy;
   p3yf=yf2*LAMBDA;
   p3z=offz-j2*Dz;

   if (prismedge_callback!=NULL)
      {
      prismedge_callback(p1x,p1y+p1yf,p1yf,p1z);
      prismedge_callback(p2x,p2y+p2yf,p2yf,p2z);
      prismedge_callback(p3x,p3y+p3yf,p3yf,p3z);
      }
   else
      {
      if (PRISMCACHE==NULL)
         {
         if ((PRISMCACHE=(float *)malloc(12*PRISMMAX*sizeof(float)))==NULL) ERRORMSG();
         PRISMCNT=0;
         }

      if (PRISMCNT>=PRISMMAX)
         {
         PRISMMAX*=2;
         if ((PRISMCACHE=(float *)realloc(PRISMCACHE,12*PRISMMAX*sizeof(float)))==NULL) ERRORMSG();
         }

      prismptr=&PRISMCACHE[12*PRISMCNT];

      *prismptr++=p1x;
      *prismptr++=p1y;
      *prismptr++=p1y+p1yf;
      *prismptr++=p1z;
      *prismptr++=p2x;
      *prismptr++=p2y;
      *prismptr++=p2y+p2yf;
      *prismptr++=p2z;
      *prismptr++=p3x;
      *prismptr++=p3y;
      *prismptr++=p3y+p3yf;
      *prismptr=p3z;

      PRISMCNT++;
      }
   }

inline void BEGINFAN()
   {
   FANSTATE=0;
   beginfan();
   }

inline void FANVERTEX(const int i,const float y,const float yf,const int j)
   {
   static int i0,i1;
   static float y0,y1;
   static float yf0,yf1;
   static int j0,j1;

   if (tid!=0)
      switch (FANSTATE)
         {
         case 0:
            fanvertex(i,y,j);
            i0=i; y0=y; yf0=yf; j0=j;
            FANSTATE++;
            break;
         case 1:
            fanvertex(i,y,j);
            i1=i; y1=y; yf1=yf; j1=j;
            FANSTATE++;
            break;
         default:
            fanvertex(i,y,j);
            if (yf0>0.0f || yf1>0.0f || yf>0.0f) cacheprism(i0,y0,yf0,j0,i1,y1,yf1,j1,i,y,yf,j);
            i1=i; y1=y; yf1=yf; j1=j;
            break;
         }
   else
      switch (FANSTATE)
         {
         case 0:
            i0=i; y0=y; yf0=yf; j0=j;
            FANSTATE++;
            break;
         case 1:
            i1=i; y1=y; yf1=yf; j1=j;
            FANSTATE++;
            break;
         default:
            if (yf0>0.0f || yf1>0.0f || yf>0.0f) cacheprism(i0,y0,yf0,j0,i1,y1,yf1,j1,i,y,yf,j);
            i1=i; y1=y; yf1=yf; j1=j;
            break;
         }
   }

// prism clipping functions:

inline void cliptetra1(const float p1x,const float p1y,const float p1z,const float d1,
                       const float p2x,const float p2y,const float p2z,const float d2,
                       const float p3x,const float p3y,const float p3z,const float d3,
                       const float p4x,const float p4y,const float p4z,const float d4)
   {
   float pp1x,pp1y,pp1z,
         pp2x,pp2y,pp2z,
         pp3x,pp3y,pp3z;

   pp1x=(d2*p1x+d1*p2x)/(d1+d2);
   pp1y=(d2*p1y+d1*p2y)/(d1+d2);
   pp1z=(d2*p1z+d1*p2z)/(d1+d2);
   pp2x=(d3*p1x+d1*p3x)/(d1+d3);
   pp2y=(d3*p1y+d1*p3y)/(d1+d3);
   pp2z=(d3*p1z+d1*p3z)/(d1+d3);
   pp3x=(d4*p1x+d1*p4x)/(d1+d4);
   pp3y=(d4*p1y+d1*p4y)/(d1+d4);
   pp3z=(d4*p1z+d1*p4z)/(d1+d4);

   beginfan();
   fanvertex(pp1x,pp1y,pp1z);
   fanvertex(pp2x,pp2y,pp2z);
   fanvertex(pp3x,pp3y,pp3z);
   }

inline void cliptetra2(const float p1x,const float p1y,const float p1z,const float d1,
                       const float p2x,const float p2y,const float p2z,const float d2,
                       const float p3x,const float p3y,const float p3z,const float d3,
                       const float p4x,const float p4y,const float p4z,const float d4)
   {
   float pp1x,pp1y,pp1z,
         pp2x,pp2y,pp2z,
         pp3x,pp3y,pp3z,
         pp4x,pp4y,pp4z;

   pp1x=(d3*p1x+d1*p3x)/(d1+d3);
   pp1y=(d3*p1y+d1*p3y)/(d1+d3);
   pp1z=(d3*p1z+d1*p3z)/(d1+d3);
   pp2x=(d3*p2x+d2*p3x)/(d2+d3);
   pp2y=(d3*p2y+d2*p3y)/(d2+d3);
   pp2z=(d3*p2z+d2*p3z)/(d2+d3);
   pp3x=(d4*p1x+d1*p4x)/(d1+d4);
   pp3y=(d4*p1y+d1*p4y)/(d1+d4);
   pp3z=(d4*p1z+d1*p4z)/(d1+d4);
   pp4x=(d4*p2x+d2*p4x)/(d2+d4);
   pp4y=(d4*p2y+d2*p4y)/(d2+d4);
   pp4z=(d4*p2z+d2*p4z)/(d2+d4);

   beginfan();
   fanvertex(pp1x,pp1y,pp1z);
   fanvertex(pp2x,pp2y,pp2z);
   fanvertex(pp4x,pp4y,pp4z);
   fanvertex(pp3x,pp3y,pp3z);
   }

inline void cliptetra(const float p1x,const float p1y,const float p1z,
                      const float p2x,const float p2y,const float p2z,
                      const float p3x,const float p3y,const float p3z,
                      const float p4x,const float p4y,const float p4z)
   {
   int flag=0;

   float d1,d2,d3,d4;

   float nearp=1.0001f*NEARP;

   d1=(p1x-EX)*DX+(p1y-EY)*DY+(p1z-EZ)*DZ-nearp;
   d2=(p2x-EX)*DX+(p2y-EY)*DY+(p2z-EZ)*DZ-nearp;
   d3=(p3x-EX)*DX+(p3y-EY)*DY+(p3z-EZ)*DZ-nearp;
   d4=(p4x-EX)*DX+(p4y-EY)*DY+(p4z-EZ)*DZ-nearp;

   if (d1<0.0f) flag|=1;
   if (d2<0.0f) flag|=2;
   if (d3<0.0f) flag|=4;
   if (d4<0.0f) flag|=8;

   switch (flag)
      {
      case 1: case 14: cliptetra1(p1x,p1y,p1z,fabs(d1),p2x,p2y,p2z,fabs(d2),p3x,p3y,p3z,fabs(d3),p4x,p4y,p4z,fabs(d4)); break;
      case 2: case 13: cliptetra1(p2x,p2y,p2z,fabs(d2),p1x,p1y,p1z,fabs(d1),p3x,p3y,p3z,fabs(d3),p4x,p4y,p4z,fabs(d4)); break;
      case 4: case 11: cliptetra1(p3x,p3y,p3z,fabs(d3),p1x,p1y,p1z,fabs(d1),p2x,p2y,p2z,fabs(d2),p4x,p4y,p4z,fabs(d4)); break;
      case 8: case 7: cliptetra1(p4x,p4y,p4z,fabs(d4),p1x,p1y,p1z,fabs(d1),p2x,p2y,p2z,fabs(d2),p3x,p3y,p3z,fabs(d3)); break;

      case 3: cliptetra2(p1x,p1y,p1z,fabs(d1),p2x,p2y,p2z,fabs(d2),p3x,p3y,p3z,fabs(d3),p4x,p4y,p4z,fabs(d4)); break;
      case 5: cliptetra2(p1x,p1y,p1z,fabs(d1),p3x,p3y,p3z,fabs(d3),p2x,p2y,p2z,fabs(d2),p4x,p4y,p4z,fabs(d4)); break;
      case 6: cliptetra2(p2x,p2y,p2z,fabs(d2),p3x,p3y,p3z,fabs(d3),p1x,p1y,p1z,fabs(d1),p4x,p4y,p4z,fabs(d4)); break;
      case 9: cliptetra2(p1x,p1y,p1z,fabs(d1),p4x,p4y,p4z,fabs(d4),p2x,p2y,p2z,fabs(d2),p3x,p3y,p3z,fabs(d3)); break;
      case 10: cliptetra2(p2x,p2y,p2z,fabs(d2),p4x,p4y,p4z,fabs(d4),p1x,p1y,p1z,fabs(d1),p3x,p3y,p3z,fabs(d3)); break;
      case 12: cliptetra2(p3x,p3y,p3z,fabs(d3),p4x,p4y,p4z,fabs(d4),p1x,p1y,p1z,fabs(d1),p2x,p2y,p2z,fabs(d2)); break;
      }
   }

void drawbase(const float p1x,const float p1y,const float p1z,
              const float p2x,const float p2y,const float p2z,
              const float p3x,const float p3y,const float p3z,
              const float p4x,const float p4y,const float p4z,
              const float p5x,const float p5y,const float p5z,
              const float p6x,const float p6y,const float p6z,
              const float alpha)
   {
   disableculling();

   beginfans();
   color(0.0f,0.0f,0.0f,alpha);
   cliptetra(p1x,p1y,p1z,p2x,p2y,p2z,p3x,p3y,p3z,p4x,p4y,p4z);
   cliptetra(p2x,p2y,p2z,p3x,p3y,p3z,p4x,p4y,p4z,p5x,p5y,p5z);
   cliptetra(p3x,p3y,p3z,p4x,p4y,p4z,p5x,p5y,p5z,p6x,p6y,p6z);
   endfans();

   enableBFculling();
   }

// render the cached prisms using an emissive optical model
// requires an alpha channel in the frame buffer
void drawfog(float *prismcache,int prismcnt)
   {
   int i;

   float *prismptr;

   float p1x,p2x,p3x,p4x,p5x,p6x;
   float p1y,p2y,p3y,p4y,p5y,p6y;
   float p1z,p2z,p3z,p4z,p5z,p6z;

   float d1,d2,d3,d4,d5,d6,mind,maxd;
   float emission;

   for (i=0; i<prismcnt; i++)
      {
      enableAwriting();

      prismptr=&prismcache[12*i];

      // fetch vertices
      p1x=p4x=*prismptr++;
      p1y=*prismptr++;
      p4y=*prismptr++;
      p1z=p4z=*prismptr++;
      p2x=p5x=*prismptr++;
      p2y=*prismptr++;
      p5y=*prismptr++;
      p2z=p5z=*prismptr++;
      p3x=p6x=*prismptr++;
      p3y=*prismptr++;
      p6y=*prismptr++;
      p3z=p6z=*prismptr;

      // calculate distances to base plane
      d1=(p1x-EX)*DX+(p1y-EY)*DY+(p1z-EZ)*DZ;
      d2=(p2x-EX)*DX+(p2y-EY)*DY+(p2z-EZ)*DZ;
      d3=(p3x-EX)*DX+(p3y-EY)*DY+(p3z-EZ)*DZ;
      d4=(p4x-EX)*DX+(p4y-EY)*DY+(p4z-EZ)*DZ;
      d5=(p5x-EX)*DX+(p5y-EY)*DY+(p5z-EZ)*DZ;
      d6=(p6x-EX)*DX+(p6y-EY)*DY+(p6z-EZ)*DZ;

      // compute distance range
      mind=maxd=d1;
      if (d2<mind) mind=d2;
      else if (d2>maxd) maxd=d2;
      if (d3<mind) mind=d3;
      else if (d3>maxd) maxd=d3;
      if (d4<mind) mind=d4;
      else if (d4>maxd) maxd=d4;
      if (d5<mind) mind=d5;
      else if (d5>maxd) maxd=d5;
      if (d6<mind) mind=d6;
      else if (d6>maxd) maxd=d6;

      if (maxd<=NEARP) continue; // near plane culling

      // normalize distances
      maxd-=mind;
      d1=(d1-mind)/maxd;
      d2=(d2-mind)/maxd;
      d3=(d3-mind)/maxd;
      d4=(d4-mind)/maxd;
      d5=(d5-mind)/maxd;
      d6=(d6-mind)/maxd;

      // render back faces
      // -> dst_alpha=1-distance
      beginfans();
      beginfan();
      color(0.0f,0.0f,0.0f,1.0f-d1);
      fanvertex(p1x,p1y,p1z);
      color(0.0f,0.0f,0.0f,1.0f-d3);
      fanvertex(p3x,p3y,p3z);
      color(0.0f,0.0f,0.0f,1.0f-d4);
      fanvertex(p4x,p4y,p4z);
      color(0.0f,0.0f,0.0f,1.0f-d5);
      fanvertex(p5x,p5y,p5z);
      color(0.0f,0.0f,0.0f,1.0f-d2);
      fanvertex(p2x,p2y,p2z);
      color(0.0f,0.0f,0.0f,1.0f-d3);
      fanvertex(p3x,p3y,p3z);
      beginfan();
      color(0.0f,0.0f,0.0f,1.0f-d6);
      fanvertex(p6x,p6y,p6z);
      color(0.0f,0.0f,0.0f,1.0f-d3);
      fanvertex(p3x,p3y,p3z);
      color(0.0f,0.0f,0.0f,1.0f-d2);
      fanvertex(p2x,p2y,p2z);
      color(0.0f,0.0f,0.0f,1.0f-d5);
      fanvertex(p5x,p5y,p5z);
      color(0.0f,0.0f,0.0f,1.0f-d4);
      fanvertex(p4x,p4y,p4z);
      color(0.0f,0.0f,0.0f,1.0f-d3);
      fanvertex(p3x,p3y,p3z);
      endfans();

      enableADDblending();

      // blend front faces
      // -> dst_alpha=1-segment_length
      beginfans();
      beginfan();
      color(0.0f,0.0f,0.0f,d1);
      fanvertex(p1x,p1y,p1z);
      color(0.0f,0.0f,0.0f,d3);
      fanvertex(p3x,p3y,p3z);
      color(0.0f,0.0f,0.0f,d2);
      fanvertex(p2x,p2y,p2z);
      color(0.0f,0.0f,0.0f,d5);
      fanvertex(p5x,p5y,p5z);
      color(0.0f,0.0f,0.0f,d4);
      fanvertex(p4x,p4y,p4z);
      color(0.0f,0.0f,0.0f,d3);
      fanvertex(p3x,p3y,p3z);
      beginfan();
      color(0.0f,0.0f,0.0f,d6);
      fanvertex(p6x,p6y,p6z);
      color(0.0f,0.0f,0.0f,d3);
      fanvertex(p3x,p3y,p3z);
      color(0.0f,0.0f,0.0f,d4);
      fanvertex(p4x,p4y,p4z);
      color(0.0f,0.0f,0.0f,d5);
      fanvertex(p5x,p5y,p5z);
      color(0.0f,0.0f,0.0f,d2);
      fanvertex(p2x,p2y,p2z);
      color(0.0f,0.0f,0.0f,d3);
      fanvertex(p3x,p3y,p3z);
      endfans();

      // treat clipped parts behind the near plane
      if (mind<NEARP) drawbase(p1x,p1y,p1z,p2x,p2y,p2z,p3x,p3y,p3z,
                               p4x,p4y,p4z,p5x,p5y,p5z,p6x,p6y,p6z,
                               (NEARP-mind)/maxd);

      enablePRJblending();
      enableRGBwriting();

      emission=EMISSION*maxd;

      // blend back faces
      // -> dst_color+=segment_length*emission*fogcolor
      beginfans();
      color(emission*FR,emission*FG,emission*FB);
      beginfan();
      fanvertex(p1x,p1y,p1z);
      fanvertex(p3x,p3y,p3z);
      fanvertex(p4x,p4y,p4z);
      fanvertex(p5x,p5y,p5z);
      fanvertex(p2x,p2y,p2z);
      fanvertex(p3x,p3y,p3z);
      beginfan();
      fanvertex(p6x,p6y,p6z);
      fanvertex(p3x,p3y,p3z);
      fanvertex(p2x,p2y,p2z);
      fanvertex(p5x,p5y,p5z);
      fanvertex(p4x,p4y,p4z);
      fanvertex(p3x,p3y,p3z);
      endfans();

      disableblending();
      }
   }

// prism clipping functions:

inline void cliptetra1(const float p1x,const float p1y,const float p1z,const float d1,const float c1,
                       const float p2x,const float p2y,const float p2z,const float d2,const float c2,
                       const float p3x,const float p3y,const float p3z,const float d3,const float c3,
                       const float p4x,const float p4y,const float p4z,const float d4,const float c4)
   {
   float pp1x,pp1y,pp1z,pc1,
         pp2x,pp2y,pp2z,pc2,
         pp3x,pp3y,pp3z,pc3;

   pp1x=(d2*p1x+d1*p2x)/(d1+d2);
   pp1y=(d2*p1y+d1*p2y)/(d1+d2);
   pp1z=(d2*p1z+d1*p2z)/(d1+d2);
   pc1=(d2*c1+d1*c2)/(d1+d2);
   pp2x=(d3*p1x+d1*p3x)/(d1+d3);
   pp2y=(d3*p1y+d1*p3y)/(d1+d3);
   pp2z=(d3*p1z+d1*p3z)/(d1+d3);
   pc2=(d3*c1+d1*c3)/(d1+d3);
   pp3x=(d4*p1x+d1*p4x)/(d1+d4);
   pp3y=(d4*p1y+d1*p4y)/(d1+d4);
   pp3z=(d4*p1z+d1*p4z)/(d1+d4);
   pc3=(d4*c1+d1*c4)/(d1+d4);

   beginfan();
   color(pc1*FR,pc1*FG,pc1*FB);
   fanvertex(pp1x,pp1y,pp1z);
   color(pc2*FR,pc2*FG,pc2*FB);
   fanvertex(pp2x,pp2y,pp2z);
   color(pc3*FR,pc3*FG,pc3*FB);
   fanvertex(pp3x,pp3y,pp3z);
   beginfan();
   color(pc1*FR,pc1*FG,pc1*FB);
   fanvertex(pp1x,pp1y,pp1z);
   color(pc3*FR,pc3*FG,pc3*FB);
   fanvertex(pp3x,pp3y,pp3z);
   color(pc2*FR,pc2*FG,pc2*FB);
   fanvertex(pp2x,pp2y,pp2z);
   }

inline void cliptetra2(const float p1x,const float p1y,const float p1z,const float d1,const float c1,
                       const float p2x,const float p2y,const float p2z,const float d2,const float c2,
                       const float p3x,const float p3y,const float p3z,const float d3,const float c3,
                       const float p4x,const float p4y,const float p4z,const float d4,const float c4)
   {
   float pp1x,pp1y,pp1z,pc1,
         pp2x,pp2y,pp2z,pc2,
         pp3x,pp3y,pp3z,pc3,
         pp4x,pp4y,pp4z,pc4;

   pp1x=(d3*p1x+d1*p3x)/(d1+d3);
   pp1y=(d3*p1y+d1*p3y)/(d1+d3);
   pp1z=(d3*p1z+d1*p3z)/(d1+d3);
   pc1=(d3*c1+d1*c3)/(d1+d3);
   pp2x=(d3*p2x+d2*p3x)/(d2+d3);
   pp2y=(d3*p2y+d2*p3y)/(d2+d3);
   pp2z=(d3*p2z+d2*p3z)/(d2+d3);
   pc2=(d3*c2+d2*c3)/(d2+d3);
   pp3x=(d4*p1x+d1*p4x)/(d1+d4);
   pp3y=(d4*p1y+d1*p4y)/(d1+d4);
   pp3z=(d4*p1z+d1*p4z)/(d1+d4);
   pc3=(d4*c1+d1*c4)/(d1+d4);
   pp4x=(d4*p2x+d2*p4x)/(d2+d4);
   pp4y=(d4*p2y+d2*p4y)/(d2+d4);
   pp4z=(d4*p2z+d2*p4z)/(d2+d4);
   pc4=(d4*c2+d2*c4)/(d2+d4);

   beginfan();
   color(pc1*FR,pc1*FG,pc1*FB);
   fanvertex(pp1x,pp1y,pp1z);
   color(pc2*FR,pc2*FG,pc2*FB);
   fanvertex(pp2x,pp2y,pp2z);
   color(pc4*FR,pc4*FG,pc4*FB);
   fanvertex(pp4x,pp4y,pp4z);
   color(pc3*FR,pc3*FG,pc3*FB);
   fanvertex(pp3x,pp3y,pp3z);
   beginfan();
   color(pc1*FR,pc1*FG,pc1*FB);
   fanvertex(pp1x,pp1y,pp1z);
   color(pc3*FR,pc3*FG,pc3*FB);
   fanvertex(pp3x,pp3y,pp3z);
   color(pc4*FR,pc4*FG,pc4*FB);
   fanvertex(pp4x,pp4y,pp4z);
   color(pc2*FR,pc2*FG,pc2*FB);
   fanvertex(pp2x,pp2y,pp2z);
   }

inline void cliptetra(const float p1x,const float p1y,const float p1z,const float c1,
                      const float p2x,const float p2y,const float p2z,const float c2,
                      const float p3x,const float p3y,const float p3z,const float c3,
                      const float p4x,const float p4y,const float p4z,const float c4)
   {
   int flag=0;

   float d1,d2,d3,d4;

   float nearp=1.0001f*NEARP;

   d1=(p1x-EX)*DX+(p1y-EY)*DY+(p1z-EZ)*DZ-nearp;
   d2=(p2x-EX)*DX+(p2y-EY)*DY+(p2z-EZ)*DZ-nearp;
   d3=(p3x-EX)*DX+(p3y-EY)*DY+(p3z-EZ)*DZ-nearp;
   d4=(p4x-EX)*DX+(p4y-EY)*DY+(p4z-EZ)*DZ-nearp;

   if (d1<0.0f) flag|=1;
   if (d2<0.0f) flag|=2;
   if (d3<0.0f) flag|=4;
   if (d4<0.0f) flag|=8;

   switch (flag)
      {
      case 1: case 14: cliptetra1(p1x,p1y,p1z,fabs(d1),c1,p2x,p2y,p2z,fabs(d2),c2,p3x,p3y,p3z,fabs(d3),c3,p4x,p4y,p4z,fabs(d4),c4); break;
      case 2: case 13: cliptetra1(p2x,p2y,p2z,fabs(d2),c2,p1x,p1y,p1z,fabs(d1),c1,p3x,p3y,p3z,fabs(d3),c3,p4x,p4y,p4z,fabs(d4),c4); break;
      case 4: case 11: cliptetra1(p3x,p3y,p3z,fabs(d3),c3,p1x,p1y,p1z,fabs(d1),c1,p2x,p2y,p2z,fabs(d2),c2,p4x,p4y,p4z,fabs(d4),c4); break;
      case 8: case 7: cliptetra1(p4x,p4y,p4z,fabs(d4),c4,p1x,p1y,p1z,fabs(d1),c1,p2x,p2y,p2z,fabs(d2),c2,p3x,p3y,p3z,fabs(d3),c3); break;

      case 3: cliptetra2(p1x,p1y,p1z,fabs(d1),c1,p2x,p2y,p2z,fabs(d2),c2,p3x,p3y,p3z,fabs(d3),c3,p4x,p4y,p4z,fabs(d4),c4); break;
      case 5: cliptetra2(p1x,p1y,p1z,fabs(d1),c1,p3x,p3y,p3z,fabs(d3),c3,p2x,p2y,p2z,fabs(d2),c2,p4x,p4y,p4z,fabs(d4),c4); break;
      case 6: cliptetra2(p2x,p2y,p2z,fabs(d2),c2,p3x,p3y,p3z,fabs(d3),c3,p1x,p1y,p1z,fabs(d1),c1,p4x,p4y,p4z,fabs(d4),c4); break;
      case 9: cliptetra2(p1x,p1y,p1z,fabs(d1),c1,p4x,p4y,p4z,fabs(d4),c4,p2x,p2y,p2z,fabs(d2),c2,p3x,p3y,p3z,fabs(d3),c3); break;
      case 10: cliptetra2(p2x,p2y,p2z,fabs(d2),c2,p4x,p4y,p4z,fabs(d4),c4,p1x,p1y,p1z,fabs(d1),c1,p3x,p3y,p3z,fabs(d3),c3); break;
      case 12: cliptetra2(p3x,p3y,p3z,fabs(d3),c3,p4x,p4y,p4z,fabs(d4),c4,p1x,p1y,p1z,fabs(d1),c1,p2x,p2y,p2z,fabs(d2),c2); break;
      }
   }

// render the cached prisms using maximum intensity projection
void mipfog(float *prismcache,int prismcnt)
   {
   int i;

   float *prismptr;

   float p1x,p2x,p3x,p4x,p5x,p6x;
   float p1y,p2y,p3y,p4y,p5y,p6y;
   float p1z,p2z,p3z,p4z,p5z,p6z;

   float c1,c2,c3,c4,c5,c6;

   enableMIPblending();

   beginfans();
   for (i=0; i<prismcnt; i++)
      {
      prismptr=&prismcache[12*i];

      // fetch vertices
      p1x=p4x=*prismptr++;
      p1y=*prismptr++;
      p4y=*prismptr++;
      p1z=p4z=*prismptr++;
      p2x=p5x=*prismptr++;
      p2y=*prismptr++;
      p5y=*prismptr++;
      p2z=p5z=*prismptr++;
      p3x=p6x=*prismptr++;
      p3y=*prismptr++;
      p6y=*prismptr++;
      p3z=p6z=*prismptr;

      // calculate vertex colors
      if (tid!=0)
         {
         c1=(p4y-p1y)/(255.0f*LAMBDA);
         c2=(p5y-p2y)/(255.0f*LAMBDA);
         c3=(p6y-p3y)/(255.0f*LAMBDA);
         c4=c5=c6=0.0f;
         }
      else
         {
         c1=c2=c3=0.0f;
         c4=(p4y-p1y)/(255.0f*LAMBDA);
         c5=(p5y-p2y)/(255.0f*LAMBDA);
         c6=(p6y-p3y)/(255.0f*LAMBDA);
         }

      // render faces
      beginfan();
      color(c1*FR,c1*FG,c1*FB);
      fanvertex(p1x,p1y,p1z);
      color(c3*FR,c3*FG,c3*FB);
      fanvertex(p3x,p3y,p3z);
      color(c4*FR,c4*FG,c4*FB);
      fanvertex(p4x,p4y,p4z);
      color(c5*FR,c5*FG,c5*FB);
      fanvertex(p5x,p5y,p5z);
      color(c2*FR,c2*FG,c2*FB);
      fanvertex(p2x,p2y,p2z);
      color(c3*FR,c3*FG,c3*FB);
      fanvertex(p3x,p3y,p3z);
      color(c2*FR,c2*FG,c2*FB);
      fanvertex(p2x,p2y,p2z);
      beginfan();
      color(c6*FR,c6*FG,c6*FB);
      fanvertex(p6x,p6y,p6z);
      color(c4*FR,c4*FG,c4*FB);
      fanvertex(p4x,p4y,p4z);
      color(c3*FR,c3*FG,c3*FB);
      fanvertex(p3x,p3y,p3z);
      color(c2*FR,c2*FG,c2*FB);
      fanvertex(p2x,p2y,p2z);
      color(c5*FR,c5*FG,c5*FB);
      fanvertex(p5x,p5y,p5z);
      color(c4*FR,c4*FG,c4*FB);
      fanvertex(p4x,p4y,p4z);
      color(c5*FR,c5*FG,c5*FB);
      fanvertex(p5x,p5y,p5z);

      cliptetra(p1x,p1y,p1z,c1,p2x,p2y,p2z,c2,p3x,p3y,p3z,c3,p4x,p4y,p4z,c4);
      cliptetra(p2x,p2y,p2z,c2,p3x,p3y,p3z,c3,p4x,p4y,p4z,c4,p5x,p5y,p5z,c5);
      cliptetra(p3x,p3y,p3z,c3,p4x,p4y,p4z,c4,p5x,p5y,p5z,c5,p6x,p6y,p6z,c6);
      }
   endfans();

   disableblending();
   }

// fog blending functions:

inline float blendFD(const int i,const int j,const float yf1,const float yf2)
   {return(blendE(bc[i][j],yf[i][j],yf1,yf2));}

inline float blendFV(const int i,const int j,const int s2,const float yf1,const float yf2)
   {return(blendM(min((i>0)?bc[i-s2][j]:maxbc(0,S-1-s2,j,s2),
                      (i<S-1)?bc[i+s2][j]:maxbc(1,s2,j,s2)),yf[i][j],yf1,yf2));}

inline float blendFH(const int i,const int j,const int s2,const float yf1,const float yf2)
   {return(blendM(min((j>0)?bc[i][j-s2]:maxbc(2,i,S-1-s2,s2),
                      (j<S-1)?bc[i][j+s2]:maxbc(3,i,s2,s2)),yf[i][j],yf1,yf2));}

// geomorph the triangulation and the prisms
void drawmap(const int i,const int j,const int s,
             const float e1,const float e2,const float e3,const float e4,
             const float f1,const float f2,const float f3,const float f4)
   {
   int s2,s4;

   float dx,dy,dz;

   float dh;

   float l,d;

   float m0,m1,m2,m3,m4,
         mf0,mf1,mf2,mf3,mf4;

   int bc1,bc2,bc3,bc4;

   s2=s/2;
   s4=s/4;

   // blend the center vertex
   if (((i+j)/s)%2==1) {m0=blendD(i,j,e1,e3); mf0=blendFD(i,j,f1,f3);}
   else {m0=blendD(i,j,e2,e4); mf0=blendFD(i,j,f2,f4);}

   // view frustum culling
   if (CULLING && s>(S>>maxcull))
      {
      dx=X(i)-EX;
      dy=m0*SCALE-EY;
      dz=Z(j)-EZ;

      dh=DH[s];

      l=DX*dx+DY*dy+DZ*dz;
      d=k1*s2+k2*dh;

      if (l<NEARP-d || l>FARP+d) return;

      if (!ORTHO)
         {
         if (nx1*dx+ny1*dy+nz1*dz>k11*s2+k12*dh) return;
         if (nx2*dx+ny2*dy+nz2*dz>k21*s2+k22*dh) return;
         if (nx3*dx+ny3*dy+nz3*dz>k31*s2+k32*dh) return;
         if (nx4*dx+ny4*dy+nz4*dz>k41*s2+k42*dh) return;
         }
      else
         {
         if (fabs(RX*dx+RY*dy+RZ*dz)>k11*s2+k12*dh+k31) return;
         if (fabs(UX*dx+UY*dy+UZ*dz)>k21*s2+k22*dh+k32) return;
         }
      }

   // blend the edge vertices
   m1=blendV(i+s2,j,s2,e4,e1);
   mf1=blendFV(i+s2,j,s2,f4,f1);
   m2=blendH(i,j+s2,s2,e1,e2);
   mf2=blendFH(i,j+s2,s2,f1,f2);
   m3=blendV(i-s2,j,s2,e2,e3);
   mf3=blendFV(i-s2,j,s2,f2,f3);
   m4=blendH(i,j-s2,s2,e3,e4);
   mf4=blendFH(i,j-s2,s2,f3,f4);

   // subdivision factors
   if (s4>0)
      {
      bc1=bc[i+s4][j+s4];
      bc2=bc[i-s4][j+s4];
      bc3=bc[i-s4][j-s4];
      bc4=bc[i+s4][j-s4];
      }
   else bc1=bc2=bc3=bc4=0;

   // first triangle fan quarter
   if (bc1!=0)
      {
      if (m1==-MAXFLOAT) {m1=(e4+e1)/2.0f; mf1=(f4+f1)/2.0f;}
      if (m2==-MAXFLOAT) {m2=(e1+e2)/2.0f; mf2=(f1+f2)/2.0f;}
      drawmap(i+s4,j+s4,s2,e1,m2,m0,m1,f1,mf2,mf0,mf1);
      }
   else
      {
      BEGINFAN();
      FANVERTEX(i,m0,mf0,j);
      if (bc4!=0)
         {
         if (m1==-MAXFLOAT) {m1=(e4+e1)/2.0f; mf1=(f4+f1)/2.0f;}
         FANVERTEX(i+s2,m1,mf1,j);
         }
      else if (m1!=-MAXFLOAT) FANVERTEX(i+s2,m1,mf1,j);
      FANVERTEX(i+s2,e1,f1,j+s2);
      }

   // second triangle fan quarter
   if (bc2!=0)
      {
      if (bc1==0)
         {
         if (m2==-MAXFLOAT) {m2=(e1+e2)/2.0f; mf2=(f1+f2)/2.0f;}
         FANVERTEX(i,m2,mf2,j+s2);
         }
      if (m3==-MAXFLOAT) {m3=(e2+e3)/2.0f; mf3=(f2+f3)/2.0f;}
      drawmap(i-s4,j+s4,s2,m2,e2,m3,m0,mf2,f2,mf3,mf0);
      }
   else
      {
      if (bc1!=0)
         {
         BEGINFAN();
         FANVERTEX(i,m0,mf0,j);
         FANVERTEX(i,m2,mf2,j+s2);
         }
      else if (m2!=-MAXFLOAT) FANVERTEX(i,m2,mf2,j+s2);
      FANVERTEX(i-s2,e2,f2,j+s2);
      }

   // third triangle fan quarter
   if (bc3!=0)
      {
      if (bc2==0)
         {
         if (m3==-MAXFLOAT) {m3=(e2+e3)/2.0f; mf3=(f2+f3)/2.0f;}
         FANVERTEX(i-s2,m3,mf3,j);
         }
      if (m4==-MAXFLOAT) {m4=(e3+e4)/2.0f; mf4=(f3+f4)/2.0f;}
      drawmap(i-s4,j-s4,s2,m0,m3,e3,m4,mf0,mf3,f3,mf4);
      }
   else
      {
      if (bc2!=0)
         {
         BEGINFAN();
         FANVERTEX(i,m0,mf0,j);
         FANVERTEX(i-s2,m3,mf3,j);
         }
      else if (m3!=-MAXFLOAT) FANVERTEX(i-s2,m3,mf3,j);
      FANVERTEX(i-s2,e3,f3,j-s2);
      }

   // fourth triangle fan quarter
   if (bc4!=0)
      {
      if (bc3==0)
         {
         if (m4==-MAXFLOAT) {m4=(e3+e4)/2.0f; mf4=(f3+f4)/2.0f;}
         FANVERTEX(i,m4,mf4,j-s2);
         }
      drawmap(i+s4,j-s4,s2,m1,m0,m4,e4,mf1,mf0,mf4,f4);
      }
   else
      {
      if (bc3!=0)
         {
         BEGINFAN();
         FANVERTEX(i,m0,mf0,j);
         FANVERTEX(i,m4,mf4,j-s2);
         }
      else if (m4!=-MAXFLOAT) FANVERTEX(i,m4,mf4,j-s2);
      FANVERTEX(i+s2,e4,f4,j-s2);
      if (bc1==0)
         if (m1!=-MAXFLOAT) FANVERTEX(i+s2,m1,mf1,j);
         else FANVERTEX(i+s2,e1,f1,j+s2);
      else FANVERTEX(i+s2,m1,mf1,j);
      }
   }

// geomorph and pipe the triangulation
void pipemap(const int i,const int j,const int s,
             const float e1,const float e2,const float e3,const float e4)
   {
   int s2,s4;

   float dx,dy,dz;

   float dh;

   float l,d;

   float m0,m1,m2,m3,m4;

   int bc1,bc2,bc3,bc4;

   s2=s/2;
   s4=s/4;

   // blend the center vertex
   if (((i+j)/s)%2==1) m0=blendD(i,j,e1,e3);
   else m0=blendD(i,j,e2,e4);

   // view frustum culling
   if (CULLING && s>(S>>maxcull))
      {
      dx=X(i)-EX;
      dy=m0*SCALE-EY;
      dz=Z(j)-EZ;

      dh=DH[s];

      l=DX*dx+DY*dy+DZ*dz;
      d=k1*s2+k2*dh;

      if (l<NEARP-d || l>FARP+d) return;

      if (!ORTHO)
         {
         if (nx1*dx+ny1*dy+nz1*dz>k11*s2+k12*dh) return;
         if (nx2*dx+ny2*dy+nz2*dz>k21*s2+k22*dh) return;
         if (nx3*dx+ny3*dy+nz3*dz>k31*s2+k32*dh) return;
         if (nx4*dx+ny4*dy+nz4*dz>k41*s2+k42*dh) return;
         }
      else
         {
         if (fabs(RX*dx+RY*dy+RZ*dz)>k11*s2+k12*dh+k31) return;
         if (fabs(UX*dx+UY*dy+UZ*dz)>k21*s2+k22*dh+k32) return;
         }
      }

   // trigger the notify callback
   if (notify_callback!=NULL) notify_callback(i,j,s);

   // blend the edge vertices
   m1=blendV(i+s2,j,s2,e4,e1);
   m2=blendH(i,j+s2,s2,e1,e2);
   m3=blendV(i-s2,j,s2,e2,e3);
   m4=blendH(i,j-s2,s2,e3,e4);

   // subdivision factors
   if (s4>0)
      {
      bc1=bc[i+s4][j+s4];
      bc2=bc[i-s4][j+s4];
      bc3=bc[i-s4][j-s4];
      bc4=bc[i+s4][j-s4];
      }
   else bc1=bc2=bc3=bc4=0;

   // first triangle fan quarter
   if (bc1!=0)
      {
      if (m1==-MAXFLOAT) m1=(e4+e1)/2.0f;
      if (m2==-MAXFLOAT) m2=(e1+e2)/2.0f;
      pipemap(i+s4,j+s4,s2,e1,m2,m0,m1);
      }
   else
      {
      beginfan_callback();
      fanvertex_callback(i,m0,j);
      if (bc4!=0)
         {
         if (m1==-MAXFLOAT) m1=(e4+e1)/2.0f;
         fanvertex_callback(i+s2,m1,j);
         }
      else if (m1!=-MAXFLOAT) fanvertex_callback(i+s2,m1,j);
      fanvertex_callback(i+s2,e1,j+s2);
      }

   // second triangle fan quarter
   if (bc2!=0)
      {
      if (bc1==0)
         {
         if (m2==-MAXFLOAT) m2=(e1+e2)/2.0f;
         fanvertex_callback(i,m2,j+s2);
         }
      if (m3==-MAXFLOAT) m3=(e2+e3)/2.0f;
      pipemap(i-s4,j+s4,s2,m2,e2,m3,m0);
      }
   else
      {
      if (bc1!=0)
         {
         beginfan_callback();
         fanvertex_callback(i,m0,j);
         fanvertex_callback(i,m2,j+s2);
         }
      else if (m2!=-MAXFLOAT) fanvertex_callback(i,m2,j+s2);
      fanvertex_callback(i-s2,e2,j+s2);
      }

   // third triangle fan quarter
   if (bc3!=0)
      {
      if (bc2==0)
         {
         if (m3==-MAXFLOAT) m3=(e2+e3)/2.0f;
         fanvertex_callback(i-s2,m3,j);
         }
      if (m4==-MAXFLOAT) m4=(e3+e4)/2.0f;
      pipemap(i-s4,j-s4,s2,m0,m3,e3,m4);
      }
   else
      {
      if (bc2!=0)
         {
         beginfan_callback();
         fanvertex_callback(i,m0,j);
         fanvertex_callback(i-s2,m3,j);
         }
      else if (m3!=-MAXFLOAT) fanvertex_callback(i-s2,m3,j);
      fanvertex_callback(i-s2,e3,j-s2);
      }

   // fourth triangle fan quarter
   if (bc4!=0)
      {
      if (bc3==0)
         {
         if (m4==-MAXFLOAT) m4=(e3+e4)/2.0f;
         fanvertex_callback(i,m4,j-s2);
         }
      pipemap(i+s4,j-s4,s2,m1,m0,m4,e4);
      }
   else
      {
      if (bc3!=0)
         {
         beginfan_callback();
         fanvertex_callback(i,m0,j);
         fanvertex_callback(i,m4,j-s2);
         }
      else if (m4!=-MAXFLOAT) fanvertex_callback(i,m4,j-s2);
      fanvertex_callback(i+s2,e4,j-s2);
      if (bc1==0)
         if (m1!=-MAXFLOAT) fanvertex_callback(i+s2,m1,j);
         else fanvertex_callback(i+s2,e1,j+s2);
      else fanvertex_callback(i+s2,m1,j);
      }
   }

// sea piping functions:

inline void extractshore(const float i1,const float y1,const float j1,
                         const float i2,const float y2,const float j2,
                         const float i3,const float y3,const float j3)
   {
   float d1,d2;

   float im1,jm1,im2,jm2;

   d1=(y1-SEALEVEL)/(y1-y2);
   d2=(y1-SEALEVEL)/(y1-y3);

   im1=i1*(1.0f-d1)+i2*d1;
   jm1=j1*(1.0f-d1)+j2*d1;

   im2=i1*(1.0f-d2)+i3*d2;
   jm2=j1*(1.0f-d2)+j3*d2;

   if (y1<SEALEVEL)
      {
      beginfan_callback();
      fanvertex_callback(i1,SEALEVEL,j1);
      fanvertex_callback(im1,SEALEVEL,jm1);
      fanvertex_callback(im2,SEALEVEL,jm2);
      }
   else
      {
      beginfan_callback();
      fanvertex_callback(im2,SEALEVEL,jm2);
      fanvertex_callback(im1,SEALEVEL,jm1);
      fanvertex_callback(i2,SEALEVEL,j2);
      fanvertex_callback(i3,SEALEVEL,j3);
      }
   }

inline void rendershore(const float i1,const float y1,const float j1,
                        const float i2,const float y2,const float j2,
                        const float i3,const float y3,const float j3)
   {
   int flag=0;

   if (y1<SEALEVEL) flag|=1;
   if (y2<SEALEVEL) flag|=2;
   if (y3<SEALEVEL) flag|=4;

   switch (flag)
      {
      case 1:
      case 2+4:
         extractshore(i1,y1,j1,i2,y2,j2,i3,y3,j3);
         break;
      case 2:
      case 4+1:
         extractshore(i2,y2,j2,i3,y3,j3,i1,y1,j1);
         break;
      case 4:
      case 1+2:
         extractshore(i3,y3,j3,i1,y1,j1,i2,y2,j2);
         break;
      case 1+2+4:
         beginfan_callback();
         fanvertex_callback(i1,SEALEVEL,j1);
         fanvertex_callback(i2,SEALEVEL,j2);
         fanvertex_callback(i3,SEALEVEL,j3);
         break;
      }
   }

inline void BEGINFAN_SEA()
   {SEASTATE=0;}

inline void FANVERTEX_SEA(const int i,const float y,const int j)
   {
   static int i0,i1;
   static float y0,y1;
   static int j0,j1;

   switch (SEASTATE)
      {
      case 0:
         i0=i; y0=y; j0=j;
         SEASTATE++;
         break;
      case 1:
         i1=i; y1=y; j1=j;
         SEASTATE++;
         break;
      default:
         rendershore(i0,y0,j0,i1,y1,j1,i,y,j);
         i1=i; y1=y; j1=j;
         break;
      }
   }

// calculate and pipe sea level
void pipesea(const int i,const int j,const int s,
             const float e1,const float e2,const float e3,const float e4)
   {
   int s2,s4;

   float dx,dy,dz;

   float dh;

   float l,d;

   float m0,m1,m2,m3,m4;

   int bc1,bc2,bc3,bc4;

   BOOLINT below,above;

   s2=s/2;
   s4=s/4;

   // blend the center vertex
   if (((i+j)/s)%2==1) m0=blendD(i,j,e1,e3);
   else m0=blendD(i,j,e2,e4);

   // view frustum culling
   if (CULLING && s>(S>>maxcull))
      {
      dx=X(i)-EX;
      dy=SEALEVEL*SCALE-EY;
      dz=Z(j)-EZ;

      l=DX*dx+DY*dy+DZ*dz;
      d=k1*s2;

      if (l<NEARP-d || l>FARP+d) return;

      if (!ORTHO)
         {
         if (nx1*dx+ny1*dy+nz1*dz>k11*s2) return;
         if (nx2*dx+ny2*dy+nz2*dz>k21*s2) return;
         if (nx3*dx+ny3*dy+nz3*dz>k31*s2) return;
         if (nx4*dx+ny4*dy+nz4*dz>k41*s2) return;
         }
      else
         {
         if (fabs(RX*dx+RY*dy+RZ*dz)>k11*s2+k31) return;
         if (fabs(UX*dx+UY*dy+UZ*dz)>k21*s2+k32) return;
         }
      }

   // trigger the notify callback
   if (notify_callback!=NULL) notify_callback(i,j,s);

   // blend the edge vertices
   m1=blendV(i+s2,j,s2,e4,e1);
   m2=blendH(i,j+s2,s2,e1,e2);
   m3=blendV(i-s2,j,s2,e2,e3);
   m4=blendH(i,j-s2,s2,e3,e4);

   // subdivision factors
   if (s4>0)
      {
      bc1=bc[i+s4][j+s4];
      bc2=bc[i-s4][j+s4];
      bc3=bc[i-s4][j-s4];
      bc4=bc[i+s4][j-s4];
      }
   else bc1=bc2=bc3=bc4=0;

   dh=DH[s];

   below=above=FALSE;

   // check if triangles are below or above sea level
   if (m0+dh<SEALEVEL)
      {
      below=TRUE;
      bc1=bc2=bc3=bc4=0;
      }
   else if (m0-dh>SEALEVEL)
      {
      above=TRUE;
      bc1=bc2=bc3=bc4=0;
      }
   else
      if (e1<SEALEVEL && e2<SEALEVEL && e3<SEALEVEL && e4<SEALEVEL &&
          (m0<SEALEVEL || m0==-MAXFLOAT) &&
          (m1<SEALEVEL || m1==-MAXFLOAT) &&
          (m2<SEALEVEL || m2==-MAXFLOAT) &&
          (m3<SEALEVEL || m3==-MAXFLOAT) &&
          (m4<SEALEVEL || m4==-MAXFLOAT)) below=TRUE;
      else
         if (e1>SEALEVEL && e2>SEALEVEL && e3>SEALEVEL && e4>SEALEVEL &&
             (m0>SEALEVEL || m0==-MAXFLOAT) &&
             (m1>SEALEVEL || m1==-MAXFLOAT) &&
             (m2>SEALEVEL || m2==-MAXFLOAT) &&
             (m3>SEALEVEL || m3==-MAXFLOAT) &&
             (m4>SEALEVEL || m4==-MAXFLOAT)) above=TRUE;

   if (below)
      {
      // first triangle fan quarter
      if (bc1!=0)
         {
         if (m1==-MAXFLOAT) m1=(e4+e1)/2.0f;
         if (m2==-MAXFLOAT) m2=(e1+e2)/2.0f;
         pipesea(i+s4,j+s4,s2,e1,m2,m0,m1);
         }
      else
         {
         beginfan_callback();
         fanvertex_callback(i,SEALEVEL,j);
         if (bc4!=0)
            {
            if (m1==-MAXFLOAT) m1=(e4+e1)/2.0f;
            fanvertex_callback(i+s2,SEALEVEL,j);
            }
         else if (m1!=-MAXFLOAT) fanvertex_callback(i+s2,SEALEVEL,j);
         fanvertex_callback(i+s2,SEALEVEL,j+s2);
         }

      // second triangle fan quarter
      if (bc2!=0)
         {
         if (bc1==0)
            {
            if (m2==-MAXFLOAT) m2=(e1+e2)/2.0f;
            fanvertex_callback(i,SEALEVEL,j+s2);
            }
         if (m3==-MAXFLOAT) m3=(e2+e3)/2.0f;
         pipesea(i-s4,j+s4,s2,m2,e2,m3,m0);
         }
      else
         {
         if (bc1!=0)
            {
            beginfan_callback();
            fanvertex_callback(i,SEALEVEL,j);
            fanvertex_callback(i,SEALEVEL,j+s2);
            }
         else if (m2!=-MAXFLOAT) fanvertex_callback(i,SEALEVEL,j+s2);
         fanvertex_callback(i-s2,SEALEVEL,j+s2);
         }

      // third triangle fan quarter
      if (bc3!=0)
         {
         if (bc2==0)
            {
            if (m3==-MAXFLOAT) m3=(e2+e3)/2.0f;
            fanvertex_callback(i-s2,SEALEVEL,j);
            }
         if (m4==-MAXFLOAT) m4=(e3+e4)/2.0f;
         pipesea(i-s4,j-s4,s2,m0,m3,e3,m4);
         }
      else
         {
         if (bc2!=0)
            {
            beginfan_callback();
            fanvertex_callback(i,SEALEVEL,j);
            fanvertex_callback(i-s2,SEALEVEL,j);
            }
         else if (m3!=-MAXFLOAT) fanvertex_callback(i-s2,SEALEVEL,j);
         fanvertex_callback(i-s2,SEALEVEL,j-s2);
         }

      // fourth triangle fan quarter
      if (bc4!=0)
         {
         if (bc3==0)
            {
            if (m4==-MAXFLOAT) m4=(e3+e4)/2.0f;
            fanvertex_callback(i,SEALEVEL,j-s2);
            }
         pipesea(i+s4,j-s4,s2,m1,m0,m4,e4);
         }
      else
         {
         if (bc3!=0)
            {
            beginfan_callback();
            fanvertex_callback(i,SEALEVEL,j);
            fanvertex_callback(i,SEALEVEL,j-s2);
            }
         else if (m4!=-MAXFLOAT) fanvertex_callback(i,SEALEVEL,j-s2);
         fanvertex_callback(i+s2,SEALEVEL,j-s2);
         if (bc1==0)
            if (m1!=-MAXFLOAT) fanvertex_callback(i+s2,SEALEVEL,j);
            else fanvertex_callback(i+s2,SEALEVEL,j+s2);
         else fanvertex_callback(i+s2,SEALEVEL,j);
         }
      }
   else
      if (!above)
         {
         // first triangle fan quarter
         if (bc1!=0)
            {
            if (m1==-MAXFLOAT) m1=(e4+e1)/2.0f;
            if (m2==-MAXFLOAT) m2=(e1+e2)/2.0f;
            pipesea(i+s4,j+s4,s2,e1,m2,m0,m1);
            }
         else
            {
            BEGINFAN_SEA();
            FANVERTEX_SEA(i,m0,j);
            if (bc4!=0)
               {
               if (m1==-MAXFLOAT) m1=(e4+e1)/2.0f;
               FANVERTEX_SEA(i+s2,m1,j);
               }
            else if (m1!=-MAXFLOAT) FANVERTEX_SEA(i+s2,m1,j);
            FANVERTEX_SEA(i+s2,e1,j+s2);
            }

         // second triangle fan quarter
         if (bc2!=0)
            {
            if (bc1==0)
               {
               if (m2==-MAXFLOAT) m2=(e1+e2)/2.0f;
               FANVERTEX_SEA(i,m2,j+s2);
               }
            if (m3==-MAXFLOAT) m3=(e2+e3)/2.0f;
            pipesea(i-s4,j+s4,s2,m2,e2,m3,m0);
            }
         else
            {
            if (bc1!=0)
               {
               BEGINFAN_SEA();
               FANVERTEX_SEA(i,m0,j);
               FANVERTEX_SEA(i,m2,j+s2);
               }
            else if (m2!=-MAXFLOAT) FANVERTEX_SEA(i,m2,j+s2);
            FANVERTEX_SEA(i-s2,e2,j+s2);
            }

         // third triangle fan quarter
         if (bc3!=0)
            {
            if (bc2==0)
               {
               if (m3==-MAXFLOAT) m3=(e2+e3)/2.0f;
               FANVERTEX_SEA(i-s2,m3,j);
               }
            if (m4==-MAXFLOAT) m4=(e3+e4)/2.0f;
            pipesea(i-s4,j-s4,s2,m0,m3,e3,m4);
            }
         else
            {
            if (bc2!=0)
               {
               BEGINFAN_SEA();
               FANVERTEX_SEA(i,m0,j);
               FANVERTEX_SEA(i-s2,m3,j);
               }
            else if (m3!=-MAXFLOAT) FANVERTEX_SEA(i-s2,m3,j);
            FANVERTEX_SEA(i-s2,e3,j-s2);
            }

         // fourth triangle fan quarter
         if (bc4!=0)
            {
            if (bc3==0)
               {
               if (m4==-MAXFLOAT) m4=(e3+e4)/2.0f;
               FANVERTEX_SEA(i,m4,j-s2);
               }
            pipesea(i+s4,j-s4,s2,m1,m0,m4,e4);
            }
         else
            {
            if (bc3!=0)
               {
               BEGINFAN_SEA();
               FANVERTEX_SEA(i,m0,j);
               FANVERTEX_SEA(i,m4,j-s2);
               }
            else if (m4!=-MAXFLOAT) FANVERTEX_SEA(i,m4,j-s2);
            FANVERTEX_SEA(i+s2,e4,j-s2);
            if (bc1==0)
               if (m1!=-MAXFLOAT) FANVERTEX_SEA(i+s2,m1,j);
               else FANVERTEX_SEA(i+s2,e1,j+s2);
            else FANVERTEX_SEA(i+s2,m1,j);
            }
         }
      else
         {
         // first quarter
         if (bc1!=0)
            {
            if (m1==-MAXFLOAT) m1=(e4+e1)/2.0f;
            if (m2==-MAXFLOAT) m2=(e1+e2)/2.0f;
            pipesea(i+s4,j+s4,s2,e1,m2,m0,m1);
            }

         // second quarter
         if (bc2!=0)
            {
            if (m2==-MAXFLOAT) m2=(e1+e2)/2.0f;
            if (m3==-MAXFLOAT) m3=(e2+e3)/2.0f;
            pipesea(i-s4,j+s4,s2,m2,e2,m3,m0);
            }

         // third quarter
         if (bc3!=0)
            {
            if (m3==-MAXFLOAT) m3=(e2+e3)/2.0f;
            if (m4==-MAXFLOAT) m4=(e3+e4)/2.0f;
            pipesea(i-s4,j-s4,s2,m0,m3,e3,m4);
            }

         // fourth quarter
         if (bc4!=0)
            {
            if (m4==-MAXFLOAT) m4=(e3+e4)/2.0f;
            if (m1==-MAXFLOAT) m1=(e4+e1)/2.0f;
            pipesea(i+s4,j-s4,s2,m1,m0,m4,e4);
            }
         }
   }

// prism piping functions:

inline void BEGINFAN_PRISMS()
   {
   FANSTATE=0;
   beginfan_callback();
   }

inline void FANVERTEX_PRISMS(const int i,const float y,const float yf,const int j)
   {
   static int i0,i1;
   static float y0,y1;
   static float yf0,yf1;
   static int j0,j1;

   switch (FANSTATE)
      {
      case 0:
         fanvertex_callback(i,y,j);
         i0=i; y0=y; yf0=yf; j0=j;
         FANSTATE++;
         break;
      case 1:
         fanvertex_callback(i,y,j);
         i1=i; y1=y; yf1=yf; j1=j;
         FANSTATE++;
         break;
      default:
         fanvertex_callback(i,y,j);
         if (yf0>0.0f || yf1>0.0f || yf>0.0f) cacheprism(i0,y0,yf0,j0,i1,y1,yf1,j1,i,y,yf,j);
         i1=i; y1=y; yf1=yf; j1=j;
         break;
      }
   }

// geomorph and pipe the triangulation and the prisms
void pipemap(const int i,const int j,const int s,
             const float e1,const float e2,const float e3,const float e4,
             const float f1,const float f2,const float f3,const float f4)
   {
   int s2,s4;

   float dx,dy,dz;

   float dh;

   float l,d;

   float m0,m1,m2,m3,m4,
         mf0,mf1,mf2,mf3,mf4;

   int bc1,bc2,bc3,bc4;

   s2=s/2;
   s4=s/4;

   // blend the center vertex
   if (((i+j)/s)%2==1) {m0=blendD(i,j,e1,e3); mf0=blendFD(i,j,f1,f3);}
   else {m0=blendD(i,j,e2,e4); mf0=blendFD(i,j,f2,f4);}

   // view frustum culling
   if (CULLING && s>(S>>maxcull))
      {
      dx=X(i)-EX;
      dy=m0*SCALE-EY;
      dz=Z(j)-EZ;

      dh=DH[s];

      l=DX*dx+DY*dy+DZ*dz;
      d=k1*s2+k2*dh;

      if (l<NEARP-d || l>FARP+d) return;

      if (!ORTHO)
         {
         if (nx1*dx+ny1*dy+nz1*dz>k11*s2+k12*dh) return;
         if (nx2*dx+ny2*dy+nz2*dz>k21*s2+k22*dh) return;
         if (nx3*dx+ny3*dy+nz3*dz>k31*s2+k32*dh) return;
         if (nx4*dx+ny4*dy+nz4*dz>k41*s2+k42*dh) return;
         }
      else
         {
         if (fabs(RX*dx+RY*dy+RZ*dz)>k11*s2+k12*dh+k31) return;
         if (fabs(UX*dx+UY*dy+UZ*dz)>k21*s2+k22*dh+k32) return;
         }
      }

   // trigger the notify callback
   if (notify_callback!=NULL) notify_callback(i,j,s);

   // blend the edge vertices
   m1=blendV(i+s2,j,s2,e4,e1);
   mf1=blendFV(i+s2,j,s2,f4,f1);
   m2=blendH(i,j+s2,s2,e1,e2);
   mf2=blendFH(i,j+s2,s2,f1,f2);
   m3=blendV(i-s2,j,s2,e2,e3);
   mf3=blendFV(i-s2,j,s2,f2,f3);
   m4=blendH(i,j-s2,s2,e3,e4);
   mf4=blendFH(i,j-s2,s2,f3,f4);

   // subdivision factors
   if (s4>0)
      {
      bc1=bc[i+s4][j+s4];
      bc2=bc[i-s4][j+s4];
      bc3=bc[i-s4][j-s4];
      bc4=bc[i+s4][j-s4];
      }
   else bc1=bc2=bc3=bc4=0;

   // first triangle fan quarter
   if (bc1!=0)
      {
      if (m1==-MAXFLOAT) {m1=(e4+e1)/2.0f; mf1=(f4+f1)/2.0f;}
      if (m2==-MAXFLOAT) {m2=(e1+e2)/2.0f; mf2=(f1+f2)/2.0f;}
      pipemap(i+s4,j+s4,s2,e1,m2,m0,m1,f1,mf2,mf0,mf1);
      }
   else
      {
      BEGINFAN_PRISMS();
      FANVERTEX_PRISMS(i,m0,mf0,j);
      if (bc4!=0)
         {
         if (m1==-MAXFLOAT) {m1=(e4+e1)/2.0f; mf1=(f4+f1)/2.0f;}
         FANVERTEX_PRISMS(i+s2,m1,mf1,j);
         }
      else if (m1!=-MAXFLOAT) FANVERTEX_PRISMS(i+s2,m1,mf1,j);
      FANVERTEX_PRISMS(i+s2,e1,f1,j+s2);
      }

   // second triangle fan quarter
   if (bc2!=0)
      {
      if (bc1==0)
         {
         if (m2==-MAXFLOAT) {m2=(e1+e2)/2.0f; mf2=(f1+f2)/2.0f;}
         FANVERTEX_PRISMS(i,m2,mf2,j+s2);
         }
      if (m3==-MAXFLOAT) {m3=(e2+e3)/2.0f; mf3=(f2+f3)/2.0f;}
      pipemap(i-s4,j+s4,s2,m2,e2,m3,m0,mf2,f2,mf3,mf0);
      }
   else
      {
      if (bc1!=0)
         {
         BEGINFAN_PRISMS();
         FANVERTEX_PRISMS(i,m0,mf0,j);
         FANVERTEX_PRISMS(i,m2,mf2,j+s2);
         }
      else if (m2!=-MAXFLOAT) FANVERTEX_PRISMS(i,m2,mf2,j+s2);
      FANVERTEX_PRISMS(i-s2,e2,f2,j+s2);
      }

   // third triangle fan quarter
   if (bc3!=0)
      {
      if (bc2==0)
         {
         if (m3==-MAXFLOAT) {m3=(e2+e3)/2.0f; mf3=(f2+f3)/2.0f;}
         FANVERTEX_PRISMS(i-s2,m3,mf3,j);
         }
      if (m4==-MAXFLOAT) {m4=(e3+e4)/2.0f; mf4=(f3+f4)/2.0f;}
      pipemap(i-s4,j-s4,s2,m0,m3,e3,m4,mf0,mf3,f3,mf4);
      }
   else
      {
      if (bc2!=0)
         {
         BEGINFAN_PRISMS();
         FANVERTEX_PRISMS(i,m0,mf0,j);
         FANVERTEX_PRISMS(i-s2,m3,mf3,j);
         }
      else if (m3!=-MAXFLOAT) FANVERTEX_PRISMS(i-s2,m3,mf3,j);
      FANVERTEX_PRISMS(i-s2,e3,f3,j-s2);
      }

   // fourth triangle fan quarter
   if (bc4!=0)
      {
      if (bc3==0)
         {
         if (m4==-MAXFLOAT) {m4=(e3+e4)/2.0f; mf4=(f3+f4)/2.0f;}
         FANVERTEX_PRISMS(i,m4,mf4,j-s2);
         }
      pipemap(i+s4,j-s4,s2,m1,m0,m4,e4,mf1,mf0,mf4,f4);
      }
   else
      {
      if (bc3!=0)
         {
         BEGINFAN_PRISMS();
         FANVERTEX_PRISMS(i,m0,mf0,j);
         FANVERTEX_PRISMS(i,m4,mf4,j-s2);
         }
      else if (m4!=-MAXFLOAT) FANVERTEX_PRISMS(i,m4,mf4,j-s2);
      FANVERTEX_PRISMS(i+s2,e4,f4,j-s2);
      if (bc1==0)
         if (m1!=-MAXFLOAT) FANVERTEX_PRISMS(i+s2,m1,mf1,j);
         else FANVERTEX_PRISMS(i+s2,e1,f1,j+s2);
      else FANVERTEX_PRISMS(i+s2,m1,mf1,j);
      }
   }

// calculate the base elevation at position (x,z)
float getbaseheight(float x,float z)
   {
   if (x<X(0)) x=X(0);
   else if (x>X(S-1)) x=X(S-1);

   if (z<Z(S-1)) z=Z(S-1);
   else if (z>Z(0)) z=Z(0);

   return(getheight(x+OX,z+OZ)-OY);
   }

// triangulate and draw the landscape
void drawlandscape(float res,
                   float ex,float ey,float ez,
                   float fx,float fy,float fz,
                   float dx,float dy,float dz,
                   float ux,float uy,float uz,
                   float fovy,float aspect,
                   float nearp,float farp,
                   void (*beginfan)(),
                   void (*fanvertex)(float i,float y,float j),
                   void (*notify)(int i,int j,int s),
                   void (*prismedge)(float x,float y,float yf,float z),
                   int state)
   {
       std::cout<<"drawlandscape2"<<std::endl;
   float length;

   float rx,ry,rz;

   float baseoff;

   if (S==0) ERRORMSG();

   if (fovy>=180.0f || aspect<=0.0f ||
       nearp<0.0f || farp<=nearp) ERRORMSG();

   if (state!=SINGLEPHASE &&
       state!=CALCMAPPHASE &&
       state!=DRAWMAPPHASE &&
       state!=DRAWSEAPHASE) return;

   if ((length=fsqrt(dx*dx+dy*dy+dz*dz))==0.0f) ERRORMSG();
   dx/=length;
   dy/=length;
   dz/=length;

   c=fsqrt(fmax(res,0.0f))*maxd2;

   EX=ex-OX;
   EY=ey-OY;
   EZ=ez-OZ;

   FX=fx-OX;
   FY=fy-OY;
   FZ=fz-OZ;

   DX=dx;
   DY=dy;
   DZ=dz;

   NEARP=nearp;
   FARP=farp;

   k1=fabs(dx)*Dx+fabs(dz)*Dz;
   k2=fabs(dy)*SCALE;

   rx=uy*dz-dy*uz;
   ry=uz*dx-dz*ux;
   rz=ux*dy-dx*uy;

   ux=ry*dz-dy*rz;
   uy=rz*dx-dz*rx;
   uz=rx*dy-dx*ry;

   ORTHO=fovy<0.0f;
   CULLING=fovy!=0.0f;

   if (!ORTHO)
      {
      baseoff=fabs(FY-getbaseheight(FX,FZ));

      if (minoff>0.0f) baseoff=fmax(baseoff-minoff*minres*D,0.0f);

      DF2=fsqr(baseoff);

      if (CULLING)
         {
         if ((length=fsqrt(rx*rx+ry*ry+rz*rz)/ftan(fovy/2.0f*RAD)/aspect)==0.0f) ERRORMSG();
         rx/=length;
         ry/=length;
         rz/=length;

         if ((length=fsqrt(ux*ux+uy*uy+uz*uz)/ftan(fovy/2.0f*RAD))==0.0f) ERRORMSG();
         ux/=length;
         uy/=length;
         uz/=length;

         nx1=(dy+ry-uy)*(dz+rz+uz)-(dy+ry+uy)*(dz+rz-uz);
         ny1=(dz+rz-uz)*(dx+rx+ux)-(dz+rz+uz)*(dx+rx-ux);
         nz1=(dx+rx-ux)*(dy+ry+uy)-(dx+rx+ux)*(dy+ry-uy);

         if ((length=fsqrt(nx1*nx1+ny1*ny1+nz1*nz1))==0.0f) ERRORMSG();
         nx1/=length;
         ny1/=length;
         nz1/=length;

         k11=fabs(nx1)*Dx+fabs(nz1)*Dz;
         k12=fabs(ny1)*SCALE;

         nx2=(dy-ry+uy)*(dz-rz-uz)-(dy-ry-uy)*(dz-rz+uz);
         ny2=(dz-rz+uz)*(dx-rx-ux)-(dz-rz-uz)*(dx-rx+ux);
         nz2=(dx-rx+ux)*(dy-ry-uy)-(dx-rx-ux)*(dy-ry+uy);

         if ((length=fsqrt(nx2*nx2+ny2*ny2+nz2*nz2))==0.0f) ERRORMSG();
         nx2/=length;
         ny2/=length;
         nz2/=length;

         k21=fabs(nx2)*Dx+fabs(nz2)*Dz;
         k22=fabs(ny2)*SCALE;

         nx3=(dy+ry+uy)*(dz-rz+uz)-(dy-ry+uy)*(dz+rz+uz);
         ny3=(dz+rz+uz)*(dx-rx+ux)-(dz-rz+uz)*(dx+rx+ux);
         nz3=(dx+rx+ux)*(dy-ry+uy)-(dx-rx+ux)*(dy+ry+uy);

         if ((length=fsqrt(nx3*nx3+ny3*ny3+nz3*nz3))==0.0f) ERRORMSG();
         nx3/=length;
         ny3/=length;
         nz3/=length;

         k31=fabs(nx3)*Dx+fabs(nz3)*Dz;
         k32=fabs(ny3)*SCALE;

         nx4=(dy-ry-uy)*(dz+rz-uz)-(dy+ry-uy)*(dz-rz-uz);
         ny4=(dz-rz-uz)*(dx+rx-ux)-(dz+rz-uz)*(dx-rx-ux);
         nz4=(dx-rx-ux)*(dy+ry-uy)-(dx+rx-ux)*(dy-ry-uy);

         if ((length=fsqrt(nx4*nx4+ny4*ny4+nz4*nz4))==0.0f) ERRORMSG();
         nx4/=length;
         ny4/=length;
         nz4/=length;

         k41=fabs(nx4)*Dx+fabs(nz4)*Dz;
         k42=fabs(ny4)*SCALE;
         }
      }
   else
      {
      if ((length=fsqrt(rx*rx+ry*ry+rz*rz))==0.0f) ERRORMSG();
      rx/=length;
      ry/=length;
      rz/=length;

      if ((length=fsqrt(ux*ux+uy*uy+uz*uz))==0.0f) ERRORMSG();
      ux/=length;
      uy/=length;
      uz/=length;

      k11=fabs(rx)*Dx+fabs(rz)*Dz;
      k12=fabs(ry)*SCALE;

      k21=fabs(ux)*Dx+fabs(uz)*Dz;
      k22=fabs(uy)*SCALE;

      k31=fabs(fovy)/2.0f*aspect;
      k32=fabs(fovy)/2.0f;

      RX=rx;
      RY=ry;
      RZ=rz;

      UX=ux;
      UY=uy;
      UZ=uz;
      }

   if (state==SINGLEPHASE || state==CALCMAPPHASE)
      {
      if (CULLING || SEALEVEL!=-MAXFLOAT)
         if (DH[0]==0.0f)
            {
            calcDH(1,S/2);
            if (yf!=NULL) recalcDH(255.0f*LAMBDA+DISPLACE,1,S/2);
            }

      calcmap(S/2,S/2,S-1);
      }

   if (state==SINGLEPHASE || state==DRAWMAPPHASE)
      if (beginfan==NULL || fanvertex==NULL)
         {
         initstate();

         color(1.0f,1.0f,1.0f);
         bindtexmap(tid,twidth,theight,S,tmipmaps);

         mtxpush();
         mtxtranslate(OX,OY,OZ);

         mtxscale(Dx,SCALE,-Dz);
         mtxtranslate(-S/2,0.0f,-S/2);

         beginfans();
         if (yf==NULL)
            drawmap(S/2,S/2,S-1,
                    y[S-1][S-1],y[0][S-1],y[0][0],y[S-1][0]);
         else
            drawmap(S/2,S/2,S-1,
                    y[S-1][S-1],y[0][S-1],y[0][0],y[S-1][0],
                    yf[S-1][S-1],yf[0][S-1],yf[0][0],yf[S-1][0]);
         endfans();

         mtxpop();

         bindtexmap(0,0,0,0,0);

         exitstate();
         }
      else
         {
         beginfan_callback=beginfan;
         fanvertex_callback=fanvertex;
         notify_callback=notify;
         prismedge_callback=prismedge;

         if (yf==NULL)
            pipemap(S/2,S/2,S-1,
                    y[S-1][S-1],y[0][S-1],y[0][0],y[S-1][0]);
         else
            pipemap(S/2,S/2,S-1,
                    y[S-1][S-1],y[0][S-1],y[0][0],y[S-1][0],
                    yf[S-1][S-1],yf[0][S-1],yf[0][0],yf[S-1][0]);
         }

   if (state==SINGLEPHASE || state==DRAWSEAPHASE)
      if (beginfan!=NULL && fanvertex!=NULL)
         if (SEALEVEL!=-MAXFLOAT)
            {
            beginfan_callback=beginfan;
            fanvertex_callback=fanvertex;
            notify_callback=notify;

            pipesea(S/2,S/2,S-1,
                    y[S-1][S-1],y[0][S-1],y[0][0],y[S-1][0]);
            }

   if (state==SINGLEPHASE)
      drawprismcache(ex,ey,ez,
                     dx,dy,dz,
                     nearp,farp,
                     EMISSION,FR,FG,FB);
   }

// check the visibility of the landscape
int checklandscape(float ex,float ey,float ez,
                   float dx,float dy,float dz,
                   float ux,float uy,float uz,
                   float fovy,float aspect,
                   float nearp,float farp)
   {
   float length;

   float rx,ry,rz;

   float dh;

   float l,d;

   if (S==0) ERRORMSG();

   if (fovy==0.0f || fovy>=180.0f || aspect<=0.0f ||
       nearp<0.0f || farp<=nearp) ERRORMSG();

   if ((length=fsqrt(dx*dx+dy*dy+dz*dz))==0.0f) ERRORMSG();
   dx/=length;
   dy/=length;
   dz/=length;

   EX=ex-OX;
   EY=ey-OY;
   EZ=ez-OZ;

   DX=dx;
   DY=dy;
   DZ=dz;

   NEARP=nearp;
   FARP=farp;

   k1=fabs(dx)*Dx+fabs(dz)*Dz;
   k2=fabs(dy)*SCALE;

   rx=uy*dz-dy*uz;
   ry=uz*dx-dz*ux;
   rz=ux*dy-dx*uy;

   ux=ry*dz-dy*rz;
   uy=rz*dx-dz*rx;
   uz=rx*dy-dx*ry;

   ORTHO=fovy<0.0f;

   if (!ORTHO)
      {
      if ((length=fsqrt(rx*rx+ry*ry+rz*rz)/ftan(fovy/2.0f*RAD)/aspect)==0.0f) ERRORMSG();
      rx/=length;
      ry/=length;
      rz/=length;

      if ((length=fsqrt(ux*ux+uy*uy+uz*uz)/ftan(fovy/2.0f*RAD))==0.0f) ERRORMSG();
      ux/=length;
      uy/=length;
      uz/=length;

      nx1=(dy+ry-uy)*(dz+rz+uz)-(dy+ry+uy)*(dz+rz-uz);
      ny1=(dz+rz-uz)*(dx+rx+ux)-(dz+rz+uz)*(dx+rx-ux);
      nz1=(dx+rx-ux)*(dy+ry+uy)-(dx+rx+ux)*(dy+ry-uy);

      if ((length=fsqrt(nx1*nx1+ny1*ny1+nz1*nz1))==0.0f) ERRORMSG();
      nx1/=length;
      ny1/=length;
      nz1/=length;

      k11=fabs(nx1)*Dx+fabs(nz1)*Dz;
      k12=fabs(ny1)*SCALE;

      nx2=(dy-ry+uy)*(dz-rz-uz)-(dy-ry-uy)*(dz-rz+uz);
      ny2=(dz-rz+uz)*(dx-rx-ux)-(dz-rz-uz)*(dx-rx+ux);
      nz2=(dx-rx+ux)*(dy-ry-uy)-(dx-rx-ux)*(dy-ry+uy);

      if ((length=fsqrt(nx2*nx2+ny2*ny2+nz2*nz2))==0.0f) ERRORMSG();
      nx2/=length;
      ny2/=length;
      nz2/=length;

      k21=fabs(nx2)*Dx+fabs(nz2)*Dz;
      k22=fabs(ny2)*SCALE;

      nx3=(dy+ry+uy)*(dz-rz+uz)-(dy-ry+uy)*(dz+rz+uz);
      ny3=(dz+rz+uz)*(dx-rx+ux)-(dz-rz+uz)*(dx+rx+ux);
      nz3=(dx+rx+ux)*(dy-ry+uy)-(dx-rx+ux)*(dy+ry+uy);

      if ((length=fsqrt(nx3*nx3+ny3*ny3+nz3*nz3))==0.0f) ERRORMSG();
      nx3/=length;
      ny3/=length;
      nz3/=length;

      k31=fabs(nx3)*Dx+fabs(nz3)*Dz;
      k32=fabs(ny3)*SCALE;

      nx4=(dy-ry-uy)*(dz+rz-uz)-(dy+ry-uy)*(dz-rz-uz);
      ny4=(dz-rz-uz)*(dx+rx-ux)-(dz+rz-uz)*(dx-rx-ux);
      nz4=(dx-rx-ux)*(dy+ry-uy)-(dx+rx-ux)*(dy-ry-uy);

      if ((length=fsqrt(nx4*nx4+ny4*ny4+nz4*nz4))==0.0f) ERRORMSG();
      nx4/=length;
      ny4/=length;
      nz4/=length;

      k41=fabs(nx4)*Dx+fabs(nz4)*Dz;
      k42=fabs(ny4)*SCALE;
      }
   else
      {
      if ((length=fsqrt(rx*rx+ry*ry+rz*rz))==0.0f) ERRORMSG();
      rx/=length;
      ry/=length;
      rz/=length;

      if ((length=fsqrt(ux*ux+uy*uy+uz*uz))==0.0f) ERRORMSG();
      ux/=length;
      uy/=length;
      uz/=length;

      k11=fabs(rx)*Dx+fabs(rz)*Dz;
      k12=fabs(ry)*SCALE;

      k21=fabs(ux)*Dx+fabs(uz)*Dz;
      k22=fabs(uy)*SCALE;

      k31=fabs(fovy)/2.0f*aspect;
      k32=fabs(fovy)/2.0f;

      RX=rx;
      RY=ry;
      RZ=rz;

      UX=ux;
      UY=uy;
      UZ=uz;
      }

   dx=X(S/2)-EX;
   dy=Y(S/2,S/2)-EY;
   dz=Z(S/2)-EZ;

   dh=DH[S-1];

   if (SEALEVEL!=-MAXFLOAT)
      if (Y(S/2,S/2)-SEALEVEL>dh) dh=Y(S/2,S/2)-SEALEVEL;

   l=DX*dx+DY*dy+DZ*dz;
   d=k1*S/2+k2*dh;

   if (l<NEARP-d || l>FARP+d) return(0);

   if (!ORTHO)
      {
      if (nx1*dx+ny1*dy+nz1*dz>k11*S/2+k12*dh) return(0);
      if (nx2*dx+ny2*dy+nz2*dz>k21*S/2+k22*dh) return(0);
      if (nx3*dx+ny3*dy+nz3*dz>k31*S/2+k32*dh) return(0);
      if (nx4*dx+ny4*dy+nz4*dz>k41*S/2+k42*dh) return(0);
      }
   else
      {
      if (fabs(RX*dx+RY*dy+RZ*dz)>k11*S/2+k12*dh+k31) return(0);
      if (fabs(UX*dx+UY*dy+UZ*dz)>k21*S/2+k22*dh+k32) return(0);
      }

   return(1);
   }

// draw the cached prisms
void drawprismcache(float ex,float ey,float ez,
                    float dx,float dy,float dz,
                    float nearp,float farp,
                    float emission,float fogR,float fogG,float fogB,
                    float *prismcache,int prismcnt)
   {
   float length;

   if (nearp<0.0f || farp<=nearp) ERRORMSG();

   if (prismcache==NULL)
      {
      prismcache=PRISMCACHE;
      prismcnt=PRISMCNT;
      PRISMCNT=0;
      }

   if (prismcnt>0)
      {
      if ((length=fsqrt(dx*dx+dy*dy+dz*dz))==0.0f) ERRORMSG();
      dx/=length;
      dy/=length;
      dz/=length;

      EX=ex;
      EY=ey;
      EZ=ez;

      DX=dx;
      DY=dy;
      DZ=dz;

      NEARP=nearp;
      FARP=farp;

      EMISSION=emission;

      FR=fogR;
      FG=fogG;
      FB=fogB;

      initstate();
      disableZwriting();

      if (emission>0.0f) drawfog(prismcache,prismcnt);
      else mipfog(prismcache,prismcnt);

      enableZwriting();
      exitstate();
      }
   }

// return the elevation at grid position (i,j)
float getheight(int i,int j)
   {
   if (S==0) ERRORMSG();

   if (i<0 || i>=S || j<0 || j>=S) ERRORMSG();

   return(Y(i,j)+OY);
   }

// calculate the elevation at coordinates (s,t)
void getheight(float s,float t,float *height)
   {
   float x,z;

   if (S==0) ERRORMSG();

   x=s*(S-1)*Dx;
   z=(1.0f-t)*(S-1)*Dz;

   x+=OX+X(0);
   z+=OZ+Z(S-1);

   *height=getheight(x,z);
   }

// calculate the elevation at position (x,z)
float getheight(float x,float z)
   {
   int mi,mj;

   float ri,rj;

   if (S==0) ERRORMSG();

   x-=OX+X(0);
   z-=OZ+Z(S-1);

   if (x<0.0f && x>-SAFETYZONE*Dx) x=0.0f;
   if (x>(S-1)*Dx && x<(S-1+SAFETYZONE)*Dx) x=(S-1)*Dx;

   if (z<0.0f && z>-SAFETYZONE*Dz) z=0.0f;
   if (z>(S-1)*Dz && z<(S-1+SAFETYZONE)*Dz) z=(S-1)*Dz;

   if (x<0.0f || x>(S-1)*Dx || z<0.0f || z>(S-1)*Dz) return(-MAXFLOAT);

   mi=ftrc(x/Dx);
   mj=ftrc(z/Dz);

   ri=x/Dx-mi;
   rj=z/Dz-mj;

   if (mi==S-1)
      {
      mi=S-2;
      ri=1.0f;
      }

   if (mj==S-1)
      {
      mj=S-2;
      rj=1.0f;
      }

   if ((mi+mj)%2==0)
      if (ri>rj)
         return(Y(mi,S-1-mj)+
                ri*(Y(mi+1,S-1-mj)-Y(mi,S-1-mj))+
                rj*(Y(mi+1,S-1-mj-1)-Y(mi+1,S-1-mj))+OY);
      else
         return(Y(mi,S-1-mj)+
                rj*(Y(mi,S-1-mj-1)-Y(mi,S-1-mj))+
                ri*(Y(mi+1,S-1-mj-1)-Y(mi,S-1-mj-1))+OY);
   else
      if (ri+rj<1.0)
         return(Y(mi,S-1-mj)+
                ri*(Y(mi+1,S-1-mj)-Y(mi,S-1-mj))+
                rj*(Y(mi,S-1-mj-1)-Y(mi,S-1-mj))+OY);
      else
         return(Y(mi+1,S-1-mj-1)+
                (1.0-ri)*(Y(mi,S-1-mj-1)-Y(mi+1,S-1-mj-1))+
                (1.0-rj)*(Y(mi+1,S-1-mj)-Y(mi+1,S-1-mj-1))+OY);
   }

// calculate the height of the fog layer at position (x,z)
float getfogheight(float x,float z)
   {
   int mi,mj;

   float ri,rj;

   if (S==0) ERRORMSG();

   if (yf==NULL) return(0.0f);

   x-=OX+X(0);
   z-=OZ+Z(S-1);

   if (x<0.0f && x>-SAFETYZONE*Dx) x=0.0f;
   if (x>(S-1)*Dx && x<(S-1+SAFETYZONE)*Dx) x=(S-1)*Dx;

   if (z<0.0f && z>-SAFETYZONE*Dz) z=0.0f;
   if (z>(S-1)*Dz && z<(S-1+SAFETYZONE)*Dz) z=(S-1)*Dz;

   if (x<0.0f || x>(S-1)*Dx || z<0.0f || z>(S-1)*Dz) return(0.0f);

   mi=ftrc(x/Dx);
   mj=ftrc(z/Dz);

   ri=x/Dx-mi;
   rj=z/Dz-mj;

   if (mi==S-1)
      {
      mi=S-2;
      ri=1.0f;
      }

   if (mj==S-1)
      {
      mj=S-2;
      rj=1.0f;
      }

   if ((mi+mj)%2==0)
      if (ri>rj)
         return(YF(mi,S-1-mj)+
                ri*(YF(mi+1,S-1-mj)-YF(mi,S-1-mj))+
                rj*(YF(mi+1,S-1-mj-1)-YF(mi+1,S-1-mj)));
      else
         return(YF(mi,S-1-mj)+
                rj*(YF(mi,S-1-mj-1)-YF(mi,S-1-mj))+
                ri*(YF(mi+1,S-1-mj-1)-YF(mi,S-1-mj-1)));
   else
      if (ri+rj<1.0)
         return(YF(mi,S-1-mj)+
                ri*(YF(mi+1,S-1-mj)-YF(mi,S-1-mj))+
                rj*(YF(mi,S-1-mj-1)-YF(mi,S-1-mj)));
      else
         return(YF(mi+1,S-1-mj-1)+
                (1.0-ri)*(YF(mi,S-1-mj-1)-YF(mi+1,S-1-mj-1))+
                (1.0-rj)*(YF(mi+1,S-1-mj)-YF(mi+1,S-1-mj-1)));
   }

// calculate the normal vector at coordinates (s,t)
void getnormal(float s,float t,float *nx,float *nz)
   {
   float x,z,ny;

   if (S==0) ERRORMSG();

   x=s*(S-1)*Dx;
   z=(1.0f-t)*(S-1)*Dz;

   x+=OX+X(0);
   z+=OZ+Z(S-1);

   getnormal(x,z,nx,&ny,nz);
   }

// calculate the normal vector at position (x,z)
void getnormal(float x,float z,float *nx,float *ny,float *nz)
   {
   int mi,mj;

   float ri,rj;

   float dx1,dy1,dz1,
         dx2,dy2,dz2;

   float length;

   if (S==0) ERRORMSG();

   *nx=*ny=*nz=0.0f;

   x-=OX+X(0);
   z-=OZ+Z(S-1);

   if (x<0.0f && x>-SAFETYZONE*Dx) x=0.0f;
   if (x>(S-1)*Dx && x<(S-1+SAFETYZONE)*Dx) x=(S-1)*Dx;

   if (z<0.0f && z>-SAFETYZONE*Dz) z=0.0f;
   if (z>(S-1)*Dz && z<(S-1+SAFETYZONE)*Dz) z=(S-1)*Dz;

   if (x<0.0f || x>(S-1)*Dx || z<0.0f || z>(S-1)*Dz) return;

   mi=ftrc(x/Dx);
   mj=ftrc(z/Dz);

   ri=x/Dx-mi;
   rj=z/Dz-mj;

   if (mi==S-1)
      {
      mi=S-2;
      ri=1.0f;
      }

   if (mj==S-1)
      {
      mj=S-2;
      rj=1.0f;
      }

   if ((mi+mj)%2==0)
      if (ri>rj)
         {
         dx1=Dx;
         dy1=Y(mi+1,S-1-mj)-Y(mi,S-1-mj);
         dz1=0.0f;
         dx2=Dx;
         dy2=Y(mi+1,S-1-mj-1)-Y(mi,S-1-mj);
         dz2=-Dz;
         }
      else
         {
         dx1=Dx;
         dy1=Y(mi+1,S-1-mj-1)-Y(mi,S-1-mj);
         dz1=-Dz;
         dx2=0.0f;
         dy2=Y(mi,S-1-mj-1)-Y(mi,S-1-mj);
         dz2=-Dz;
         }
   else
      if (ri+rj<1.0)
         {
         dx1=0.0f;
         dy1=Y(mi,S-1-mj)-Y(mi,S-1-mj-1);
         dz1=Dz;
         dx2=Dx;
         dy2=Y(mi+1,S-1-mj)-Y(mi,S-1-mj-1);
         dz2=Dz;
         }
      else
         {
         dx1=Dx;
         dy1=Y(mi+1,S-1-mj)-Y(mi,S-1-mj-1);
         dz1=Dz;
         dx2=Dx;
         dy2=Y(mi+1,S-1-mj-1)-Y(mi,S-1-mj-1);
         dz2=0.0f;
         }

   *nx=dy1*dz2-dz1*dy2;
   *ny=dz1*dx2-dx1*dz2;
   *nz=dy1*dx2-dx1*dy2;

   if ((length=fsqrt((*nx)*(*nx)+(*ny)*(*ny)+(*nz)*(*nz)))==0.0f) ERRORMSG();
   *nx/=length;
   *ny/=length;
   *nz/=length;
   }

// calculate the maximum size of the triangulation
int getmaxsize(float res,float fx,float fy,float fz,float fovy)
   {
   float f;

   float dx,dz;

   float baseoff;

   int lvl,size;

   if (S==0) ERRORMSG();

   c=fsqrt(fmax(res,0.0f))*maxd2;

   ORTHO=fovy<0.0f;

   if (!ORTHO)
      {
      FX=fx-OX;
      FY=fy-OY;
      FZ=fz-OZ;

      if (FX<X(0)) dx=X(0)-FX;
      else if (FX>X(S-1)) dx=FX-X(S-1);
      else dx=0.0f;

      if (FZ<Z(S-1)) dz=Z(S-1)-FZ;
      else if (FZ>Z(0)) dz=FZ-Z(0);
      else dz=0.0f;

      baseoff=fabs(FY-getbaseheight(FX,FZ));

      if (minoff>0.0f) baseoff=fmax(baseoff-minoff*minres*D,0.0f);

      DF2=fsqr(baseoff);

      f=(dx*dx+DF2+dz*dz)/fsqr((S-1)*D*fmax(c*dcpr(S/2,S/2,S/2),minres));
      }
   else f=1.0f/fsqr(fmax(c*dcpr(S/2,S/2,S/2),minres));

   if (f==0.0f) size=S;
   else
      {
      lvl=max(ftrc(fceil(-flog(f)/flog(4.0f)))+1,1);
      size=ftrc(fmin(fpow(2,lvl)+1,S)+0.5f);
      }

   return(size);
   }

// set the elevation at grid position (i,j)
void setheight(int i,int j,float h)
   {
   if (S==0) ERRORMSG();

   if (i<0 || i>=S || j<0 || j>=S) ERRORMSG();

   y[i][j]=h;
   }

// set the elevation at position (x,z)
int setheight(float x,float z,float h)
   {
   int mi,mj;

   float ri,rj;

   if (S==0) ERRORMSG();

   x-=OX+X(0);
   z-=OZ+Z(S-1);

   if (x<0.0f && x>-SAFETYZONE*Dx) x=0.0f;
   if (x>(S-1)*Dx && x<(S-1+SAFETYZONE)*Dx) x=(S-1)*Dx;

   if (z<0.0f && z>-SAFETYZONE*Dz) z=0.0f;
   if (z>(S-1)*Dz && z<(S-1+SAFETYZONE)*Dz) z=(S-1)*Dz;

   if (x<0.0f || x>(S-1)*Dx || z<0.0f || z>(S-1)*Dz) return(0);

   mi=ftrc(x/Dx);
   mj=ftrc(z/Dz);

   ri=x/Dx-mi;
   rj=z/Dz-mj;

   if (mi==S-1)
      {
      mi=S-2;
      ri=1.0f;
      }

   if (mj==S-1)
      {
      mj=S-2;
      rj=1.0f;
      }

   setheight(mi,mj,y[mi][mj]*ri*rj+h*(1.0f-ri)*(1.0f-rj));
   setheight(mi+1,mj,y[mi+1][mj]*(1.0f-ri)*rj+h*ri*(1.0f-rj));
   setheight(mi,mj+1,y[mi][mj+1]*ri*(1.0f-rj)+h*(1.0f-ri)*rj);
   setheight(mi+1,mj+1,y[mi+1][mj+1]*(1.0f-ri)*(1.0f-rj)+h*ri*rj);

   return(1);
   }

// set the real elevation at grid position (i,j)
void setrealheight(int i,int j,float h)
   {
   if (SCALE==0.0f) ERRORMSG();
   setheight(i,j,(h-OY)/SCALE);
   }

// set the real elevation at position (x,z)
int setrealheight(float x,float z,float h)
   {
   if (SCALE==0.0f) ERRORMSG();
   return(setheight(x,z,(h-OY)/SCALE));
   }

// update d2- and dh-values after modification of height map
void updatemaps(int fast,float avgd2,int recalc)
   {
   int i,s;

   if (S==0) ERRORMSG();

   for (i=0,s=2; i<fast && s<S-1; i++) s*=2;

   if (recalc==0)
      if (fast==0)
         {
         calcDH();
         calcD2();
         }
      else
         {
         if (SEALEVEL!=-MAXFLOAT) calcDH();
         else
            {
            calcDH(S/2,S-1,s/2);
            DH[0]=0.0f;
            }

         calcD2(s,avgd2);
         }

   if (yf!=NULL)
      {
      recalcDH(255.0f*LAMBDA+DISPLACE);
      recalcD2(FOGATT,s);
      }
   }

// delete the height field and all associated maps
void deletemaps()
   {
   int i;

   if (S==0) ERRORMSG();

   for (i=0; i<=S; i++) free(y[i]);
   for (i=0; i<S-1; i++) free(bc[i]);

   free(y);
   free(bc);

   if (yf!=NULL)
      {
      for (i=0; i<S; i++) free(yf[i]);
      free(yf);
      }

   if (tid!=0) deletetexmap(tid);

   if (PRISMCACHE!=NULL && PRISMCNT==0)
      {
      free(PRISMCACHE);
      PRISMCACHE=NULL;
      }

   S=0;
   }

}
