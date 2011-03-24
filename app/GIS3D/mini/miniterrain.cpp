// (c) by Stefan Roettger

#include "datacache.h"

#include "miniOGL.h"
#include "minishader.h"

#include "miniwarp.h"

#include "miniterrain.h"

// default constructor
miniterrain::miniterrain()
   {
   // auto-determined parameters:

   TPARAMS.time=0.0; // actual time

   // configurable parameters:

   TPARAMS.warpmode=0;             // warp mode: linear=0 flat=1 flat_ref=2 affine=3 affine_ref=4

   TPARAMS.scale=1.0f;             // scaling of scene
   TPARAMS.exaggeration=1.0f;      // exaggeration of elevations
   TPARAMS.maxelev=15000.0f;       // absolute maximum of expected elevations

   TPARAMS.load=0.01f;             // initially loaded area relative to far plane
   TPARAMS.preload=1.25f;          // continuously preloaded area relative to far plane

   TPARAMS.minres=9.0f;            // minimum resolution of triangulation
   TPARAMS.fastinit=1;             // fast initialization level
   TPARAMS.avgd2value=0.1f;        // average d2value for fast initialization

   TPARAMS.sead2=0.1f;             // influence of sea level on d2-values
   TPARAMS.seamin=-1.0f;           // lower boundary of sea level influence range
   TPARAMS.seamax=1.0f;            // upper boundary of sea level influence range

   TPARAMS.lazyness=1;             // lazyness of tileset paging
   TPARAMS.update=1.0f;            // update period for tileset paging in seconds
   TPARAMS.expire=60.0f;           // tile expiration time in seconds

   TPARAMS.upload=0.25f;           // tile upload time per frame relative to 1/fps
   TPARAMS.keep=0.25f;             // time to keep tiles in the cache in minutes
   TPARAMS.maxdelay=1.0f;          // time after which tiles are regarded as delayed relative to update time
   TPARAMS.cache=128.0f;           // memory footprint of the cache in mega bytes

   TPARAMS.keepalive=10.0f;        // time for which idling threads are kept alive in seconds
   TPARAMS.timeslice=0.001f;       // time for which idling threads sleep in seconds

   TPARAMS.fps=25.0f;              // frames per second (target frame rate)
   TPARAMS.spu=0.5f;               // update period for render buffer in seconds

   TPARAMS.res=1.0E3f;             // global resolution of triangulation

   TPARAMS.fovy=60.0f;             // field of view (degrees)
   TPARAMS.nearp=10.0f;            // near plane (meters)
   TPARAMS.farp=10000.0f;          // far plane (meters)

   TPARAMS.reduction1=2.0f;        // reduction parameter #1 for invisible tiles
   TPARAMS.reduction2=3.0f;        // reduction parameter #2 for invisible tiles

   TPARAMS.cullslope=0.05f;        // slope under which the terrain is culled

   TPARAMS.range=0.001f;           // texture paging range relative to far plane
   TPARAMS.refres=1.0f;            // reference resolution for texture paging in meters
   TPARAMS.radius=3.0f;            // non-linear kick-in distance relative to texture range
   TPARAMS.dropoff=1.0f;           // non-linear lod dropoff at kick-in distance

   TPARAMS.overlap=0.001f;         // tile overlap in texels

   TPARAMS.sealevel=-MAXFLOAT;     // sea-level height in meters (off=-MAXFLOAT)

   TPARAMS.omitsea=FALSE;          // omit sea level when shooting rays

   TPARAMS.genmipmaps=FALSE;       // enable on-the-fly generation of mipmaps
   TPARAMS.automipmap=FALSE;       // auto mip-map raw textures

   TPARAMS.autocompress=FALSE;     // auto-compress raw textures with S3TC
   TPARAMS.lod0uncompressed=FALSE; // keep LOD0 textures uncompressed

   TPARAMS.elevdir="elev";         // default elev directory
   TPARAMS.imagdir="imag";         // default imag directory

   TPARAMS.proxyname=NULL;         // proxy server name
   TPARAMS.proxyport=NULL;         // proxy server port

#ifndef _WIN32
   TPARAMS.localpath="/var/tmp/";           // local directory
#else
   TPARAMS.localpath="C:\\Windows\\Temp\\"; // local directory for Windows
#endif

   TPARAMS.altpath="data/";             // alternative data path
   TPARAMS.instpath="/usr/share/mini/"; // installation data path

   // optional feature switches:

   TPARAMS.usefog=FALSE;
   TPARAMS.useshaders=FALSE;
   TPARAMS.usediffuse=FALSE;
   TPARAMS.usedimming=FALSE;
   TPARAMS.usevisshader=FALSE;
   TPARAMS.usebathymap=FALSE;
   TPARAMS.usecontours=FALSE;
   TPARAMS.usenprshader=FALSE;
   TPARAMS.usewaypoints=FALSE;
   TPARAMS.usebricks=FALSE;

   // optional lighting:

   TPARAMS.lightdir=miniv3d(0.0,0.0,1.0); // directional light

   TPARAMS.lightbias=0.75f;   // lighting bias
   TPARAMS.lightoffset=0.25f; // lighting offset

   // optional spherical fog:

   TPARAMS.fogcolor[0]=0.65f;
   TPARAMS.fogcolor[1]=0.7f;
   TPARAMS.fogcolor[2]=0.7f;

   TPARAMS.fogstart=0.5f;   // start of fog relative to far plane
   TPARAMS.fogdensity=0.5f; // relative fog density

   // optional color mapping:

   TPARAMS.bathystart=0.0f;      // start of colormapped elevation range
   TPARAMS.bathyend=0.0f;        // end of colormapped elevation range

   TPARAMS.bathymap=NULL;        // RGB[A] colormap
   TPARAMS.bathywidth=0;         // width of colormap
   TPARAMS.bathyheight=0;        // height of colormap
   TPARAMS.bathycomps=0;         // components of colormap (either 3 or 4)

   TPARAMS.contours=10.0f;       // contour distance in meters

   TPARAMS.seacolor[0]=0.0f;     // color of sea surface (matches with Blue Marble default sea color)
   TPARAMS.seacolor[1]=0.0f;     // color of sea surface (matches with Blue Marble default sea color)
   TPARAMS.seacolor[2]=0.2f;     // color of sea surface (matches with Blue Marble default sea color)

   TPARAMS.seatrans=0.25f;       // transparency of sea surface
   TPARAMS.seamodulate=0.0f;     // modulation factor of sea surface texture
   TPARAMS.seabottom=-10.0f;     // depth where sea transparency gets saturated
   TPARAMS.bottomtrans=1.0f;     // saturated transparency at sea bottom

   TPARAMS.bottomcolor[0]=0.0f;  // saturated color at sea bottom
   TPARAMS.bottomcolor[1]=0.1f;  // saturated color at sea bottom
   TPARAMS.bottomcolor[2]=0.25f; // saturated color at sea bottom

   // optional NPR rendering:

   TPARAMS.nprfadefactor=0.25f;  // NPR fade factor

   TPARAMS.nprbathystart=0.0f;   // start of colormapped elevation range
   TPARAMS.nprbathyend=0.0f;     // end of colormapped elevation range

   TPARAMS.nprbathymap=NULL;     // RGB[A] colormap
   TPARAMS.nprbathywidth=0;      // width of colormap
   TPARAMS.nprbathyheight=0;     // height of colormap
   TPARAMS.nprbathycomps=0;      // components of colormap (either 3 or 4)

   TPARAMS.nprcontours=100.0f;   // NPR contour distance in meters

   TPARAMS.nprseacolor[0]=0.5f;  // NPR color of sea surface
   TPARAMS.nprseacolor[1]=0.75f; // NPR color of sea surface
   TPARAMS.nprseacolor[2]=1.0f;  // NPR color of sea surface

   TPARAMS.nprseatrans=0.25f;    // NPR transparency of sea surface
   TPARAMS.nprseagray=0.5f;      // NPR grayness of bathymetry

   // optional way-points:

   TPARAMS.waypoints="Waypoints.txt"; // waypoint file

   TPARAMS.signpostturn=0.0f;     // horizontal orientation of signposts in degrees
   TPARAMS.signpostincline=0.0f;  // vertical orientation of signposts in degrees

   TPARAMS.signpostheight=100.0f; // height of signposts in meters
   TPARAMS.signpostrange=0.1f;    // viewing range of signposts relative to far plane

   TPARAMS.brick="Cone.db";     // brick file

   TPARAMS.bricksize=100.0f;    // brick size in meters
   TPARAMS.brickradius=1000.0f; // brick display radius in meters

   TPARAMS.brickpasses=4;       // brick render passes
   TPARAMS.brickceiling=3.0f;   // upper boundary for brick color mapping relative to elevation of first waypoint
   TPARAMS.brickscroll=0.5f;    // scroll period of striped bricks in seconds

   // create the render cache
   CACHE=new minicache;

   // initialize state:

   LAYER=NULL;
   LNUM=LMAX=0;

   NULL_LAYER=setnull();
   EARTH_LAYER=setearth();

   DEFAULT_LAYER=LNUM;
   setreference(DEFAULT_LAYER);

   THREADDATA=NULL;
   THREADINIT=NULL;
   THREADEXIT=NULL;
   STARTTHREAD=NULL;
   JOINTHREAD=NULL;
   LOCK_CS=NULL;
   UNLOCK_CS=NULL;
   LOCK_IO=NULL;
   UNLOCK_IO=NULL;

   CURLDATA=NULL;
   CURLINIT=NULL;
   CURLEXIT=NULL;
   GETURL=NULL;
   CHECKURL=NULL;

   DATAGRID=NULL;
   SORT=FALSE;
   }

// destructor
miniterrain::~miniterrain()
   {
   int n;

   // delete the tileset layers
   if (LAYER!=NULL)
      {
      for (n=0; n<LNUM; n++) delete LAYER[n];
      delete LAYER;
      }

   // delete the render cache
   delete CACHE;
   }

// reserve space in layer array
int miniterrain::reserve()
   {
   // create layer array
   if (LAYER==NULL)
      {
      LMAX=1;
      if ((LAYER=(minilayer **)malloc(LMAX*sizeof(minilayer *)))==NULL) ERRORMSG();
      }

   // enlarge layer array
   if (LNUM>=LMAX)
      {
      LMAX*=2;
      if ((LAYER=(minilayer **)realloc(LAYER,LMAX*sizeof(minilayer *)))==NULL) ERRORMSG();
      }

   LAYER[LNUM]=NULL;

   return(LNUM++);
   }

// get parameters
void miniterrain::get(MINITERRAIN_PARAMS &tparams)
   {tparams=TPARAMS;}

// set parameters
void miniterrain::set(MINITERRAIN_PARAMS &tparams)
   {
   int n;

   minilayer::MINILAYER_PARAMS lparams;

   // set new state
   TPARAMS=tparams;

   // propagate layer parameters
   if (LAYER!=NULL)
      for (n=0; n<LNUM; n++)
         {
         // get the actual layer state
         LAYER[n]->get(lparams);

         // update the layer state:

         lparams.warpmode=TPARAMS.warpmode;

         lparams.scale=TPARAMS.scale;
         lparams.exaggeration=TPARAMS.exaggeration;
         lparams.maxelev=TPARAMS.maxelev;

         lparams.load=TPARAMS.load;
         lparams.preload=TPARAMS.preload;

         lparams.minres=TPARAMS.minres;
         lparams.fastinit=TPARAMS.fastinit;
         lparams.avgd2value=TPARAMS.avgd2value;

         lparams.sead2=TPARAMS.sead2;
         lparams.seamin=TPARAMS.seamin;
         lparams.seamax=TPARAMS.seamax;

         lparams.lazyness=TPARAMS.lazyness;
         lparams.update=TPARAMS.update;
         lparams.expire=TPARAMS.expire;

         lparams.upload=TPARAMS.upload;
         lparams.keep=TPARAMS.keep;
         lparams.maxdelay=TPARAMS.maxdelay;
         lparams.cache=TPARAMS.cache;

         lparams.keepalive=TPARAMS.keepalive;
         lparams.timeslice=TPARAMS.timeslice;

         lparams.fps=TPARAMS.fps;
         lparams.spu=TPARAMS.spu;

         lparams.res=TPARAMS.res;

         lparams.fovy=TPARAMS.fovy;
         lparams.nearp=TPARAMS.nearp;
         lparams.farp=TPARAMS.farp;

         lparams.reduction1=TPARAMS.reduction1;
         lparams.reduction2=TPARAMS.reduction2;

         lparams.cullslope=TPARAMS.cullslope;

         lparams.range=TPARAMS.range;
         lparams.refres=TPARAMS.refres;
         lparams.radius=TPARAMS.radius;
         lparams.dropoff=TPARAMS.dropoff;

         lparams.sealevel=TPARAMS.sealevel;

         lparams.genmipmaps=TPARAMS.genmipmaps;
         lparams.automipmap=TPARAMS.automipmap;

         lparams.autocompress=TPARAMS.autocompress;
         lparams.lod0uncompressed=TPARAMS.lod0uncompressed;

         lparams.proxyname=TPARAMS.proxyname;
         lparams.proxyport=TPARAMS.proxyport;

         lparams.localpath=TPARAMS.localpath;

         lparams.altpath=TPARAMS.altpath;
         lparams.instpath=TPARAMS.instpath;

         // finally pass the updated layer state
         LAYER[n]->set(lparams);
         }

   // propagate waypoint parameters
   propagate_wp();

   // update color maps
   minishader::setVISbathymap(TPARAMS.bathymap,TPARAMS.bathywidth,TPARAMS.bathyheight,TPARAMS.bathycomps);
   minishader::setNPRbathymap(TPARAMS.nprbathymap,TPARAMS.nprbathywidth,TPARAMS.nprbathyheight,TPARAMS.nprbathycomps);

   // reset reference layer
   setreference(LREF);
   }

// propagate parameters
void miniterrain::propagate()
   {set(TPARAMS);}

// propagate waypoint parameters
void miniterrain::propagate_wp()
   {
   int n;

   minilayer::MINILAYER_PARAMS *lparams;

   // propagate waypoint parameters
   if (LAYER!=NULL)
      for (n=0; n<LNUM; n++)
         {
         // get a reference to the actual layer state
         lparams=LAYER[n]->get();

         // update the waypoint state:

         lparams->usewaypoints=TPARAMS.usewaypoints;
         lparams->usebricks=TPARAMS.usebricks;

         lparams->waypoints=TPARAMS.waypoints;

         lparams->signpostheight=TPARAMS.signpostheight;
         lparams->signpostrange=TPARAMS.signpostrange;

         lparams->signpostturn=TPARAMS.signpostturn;
         lparams->signpostincline=TPARAMS.signpostincline;

         lparams->brick=TPARAMS.brick;

         lparams->bricksize=TPARAMS.bricksize;
         lparams->brickradius=TPARAMS.brickradius;

         lparams->brickpasses=TPARAMS.brickpasses;
         lparams->brickceiling=TPARAMS.brickceiling;
         lparams->brickscroll=TPARAMS.brickscroll;
         }
   }

// set internal callbacks
void miniterrain::setcallbacks(void *threaddata,
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
                               int (*checkurl)(const char *src_url,const char *src_id,const char *src_file,int id,void *data))
   {
   THREADDATA=threaddata;

   THREADINIT=threadinit;
   THREADEXIT=threadexit;

   STARTTHREAD=startthread;
   JOINTHREAD=jointhread;
   LOCK_CS=lock_cs;
   UNLOCK_CS=unlock_cs;
   LOCK_IO=lock_io;
   UNLOCK_IO=unlock_io;

   CURLDATA=curldata;

   CURLINIT=curlinit;
   CURLEXIT=curlexit;

   GETURL=geturl;
   CHECKURL=checkurl;
   }

// load tileset (short version)
BOOLINT miniterrain::load(const char *url,
                          BOOLINT loadopts,BOOLINT reset)
   {
   char *baseurl;
   char *lastslash,*lastbslash;
   char *baseid;

   BOOLINT success;

   baseurl=strdup(url);

   // remove trailing slash
   if (strlen(baseurl)>1)
      if (baseurl[strlen(baseurl)-1]=='/') baseurl[strlen(baseurl)-1]='\0';

   // remove trailing backslash
   if (strlen(baseurl)>1)
      if (baseurl[strlen(baseurl)-1]=='\\') baseurl[strlen(baseurl)-1]='\0';

   // search for last slash
   lastslash=strrchr(baseurl,'/');

   // search for last backslash
   lastbslash=strrchr(baseurl,'\\');

   // give up if no slash or backslash was found
   if (lastslash==NULL && lastbslash==NULL) success=FALSE;
   else
      {
      // decompose url into baseurl and baseid
      if (lastslash!=NULL)
         {
         baseid=strcct(++lastslash,"/");
         *lastslash='\0';
         }
      else
         {
         baseid=strcct(++lastbslash,"\\");
         *lastbslash='\0';
         }

      // load tileset
      success=load(baseurl,baseid,TPARAMS.elevdir,TPARAMS.imagdir,loadopts,reset);

      free(baseid);
      }

   free(baseurl);

   return(success);
   }

// load tileset (long version)
BOOLINT miniterrain::load(const char *baseurl,const char *baseid,const char *basepath1,const char *basepath2,
                          BOOLINT loadopts,BOOLINT reset)
   {
   int n;

   // reserve space in layer array
   n=reserve();

   // create the tileset layer
   LAYER[n]=new minilayer(CACHE);

   // propagate parameters
   propagate();

   // set reference coordinate system
   LAYER[n]->setreference(LAYER[getreference()]);

   // register callbacks
   LAYER[n]->setcallbacks(THREADDATA,
                          THREADINIT,THREADEXIT,
                          STARTTHREAD,JOINTHREAD,
                          LOCK_CS,UNLOCK_CS,
                          LOCK_IO,UNLOCK_IO,
                          CURLDATA,
                          CURLINIT,CURLEXIT,
                          GETURL,CHECKURL);

   // load the tileset layer
   if (!LAYER[n]->load(baseurl,baseid,basepath1,basepath2,reset))
      {
      remove(n);
      return(FALSE);
      }

   // load optional features
   if (loadopts) LAYER[n]->loadopts();

   // reset reference layer
   setreference(LREF);

   // set pre and post sea surface render callbacks
   CACHE->setseacb(preseacb,postseacb,this);

   // enable alpha test
   CACHE->setalphatest((float)254/255);

   // set tile overlap
   CACHE->configure_overlap(TPARAMS.overlap);

   // turn on ray object
   CACHE->configure_enableray(1);
   CACHE->configure_omitsea(TPARAMS.omitsea);

   // success
   return(TRUE);
   }

// set null layer
int miniterrain::setnull()
   {
   int n;

   // reserve space in layer array
   n=reserve();

   // create the layer
   LAYER[n]=new minilayer(NULL);

   // setup the earth layer
   LAYER[n]->setnull();

   return(n);
   }

// set earth layer
int miniterrain::setearth()
   {
   int n;

   // reserve space in layer array
   n=reserve();

   // create the layer
   LAYER[n]=new minilayer(NULL);

   // setup the earth layer
   LAYER[n]->setearth();

   return(n);
   }

// create empty layer
minilayer *miniterrain::create(minicoord &center,minicoord &north)
   {
   int n;

   // reserve space
   n=reserve();

   // create the layer
   LAYER[n]=new minilayer(NULL);

   // setup the earth layer
   LAYER[n]->setempty(center,north);

   return(LAYER[n]);
   }

// get null layer
int miniterrain::getnull()
   {return(NULL_LAYER);}

// get earth layer
int miniterrain::getearth()
   {return(EARTH_LAYER);}

// get default layer
int miniterrain::getdefault()
   {return(DEFAULT_LAYER);}

// set reference layer
void miniterrain::setreference(int ref)
   {
   int n;

   // set new reference
   LREF=ref;

   // set new reference layer for default coordinate conversions
   if (LNUM>0) REFERENCE=LAYER[getreference()];
   else REFERENCE=NULL;

   // propagate new reference coordinate system
   for (n=0; n<LNUM; n++)
      LAYER[n]->setreference(LAYER[getreference()]);
   }

// get reference layer
int miniterrain::getreference()
   {return(min(LREF,LNUM-1));}

// get the nth terrain layer
minilayer *miniterrain::getlayer(int n)
   {
   if (n<0 || n>=LNUM) return(NULL);
   else return(LAYER[n]);
   }

// get the serial number of a terrain layer
int miniterrain::getnum(minilayer *layer)
   {
   int n;

   for (n=0; n<LNUM; n++)
      if (LAYER[n]==layer) return(n);

   return(-1);
   }

// remove the nth terrain layer
void miniterrain::remove(int n)
   {
   if (n<0 || n>=LNUM) return;

   delete LAYER[n];

   if (n<LNUM-1) LAYER[n]=LAYER[--LNUM];
   else LNUM--;

   if (n==LREF) setreference(getdefault());
   }

// get extent of a tileset
miniv3d miniterrain::getextent(int n)
   {
   if (n<0 || n>=LNUM) return(miniv3d(0.0));
   else return(LAYER[n]->getextent());
   }

// get center of a tileset
minicoord miniterrain::getcenter(int n)
   {
   if (n<0 || n>=LNUM) return(minicoord());
   else return(LAYER[n]->getcenter());
   }

// get the elevation at position (x,y,z)
double miniterrain::getheight(const minicoord &p)
   {
   int n;

   int nst;

   double elev;

   if (LNUM>0)
      {
      nst=getnearest(p);

      if (isdisplayed(nst) && !isculled(nst))
         {
         elev=LAYER[nst]->getheight(p);
         if (elev!=-MAXFLOAT) return(LAYER[getreference()]->len_l2g(LAYER[nst]->len_g2l(elev)));
         }

      for (n=0; n<LNUM; n++)
         if (n!=nst)
            if (isdisplayed(n) && !isculled(n))
               {
               elev=LAYER[n]->getheight(p);
               if (elev!=-MAXFLOAT) return(LAYER[getreference()]->len_l2g(LAYER[n]->len_g2l(elev)));
               }
      }

   return(-MAXFLOAT);
   }

// get initial view point
minicoord miniterrain::getinitial()
   {
   if (LNUM<1) return(minicoord());
   else return(LAYER[getreference()]->getinitial());
   }

// set initial eye point
void miniterrain::initeyepoint(const minicoord &e)
   {
   int n;

   for (n=0; n<LNUM; n++)
      LAYER[n]->initeyepoint(e);
   }

// get nearest layer
int miniterrain::getnearest(const minicoord &e)
   {
   int n;

   int nst;
   double dist,mindist;
   minicoord offset;

   nst=-1;
   mindist=MAXFLOAT;

   for (n=0; n<LNUM; n++)
      if (isdisplayed(n))
         {
         offset=LAYER[n]->getcenter()-e;
         dist=offset.vec.getlength();

         if (dist<mindist)
            {
            mindist=dist;
            nst=n;
            }
         }

   return(nst);
   }

// enable a specific focus point
void miniterrain::enablefocus(const minicoord &f)
   {
   int n;

   for (n=0; n<LNUM; n++)
      LAYER[n]->enablefocus(f);
   }

// disable the focus point
void miniterrain::disablefocus()
   {
   int n;

   for (n=0; n<LNUM; n++)
      LAYER[n]->disablefocus();
   }

// trigger complete render buffer update at next frame
void miniterrain::update()
   {
   int n;

   for (n=0; n<LNUM; n++)
      LAYER[n]->update();
   }

// generate and cache scene for a particular eye point
void miniterrain::cache(const minicoord &e,const miniv3d &d,const miniv3d &u,float aspect,double time)
   {
   int n;

   TPARAMS.time=time;

   for (n=0; n<LNUM; n++)
      LAYER[n]->cache(e,d,u,aspect,time);
   }

// render cached scene
void miniterrain::render()
   {
   int n;

   miniv3d lgl;

   if (LNUM>0)
      {
      // enable shaders
      if (TPARAMS.useshaders)
         {
         if (TPARAMS.usevisshader)
            minishader::setVISshader(CACHE,
                                     LAYER[getreference()]->len_o2g(1.0),TPARAMS.exaggeration,
                                     (TPARAMS.usefog)?TPARAMS.fogstart/2.0f*TPARAMS.farp:0.0f,(TPARAMS.usefog)?TPARAMS.farp:0.0f,
                                     TPARAMS.fogdensity,
                                     TPARAMS.fogcolor,
                                     (TPARAMS.usebathymap)?TPARAMS.bathystart:TPARAMS.bathyend,TPARAMS.bathyend,
                                     (TPARAMS.usecontours)?TPARAMS.contours:0.0f,
                                     TPARAMS.sealevel,TPARAMS.seabottom,
                                     TPARAMS.seacolor,
                                     TPARAMS.seatrans,TPARAMS.bottomtrans,
                                     TPARAMS.bottomcolor,
                                     TPARAMS.seamodulate);
         else if (TPARAMS.usenprshader)
            minishader::setNPRshader(CACHE,
                                     LAYER[getreference()]->len_o2g(1.0),TPARAMS.exaggeration,
                                     (TPARAMS.usefog)?TPARAMS.fogstart/2.0f*TPARAMS.farp:0.0f,(TPARAMS.usefog)?TPARAMS.farp:0.0f,
                                     TPARAMS.fogdensity,
                                     TPARAMS.fogcolor,
                                     TPARAMS.nprbathystart,TPARAMS.nprbathyend,
                                     (TPARAMS.usecontours)?TPARAMS.nprcontours:0.0f,
                                     TPARAMS.nprfadefactor,
                                     fmax(TPARAMS.sealevel,0.0f),
                                     TPARAMS.nprseacolor,TPARAMS.nprseatrans,
                                     TPARAMS.nprseagray);
         else
            {
            CACHE->setvtxshader();
            CACHE->usevtxshader(1);
            CACHE->setpixshader();
            CACHE->usepixshader(1);
            CACHE->setseashader();
            CACHE->useseashader(1);
            }

         for (n=0; n<LNUM; n++)
            if (LAYER[n]->getterrain()!=NULL)
               if (TPARAMS.usediffuse)
                  {
                  lgl=LAYER[getnull()]->rot_g2o(TPARAMS.lightdir,LAYER[getnull()]->getcenter());

                  if (TPARAMS.usedimming)
                     CACHE->setlight(LAYER[n]->getterrain()->getminitile(),
                                     lgl.x,lgl.y,lgl.z,0.5f*TPARAMS.lightbias,0.5f*TPARAMS.lightoffset);
                  else
                     CACHE->setlight(LAYER[n]->getterrain()->getminitile(),
                                     lgl.x,lgl.y,lgl.z,TPARAMS.lightbias,TPARAMS.lightoffset);
                  }
               else
                  if (TPARAMS.usedimming)
                     CACHE->setlight(LAYER[n]->getterrain()->getminitile(),
                                     0.0f,0.0f,0.0f,0.0f,0.5f);
                  else
                     CACHE->setlight(LAYER[n]->getterrain()->getminitile(),
                                     0.0f,0.0f,0.0f,0.0f,1.0f);
         }

      // render vertex arrays
      CACHE->rendercache();

      // disable shaders
      if (TPARAMS.useshaders)
         if (TPARAMS.usevisshader || TPARAMS.usenprshader)
            minishader::unsetshaders(CACHE);
         else
            {
            CACHE->usevtxshader(0);
            CACHE->usepixshader(0);
            CACHE->useseashader(0);
            }
      }
   }

// pre sea render callback
void miniterrain::preseacb(void *data)
   {
   miniterrain *viewer=(miniterrain *)data;
   viewer->render_presea();
   }

// post sea render callback
void miniterrain::postseacb(void *data)
   {
   miniterrain *viewer=(miniterrain *)data;
   viewer->render_postsea();
   }

// pre sea render function
void miniterrain::render_presea()
   {
   int n;

   minicoord el;

   minilayer::MINILAYER_PARAMS lparams;

   miniwarp warp;
   miniv4d mtx[3];

   for (n=0; n<LNUM; n++)
      if (isdisplayed(n) && !isculled(n))
         {
         LAYER[n]->get(lparams);

         el=LAYER[n]->map_g2l(lparams.eye);

         // render waypoints before sea surface
         if (el.vec.z>=lparams.sealevel/lparams.scale) LAYER[n]->renderpoints();
         }

   if (DATAGRID!=NULL)
      if (!DATAGRID->isclear())
         // trigger data grid before sea surface
         if (el.vec.z>=lparams.sealevel/lparams.scale)
            {
            // set post matrix (world to rendering coordinates)
            warp=*REFERENCE->getwarp();
            warp.setwarp(miniwarp::MINIWARP_METRIC,miniwarp::MINIWARP_FINAL);
            warp.getwarp(mtx);
            DATAGRID->specmtx(mtx);

            // push either sorted or unsorted grid
            if (!SORT) DATAGRID->trigger(TPARAMS.time);
            else DATAGRID->trigger(TPARAMS.time,lparams.eye.vec,lparams.dir,lparams.nearp,lparams.farp,lparams.fovy,lparams.aspect);
            }
   }

// post sea render function
void miniterrain::render_postsea()
   {
   int n;

   minicoord el;

   minilayer::MINILAYER_PARAMS lparams;

   miniwarp warp;
   miniv4d mtx[3];

   for (n=0; n<LNUM; n++)
      if (isdisplayed(n) && !isculled(n))
         {
         LAYER[n]->get(lparams);

         el=LAYER[n]->map_g2l(lparams.eye);

         // render waypoints after sea surface
         if (el.vec.z<lparams.sealevel/lparams.scale) LAYER[n]->renderpoints();
         }

   if (DATAGRID!=NULL)
      if (!DATAGRID->isclear())
         // trigger data grid after sea surface
         if (el.vec.z<lparams.sealevel/lparams.scale)
            {
            // set post matrix (world to rendering coordinates)
            warp=*REFERENCE->getwarp();
            warp.setwarp(miniwarp::MINIWARP_METRIC,miniwarp::MINIWARP_FINAL);
            warp.getwarp(mtx);
            DATAGRID->specmtx(mtx);

            // push either sorted or unsorted grid
            if (!SORT) DATAGRID->trigger(TPARAMS.time);
            else DATAGRID->trigger(TPARAMS.time,lparams.eye.vec,lparams.dir,lparams.nearp,lparams.farp,lparams.fovy,lparams.aspect);
            }
   }

// determine whether or not a layer is displayed
void miniterrain::display(int n,BOOLINT yes)
   {
   if (n>=0 && n<LNUM)
      if (LAYER[n]->getterrain()!=NULL) LAYER[n]->display(yes);
   }

// check whether or not a layer is displayed
BOOLINT miniterrain::isdisplayed(int n)
   {
   if (n>=0 && n<LNUM) return(LAYER[n]->isdisplayed());
   return(FALSE);
   }

// check whether or not a layer is culled
BOOLINT miniterrain::isculled(int n)
   {
   if (n>=0 && n<LNUM) return(LAYER[n]->isculled());
   return(TRUE);
   }

// flatten the terrain by a relative scaling factor (in the range [0-1])
void miniterrain::flatten(float relscale)
   {
   int n;

   for (n=0; n<LNUM; n++)
      LAYER[n]->flatten(relscale);
   }

// get the flattening factor
float miniterrain::getflattening()
   {
   if (LNUM>0) return(LAYER[getreference()]->getflattening());
   else return(1.0f);
   }

// shoot a ray at the scene
double miniterrain::shoot(const minicoord &o,const miniv3d &d,double hitdist,int *id)
   {
   int n;

   int ref,nst;

   double dist;
   double dn;

   minicoord ogl;
   miniv3d dgl;

   int id_hit;

   dist=MAXFLOAT;
   id_hit=-1;

   if (LNUM>0)
      {
      // get reference layer
      ref=getreference();

      // transform coordinates
      ogl=LAYER[ref]->map_g2o(o);
      dgl=LAYER[ref]->rot_g2o(d,o);

      // get nearest layer
      nst=getnearest(o);

      // shoot a ray at the nearest layer
      if (isdisplayed(nst) && !isculled(nst)) dist=CACHE->getray(LAYER[nst]->getcacheid())->shoot(ogl.vec,dgl,hitdist);
      else dist=MAXFLOAT;

      // check for valid hit
      if (dist!=MAXFLOAT)
         {
         dist=LAYER[ref]->len_o2g(dist);
         id_hit=nst;
         }
      else
         for (n=0; n<LNUM; n++)
            if (n!=nst)
               if (isdisplayed(n) && !isculled(n))
                  {
                  // shoot a ray and get the traveled distance
                  dn=CACHE->getray(LAYER[n]->getcacheid())->shoot(ogl.vec,dgl,hitdist);

                  // check for valid hit
                  if (dn!=MAXFLOAT) dn=LAYER[ref]->len_o2g(dn);

                  // remember nearest hit
                  if (dn<dist)
                     {
                     dist=dn;
                     id_hit=n;
                     }

                  // stop if actual distance is already lower than required hitdist
                  if (dist<hitdist) break;
                  }
      }

   if (id!=NULL) *id=id_hit;

   return(dist);
   }

// set locking callbacks for ray shooting
void miniterrain::setraycallbacks(void (*lock)(void *data),void *data,
                                  void (*unlock)(void *data))
   {miniray::setcallbacks(lock,data,unlock);}

// return memory consumption
double miniterrain::getmem()
   {
   int n;

   double mem;

   mem=0.0;

   for (n=0; n<LNUM; n++)
      if (LAYER[n]->getterrain()!=NULL) mem+=LAYER[n]->getterrain()->getmem();

   return(mem);
   }

// return raw texture memory consumption
double miniterrain::gettexmem()
   {
   int n;

   double texmem;

   texmem=0.0;

   for (n=0; n<LNUM; n++)
      if (LAYER[n]->getterrain()!=NULL) texmem+=LAYER[n]->getterrain()->gettexmem();

   return(texmem);
   }

// get total number of pending tiles
int miniterrain::getpending()
   {
   int n;

   int pending;

   pending=0;

   for (n=0; n<LNUM; n++)
      if (LAYER[n]->getcache()!=NULL) pending+=LAYER[n]->getcache()->getpending();

   return(pending);
   }

// get total amount of cache memory
double miniterrain::getcachemem()
   {
   int n;

   double cachemem;

   cachemem=0.0;

   for (n=0; n<LNUM; n++)
      if (LAYER[n]->getcache()!=NULL) cachemem+=LAYER[n]->getcache()->getmem();

   return(cachemem);
   }

// register waypoint renderer
void miniterrain::registerrndr(minipointrndr *rndr)
   {
   int n;

   for (n=0; n<LNUM; n++)
      if (LAYER[n]->getpoints()!=NULL) LAYER[n]->getpoints()->registerrndr(rndr);
   }

// add datagrid object
void miniterrain::addgrid(datagrid *obj,BOOLINT sort)
   {
   DATAGRID=obj;
   SORT=sort;
   }
