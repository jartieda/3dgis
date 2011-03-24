// (c) by Stefan Roettger

#ifndef MINIPOINT_H
#define MINIPOINT_H

#include "minibase.h"

#include "datacache.h"

#include "minitile.h"
#include "minilod.h"

// forward declarations
class minipoint;
class minipointopts;
class minipointrndr;

//! waypoint data
typedef struct
   {
   float x,y;
   float elev; // waypoint elevation
   float height; // terrain height

   char *desc;
   char *meta;
   char *comment;
   char *system;
   char *latitude;
   char *longitude;
   char *elevation;
   int zone,datum;

   minipointopts *opts;
   minipointrndr *rndr;

   int number;
   }
minipointdata;

//! comparison operator of waypoint data
inline int operator < (const minipointdata &a,const minipointdata &b);

//! waypoint options
class minipointopts
   {
   public:

   enum
      {
      OPTION_DELIMITER=1,
      OPTION_EQ=2,
      OPTION_TYPE=3,
      OPTION_SIGNPOSTSIZE=4,
      OPTION_SIGNPOSTHEIGHT=5,
      OPTION_SIGNPOSTNOAUTO=6,
      OPTION_SIGNPOSTTURN=7,
      OPTION_SIGNPOSTINCLINE=8,
      OPTION_SIGNPOSTALPHA=9,
      OPTION_BRICKFILE=10,
      OPTION_BRICKSIZE=11,
      OPTION_BRICKTURN=12,
      OPTION_BRICKINCLINE=13,
      OPTION_BRICKCOLOR_RED=14,
      OPTION_BRICKCOLOR_GREEN=15,
      OPTION_BRICKCOLOR_BLUE=16,
      OPTION_BRICKALPHA=17,
      OPTION_DATAFILE=18,
      OPTION_DATASIZE=19,
      OPTION_DATATURN=20,
      OPTION_DATAINCLINE=21,
      OPTION_DATACOLOR_RED=22,
      OPTION_DATACOLOR_GREEN=23,
      OPTION_DATACOLOR_BLUE=24,
      OPTION_DATAALPHA=25,
      OPTION_DATARANGE=26
      };

   enum
      {
      OPTION_TYPE_NONE=-1, // invisible type
      OPTION_TYPE_ANY=-2, // non-specific type
      OPTION_TYPE_SIGNPOST=0, // signpost type
      OPTION_TYPE_BRICK1=1, // 1-pass brick type
      OPTION_TYPE_BRICK2=2, // 2-pass brick type
      OPTION_TYPE_BRICK3=3, // 3-pass brick type
      OPTION_TYPE_BRICK4=4, // 4-pass brick type
      OPTION_TYPE_UNUSED=5, // unused type
      };

   //! default constructor
   minipointopts();

   //! destructor
   ~minipointopts();

   int type; // type of the waypoint

   // signpost parameters
   float signpostsize; // 0=default size
   float signpostheight; // height of post
   BOOLINT signpostnoauto; // automatic orientation
   float signpostturn; // clockwise turning angle
   float signpostincline; // downward inclination angle
   float signpostalpha; // opacity

   // global signpost parameters
   float signpostrange; // display range

   // brick parameters
   char *brickfile; // data file
   float bricksize; // 0=default size
   float brickturn; // clockwise turning angle
   float brickincline; // downward inclination angle
   float brickcolor_red; // red color component
   float brickcolor_green; // green color component
   float brickcolor_blue; // blue color component
   float brickalpha; // opacity

   // global brick parameters
   float brickradius; // brick display radius
   float brickceiling; // elevation modulates brick color
   int bricklods; // number of brick LODs
   float brickstagger; // staggering of brick LODs
   float brickstripes; // offset of brick stripes

   // brick variables
   BOOLINT brickloaded;
   int brickindex;

   // generic parameters
   char *datafile;
   float datasize;
   float dataturn;
   float dataincline;
   float datacolor_red;
   float datacolor_green;
   float datacolor_blue;
   float dataalpha;
   float datarange;

   // generic variables
   BOOLINT dataloaded;
   int dataindex;

   // generic data
   void *data;
   };

//! waypoint renderer prototype
class minipointrndr
   {
   public:

   //! default constructor
   minipointrndr(int type=minipointopts::OPTION_TYPE_NONE,int passes=0)
      {TYPE=type; PASSES=passes;}

   //! destructor
   virtual ~minipointrndr() {}

   int gettype() {return(TYPE);}
   int getpasses() {return(PASSES);}

   virtual void init(minipoint *points,
                     float ex,float ey,float ez,
                     float dx,float dy,float dz,
                     float nearp,float farp,float fovy,float aspect,
                     double time,minipointopts *global)
      {
      if (points==NULL ||
          ex==MAXFLOAT || ey==MAXFLOAT || ez==MAXFLOAT ||
          dx==MAXFLOAT || dy==MAXFLOAT || dz==MAXFLOAT ||
          nearp<=0.0f || farp<=0.0f || fovy<=0.0f || aspect<=0.0f ||
          time<0.0 || global==NULL) ERRORMSG();
      }

   virtual void pre(int pass)
      {if (pass<0) ERRORMSG();}

   virtual void render(minipointdata *vpoint,int pass)
      {if (vpoint==NULL || pass<0) ERRORMSG();}

   virtual void post(int pass)
      {if (pass<0) ERRORMSG();}

   virtual void exit() {}

   protected:

   int TYPE;
   int PASSES;
   };

//! signpost renderer
class minipointrndr_signpost: public minipointrndr
   {
   public:

   //! default constructor
   minipointrndr_signpost():
      minipointrndr(minipointopts::OPTION_TYPE_SIGNPOST,2) {}

   //! destructor
   ~minipointrndr_signpost() {}

   void init(minipoint *points,
             float ex,float ey,float ez,
             float dx,float dy,float dz,
             float nearp,float farp,float fovy,float aspect,
             double time,minipointopts *global);

   void pre(int pass);
   void render(minipointdata *vpoint,int pass);
   void post(int pass);

   void exit();

   private:

   minipoint *POINTS;

   float EX,EY,EZ;
   minipointopts *GLOBAL;
   float SCALEELEV;

   minipointdata *NEAREST;
   };

//! brick renderer
class minipointrndr_brick: public minipointrndr
   {
   public:

   //! default constructor
   minipointrndr_brick(int type);

   //! destructor
   ~minipointrndr_brick();

   void init(minipoint *points,
             float ex,float ey,float ez,
             float dx,float dy,float dz,
             float nearp,float farp,float fovy,float aspect,
             double time,minipointopts *global);

   void render(minipointdata *vpoint,int pass);

   void exit();

   private:

   minipoint *POINTS;

   float EX,EY,EZ;
   float NEARP,FARP,FOVY,ASPECT;
   minipointopts *GLOBAL;
   float OFFSETLAT,OFFSETLON;
   float SCALEX,SCALEY,SCALEELEV;

   minilod *LODS;
   };

//! waypoint class
class minipoint
   {
   public:

   //! default constructor
   minipoint(minitile *tile=NULL);

   //! destructor
   ~minipoint();

   //! add waypoint
   BOOLINT add(minipointdata *point);

   //! register renderer
   void registerrndr(minipointrndr *rndr);

   //! set file cache
   void setcache(datacache *cache,const char *altpath);

   //! load waypoints
   void load(const char *filename,
             float offsetlat=0.0f,float offsetlon=0.0f,
             float scalex=1.0f,float scaley=1.0f,float scaleelev=1.0f,
             minitile *tile=0,
             char delimiter=';');

   //! get number of waypoints
   int getnum() {return(PNUM);}

   //! get waypoint
   minipointdata *getpoint(int p);

   //! get first waypoint
   minipointdata *getfirst() {return(getpoint(0));}

   //! get last waypoint
   minipointdata *getlast() {return(getpoint(PNUM-1));}

   //! calculate visible waypoints
   void calcvdata();

   //! sort visible waypoints
   void sortvdata(float x,float y,float elev,
                  float dx,float dy,float de);

   //! get visible waypoints
   minipointdata **getvdata() {return(VPOINTS);}

   //! get number of visible waypoints
   int getvnum() {return(VNUM);}

   //! get nearest waypoint
   minipointdata *getnearest(float x,float y,float elev);

   //! get squared distance to waypoint
   float getdistance2(float x,float y,float elev,minipointdata *point);

   //! render waypoints
   void draw(float ex,float ey,float ez,
             float dx,float dy,float dz,
             float nearp,float farp,float fovy,float aspect,
             double time,minipointopts *global,
             minipointrndr *fallback=NULL);

   //! render waypoints with signposts
   void drawsignposts(float ex,float ey,float ez,
                      float height,float range,
                      float turn,float yon);

   //! set default brick file name
   void setbrick(const char *filename);

   //! get default brick file name
   char *getbrick();

   //! render waypoints with bricks
   void drawbricks(float ex,float ey,float ez,
                   float brad,float farp,
                   float fovy,float aspect,
                   float size);

   //! getters
   float getoffsetlat() {return(OFFSETLAT);}
   float getoffsetlon() {return(OFFSETLON);}
   float getscalex() {return(SCALEX);}
   float getscaley() {return(SCALEY);}
   float getscaleelev() {return(SCALEELEV);}
   int getzone() {return(CONFIGURE_DSTZONE);}
   int getdatum() {return(CONFIGURE_DSTDATUM);}

   //! get file through data cache
   char *getfile(const char *filename) {return(getfile(filename,ALTPATH));}

   static minipointrndr *getrndr_none() {return(&RNDR_NONE);}
   static minipointrndr *getrndr_signpost() {return(&RNDR_SIGNPOST);}
   static minipointrndr *getrndr_brick(int passes) {return(&RNDR_BRICK[passes]);}

   //! configuring
   void configure_srcdatum(int datum=3); // source UTM datum
   void configure_dstzone(int zone=0); // dest UTM zone (LL=0)
   void configure_dstdatum(int datum=3); // dest UTM datum
   void configure_automap(int automap=0); // take mapping from first waypoint

   //! configuring of signpost renderer
   void configure_signpostalpha(float signpostalpha=0.5f); // alpha value of signposts

   //! configuring of brick renderer
   void configure_brickalpha(float brickalpha=0.5f); // alpha value of bricks
   void configure_brickceiling(float brickceiling=0.0f); // elevation modulates brick color
   void configure_bricklods(int bricklods=16); // number of brick LODs
   void configure_brickstagger(float brickstagger=1.25f); // staggering of brick LODs
   void configure_brickpasses(float brickpasses=1); // brick render passes
   void configure_brickstripes(float brickstripes=0.0f); // offset of brick stripes

   protected:

   minitile *TILE;
   int COLS,ROWS;

   minipointdata **POINTS;
   int *NUM,*MAXNUM;
   int PNUM;

   minipointdata **VPOINTS;
   int VNUM,MAXVNUM;

   minipointrndr **RNDRS;
   int RNUM,MAXRNUM;

   BOOLINT LOOKUP;

   datacache *CACHE;
   char *ALTPATH;

   char *addch(char *str,char ch);

   void parsecomment(minipointdata *point);
   void parseoption(minipointdata *point,lunascan *scanner);

   char *getfile(const char *filename,const char *altpath);

   private:

   BOOLINT TAKEN,TRANS;

   float OFFSETLAT,OFFSETLON;
   float SCALEX,SCALEY,SCALEELEV;

   char *BRICKNAME;

   int CONFIGURE_SRCDATUM;
   int CONFIGURE_DSTZONE;
   int CONFIGURE_DSTDATUM;
   int CONFIGURE_AUTOMAP;

   float CONFIGURE_SIGNPOSTALPHA;

   float CONFIGURE_BRICKALPHA;
   float CONFIGURE_BRICKCEILING;
   int CONFIGURE_BRICKLODS;
   float CONFIGURE_BRICKSTAGGER;
   int  CONFIGURE_BRICKPASSES;
   float CONFIGURE_BRICKSTRIPES;

   static float SHELLSORT_X,SHELLSORT_Y,SHELLSORT_ELEV;
   static float SHELLSORT_DX,SHELLSORT_DY,SHELLSORT_DE;

   friend inline int operator < (const minipointdata &a,const minipointdata &b);

   static minipointrndr RNDR_NONE;
   static minipointrndr_signpost RNDR_SIGNPOST;
   static minipointrndr_brick RNDR_BRICK[4];

   void lookuprndrs();
   };

#endif
