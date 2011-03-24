// (c) by Stefan Roettger

#ifndef MINITERRAIN_H
#define MINITERRAIN_H

#include "minibase.h"

#include "minicache.h"

#include "miniv3d.h"
#include "miniv4d.h"

#include "minipoint.h"
#include "datagrid.h"

#include "minilayer.h"

//! aggregate class for rendering multiple layers
class miniterrain
   {
   public:

   struct MINITERRAIN_PARAMS_STRUCT
      {
      // auto-determined parameters:

      double time; // actual time

      // configurable parameters:
      // [parameters marked with * must be changed via set()]
      // [parameters marked with + must not be changed after calling load()]

      int warpmode;             //+ warp mode: linear=0 flat=1 flat_ref=2 affine=3 affine_ref=4

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

      float cullslope;          //  slope under which the terrain is culled

      float range;              //* texture paging range relative to far plane
      float refres;             //* reference resolution for texture paging in meters
      float radius;             //* non-linear kick-in distance relative to texture range
      float dropoff;            //* non-linear lod dropoff at kick-in distance

      float overlap;            //* tile overlap in texels

      float sealevel;           //* sea-level height in meters (off=-MAXFLOAT)

      BOOLINT omitsea;          //+ omit sea level when shooting rays

      BOOLINT genmipmaps;       //+ enable on-the-fly generation of mipmaps
      BOOLINT automipmap;       //+ auto mip-map raw textures

      BOOLINT autocompress;     //+ auto-compress raw textures with S3TC
      BOOLINT lod0uncompressed; //+ keep LOD0 textures uncompressed

      const char *elevdir;      //+ default elev directory
      const char *imagdir;      //+ default imag directory

      const char *proxyname;    //+ proxy server name
      const char *proxyport;    //+ proxy server port

      const char *localpath;    //+ local directory

      const char *altpath;      //+ alternative data path
      const char *instpath;     //+ installation data path

      // optional feature switches:

      BOOLINT usefog;
      BOOLINT useshaders;
      BOOLINT usediffuse;
      BOOLINT usedimming;
      BOOLINT usevisshader;
      BOOLINT usebathymap;
      BOOLINT usecontours;
      BOOLINT usenprshader;
      BOOLINT usewaypoints;
      BOOLINT usebricks;

      // optional lighting:

      miniv3d lightdir;

      float lightbias;
      float lightoffset;

      // optional spherical fog:

      float fogcolor[3];

      float fogstart;
      float fogdensity;

      // optional color mapping:

      float bathystart;
      float bathyend;

      unsigned char *bathymap;
      int bathywidth,bathyheight,bathycomps;

      float contours;

      float seacolor[3];

      float seatrans;
      float seamodulate;
      float seabottom;
      float bottomtrans;

      float bottomcolor[3];

      // optional NPR rendering:

      float nprfadefactor;

      float nprbathystart;
      float nprbathyend;

      unsigned char *nprbathymap;
      int nprbathywidth,nprbathyheight,nprbathycomps;

      float nprcontours;

      float nprseacolor[3];

      float nprseatrans;
      float nprseagray;

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

   typedef MINITERRAIN_PARAMS_STRUCT MINITERRAIN_PARAMS;

   //! default constructor
   miniterrain();

   //! destructor
   ~miniterrain();

   //! get parameters
   void get(MINITERRAIN_PARAMS &tparams);

   //! set parameters
   void set(MINITERRAIN_PARAMS &tparams);

   //! get parameters
   MINITERRAIN_PARAMS *get() {return(&TPARAMS);}

   //! set parameters
   void set(MINITERRAIN_PARAMS *tparams) {set(*tparams);}

   //! propagate parameters
   void propagate();

   //! propagate waypoint parameters
   void propagate_wp();

   //! get the encapsulated render buffer
   minicache *getbuffer() {return(CACHE);}

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

   //! load tileset (short version)
   BOOLINT load(const char *url,
                BOOLINT loadopts=FALSE,BOOLINT reset=FALSE);

   //! load tileset (long version)
   BOOLINT load(const char *baseurl,const char *baseid,const char *basepath1,const char *basepath2,
                BOOLINT loadopts=FALSE,BOOLINT reset=FALSE);

   //! create empty layer
   minilayer *create(minicoord &center,minicoord &north);

   //! get the number of terrain layers
   int getlnum() {return(LNUM);}

   //! get null layer
   int getnull();

   //! get earth layer
   int getearth();

   //! get default layer
   int getdefault();

   //! set reference layer
   void setreference(int ref);

   //! get reference layer
   int getreference();

   //! get the nth terrain layer
   minilayer *getlayer(int n);

   //! get the serial number of a terrain layer
   int getnum(minilayer *layer);

   //! remove the nth terrain layer
   void remove(int n);

   //! get extent of a tileset
   miniv3d getextent(int n);

   //! get center of a tileset
   minicoord getcenter(int n);

   //! get the elevation at position (x,y,z)
   double getheight(const minicoord &p);

   //! get initial view point
   minicoord getinitial();

   //! set initial eye point
   void initeyepoint(const minicoord &e);

   //! get nearest layer
   int getnearest(const minicoord &e);

   //! enable a specific focus point other than the eye point
   void enablefocus(const minicoord &f);

   //! disable the focus point
   void disablefocus();

   //! trigger complete render buffer update at next frame
   void update();

   //! generate and cache scene for a particular eye point
   void cache(const minicoord &e,const miniv3d &d,const miniv3d &u,float aspect,double time=0.0);

   //! render cached scene
   void render();

   //! determine whether or not a layer is displayed
   void display(int n,BOOLINT yes=TRUE);

   //! check whether or not a layer is displayed
   BOOLINT isdisplayed(int n);

   //! check whether or not a layer is culled
   BOOLINT isculled(int n);

   //! flatten the terrain by a relative scaling factor (in the range [0-1])
   void flatten(float relscale);

   //! get the flattening factor
   float getflattening();

   //! shoot a ray at the scene
   double shoot(const minicoord &o,const miniv3d &d,double hitdist=0.0,int *id=NULL);

   //! set locking callbacks for ray shooting
   //! when the callbacks are set ray shooting can be triggered safely from a separate thread
   static void setraycallbacks(void (*lock)(void *data),void *data,
                               void (*unlock)(void *data));

   //! return memory consumption
   double getmem();

   //! return raw texture memory consumption
   double gettexmem();

   //! get total number of pending tiles
   int getpending();

   //! get total amount of cache memory
   double getcachemem();

   //! register waypoint renderer
   void registerrndr(minipointrndr *rndr);

   //! add datagrid object
   void addgrid(datagrid *obj,BOOLINT sort=FALSE);

   // default coordinate conversions (g=global, l=local, o=opengl):

   //! map coordinates
   minicoord map_g2l(const minicoord &p) {return((REFERENCE==NULL)?minicoord():REFERENCE->map_g2l(p));}
   minicoord map_l2g(const minicoord &p) {return((REFERENCE==NULL)?minicoord():REFERENCE->map_l2g(p));}
   minicoord map_l2o(const minicoord &p) {return((REFERENCE==NULL)?minicoord():REFERENCE->map_l2o(p));}
   minicoord map_o2l(const minicoord &p) {return((REFERENCE==NULL)?minicoord():REFERENCE->map_o2l(p));}
   minicoord map_g2o(const minicoord &p) {return((REFERENCE==NULL)?minicoord():REFERENCE->map_g2o(p));}
   minicoord map_o2g(const minicoord &p) {return((REFERENCE==NULL)?minicoord():REFERENCE->map_o2g(p));}

   //! rotate vector
   miniv3d rot_g2l(const miniv3d &v,const minicoord &p) {return((REFERENCE==NULL)?miniv3d(0.0):REFERENCE->rot_g2l(v,p));}
   miniv3d rot_l2g(const miniv3d &v,const minicoord &p) {return((REFERENCE==NULL)?miniv3d(0.0):REFERENCE->rot_l2g(v,p));}
   miniv3d rot_l2o(const miniv3d &v,const minicoord &p) {return((REFERENCE==NULL)?miniv3d(0.0):REFERENCE->rot_l2o(v,p));}
   miniv3d rot_o2l(const miniv3d &v,const minicoord &p) {return((REFERENCE==NULL)?miniv3d(0.0):REFERENCE->rot_o2l(v,p));}
   miniv3d rot_g2o(const miniv3d &v,const minicoord &p) {return((REFERENCE==NULL)?miniv3d(0.0):REFERENCE->rot_g2o(v,p));}
   miniv3d rot_o2g(const miniv3d &v,const minicoord &p) {return((REFERENCE==NULL)?miniv3d(0.0):REFERENCE->rot_o2g(v,p));}

   //! map length
   double len_g2l(double l) {return((REFERENCE==NULL)?0.0:REFERENCE->len_g2l(l));}
   double len_l2g(double l) {return((REFERENCE==NULL)?0.0:REFERENCE->len_l2g(l));}
   double len_l2o(double l) {return((REFERENCE==NULL)?0.0:REFERENCE->len_l2o(l));}
   double len_o2l(double l) {return((REFERENCE==NULL)?0.0:REFERENCE->len_o2l(l));}
   double len_g2o(double l) {return((REFERENCE==NULL)?0.0:REFERENCE->len_g2o(l));}
   double len_o2g(double l) {return((REFERENCE==NULL)?0.0:REFERENCE->len_o2g(l));}

   protected:

   MINITERRAIN_PARAMS TPARAMS;

   private:

   minicache *CACHE;

   minilayer **LAYER;
   int LNUM,LMAX;

   int NULL_LAYER;
   int EARTH_LAYER;
   int DEFAULT_LAYER;

   int LREF;
   minilayer *REFERENCE;

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

   datagrid *DATAGRID;
   BOOLINT SORT;

   int reserve();

   int setnull();
   int setearth();

   void render_presea();
   void render_postsea();

   static void preseacb(void *data);
   static void postseacb(void *data);
   };

#endif
