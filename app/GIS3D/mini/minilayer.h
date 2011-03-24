// (c) by Stefan Roettger

#ifndef MINILAYER_H
#define MINILAYER_H

#include "minibase.h"

#include "miniload.h"
#include "minicache.h"
#include "datacache.h"

#include "minipoint.h"

#include "miniv3d.h"
#include "miniv4d.h"

#include "miniutm.h"
#include "miniwarp.h"

//! aggregate class for rendering a layer
class minilayer
   {
   public:

   struct MINILAYER_PARAMS_STRUCT
      {
      // auto-determined parameters upon load:

      int cols;            // number of columns per tileset
      int rows;            // number of rows per tileset

      float stretch;       // horizontal coordinate stretching
      float stretchx;      // horizontal stretching in x-direction
      float stretchy;      // horizontal stretching in y-direction

      int basesize;        // base size of texture maps

      BOOLINT usepnm;      // use either PNM or DB loader

      float extent[3];     // extent of tileset

      float offset[3];     // offset of tileset center
      float scaling[3];    // scaling factor of tileset

      minicoord offsetDAT; // original tileset offset
      minicoord extentDAT; // original tileset extent

      minicoord centerGEO; // geo-referenced center point
      minicoord northGEO;  // geo-referenced north point

      // auto-set parameters during rendering:

      minicoord eye; // eye point
      miniv3d dir;   // viewing direction
      miniv3d up;    // up vector

      float aspect;  //  aspect ratio

      double time;   // local time

      // configurable parameters:
      // [parameters marked with * must be changed via set()]
      // [parameters marked with + must not be changed after calling load()]

      int warpmode;             //+ warp mode: linear=0 flat=1 flat_ref=2 affine=3 affine_ref=4

      float vicinity;           //+ projected vicinity of flat warp mode relative to earth radius

      float shift[3];           //+ manual scene shift

      float scale;              //+ scaling of scene
      float exaggeration;       //+ exaggeration of elevations
      float maxelev;            //+ absolute maximum of expected elevations

      float load;               //+ initially loaded area relative to far plane
      float preload;            //+ continuously preloaded area relative to far plane

      float minres;             //+ minimum resolution of triangulation
      int fastinit;             //+ fast initialization level
      float avgd2value;         //+ average d2value for fast initialization

      float sead2;              //+ influence of sea level on d2-values
      float seamin;             //+ lower boundary of sea level influence range
      float seamax;             //+ upper boundary of sea level influence range

      int lazyness;             //+ lazyness of tileset paging
      float update;             //* update period for tileset paging in seconds
      float expire;             //* tile expiration time in seconds

      float upload;             //* tile upload time per frame relative to 1/fps
      float keep;               //* time to keep tiles in the cache in minutes
      float maxdelay;           //* time after which tiles are regarded as delayed relative to update time
      float cache;              //* memory footprint of the cache in mega bytes

      float keepalive;          //* time for which idling threads are kept alive in seconds
      float timeslice;          //* time for which idling threads sleep in seconds

      float fps;                //* frames per second (target frame rate)
      float spu;                //* update period for render buffer in seconds

      float res;                //  global resolution of triangulation

      float fovy;               //  field of view (degrees)
      float nearp;              //  near plane (meters)
      float farp;               //  far plane (meters)

      float reduction1;         //  reduction parameter #1 for invisible tiles
      float reduction2;         //  reduction parameter #2 for invisible tiles

      float cullslope;          // slope under which the terrain is culled

      float range;              //* texture paging range relative to far plane
      float refres;             //* reference resolution for texture paging in meters
      float radius;             //* non-linear kick-in distance relative to texture range
      float dropoff;            //* non-linear lod dropoff at kick-in distance

      float sealevel;           //* sea-level height in meters (off=-MAXFLOAT)

      BOOLINT genmipmaps;       //+ enable on-the-fly generation of mipmaps
      BOOLINT automipmap;       //+ auto mip-map raw textures

      BOOLINT autocompress;     //+ auto-compress raw textures with S3TC
      BOOLINT lod0uncompressed; //+ keep LOD0 textures uncompressed

      int locthreads;           //+ number of local threads
      int numthreads;           //+ number of net threads

      const char *proxyname;    //+ proxy server name
      const char *proxyport;    //+ proxy server port

      const char *elevprefix;   //+ elev tileset prefix
      const char *imagprefix;   //+ imag tileset prefix

      const char *tilesetfile;  //+ tileset sav file
      const char *vtbinisuffix; //+ suffix of vtb ini file
      const char *startupfile;  //+ startup sav file

      const char *localpath;    //+ local directory

      const char *altpath;      //+ alternative data path
      const char *instpath;     //+ installation data path

      // optional feature switches:

      BOOLINT usewaypoints;
      BOOLINT usebricks;

      // optional way-points:

      const char *waypoints;

      float signpostheight;
      float signpostrange;

      float signpostturn;
      float signpostincline;

      const char *brick;

      float bricksize;
      float brickradius;

      int brickpasses;
      float brickceiling;
      float brickscroll;
      };

   typedef MINILAYER_PARAMS_STRUCT MINILAYER_PARAMS;

   //! default constructor
   minilayer(minicache *cache);

   //! destructor
   ~minilayer();

   //! get parameters
   void get(MINILAYER_PARAMS &lparams);

   //! set parameters
   void set(MINILAYER_PARAMS &lparams);

   //! get parameters
   MINILAYER_PARAMS *get() {return(&LPARAMS);}

   //! set parameters
   void set(MINILAYER_PARAMS *lparams) {set(*lparams);}

   //! get the encapsulated terrain object
   miniload *getterrain() {return(TERRAIN);}

   //! get the encapsulated cache object
   datacache *getcache() {return(TILECACHE);}

   //! get the encapsulated waypoint object
   minipoint *getpoints() {return(POINTS);}

   //! get the encapsulated warp object
   miniwarp *getwarp() {return(WARP);}

   //! set internal callbacks
   void setcallbacks(void *threaddata,
                     void (*threadinit)(int threads,int id,void *data),
                     void (*threadexit)(int id,void *data),
                     void (*startthread)(void *(*thread)(void *background),backarrayelem *background,int id,void *data),
                     void (*jointhread)(backarrayelem *background,int id,void *data),
                     void (*lock_cs)(int id,void *data),void (*unlock_cs)(int id,void *data),
                     void (*lock_io)(int id,void *data),void (*unlock_io)(int id,void *data),
                     void *curldata,
                     void (*curlinit)(int threads,int id,const char *proxyname,const char *proxyport,void *data),
                     void (*curlexit)(int id,void *data),
                     void (*geturl)(const char *src_url,const char *src_id,const char *src_file,const char *dst_file,int background,int id,void *data),
                     int (*checkurl)(const char *src_url,const char *src_id,const char *src_file,int id,void *data));

   //! load tileset
   BOOLINT load(const char *baseurl,const char *baseid,const char *basepath1,const char *basepath2,BOOLINT reset=FALSE);

   //! load optional features
   void loadopts();

   //! create null reference layer
   void setnull();

   //! create earth reference layer
   void setearth();

   //! create empty reference layer
   void setempty(minicoord &center,minicoord &north);

   //! set reference layer
   void setreference(minilayer *ref);

   //! get extent of tileset
   miniv3d getextent();

   //! get center of tileset
   minicoord getcenter();

   //! get normal of tileset
   miniv3d getnormal();

   //! get the elevation at position (x,y,z)
   double getheight(const minicoord &p);

   //! get initial view point
   minicoord getinitial();

   //! set initial eye point
   void initeyepoint(const minicoord &e);

   //! enable a specific focus point other than the eye point
   void enablefocus(const minicoord &f);

   //! disable the focus point
   void disablefocus();

   //! trigger complete render buffer update at next frame
   void update();

   //! generate and cache scene for a particular eye point
   void cache(const minicoord &e,const miniv3d &d,const miniv3d &u,float aspect,double time);

   //! determine whether or not the layer is displayed
   void display(BOOLINT yes=TRUE);

   //! check whether or not the layer is displayed
   BOOLINT isdisplayed();

   //! check whether or not the layer is culled
   BOOLINT isculled();

   //! flatten the terrain by a relative scaling factor (in the range [0-1])
   void flatten(float relscale);

   //! get the flattening factor
   float getflattening();

   //! get the internal cache id
   int getcacheid(); // -1 = none

   //! render waypoints
   void renderpoints();

   // coordinate conversions (g=global, l=local, o=opengl):

   //! map coordinates
   minicoord map_g2l(const minicoord &p) {return(WARP_G2L.warp(p));}
   minicoord map_l2g(const minicoord &p) {return(WARP_L2G.warp(p));}
   minicoord map_g2i(const minicoord &p) {return(WARP_G2I.warp(p));}
   minicoord map_i2g(const minicoord &p) {return(WARP_I2G.warp(p));}
   minicoord map_l2o(const minicoord &p) {return(WARP_L2O.warp(p));}
   minicoord map_o2l(const minicoord &p) {return(WARP_O2L.warp(p));}
   minicoord map_g2o(const minicoord &p) {return(WARP_G2O.warp(p));}
   minicoord map_o2g(const minicoord &p) {return(WARP_O2G.warp(p));}
   minicoord map_g2t(const minicoord &p) {return(WARP_G2T.warp(p));}
   minicoord map_t2g(const minicoord &p) {return(WARP_T2G.warp(p));}

   //! rotate vector
   miniv3d rot_g2l(const miniv3d &v,const minicoord &p) {return(WARP_G2L.invtra(v,p));}
   miniv3d rot_l2g(const miniv3d &v,const minicoord &p) {return(WARP_L2G.invtra(v,p));}
   miniv3d rot_g2i(const miniv3d &v,const minicoord &p) {return(WARP_G2I.invtra(v,p));}
   miniv3d rot_i2g(const miniv3d &v,const minicoord &p) {return(WARP_I2G.invtra(v,p));}
   miniv3d rot_l2o(const miniv3d &v,const minicoord &p) {return(WARP_L2O.invtra(v,p));}
   miniv3d rot_o2l(const miniv3d &v,const minicoord &p) {return(WARP_O2L.invtra(v,p));}
   miniv3d rot_g2o(const miniv3d &v,const minicoord &p) {return(WARP_G2O.invtra(v,p));}
   miniv3d rot_o2g(const miniv3d &v,const minicoord &p) {return(WARP_O2G.invtra(v,p));}
   miniv3d rot_g2t(const miniv3d &v,const minicoord &p) {return(WARP_G2T.invtra(v,p));}
   miniv3d rot_t2g(const miniv3d &v,const minicoord &p) {return(WARP_T2G.invtra(v,p));}

   //! map length
   double len_g2l(double l) {return(l*WARP_G2L.getscale());}
   double len_l2g(double l) {return(l*WARP_L2G.getscale());}
   double len_g2i(double l) {return(l*WARP_G2I.getscale());}
   double len_i2g(double l) {return(l*WARP_I2G.getscale());}
   double len_l2o(double l) {return(l*WARP_L2O.getscale());}
   double len_o2l(double l) {return(l*WARP_O2L.getscale());}
   double len_g2o(double l) {return(l*WARP_G2O.getscale());}
   double len_o2g(double l) {return(l*WARP_O2G.getscale());}
   double len_g2t(double l) {return(l*WARP_G2T.getscale());}
   double len_t2g(double l) {return(l*WARP_T2G.getscale());}

   protected:

   MINILAYER_PARAMS LPARAMS;

   miniwarp WARP_G2L;
   miniwarp WARP_L2G;
   miniwarp WARP_G2I;
   miniwarp WARP_I2G;
   miniwarp WARP_L2O;
   miniwarp WARP_O2L;
   miniwarp WARP_G2O;
   miniwarp WARP_O2G;
   miniwarp WARP_G2T;
   miniwarp WARP_T2G;

   private:

   minicache *CACHE;

   miniload *TERRAIN;
   datacache *TILECACHE;

   minipoint *POINTS;

   miniwarp *WARP;

   int WARPMODE;
   double SCALE;

   minilayer *REFERENCE;
   miniv4d MTXREF[3];

   BOOLINT LOADED;

   BOOLINT VISIBLE;

   int UPD;

   void createwarp(minicoord offsetDAT,minicoord extentDAT,
                   minicoord centerGEO,minicoord northGEO,
                   miniv3d offsetLOC,miniv3d scalingLOC,
                   double scaleLOC);

   void pointwarp(minicoord &center,minicoord &north,minicoord &normal,
                  double scale,miniv4d mtx[3]);

   void updatecoords();

   int getthreadid();

   void *THREADDATA;
   void (*THREADINIT)(int threads,int id,void *data);
   void (*THREADEXIT)(int id,void *data);
   void (*STARTTHREAD)(void *(*thread)(void *background),backarrayelem *background,int id,void *data);
   void (*JOINTHREAD)(backarrayelem *background,int id,void *data);
   void (*LOCK_CS)(int id,void *data);
   void (*UNLOCK_CS)(int id,void *data);
   void (*LOCK_IO)(int id,void *data);
   void (*UNLOCK_IO)(int id,void *data);

   void *CURLDATA;
   void (*CURLINIT)(int threads,int id,const char *proxyname,const char *proxyport,void *data);
   void (*CURLEXIT)(int id,void *data);
   void (*GETURL)(const char *src_url,const char *src_id,const char *src_file,const char *dst_file,int background,int id,void *data);
   int (*CHECKURL)(const char *src_url,const char *src_id,const char *src_file,int id,void *data);

   static void request_callback(const char *file,int istexture,databuf *buf,void *data);

   void threadinit(int threads,int id);
   void threadexit(int id);

   static void startthread(void *(*thread)(void *background),backarrayelem *background,void *data);
   static void jointhread(backarrayelem *background,void *data);

   static void lock_cs(void *data);
   static void unlock_cs(void *data);

   static void lock_io(void *data);
   static void unlock_io(void *data);

   void curlinit(int threads,int id,const char *proxyname,const char *proxyport);
   void curlexit(int id);

   static void getURL(const char *src_url,const char *src_id,const char *src_file,const char *dst_file,int background,void *data);
   static int checkURL(const char *src_url,const char *src_id,const char *src_file,void *data);
   };

#endif
