// (c) by Stefan Roettger

#ifndef DATACLOUD_H
#define DATACLOUD_H

/* This class manages the uploading of tiled terrain data to the render core.
   It creates a receiver thread which caches the incoming data and schedules
   the upload to the core using a specified time slot per rendered frame. */

#include "minibase.h"

#include "database.h"
#include "miniload.h"

// a hash map for easy identification of existing items
class datahash
   {
   public:

   //! default constructor
   datahash(int size);

   //! destructor
   ~datahash();

   //! insert data item
   void insertdata(const unsigned char *str,const unsigned char *str2,const unsigned char *str3,void *elem);

   //! remove data item
   void removedata(const unsigned char *str,const unsigned char *str2,const unsigned char *str3);

   //! check for data item
   void *checkdata(const unsigned char *str,const unsigned char *str2,const unsigned char *str3) const;

   protected:

   struct datahash_struct
      {
      const unsigned char *str;
      const unsigned char *str2,*str3;

      void *elem;

      datahash_struct *next;
      };

   typedef datahash_struct datahash_type;
   typedef datahash_type *datahash_ptr;

   datahash_ptr *HASHMAP;
   int HASHSIZE,HASHNUM;

   private:

   inline unsigned int calcid(const unsigned char *str) const;
   inline BOOLINT calceq(const datahash_ptr ptr,const unsigned char *str,const unsigned char *str2,const unsigned char *str3) const;
   };

// one element of the tile cache
struct tilecacheelem
  {
  unsigned char *tileid; // tile id (usually the file name on disk)
  databuf *tile; // pointer to tile data

  int col,row; // actual tile column and row

  BOOLINT istexture; // is the tile a texture map

  BOOLINT loprio; // signal low tile priority

  int lod; // level of detail

  int refcount; // number of references

  BOOLINT isavailable; // is the tile already loaded and available
  BOOLINT isloading; // is the tile actually being loaded
  BOOLINT isdelayed; // is the tile being delayed

  int background; // associated background thread

  double access; // time of last access in seconds

  tilecacheelem *prev,*next; // pointer to prev/next tile in the cache list
  };

// one element of the job queue
struct jobqueueelem
  {
  int col,row; // column and row of tile

  tilecacheelem *hfield; // pointer to tile data
  int hlod; // actual LOD

  tilecacheelem *texture; // pointer to tile data
  int tlod; // actual LOD

  tilecacheelem *fogmap; // pointer to tile data

  BOOLINT loprio; // signal low job priority

  jobqueueelem *prev,*next; // pointer to prev/next element in the job queue
  };

// one element of the background array
struct backarrayelem
  {
  int background; // number of background thread starting at 1
  class datacloud *cloud; // reference to the calling datacloud object
  };

// a collection of terrain data tiles (a data cloud)
class datacloud
   {
   public:

   //! default constructor
   datacloud(miniload *terrain,int maxthreads=1);

   //! destructor
   ~datacloud();

   //! terrain getter
   miniload *getterrain() {return(TERRAIN);}

   //! set callbacks for requesting tiles
   void setloader(void (*request)(const unsigned char *mapfile,databuf *map,int istexture,int background,void *data),void *data,
                  int (*check)(const unsigned char *mapfile,int istexture,void *data),
                  int paging=0,
                  float pfarp=0.0f,
                  float prange=0.0f,int pbasesize=0,
                  int plazyness=0,int pupdate=0,
                  int expire=0);

   //! set optional callback for inquiry of height map elevation range
   void setinquiry(int (*inquiry)(int col,int row,const unsigned char *mapfile,int hlod,void *data,float *minvalue,float *maxvalue),void *data);

   //! set optional callback for query of texture map base size
   void setquery(void (*query)(int col,int row,const unsigned char *texfile,int tlod,void *data,int *tsizex,int *tsizey),void *data);

   //! set scheduling properties
   void setschedule(double timeslot=0.02,double expire=0.5,double maxdelay=1.0);

   //! set maximum allowed tile cache size in MB
   void setmaxsize(double maxsize=128.0);

   //! set optional background thread
   void setthread(void (*start)(void *(*thread)(void *background),backarrayelem *background,void *data)=0,void *data=0,
                  void (*join)(backarrayelem *background,void *data)=0,
                  void (*lock_cs)(void *data)=0,
                  void (*unlock_cs)(void *data)=0,
                  void (*lock_io)(void *data)=0,
                  void (*unlock_io)(void *data)=0);

   //! set number of multiple background threads
   void setmulti(int multi);

   //! signal background thread to finish
   void signalthread(int background=0);

   //! stop background thread
   void stopthread(int background=0);

   //! lock background thread
   void lockthread();

   //! unlock background thread
   void unlockthread();

   //! lock io operation
   void lockio();

   //! unlock io operation
   void unlockio();

   //! query total number of tiles in the cache
   int gettotal() {return(TILECOUNT);}

   //! query total number of pending tiles and jobs
   int getpending() {return(PENDINGTILES+JOBCOUNT);}

   //! query total amount of memory allocated in the cache (in MB)
   double getmem();

   //! configuring
   void configure_dontfree(int dontfree=0); // free memory chunks returned by callbacks
   void configure_autolockio(int autolockio=0); // automatically lock io operations
   void configure_automipmap(int automipmap=0); // automatically mip-map texture data
   void configure_autocompress(int autocompress=0); // automatically compress texture data with s3tc
   void configure_lod0uncompressed(int lod0uncompressed=0); // automatically keep LOD0 uncompressed
   void configure_keepalive(float keepalive=10.0f); // time for which idling threads are kept alive
   void configure_timeslice(float timeslice=0.001f); // time for which idling threads sleep

   protected:

   int myrequest(int col,int row,const unsigned char *mapfile,int hlod,const unsigned char *texfile,int tlod,const unsigned char *fogfile,databuf *hfield,databuf *texture,databuf *fogmap);
   void mypreload(int col,int row,const unsigned char *mapfile,int hlod,const unsigned char *texfile,int tlod,const unsigned char *fogfile);
   void mydeliver(int *col,int *row,databuf *hfield,int *hlod,databuf *texture,int *tlod,databuf *fogmap);

   BOOLINT checkfile(const unsigned char *mapfile,BOOLINT istexture);

   void insertjob(int col,int row,const unsigned char *mapfile,int hlod,const unsigned char *texfile,int tlod,const unsigned char *fogfile,BOOLINT immediate,BOOLINT loprio);
   jobqueueelem *checkjob(int col,int row,const unsigned char *mapfile,int hlod,const unsigned char *texfile,int tlod,const unsigned char *fogfile,BOOLINT immediate,BOOLINT loprio);
   void insertjob(jobqueueelem *job,jobqueueelem *newjob);
   void deletejob(jobqueueelem *job);

   tilecacheelem *inserttile(const unsigned char *tileid,int col,int row,BOOLINT istexture,BOOLINT immediate,BOOLINT loprio,int lod);
   tilecacheelem *checktile(const unsigned char *tileid,int col,int row,BOOLINT istexture,BOOLINT immediate,BOOLINT loprio,int lod);
   void inserttile(tilecacheelem *tile,tilecacheelem *newtile);
   void deletetile(tilecacheelem *tile);

   float getpriority(tilecacheelem *tile);
   void loadtile(tilecacheelem *tile,int background=0);
   BOOLINT loadpendingtile(int background);
   void deletedelayedtiles();
   void deleteexpiredtiles();
   void shrinkcache(double maxsize);

   void deletedelayedjobs();

   void deliverdata(databuf *hfield,databuf *texture,databuf *fogmap,BOOLINT immediate,int *col=NULL,int *row=NULL,int *hlod=NULL,int *tlod=NULL);

   void startthread();
   void pager(int background=0);

   private:

   miniload *TERRAIN;

   int MAXTHREADS;
   int NUMTHREADS;

   tilecacheelem *TILECACHE,*TILECACHETAIL;
   int TILECOUNT;

   datahash *TILECACHEMAP;

   int PENDINGTILES;

   jobqueueelem *JOBQUEUE,*JOBQUEUETAIL;
   int JOBCOUNT;

   datahash *JOBQUEUEMAP;

   void (*REQUEST_CALLBACK)(const unsigned char *mapfile,databuf *map,int istexture,int background,void *data);
   int (*CHECK_CALLBACK)(const unsigned char *mapfile,int istexture,void *data);
   void *REQUEST_DATA;

   double TIMESLOT; // seconds
   double EXPIRE; // minutes
   double MAXDELAY; // seconds

   double MAXSIZE; // MB

   BOOLINT DELIVERED;
   double DELIVERYSTART;

   void (*START_CALLBACK)(void *(*thread)(void *background),backarrayelem *background,void *data);
   void (*JOIN_CALLBACK)(backarrayelem *background,void *data);
   void (*LOCK_CALLBACK)(void *data);
   void (*UNLOCK_CALLBACK)(void *data);
   void (*LOCKIO_CALLBACK)(void *data);
   void (*UNLOCKIO_CALLBACK)(void *data);
   void *START_DATA;

   BOOLINT *ISRUNNING;
   BOOLINT *SHOULDSTOP;
   BOOLINT *ISREADY;

   backarrayelem *BACKARRAY;

   BOOLINT ISNOTREADY;

   int CONFIGURE_DONTFREE;
   int CONFIGURE_AUTOLOCKIO;
   int CONFIGURE_AUTOMIPMAP;
   int CONFIGURE_AUTOCOMPRESS;
   int CONFIGURE_LOD0UNCOMPRESSED;

   float CONFIGURE_KEEPALIVE;
   float CONFIGURE_TIMESLICE;

   static void *triggerpager(void *background);

   static int mystaticrequest(int col,int row,const unsigned char *mapfile,int hlod,const unsigned char *texfile,int tlod,const unsigned char *fogfile,void *data,databuf *hfield,databuf *texture,databuf *fogmap);
   static void mystaticpreload(int col,int row,const unsigned char *mapfile,int hlod,const unsigned char *texfile,int tlod,const unsigned char *fogfile,void *data);
   static void mystaticdeliver(int *col,int *row,databuf *hfield,int *hlod,databuf *texture,int *tlod,databuf *fogmap,void *data);
   };

#endif
