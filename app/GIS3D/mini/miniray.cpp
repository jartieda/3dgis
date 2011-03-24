// (c) by Stefan Roettger

#include "minibase.h"

#include "miniOGL.h"

#include "miniv3f.h"
#include "miniv4d.h"

#include "miniray.h"

void (*miniray::LOCK_CALLBACK)(void *data)=NULL;
void (*miniray::UNLOCK_CALLBACK)(void *data)=NULL;
void *miniray::LOCK_DATA=NULL;

// default constructor
miniray::miniray()
   {
   FRONT=BACK=NULL;

   CONFIGURE_MAXCHUNKSIZE_TRIANGLES=100;
   CONFIGURE_MAXCHUNKSIZE_TRIANGLEFANS=20;
   }

// destructor
miniray::~miniray()
   {
   clearbuffer();
   swapbuffer();
   clearbuffer();
   }

// clear back triangle reference buffer
void miniray::clearbuffer()
   {
   TRIANGLEREF *ref,*next;

   ref=BACK;

   while (ref!=NULL)
      {
      next=ref->next;
      delete ref;

      ref=next;
      }

   BACK=NULL;
   }

// add reference to triangles to the back buffer
void miniray::addtriangles(float **array,int index,int num,int stride,
                           miniv3d *scaling,miniv3d *offset,
                           int swapyz,miniwarp *warp)
   {
   int n;

   if (num<=CONFIGURE_MAXCHUNKSIZE_TRIANGLES) addtriangles_chunked(array,index,num,stride,scaling,offset,swapyz,warp);
   else
      {
      for (n=0; n<num-CONFIGURE_MAXCHUNKSIZE_TRIANGLES; n+=CONFIGURE_MAXCHUNKSIZE_TRIANGLES)
         {
         addtriangles_chunked(array,index,CONFIGURE_MAXCHUNKSIZE_TRIANGLES,stride,scaling,offset,swapyz,warp);
         index+=(3+stride)*CONFIGURE_MAXCHUNKSIZE_TRIANGLES;
         }

      addtriangles_chunked(array,index,num-n,stride,scaling,offset,swapyz,warp);
      }
   }

// add reference to triangles to the back buffer
void miniray::addtriangles_chunked(float **array,int index,int num,int stride,
                                   miniv3d *scaling,miniv3d *offset,
                                   int swapyz,miniwarp *warp)
   {
   TRIANGLEREF *ref;

   if (num==0) return;

   ref=new TRIANGLEREF;

   ref->array=array;
   ref->index=index;

   ref->num=num;
   ref->stride=stride;

   ref->isfan=0;

   if (scaling!=NULL) ref->scaling=*scaling;
   else ref->scaling=miniv3d(1.0,1.0,1.0);

   if (offset!=NULL) ref->offset=*offset;
   else ref->offset=miniv3d(0.0,0.0,0.0);

   ref->swapyz=swapyz;

   ref->warp=warp;

   calcbound(ref);

   ref->next=BACK;
   BACK=ref;
   }

// add reference to triangle fans to the back buffer
void miniray::addtrianglefans(float **array,int index,int num,int stride,
                              miniv3d *scaling,miniv3d *offset,
                              int swapyz,miniwarp *warp)
   {
   int n;

   int i,k;

   if (num<=CONFIGURE_MAXCHUNKSIZE_TRIANGLEFANS) addtrianglefans_chunked(array,index,num,stride,scaling,offset,swapyz,warp);
   else
      {
      for (n=0; n<num-CONFIGURE_MAXCHUNKSIZE_TRIANGLEFANS; n+=CONFIGURE_MAXCHUNKSIZE_TRIANGLEFANS)
         {
         addtrianglefans_chunked(array,index,CONFIGURE_MAXCHUNKSIZE_TRIANGLEFANS,stride,scaling,offset,swapyz,warp);

         for (i=0; i<CONFIGURE_MAXCHUNKSIZE_TRIANGLEFANS; i++)
            {
            k=ftrc((*array)[index]+0.5f);
            index+=3+k*(3+stride);
            }
         }

      addtrianglefans_chunked(array,index,num-n,stride,scaling,offset,swapyz,warp);
      }
   }

// add reference to triangle fans to the back buffer
void miniray::addtrianglefans_chunked(float **array,int index,int num,int stride,
                                      miniv3d *scaling,miniv3d *offset,
                                      int swapyz,miniwarp *warp)
   {
   TRIANGLEREF *ref;

   if (num==0) return;

   ref=new TRIANGLEREF;

   ref->array=array;
   ref->index=index;

   ref->num=num;
   ref->stride=stride;

   ref->isfan=1;

   if (scaling!=NULL) ref->scaling=*scaling;
   else ref->scaling=miniv3d(1.0,1.0,1.0);

   if (offset!=NULL) ref->offset=*offset;
   else ref->offset=miniv3d(0.0,0.0,0.0);

   ref->swapyz=swapyz;

   ref->warp=warp;

   calcbound(ref);

   ref->next=BACK;
   BACK=ref;
   }

// swap front and back triangle reference buffer
void miniray::swapbuffer()
   {
   TRIANGLEREF *ref;

   lock();

   ref=FRONT;
   FRONT=BACK;
   BACK=ref;

   unlock();
   }

// shoot a ray and return the distance to the closest triangle
double miniray::shoot(const miniv3d &o,const miniv3d &d,double hitdist)
   {
   double result;

   miniv3d dn;

   TRIANGLEREF *ref;

   lock();

   dn=d;
   dn.normalize();

   result=MAXFLOAT;

   ref=FRONT;

   while (ref!=NULL)
      {
      if (checkbound(o,dn,ref->b,ref->r2)!=0)
         {
         result=calcdist(ref,o,d,result);
         if (result<hitdist) break;
         }

      ref=ref->next;
      }

   unlock();

   return(result);
   }

// set locking callbacks
void miniray::setcallbacks(void (*lock)(void *data),void *data,
                           void (*unlock)(void *data))
   {
   LOCK_CALLBACK=lock;
   UNLOCK_CALLBACK=unlock;
   LOCK_DATA=data;
   }

// lock ray shooting
void miniray::lock()
   {if (LOCK_CALLBACK!=NULL) LOCK_CALLBACK(LOCK_DATA);}

// unlock ray shooting
void miniray::unlock()
   {if (LOCK_CALLBACK!=NULL) UNLOCK_CALLBACK(LOCK_DATA);}

void miniray::calcbound(TRIANGLEREF *ref)
   {
   int i,j,k;

   float *array;
   int num,stride;

   miniv3d v;
   miniv3d vmin,vmax;

   miniv4d mtx[3],v1;

   array=*(ref->array)+ref->index;
   num=ref->num;
   stride=ref->stride;

   vmin.x=vmin.y=vmin.z=MAXFLOAT;
   vmax.x=vmax.y=vmax.z=-MAXFLOAT;

   if (ref->isfan==0)
      if (ref->swapyz==0)
         for (i=0; i<3*num; i++)
            {
            v.x=*array++;
            v.y=*array++;
            v.z=*array++;

            array+=stride;

            if (v.x<vmin.x) vmin.x=v.x;
            if (v.y<vmin.y) vmin.y=v.y;
            if (v.z<vmin.z) vmin.z=v.z;

            if (v.x>vmax.x) vmax.x=v.x;
            if (v.y>vmax.y) vmax.y=v.y;
            if (v.z>vmax.z) vmax.z=v.z;
            }
      else
         for (i=0; i<3*num; i++)
            {
            v.x=*array++;
            v.z=*array++;
            v.y=*array++;

            array+=stride;

            if (v.x<vmin.x) vmin.x=v.x;
            if (v.y<vmin.y) vmin.y=v.y;
            if (v.z<vmin.z) vmin.z=v.z;

            if (v.x>vmax.x) vmax.x=v.x;
            if (v.y>vmax.y) vmax.y=v.y;
            if (v.z>vmax.z) vmax.z=v.z;
            }
   else
      if (ref->swapyz==0)
         for (i=0; i<num; i++)
            {
            k=ftrc(*array+0.5f);

            array+=3;

            for (j=0; j<k; j++)
               {
               v.x=*array++;
               v.y=*array++;
               v.z=*array++;

               array+=stride;

               if (v.x<vmin.x) vmin.x=v.x;
               if (v.y<vmin.y) vmin.y=v.y;
               if (v.z<vmin.z) vmin.z=v.z;

               if (v.x>vmax.x) vmax.x=v.x;
               if (v.y>vmax.y) vmax.y=v.y;
               if (v.z>vmax.z) vmax.z=v.z;
               }
            }
      else
         for (i=0; i<num; i++)
            {
            k=ftrc(*array+0.5f);

            array+=3;

            for (j=0; j<k; j++)
               {
               v.x=*array++;
               v.z=*array++;
               v.y=*array++;

               array+=stride;

               if (v.x<vmin.x) vmin.x=v.x;
               if (v.y<vmin.y) vmin.y=v.y;
               if (v.z<vmin.z) vmin.z=v.z;

               if (v.x>vmax.x) vmax.x=v.x;
               if (v.y>vmax.y) vmax.y=v.y;
               if (v.z>vmax.z) vmax.z=v.z;
               }
            }

   vmin.x=vmin.x*ref->scaling.x+ref->offset.x;
   vmin.y=vmin.y*ref->scaling.y+ref->offset.y;
   vmin.z=vmin.z*ref->scaling.z+ref->offset.z;

   vmax.x=vmax.x*ref->scaling.x+ref->offset.x;
   vmax.y=vmax.y*ref->scaling.y+ref->offset.y;
   vmax.z=vmax.z*ref->scaling.z+ref->offset.z;

   ref->b=(vmin+vmax)/2.0;

   // warp matrix is assumed to be ortho-normal
   if (ref->warp!=NULL)
      {
      ref->warp->getwarp(mtx);
      v1=miniv4d(ref->b,1.0);
      ref->b=miniv3d(mtx[0]*v1,mtx[1]*v1,mtx[2]*v1);
      }

   ref->r2=0.75*(FSQR(vmax.x-vmin.x)+
                 FSQR(vmax.y-vmin.y)+
                 FSQR(vmax.z-vmin.z));
   }

double miniray::calcdist(TRIANGLEREF *ref,
                         const miniv3d &o,const miniv3d &d,
                         double dist)
   {
   int i,j,k;

   double result;

   float *array;
   int num,stride;

   miniv4d o1;
   miniv3d oi,di;

   miniv4d inv[3];
   miniv3d tra[3];

   miniv3d v1,v2,v3;

   array=*(ref->array)+ref->index;
   num=ref->num;
   stride=ref->stride;

   // warp matrix is assumed to be ortho-normal
   if (ref->warp!=NULL)
      {
      ref->warp->getinv(inv);
      o1=miniv4d(o.x,o.y,o.z,1.0);
      oi=miniv3d(inv[0]*o1,inv[1]*o1,inv[2]*o1);

      ref->warp->gettra(tra);
      di=miniv3d(tra[0]*d,tra[1]*d,tra[2]*d);
      }
   else
      {
      oi=o;
      di=d;
      }

   result=dist;

   if (ref->isfan==0)
      if (ref->swapyz==0)
         for (i=0; i<num; i++)
            {
            v1.x=*array++;
            v1.y=*array++;
            v1.z=*array++;

            array+=stride;

            v2.x=*array++;
            v2.y=*array++;
            v2.z=*array++;

            array+=stride;

            v3.x=*array++;
            v3.y=*array++;
            v3.z=*array++;

            array+=stride;

            v1.x=v1.x*ref->scaling.x+ref->offset.x;
            v1.y=v1.y*ref->scaling.y+ref->offset.y;
            v1.z=v1.z*ref->scaling.z+ref->offset.z;

            v2.x=v2.x*ref->scaling.x+ref->offset.x;
            v2.y=v2.y*ref->scaling.y+ref->offset.y;
            v2.z=v2.z*ref->scaling.z+ref->offset.z;

            v3.x=v3.x*ref->scaling.x+ref->offset.x;
            v3.y=v3.y*ref->scaling.y+ref->offset.y;
            v3.z=v3.z*ref->scaling.z+ref->offset.z;

            dist=checkdist(oi,di,v1,v2,v3);

            if (dist>0.0f) result=fmin(result,dist);
            }
      else
         for (i=0; i<num; i++)
            {
            v1.x=*array++;
            v1.z=*array++;
            v1.y=*array++;

            array+=stride;

            v2.x=*array++;
            v2.z=*array++;
            v2.y=*array++;

            array+=stride;

            v3.x=*array++;
            v3.z=*array++;
            v3.y=*array++;

            array+=stride;

            v1.x=v1.x*ref->scaling.x+ref->offset.x;
            v1.y=v1.y*ref->scaling.y+ref->offset.y;
            v1.z=v1.z*ref->scaling.z+ref->offset.z;

            v2.x=v2.x*ref->scaling.x+ref->offset.x;
            v2.y=v2.y*ref->scaling.y+ref->offset.y;
            v2.z=v2.z*ref->scaling.z+ref->offset.z;

            v3.x=v3.x*ref->scaling.x+ref->offset.x;
            v3.y=v3.y*ref->scaling.y+ref->offset.y;
            v3.z=v3.z*ref->scaling.z+ref->offset.z;

            dist=checkdist(oi,di,v1,v2,v3);

            if (dist>0.0f) result=fmin(result,dist);
            }
   else
      if (ref->swapyz==0)
         for (i=0; i<num; i++)
            {
            k=ftrc(*array+0.5f);

            array+=3;

            v1.x=*array++;
            v1.y=*array++;
            v1.z=*array++;

            array+=stride;

            v2.x=*array++;
            v2.y=*array++;
            v2.z=*array++;

            array+=stride;

            v1.x=v1.x*ref->scaling.x+ref->offset.x;
            v1.y=v1.y*ref->scaling.y+ref->offset.y;
            v1.z=v1.z*ref->scaling.z+ref->offset.z;

            v2.x=v2.x*ref->scaling.x+ref->offset.x;
            v2.y=v2.y*ref->scaling.y+ref->offset.y;
            v2.z=v2.z*ref->scaling.z+ref->offset.z;

            for (j=2; j<k; j++)
               {
               v3.x=*array++;
               v3.y=*array++;
               v3.z=*array++;

               array+=stride;

               v3.x=v3.x*ref->scaling.x+ref->offset.x;
               v3.y=v3.y*ref->scaling.y+ref->offset.y;
               v3.z=v3.z*ref->scaling.z+ref->offset.z;

               dist=checkdist(oi,di,v1,v2,v3);

               if (dist>0.0f) result=fmin(result,dist);

               v2=v3;
               }
            }
      else
         for (i=0; i<num; i++)
            {
            k=ftrc(*array+0.5f);

            array+=3;

            v1.x=*array++;
            v1.z=*array++;
            v1.y=*array++;

            array+=stride;

            v2.x=*array++;
            v2.z=*array++;
            v2.y=*array++;

            array+=stride;

            v1.x=v1.x*ref->scaling.x+ref->offset.x;
            v1.y=v1.y*ref->scaling.y+ref->offset.y;
            v1.z=v1.z*ref->scaling.z+ref->offset.z;

            v2.x=v2.x*ref->scaling.x+ref->offset.x;
            v2.y=v2.y*ref->scaling.y+ref->offset.y;
            v2.z=v2.z*ref->scaling.z+ref->offset.z;

            for (j=2; j<k; j++)
               {
               v3.x=*array++;
               v3.z=*array++;
               v3.y=*array++;

               array+=stride;

               v3.x=v3.x*ref->scaling.x+ref->offset.x;
               v3.y=v3.y*ref->scaling.y+ref->offset.y;
               v3.z=v3.z*ref->scaling.z+ref->offset.z;

               dist=checkdist(oi,di,v1,v2,v3);

               if (dist>0.0f) result=fmin(result,dist);

               v2=v3;
               }
            }

   return(result);
   }

void miniray::renderwire()
   {
   TRIANGLEREF *ref;

   initstate();

   ref=FRONT;

   while (ref!=NULL)
      {
      renderwire(ref);
      ref=ref->next;
      }

   exitstate();
   }

void miniray::renderwire(TRIANGLEREF *ref)
   {
   int i,j,k;

   float *array;
   int num,stride;

   miniv4d mtx[3];
   double oglmtx[16];

   miniv3f v1,v2,v3;

   array=*(ref->array)+ref->index;
   num=ref->num;
   stride=ref->stride;

   mtxpush();
   mtxtranslate(ref->offset.x,ref->offset.y,ref->offset.z);
   mtxscale(ref->scaling.x,ref->scaling.y,ref->scaling.z);

   if (ref->warp!=NULL)
      {
      ref->warp->getwarp(mtx);

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

   color(0.0f,0.0f,0.0f);

   if (ref->isfan==0)
      if (ref->swapyz==0)
         for (i=0; i<num; i++)
            {
            v1.x=*array++;
            v1.y=*array++;
            v1.z=*array++;

            array+=stride;

            v2.x=*array++;
            v2.y=*array++;
            v2.z=*array++;

            array+=stride;

            v3.x=*array++;
            v3.y=*array++;
            v3.z=*array++;

            array+=stride;

            drawline(v1.x,v1.y,v1.z,v2.x,v2.y,v2.z);
            drawline(v2.x,v2.y,v2.z,v3.x,v3.y,v3.z);
            drawline(v3.x,v3.y,v3.z,v1.x,v1.y,v1.z);
            }
      else
         for (i=0; i<num; i++)
            {
            v1.x=*array++;
            v1.z=*array++;
            v1.y=*array++;

            array+=stride;

            v2.x=*array++;
            v2.z=*array++;
            v2.y=*array++;

            array+=stride;

            v3.x=*array++;
            v3.z=*array++;
            v3.y=*array++;

            array+=stride;

            drawline(v1.x,v1.y,v1.z,v2.x,v2.y,v2.z);
            drawline(v2.x,v2.y,v2.z,v3.x,v3.y,v3.z);
            drawline(v3.x,v3.y,v3.z,v1.x,v1.y,v1.z);
            }
   else
      if (ref->swapyz==0)
         for (i=0; i<num; i++)
            {
            k=ftrc(*array+0.5f);

            array+=3;

            v1.x=*array++;
            v1.y=*array++;
            v1.z=*array++;

            array+=stride;

            v2.x=*array++;
            v2.y=*array++;
            v2.z=*array++;

            array+=stride;

            for (j=2; j<k; j++)
               {
               v3.x=*array++;
               v3.y=*array++;
               v3.z=*array++;

               array+=stride;

               drawline(v1.x,v1.y,v1.z,v2.x,v2.y,v2.z);
               drawline(v2.x,v2.y,v2.z,v3.x,v3.y,v3.z);
               drawline(v3.x,v3.y,v3.z,v1.x,v1.y,v1.z);

               v2=v3;
               }
            }
      else
         for (i=0; i<num; i++)
            {
            k=ftrc(*array+0.5f);

            array+=3;

            v1.x=*array++;
            v1.z=*array++;
            v1.y=*array++;

            array+=stride;

            v2.x=*array++;
            v2.z=*array++;
            v2.y=*array++;

            array+=stride;

            for (j=2; j<k; j++)
               {
               v3.x=*array++;
               v3.z=*array++;
               v3.y=*array++;

               array+=stride;

               drawline(v1.x,v1.y,v1.z,v2.x,v2.y,v2.z);
               drawline(v2.x,v2.y,v2.z,v3.x,v3.y,v3.z);
               drawline(v3.x,v3.y,v3.z,v1.x,v1.y,v1.z);

               v2=v3;
               }
            }

   mtxpop();
   }

// geometric ray/sphere intersection test
int miniray::checkbound(const miniv3d &o,const miniv3d &d,
                        const miniv3d &b,const double r2)
   {
   miniv3d bmo;
   double bmo2,bmod;

   bmo=b-o;
   bmo2=bmo*bmo;
   if (bmo2<r2) return(1);

   bmod=bmo*d;
   if (bmod<0.0) return(0);
   if (r2+bmod*bmod>bmo2) return(1);

   return(0);
   }

double miniray::checkdist(const miniv3d &o,const miniv3d &d,
                         const miniv3d &v1,const miniv3d &v2,const miniv3d &v3)
   {
   miniv3d tuv;

   if (intersect(o,d,v1,v2,v3,&tuv)==0) return(MAXFLOAT);
   else return(tuv.x);
   }

// Moeller-Trumbore ray/triangle intersection
int miniray::intersect(const miniv3d &o,const miniv3d &d,
                       const miniv3d &v0,const miniv3d &v1,const miniv3d &v2,
                       miniv3d *tuv)
   {
   static const double epsilon=1E-5;

   double t,u,v;
   miniv3d edge1,edge2,tvec,pvec,qvec;
   double det,inv_det;

   // find vectors for two edges sharing v0
   edge1=v1-v0;
   edge2=v2-v0;

   // begin calculating determinant - also used to calculate U parameter
   pvec=d/edge2;

   // if determinant is near zero, ray lies in plane of triangle
   det=edge1*pvec;

   // cull triangles with determinant near zero
   if (fabs(det)<epsilon) return(0);

   // calculate inverse determinant
   inv_det=1.0/det;

   // calculate distance from v0 to ray origin
   tvec=o-v0;

   // calculate U parameter and test bounds
   u=(tvec*pvec)*inv_det;
   if (u<0.0 || u>1.0) return(0);

   // prepare to test V parameter
   qvec=tvec/edge1;

   // calculate V parameter and test bounds
   v=(d*qvec)*inv_det;
   if (v<0.0 || u+v>1.0) return(0);

   // calculate t, ray intersects triangle
   t=(edge2*qvec)*inv_det;

   *tuv=miniv3f(t,u,v);

   return(1);
   }

// configuring:

void miniray::configure_maxchunksize_triangles(int maxchunksize)
   {CONFIGURE_MAXCHUNKSIZE_TRIANGLES=maxchunksize;}

void miniray::configure_maxchunksize_trianglefans(int maxchunksize)
   {CONFIGURE_MAXCHUNKSIZE_TRIANGLEFANS=maxchunksize;}
