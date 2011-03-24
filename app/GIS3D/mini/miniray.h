// (c) by Stefan Roettger

#ifndef MINIRAY_H
#define MINIRAY_H

#include "miniv3d.h"

#include "miniwarp.h"

//! class for intersecting a ray with a set of triangles
//! the triangles are stored as a set of references to vertex arrays
class miniray
   {
   public:

   //! default constructor
   miniray();

   //! destructor
   ~miniray();

   //! clear back triangle reference buffer
   void clearbuffer();

   //! add reference to triangles to the back buffer
   void addtriangles(float **array,int index,int num,int stride=0,
                     miniv3d *scaling=0,miniv3d *offset=0,
                     int swapyz=0,miniwarp *warp=0);

   //! add reference to triangle fans to the back buffer
   void addtrianglefans(float **array,int index,int num,int stride=0,
                        miniv3d *scaling=0,miniv3d *offset=0,
                        int swapyz=0,miniwarp *warp=0);

   //! swap front and back triangle reference buffer
   void swapbuffer();

   //! shoot a ray and return the distance to the closest triangle
   //! only the triangles referenced in the front buffer are considered
   //! only the triangles with a positive distance are considered
   //! o is the origin of the ray, d is the ray direction
   //! a return value of MAXFLOAT indicates that there was no hit
   //! the first hit with a smaller distance than hitdist will be returned
   double shoot(const miniv3d &o,const miniv3d &d,double firsthit=0.0);

   //! set locking callbacks
   //! when the callbacks are set ray shooting can be triggered safely from a separate thread
   static void setcallbacks(void (*lock)(void *data),void *data,
                            void (*unlock)(void *data));

   //! lock ray shooting
   static void lock();

   //! unlock ray shooting
   static void unlock();

   //! render triangles as wireframe for debugging purposes
   void renderwire();

   //! configuring
   void configure_maxchunksize_triangles(int maxchunksize=100); // set maximum chunk size for triangle arrays
   void configure_maxchunksize_trianglefans(int maxchunksize=100); // set maximum chunk size for triangle fan arrays

   protected:

   private:

   struct TRIANGLEREF
      {
      float **array;
      int index;
      int num;

      int stride;

      int isfan;

      miniv3d scaling;
      miniv3d offset;

      int swapyz;

      miniwarp *warp;

      miniv3d b;
      double r2;

      TRIANGLEREF *next;
      };

   TRIANGLEREF *FRONT,*BACK;

   static void (*LOCK_CALLBACK)(void *data);
   static void (*UNLOCK_CALLBACK)(void *data);
   static void *LOCK_DATA;

   int CONFIGURE_MAXCHUNKSIZE_TRIANGLES;
   int CONFIGURE_MAXCHUNKSIZE_TRIANGLEFANS;

   void addtriangles_chunked(float **array,int index,int num,int stride=0,
                             miniv3d *scaling=0,miniv3d *offset=0,
                             int swapyz=0,miniwarp *warp=0);

   void addtrianglefans_chunked(float **array,int index,int num,int stride=0,
                                miniv3d *scaling=0,miniv3d *offset=0,
                                int swapyz=0,miniwarp *warp=0);

   void calcbound(TRIANGLEREF *ref);

   double calcdist(TRIANGLEREF *ref,
                   const miniv3d &o,const miniv3d &d,
                   double dist);

   void renderwire(TRIANGLEREF *ref);

   inline int checkbound(const miniv3d &o,const miniv3d &d,
                         const miniv3d &b,const double r2);

   inline double checkdist(const miniv3d &o,const miniv3d &d,
                           const miniv3d &v1,const miniv3d &v2,const miniv3d &v3);

   // Moeller-Trumbore ray/triangle intersection
   inline int intersect(const miniv3d &o,const miniv3d &d,
                        const miniv3d &v0,const miniv3d &v1,const miniv3d &v2,
                        miniv3d *tuv);
   };

#endif
