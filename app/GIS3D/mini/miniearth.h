// (c) by Stefan Roettger

#ifndef MINIEARTH_H
#define MINIEARTH_H

#include "minibase.h"

#include "database.h"

#include "miniv3d.h"
#include "miniv4d.h"

#include "miniterrain.h"

#include "minisky.h"
#include "miniglobe.h"

//! base class for viewing tilesets
class miniearth
   {
   public:

   struct MINIEARTH_PARAMS_STRUCT
      {
      // configurable parameters:

      int warpmode; // warp mode: linear=0 flat=1 flat_ref=2 affine=3 affine_ref=4

      float fps;    // frames per second (target frame rate)

      float fovy;   //  field of view (degrees)
      float nearp;  //  near plane (meters)
      float farp;   //  far plane (meters)

      // feature switches:

      BOOLINT usefog;
      BOOLINT useshaders;
      BOOLINT usediffuse;
      BOOLINT usevisshader;
      BOOLINT usebathymap;
      BOOLINT usecontours;
      BOOLINT usenprshader;
      BOOLINT usewireframe;
      BOOLINT useskydome;
      BOOLINT usewaypoints;
      BOOLINT usebricks;
      BOOLINT useearth;
      BOOLINT useflat;

      // optional spherical fog:

      float fogcolor[3];

      float fogstart;
      float fogdensity;

      // optional void display:

      float voidstart;
      float voidcolor[3];

      // optional sky-dome:

      const char *skydome;

      // optional earth globe:

      miniv3d lightdir;

      float lightbias;
      float lightoffset;

      float transbias;
      float transoffset;

      BOOLINT shrinkZpass;

      // earth textures as a PPM file
      const char *frontname;
      const char *backname;

      // earth textures as a databuf object
      databuf *frontbuf;
      databuf *backbuf;

      // earth textures as a DB file
      const char *frontbufname;
      const char *backbufname;
      };

   typedef MINIEARTH_PARAMS_STRUCT MINIEARTH_PARAMS;

   //! default constructor
   miniearth();

   //! destructor
   ~miniearth();

   //! get parameters
   void get(MINIEARTH_PARAMS &eparams);

   //! set parameters
   void set(MINIEARTH_PARAMS &eparams);

   //! get parameters
   MINIEARTH_PARAMS *get() {return(&EPARAMS);}

   //! set parameters
   void set(MINIEARTH_PARAMS *eparams) {set(*eparams);}

   // propagate parameters
   void propagate();

   //! get the encapsulated terrain object
   miniterrain *getterrain() {return(TERRAIN);}

   //! load tileset (short version)
   BOOLINT load(const char *url,
                BOOLINT loadopts=FALSE,BOOLINT reset=FALSE);

   //! load tileset (long version)
   BOOLINT load(const char *baseurl,const char *baseid,const char *basepath1,const char *basepath2,
                BOOLINT loadopts=FALSE,BOOLINT reset=FALSE);

   //! load optional features
   void loadopts();

   //! get initial view point
   minicoord getinitial();

   //! set initial eye point
   void initeyepoint(const minicoord &e);

   //! clear scene
   void clear();

   //! generate and cache scene for a particular eye point and time step
   void cache(const minicoord &e,const miniv3d &d,const miniv3d &u,float aspect,double time=0.0);

   //! render scene
   void render();

   //! shoot a ray at the scene
   double shoot(const minicoord &o,const miniv3d &d,double hitdist=0.0);

   //! get null layer
   minilayer *getnull() {return(TERRAIN->getlayer(TERRAIN->getnull()));}

   //! get earth layer
   minilayer *getearth() {return(TERRAIN->getlayer(TERRAIN->getearth()));}

   //! get reference layer
   minilayer *getreference() {return(TERRAIN->getlayer(TERRAIN->getreference()));}

   //! get nearest layer
   minilayer *getnearest(const minicoord &p) {return(TERRAIN->getlayer(TERRAIN->getnearest(p)));}

   protected:

   MINIEARTH_PARAMS EPARAMS;

   private:

   miniterrain *TERRAIN;

   minisky *SKYDOME;
   miniglobe *EARTH;

   BOOLINT LOADED;

   BOOLINT CLEAR;

   void initOGL();

   char *getfile(const char *src_file,const char *altpath=NULL);

   double intersect_unitsphere(miniv3d p,miniv3d d);

   double intersect_ellipsoid(miniv3d p,miniv3d d,
                              miniv3d o,double r1,double r2,double r3);
   };

#endif
