// (c) by Stefan Roettger

#include "minibase.h"

#include "minitime.h"

#include "datacloud.h"

// default constructor
datahash::datahash(int size)
   {
   HASHMAP=NULL;

   HASHSIZE=size;
   HASHNUM=0;
   }

// destructor
datahash::~datahash()
   {
   int i;

   datahash_ptr ptr,next;

   if (HASHMAP!=NULL)
      {
      for (i=0; i<HASHSIZE; i++)
         {
         ptr=HASHMAP[i];

         while (ptr!=NULL)
            {
            next=ptr->next;

            delete ptr;
            ptr=next;
            }
         }

      delete HASHMAP;
      }
   }

// insert data item
void datahash::insertdata(const unsigned char *str,const unsigned char *str2,const unsigned char *str3,void *elem)
   {
   int i;

   unsigned int id;

   datahash_ptr ptr,item;

   if (str==NULL) ERRORMSG();

   if (HASHMAP==NULL)
      {
      HASHMAP=new datahash_ptr[HASHSIZE];

      for (i=0; i<HASHSIZE; i++) HASHMAP[i]=NULL;
      }

   id=(calcid(str)+calcid(str2)+calcid(str3))%HASHSIZE;

   ptr=HASHMAP[id];

   while (ptr!=NULL)
      {
      if (calceq(ptr,str,str2,str3)) return;

      ptr=ptr->next;
      }

   item=new datahash_type;

   item->str=str;
   item->str2=str2;
   item->str3=str3;

   item->elem=elem;

   HASHNUM++;

   item->next=HASHMAP[id];
   HASHMAP[id]=item;
   }

// remove data item
void datahash::removedata(const unsigned char *str,const unsigned char *str2,const unsigned char *str3)
   {
   unsigned int id;

   datahash_ptr ptr,last;

   if (HASHMAP==NULL) return;

   id=(calcid(str)+calcid(str2)+calcid(str3))%HASHSIZE;

   ptr=HASHMAP[id];
   last=NULL;

   while (ptr!=NULL)
      {
      if (calceq(ptr,str,str2,str3))
         {
         if (last==NULL) HASHMAP[id]=ptr->next;
         else last->next=ptr->next;

         delete ptr;

         HASHNUM--;

         break;
         }

      last=ptr;
      ptr=ptr->next;
      }
   }

// check for data item
void *datahash::checkdata(const unsigned char *str,const unsigned char *str2,const unsigned char *str3) const
   {
   unsigned int id;

   datahash_ptr ptr;

   if (HASHMAP==NULL) return(NULL);

   id=(calcid(str)+calcid(str2)+calcid(str3))%HASHSIZE;

   ptr=HASHMAP[id];

   while (ptr!=NULL)
      {
      if (calceq(ptr,str,str2,str3)) return(ptr->elem);

      ptr=ptr->next;
      }

   return(NULL);
   }

// calculate injective item identifier
unsigned int datahash::calcid(const unsigned char *str) const
   {
   static const unsigned int hashconst=271;

   const unsigned char *ptr;

   unsigned int hash;

   if (str==NULL) return(0);

   ptr=str;

   hash=0;

   for (ptr=str; *ptr!='\0'; ptr++) hash=hashconst*(hash+*ptr)+hash/HASHSIZE;

   return(hash);
   }

// check for item equality
BOOLINT datahash::calceq(const datahash_ptr ptr,const unsigned char *str,const unsigned char *str2,const unsigned char *str3) const
   {
   if (strcmp((char *)ptr->str,(char *)str)!=0) return(FALSE);

   if (ptr->str2==NULL && str2==NULL) return(TRUE);
   if (ptr->str2==NULL || str2==NULL) return(FALSE);

   if (strcmp((char *)ptr->str2,(char *)str2)!=0) return(FALSE);

   if (ptr->str3==NULL && str3==NULL) return(TRUE);
   if (ptr->str3==NULL || str3==NULL) return(FALSE);

   if (strcmp((char *)ptr->str3,(char *)str3)!=0) return(FALSE);

   return(TRUE);
   }

// default constructor
datacloud::datacloud(miniload *terrain,int maxthreads)
   {
   int i;

   TERRAIN=terrain;

   if (maxthreads<1) maxthreads=1;

   MAXTHREADS=maxthreads;
   NUMTHREADS=0;

   TILECACHE=TILECACHETAIL=NULL;
   TILECOUNT=PENDINGTILES=0;

   TILECACHEMAP=new datahash(10000);

   JOBQUEUE=JOBQUEUETAIL=NULL;
   JOBCOUNT=0;

   JOBQUEUEMAP=new datahash(1000);

   REQUEST_CALLBACK=NULL;
   CHECK_CALLBACK=NULL;

   DELIVERED=FALSE;

   TIMESLOT=0.02; // 20ms
   EXPIRE=0.5; // 30sec
   MAXDELAY=1.0f; // 1s

   MAXSIZE=128.0; // 128MB

   START_CALLBACK=NULL;
   JOIN_CALLBACK=NULL;
   LOCK_CALLBACK=NULL;
   UNLOCK_CALLBACK=NULL;
   LOCKIO_CALLBACK=NULL;
   UNLOCKIO_CALLBACK=NULL;

   ISRUNNING=new BOOLINT[MAXTHREADS+1];
   SHOULDSTOP=new BOOLINT[MAXTHREADS+1];
   ISREADY=new BOOLINT[MAXTHREADS+1];

   BACKARRAY=new backarrayelem[MAXTHREADS+1];

   for (i=0; i<=MAXTHREADS; i++)
      {
      ISRUNNING[i]=FALSE;
      SHOULDSTOP[i]=FALSE;
      ISREADY[i]=TRUE;

      BACKARRAY[i].background=i;
      BACKARRAY[i].cloud=this;
      }

   ISNOTREADY=FALSE;

   CONFIGURE_DONTFREE=0;
   CONFIGURE_AUTOLOCKIO=0;
   CONFIGURE_AUTOMIPMAP=0;
   CONFIGURE_AUTOCOMPRESS=0;
   CONFIGURE_LOD0UNCOMPRESSED=0;

   CONFIGURE_KEEPALIVE=10.0f;
   CONFIGURE_TIMESLICE=0.001f;

   TERRAIN->configure_dontfree(1);
   }

// destructor
datacloud::~datacloud()
   {
   // stop background thread
   stopthread();

   delete[] ISRUNNING;
   delete[] SHOULDSTOP;
   delete[] ISREADY;

   delete[] BACKARRAY;

   tilecacheelem *tile=TILECACHE;

   // delete tile cache
   while (tile!=NULL)
      {
      tilecacheelem *next=tile->next;

      free(tile->tileid);

      if (CONFIGURE_DONTFREE==0) tile->tile->release();
      delete tile->tile;

      delete tile;
      tile=next;
      }

   // delete tile cache hash map
   delete TILECACHEMAP;

   jobqueueelem *job=JOBQUEUE;

   // delete job queue
   while (job!=NULL)
      {
      jobqueueelem *next=job->next;

      delete job;
      job=next;
      }

   // delete job queue hash map
   delete JOBQUEUEMAP;
   }

// set callbacks for requesting tiles
void datacloud::setloader(void (*request)(const unsigned char *mapfile,databuf *map,int istexture,int background,void *data),void *data,
                          int (*check)(const unsigned char *mapfile,int istexture,void *data),
                          int paging,
                          float pfarp,
                          float prange,int pbasesize,
                          int plazyness,int pupdate,
                          int expire)
   {
   if (request==NULL || check==NULL) ERRORMSG();

   // propagate callbacks to the wrapped miniload object
   TERRAIN->setloader(mystaticrequest,this,mystaticpreload,mystaticdeliver,
                      paging,pfarp,prange,pbasesize,plazyness,pupdate,expire);

   REQUEST_CALLBACK=request;
   CHECK_CALLBACK=check;
   REQUEST_DATA=data;
   }

// set optional callback for inquiry of height map elevation range
void datacloud::setinquiry(int (*inquiry)(int col,int row,const unsigned char *mapfile,int hlod,void *data,float *minvalue,float *maxvalue),void *data)
   {TERRAIN->setinquiry(inquiry,data);}

// set optional callback for query of texture map base size
void datacloud::setquery(void (*query)(int col,int row,const unsigned char *texfile,int tlod,void *data,int *tsizex,int *tsizey),void *data)
   {TERRAIN->setquery(query,data);}

// set scheduling properties
void datacloud::setschedule(double timeslot,double expire,double maxdelay)
   {
   if (timeslot<0.0 || expire<0.0 || MAXDELAY<0.0) ERRORMSG();

   TIMESLOT=timeslot;
   EXPIRE=expire;
   MAXDELAY=maxdelay;
   }

// set maximum allowed tile cache size
void datacloud::setmaxsize(double maxsize)
   {
   if (maxsize<0.0) ERRORMSG();
   MAXSIZE=maxsize;
   }

// set optional background thread
void datacloud::setthread(void (*start)(void *(*thread)(void *background),backarrayelem *background,void *data),void *data,
                          void (*join)(backarrayelem *background,void *data),
                          void (*lock_cs)(void *data),
                          void (*unlock_cs)(void *data),
                          void (*lock_io)(void *data),
                          void (*unlock_io)(void *data))
   {
   if (start!=NULL && (join==NULL || lock_cs==NULL || unlock_cs==NULL)) ERRORMSG();
   if (lock_io!=NULL && unlock_io==NULL) ERRORMSG();

   // stop background thread
   stopthread();

   START_CALLBACK=start;
   JOIN_CALLBACK=join;
   LOCK_CALLBACK=lock_cs;
   UNLOCK_CALLBACK=unlock_cs;
   LOCKIO_CALLBACK=lock_io;
   UNLOCKIO_CALLBACK=unlock_io;
   START_DATA=data;
   }

// set number of multiple background threads
void datacloud::setmulti(int multi)
   {
   int i;

   if (multi==MAXTHREADS) return;

   // stop background thread
   stopthread();

   delete[] ISRUNNING;
   delete[] SHOULDSTOP;
   delete[] ISREADY;

   delete[] BACKARRAY;

   if (multi<1) multi=1;

   MAXTHREADS=multi;

   ISRUNNING=new BOOLINT[MAXTHREADS+1];
   SHOULDSTOP=new BOOLINT[MAXTHREADS+1];
   ISREADY=new BOOLINT[MAXTHREADS+1];

   BACKARRAY=new backarrayelem[MAXTHREADS+1];

   for (i=0; i<=MAXTHREADS; i++)
      {
      ISRUNNING[i]=FALSE;
      SHOULDSTOP[i]=FALSE;
      ISREADY[i]=TRUE;

      BACKARRAY[i].background=i;
      BACKARRAY[i].cloud=this;
      }
   }

// signal background thread to finish
void datacloud::signalthread(int background)
   {
   int i;

   if (!ISNOTREADY) return;

   if (background==0)
      // signal all threads to finish
      for (i=1; i<=MAXTHREADS; i++) signalthread(i);
   else
      // signal thread if it is still running
      if (!ISREADY[background]) SHOULDSTOP[background]=TRUE;
   }

// stop background thread
void datacloud::stopthread(int background)
   {
   int i;

   if (!ISNOTREADY) return;

   if (background==0)
      // stop all threads
      {
      signalthread();
      for (i=1; i<=MAXTHREADS; i++) stopthread(i);
      }
   else
      // join thread if it is still running
      if (!ISREADY[background])
         {
         SHOULDSTOP[background]=TRUE;
         JOIN_CALLBACK(&BACKARRAY[background],START_DATA);
         SHOULDSTOP[background]=FALSE;
         ISREADY[background]=TRUE;
         NUMTHREADS--;
         }

   if (NUMTHREADS==0) ISNOTREADY=FALSE;
   }

// lock background thread
void datacloud::lockthread()
   {if (ISNOTREADY) LOCK_CALLBACK(START_DATA);}

// unlock background thread
void datacloud::unlockthread()
   {if (ISNOTREADY) UNLOCK_CALLBACK(START_DATA);}

// lock io operation
void datacloud::lockio()
   {
   if (LOCKIO_CALLBACK!=NULL)
      if (ISNOTREADY) LOCKIO_CALLBACK(START_DATA);
   }

// unlock io operation
void datacloud::unlockio()
   {
   if (LOCKIO_CALLBACK!=NULL)
      if (ISNOTREADY) UNLOCKIO_CALLBACK(START_DATA);
   }

// query total amount of memory allocated in the cache
double datacloud::getmem()
   {
   double mem=0.0;

   tilecacheelem *tile;

   // scan for tiles that are already available
   for (tile=TILECACHE; tile!=NULL; tile=tile->next)
      if (tile->isavailable) mem+=tile->tile->bytes;

   return(mem/(1024*1024));
   }

// own callbacks:

int datacloud::myrequest(int col,int row,const unsigned char *mapfile,int hlod,const unsigned char *texfile,int tlod,const unsigned char *fogfile,databuf *hfield,databuf *texture,databuf *fogmap)
   {
   BOOLINT immediate=FALSE;

   // catch file existence check
   if (hfield==NULL && texture==NULL && fogmap==NULL)
      {
      if (!checkfile(mapfile,FALSE)) return(0);
      if (!checkfile(texfile,TRUE)) return(0);
      if (!checkfile(fogfile,FALSE)) return(0);

      return(1);
      }

   // check for height fields that should be delivered immediately
   if (mapfile!=NULL)
      if (TERRAIN->getminitile()==NULL) immediate=TRUE;
      else if (!TERRAIN->getminitile()->isloaded(col,row)) immediate=TRUE;

   // check for texture maps that should be delivered immediately
   if (texfile!=NULL)
      if (TERRAIN->getminitile()==NULL) immediate=TRUE;
      else if (TERRAIN->getminitile()->gettexid(col,row)==0) immediate=TRUE;

   // check for request of texture map only
   if (mapfile==NULL && texfile!=NULL && fogfile==NULL) immediate=TRUE;

   // schedule job as immediate or deferred with hi prio
   insertjob(col,row,mapfile,hlod,texfile,tlod,fogfile,immediate,FALSE);

   // immediate data delivery
   if (immediate) deliverdata(hfield,texture,fogmap,TRUE);

   return(0);
   }

void datacloud::mypreload(int col,int row,const unsigned char *mapfile,int hlod,const unsigned char *texfile,int tlod,const unsigned char *fogfile)
   {
   // schedule job as deferred with lo prio
   insertjob(col,row,mapfile,hlod,texfile,tlod,fogfile,FALSE,TRUE);
   }

void datacloud::mydeliver(int *col,int *row,databuf *hfield,int *hlod,databuf *texture,int *tlod,databuf *fogmap)
   {
   // deferred data delivery
   deliverdata(hfield,texture,fogmap,FALSE,col,row,hlod,tlod);
   }

// file existence check
BOOLINT datacloud::checkfile(const unsigned char *mapfile,BOOLINT istexture)
   {
   BOOLINT check;

   if (mapfile==NULL) return(TRUE);

   check=(CHECK_CALLBACK(mapfile,istexture?1:0,REQUEST_DATA)!=0);

   return(check);
   }

// insert one job into the queue
void datacloud::insertjob(int col,int row,const unsigned char *mapfile,int hlod,const unsigned char *texfile,int tlod,const unsigned char *fogfile,BOOLINT immediate,BOOLINT loprio)
   {
   double time;

   jobqueueelem *oldjob,*newjob;

   // check for already existing job
   oldjob=checkjob(col,row,mapfile,hlod,texfile,tlod,fogfile,immediate,loprio);

   // reset access time
   if (oldjob!=NULL)
      {
      time=minigettime();

      if (mapfile!=NULL) oldjob->hfield->access=time;
      if (texfile!=NULL) oldjob->texture->access=time;
      if (fogfile!=NULL) oldjob->fogmap->access=time;

      return;
      }

   // create new job
   newjob=new jobqueueelem;

   newjob->col=col;
   newjob->row=row;

   // schedule height field
   if (mapfile!=NULL) newjob->hfield=inserttile(mapfile,col,row,FALSE,immediate,loprio,hlod);
   else newjob->hfield=NULL;
   newjob->hlod=hlod;

   // schedule texture map
   if (texfile!=NULL) newjob->texture=inserttile(texfile,col,row,TRUE,immediate,loprio,tlod);
   else newjob->texture=NULL;
   newjob->tlod=tlod;

   // schedule fog field
   if (fogfile!=NULL) newjob->fogmap=inserttile(fogfile,col,row,FALSE,immediate,loprio,0);
   else newjob->fogmap=NULL;

   newjob->loprio=loprio;

   // insert job into job queue
   if (immediate) insertjob(NULL,newjob); // insert immediate job at head
   else insertjob(JOBQUEUETAIL,newjob); // insert job at tail
   }

// check job for existence
jobqueueelem *datacloud::checkjob(int col,int row,const unsigned char *mapfile,int hlod,const unsigned char *texfile,int tlod,const unsigned char *fogfile,BOOLINT immediate,BOOLINT loprio)
   {
   jobqueueelem *job;

   // check hash map for already existing job
   job=(jobqueueelem *)JOBQUEUEMAP->checkdata(mapfile,texfile,fogfile);

   if (job==NULL) return(NULL);

   // check column/row
   if (col!=job->col || row!=job->row) return(NULL);

   // check mapfile:

   if (mapfile!=NULL && job->hfield==NULL) return(NULL);
   if (mapfile==NULL && job->hfield!=NULL) return(NULL);

   if (mapfile!=NULL)
      {
      if (hlod!=job->hlod) return(NULL);
      if (strcmp((char *)mapfile,(char *)(job->hfield->tileid))!=0) return(NULL);
      }

   // check texfile:

   if (texfile!=NULL && job->texture==NULL) return(NULL);
   if (texfile==NULL && job->texture!=NULL) return(NULL);

   if (texfile!=NULL)
      {
      if (tlod!=job->tlod) return(NULL);
      if (strcmp((char *)texfile,(char *)(job->texture->tileid))!=0) return(NULL);
      }

   // check fogfile:

   if (fogfile!=NULL && job->fogmap==NULL) return(NULL);
   if (fogfile==NULL && job->fogmap!=NULL) return(NULL);

   if (fogfile!=NULL)
      if (strcmp((char *)fogfile,(char *)(job->fogmap->tileid))!=0) return(NULL);

   // delete existing job if necessary
   if (immediate || (!loprio && job->loprio))
      {
      deletejob(job);
      return(NULL);
      }

   return(job);
   }

// insert a job into the queue after a given element
void datacloud::insertjob(jobqueueelem *job,jobqueueelem *newjob)
   {
   // insert job into hash map
   JOBQUEUEMAP->insertdata(newjob->hfield->tileid,
                           (newjob->texture==NULL)?NULL:newjob->texture->tileid,
                           (newjob->fogmap==NULL)?NULL:newjob->fogmap->tileid,
                           (void *)newjob);

   // insert in empty queue
   if (JOBQUEUE==NULL)
      {
      JOBQUEUE=JOBQUEUETAIL=newjob;
      newjob->prev=newjob->next=NULL;
      }
   // insert at queue head
   else if (job==NULL)
      {
      newjob->prev=NULL;
      newjob->next=JOBQUEUE;
      JOBQUEUE->prev=newjob;
      JOBQUEUE=newjob;
      }
   // insert at queue tail
   else if (job==JOBQUEUETAIL)
      {
      newjob->prev=JOBQUEUETAIL;
      newjob->next=NULL;
      JOBQUEUETAIL->next=newjob;
      JOBQUEUETAIL=newjob;
      }
   // insert after given element
   else
      {
      newjob->prev=job;
      newjob->next=job->next;
      job->next=newjob;
      newjob->next->prev=newjob;
      }

   // one job more
   JOBCOUNT++;
   }

// delete a job from the queue
void datacloud::deletejob(jobqueueelem *job)
   {
   // remove job from hash map
   JOBQUEUEMAP->removedata(job->hfield->tileid,
                           (job->texture==NULL)?NULL:job->texture->tileid,
                           (job->fogmap==NULL)?NULL:job->fogmap->tileid);

   // decrease refcounts
   if (job->hfield!=NULL) job->hfield->refcount--;
   if (job->texture!=NULL) job->texture->refcount--;
   if (job->fogmap!=NULL) job->fogmap->refcount--;

   // delete from head
   if (job==JOBQUEUE)
      {
      JOBQUEUE=job->next;
      if (JOBQUEUE!=NULL) JOBQUEUE->prev=NULL;
      else JOBQUEUETAIL=NULL;
      delete job;
      }
   // delete from tail
   else if (job==JOBQUEUETAIL)
      {
      JOBQUEUETAIL=job->prev;
      if (JOBQUEUETAIL!=NULL) JOBQUEUETAIL->next=NULL;
      else JOBQUEUE=NULL;
      delete job;
      }
   // delete from queue
   else
      {
      jobqueueelem *prev=job->prev;
      jobqueueelem *next=job->next;
      prev->next=next;
      next->prev=prev;
      delete job;
      }

   // one job less
   JOBCOUNT--;
   }

// insert a tile into the cache
tilecacheelem *datacloud::inserttile(const unsigned char *tileid,int col,int row,BOOLINT istexture,BOOLINT immediate,BOOLINT loprio,int lod)
   {
   tilecacheelem *oldtile,*newtile;

   // check for already existing tile
   oldtile=checktile(tileid,col,row,istexture,immediate,loprio,lod);

   // lock critical section
   if (ISNOTREADY) LOCK_CALLBACK(START_DATA);

   // already existing tile
   if (oldtile!=NULL)
      {
      oldtile->refcount++;
      newtile=oldtile;
      }
   // allocate and initialize new tile
   else
      {
      newtile=new tilecacheelem;

      newtile->tileid=(unsigned char *)strdup((char *)tileid);
      newtile->tile=new databuf;

      newtile->col=col;
      newtile->row=row;

      newtile->istexture=istexture;

      newtile->loprio=loprio;

      newtile->lod=lod;

      newtile->refcount=1;

      newtile->isavailable=FALSE;
      newtile->isloading=FALSE;

      newtile->background=0;
      }

   // load immediate data
   if (immediate && !newtile->isavailable)
      {
      // signal background thread to finish
      signalthread();

      // check if actual tile is already being loaded in the background
      if (newtile->isloading)
         {
         // unlock critical section
         if (ISNOTREADY) UNLOCK_CALLBACK(START_DATA);

         // wait for background thread to finish loading
         stopthread(newtile->background);

         // relock critical section
         if (ISNOTREADY) LOCK_CALLBACK(START_DATA);
         }

      // load actual tile directly
      if (!newtile->isavailable)
         {
         // load in foreground
         loadtile(newtile);

         // decrease pending tile count
         if (oldtile!=NULL) PENDINGTILES--;
         }
      }

   // reset access time
   newtile->access=minigettime();
   newtile->isdelayed=FALSE;

   // unlock critical section
   if (ISNOTREADY) UNLOCK_CALLBACK(START_DATA);

   // insert tile at cache tail
   if (oldtile==NULL) inserttile(TILECACHETAIL,newtile);

   return(newtile);
   }

// check tile for existence
tilecacheelem *datacloud::checktile(const unsigned char *tileid,int col,int row,BOOLINT istexture,BOOLINT immediate,BOOLINT loprio,int lod)
   {
   tilecacheelem *tile;

   // check hash map for already existing tile
   tile=(tilecacheelem *)TILECACHEMAP->checkdata(tileid,NULL,NULL);

   if (tile==NULL) return(NULL);

   // mismatch of tile spec
   if (col!=tile->col || row!=tile->row) return(NULL);
   if (istexture!=tile->istexture) return(NULL);
   if (lod!=tile->lod) return(NULL);

   // mismatch of tile identifier
   if (strcmp((char *)tileid,(char *)(tile->tileid))!=0) return(NULL);

   // upgrade lo prio to hi prio tile if necessary
   if (immediate || (!loprio && tile->loprio)) tile->loprio=FALSE;

   return(tile);
   }

// insert a tile into the cache after a given element
void datacloud::inserttile(tilecacheelem *tile,tilecacheelem *newtile)
   {
   // lock critical section
   if (ISNOTREADY) LOCK_CALLBACK(START_DATA);

   // insert tile into hash map
   TILECACHEMAP->insertdata(newtile->tileid,NULL,NULL,(void *)newtile);

   // increase pending tile count
   if (!newtile->isavailable) PENDINGTILES++;

   // insert in empty cache
   if (TILECACHE==NULL)
      {
      TILECACHE=TILECACHETAIL=newtile;
      newtile->prev=newtile->next=NULL;
      }
   // insert at cache head
   else if (tile==NULL)
      {
      newtile->prev=NULL;
      newtile->next=TILECACHE;
      TILECACHE->prev=newtile;
      TILECACHE=newtile;
      }
   // insert at cache tail
   else if (tile==TILECACHETAIL)
      {
      newtile->prev=TILECACHETAIL;
      newtile->next=NULL;
      TILECACHETAIL->next=newtile;
      TILECACHETAIL=newtile;
      }
   // insert after given element
   else
      {
      newtile->prev=tile;
      newtile->next=tile->next;
      tile->next=newtile;
      newtile->next->prev=newtile;
      }

   // one more tile
   TILECOUNT++;

   // unlock critical section
   if (ISNOTREADY) UNLOCK_CALLBACK(START_DATA);
   }

// delete a tile from the cache
void datacloud::deletetile(tilecacheelem *tile)
   {
   // lock critical section
   if (ISNOTREADY) LOCK_CALLBACK(START_DATA);

   // delete tile from hash map
   TILECACHEMAP->removedata(tile->tileid,NULL,NULL);

   // decrease pending tile count
   if (!tile->isavailable) PENDINGTILES--;

   // release tile identifier
   free(tile->tileid);

   // release memory chunk
   if (CONFIGURE_DONTFREE==0) tile->tile->release();

   // release databuf object
   delete tile->tile;

   // delete from head
   if (tile==TILECACHE)
      {
      TILECACHE=tile->next;
      if (TILECACHE!=NULL) TILECACHE->prev=NULL;
      else TILECACHETAIL=NULL;
      delete tile;
      }
   // delete from tail
   else if (tile==TILECACHETAIL)
      {
      TILECACHETAIL=tile->prev;
      if (TILECACHETAIL!=NULL) TILECACHETAIL->next=NULL;
      else TILECACHE=NULL;
      delete tile;
      }
   // delete from queue
   else
      {
      tilecacheelem *prev=tile->prev;
      tilecacheelem *next=tile->next;
      prev->next=next;
      next->prev=prev;
      delete tile;
      }

   // one tile less
   TILECOUNT--;

   // unlock critical section
   if (ISNOTREADY) UNLOCK_CALLBACK(START_DATA);
   }

// load a pending tile in the background
void datacloud::loadtile(tilecacheelem *tile,int background)
   {
   // signal loading
   tile->isloading=TRUE;
   tile->background=background;

   // unlock critical section
   if (ISNOTREADY) UNLOCK_CALLBACK(START_DATA);

   // lock io
   if (ISNOTREADY)
      if (CONFIGURE_AUTOLOCKIO!=0)
         if (LOCKIO_CALLBACK!=NULL) LOCKIO_CALLBACK(START_DATA);

   // load data
   REQUEST_CALLBACK(tile->tileid,tile->tile,tile->istexture,background,REQUEST_DATA);

   // auto mip-map textures in the background
   if (CONFIGURE_AUTOMIPMAP!=0)
      if (CONFIGURE_DONTFREE==0) tile->tile->automipmap();

   // autocompress textures in the background
   if (CONFIGURE_AUTOCOMPRESS!=0)
      if (CONFIGURE_DONTFREE==0)
         if (CONFIGURE_LOD0UNCOMPRESSED==0 || tile->lod>0) tile->tile->autocompress();

   // unlock io
   if (ISNOTREADY)
      if (CONFIGURE_AUTOLOCKIO!=0)
         if (LOCKIO_CALLBACK!=NULL) UNLOCKIO_CALLBACK(START_DATA);

   // relock critical section
   if (ISNOTREADY) LOCK_CALLBACK(START_DATA);

   // set access time
   tile->access=minigettime();

   // signal availability
   tile->isloading=FALSE;
   tile->isavailable=TRUE;
   }

// check for pending tiles in the background
BOOLINT datacloud::loadpendingtile(int background)
   {
   float prio,p;

   tilecacheelem *tile,*scan;

   double time;

   // lock critical section
   if (ISNOTREADY) LOCK_CALLBACK(START_DATA);

   time=minigettime();

   prio=0.0f;

   tile=NULL;
   scan=TILECACHETAIL;

   // scan tiles
   while (scan!=NULL)
      {
      if (MAXDELAY>0.0)
         if (!scan->isavailable && !scan->isloading && !scan->isdelayed)
            if (time-scan->access>MAXDELAY) scan->isdelayed=TRUE;

      if (!scan->isavailable && !scan->isloading && !scan->isdelayed)
         {
         p=getpriority(scan);

         if (p>=prio)
            {
            prio=p;
            tile=scan;
            }
         }

      scan=scan->prev;
      }

   // trigger loading
   if (tile!=NULL)
      {
      // load in background
      loadtile(tile,background);

      // decrease pending tile count
      PENDINGTILES--;
      }

   // unlock critical section
   if (ISNOTREADY) UNLOCK_CALLBACK(START_DATA);

   if (tile!=NULL) return(TRUE);
   else return(FALSE);
   }

// delete delayed tiles
void datacloud::deletedelayedtiles()
   {
   tilecacheelem *tile,*next;

   if (MAXDELAY==0.0) return;

   tile=TILECACHE;

   // check all tiles
   while (tile!=NULL)
      {
      next=tile->next;

      if (tile->isdelayed && tile->refcount==0) deletetile(tile);

      tile=next;
      }
   }

// delete expired tiles
void datacloud::deleteexpiredtiles()
   {
   double time;

   tilecacheelem *tile,*next;

   if (EXPIRE==0.0) return;

   time=minigettime();

   tile=TILECACHE;

   // check all tiles
   while (tile!=NULL)
      {
      next=tile->next;

      if (tile->isavailable && tile->refcount==0)
         if (time-tile->access>60*EXPIRE) deletetile(tile);

      tile=next;
      }
   }

// shrink cache to maximum allowed size
void datacloud::shrinkcache(double maxsize)
   {
   double totalsize;

   tilecacheelem *tile,*next;

   if (maxsize==0.0) return;

   totalsize=0.0;

   tile=TILECACHE;

   // sum up all tiles
   while (tile!=NULL)
      {
      if (tile->isavailable) totalsize+=tile->tile->bytes;

      tile=tile->next;
      }

   tile=TILECACHE;

   // delete old tiles
   while (tile!=NULL && totalsize>maxsize)
      {
      next=tile->next;

      if (tile->isavailable && tile->refcount==0)
         {
         totalsize-=tile->tile->bytes;
         deletetile(tile);
         }

      tile=next;
      }
   }

// calculate the tile priority
float datacloud::getpriority(tilecacheelem *tile)
   {
   float prio;

   int cols=TERRAIN->getminitile()->getcols();
   int rows=TERRAIN->getminitile()->getrows();

   float col0=TERRAIN->getminitile()->getcenteri();
   float row0=TERRAIN->getminitile()->getcenterj();

   prio=(fsqr(tile->col+0.5f-col0)+fsqr(tile->row+0.5f-row0))/(cols*cols+rows*rows);

   if (prio>1.0f) prio=1.0f;
   if (tile->loprio) prio+=1.0f;

   return(1.0f-prio/2.0f);
   }

// delete delayed jobs
void datacloud::deletedelayedjobs()
   {
   jobqueueelem *job,*prev;

   BOOLINT flag;

   job=JOBQUEUETAIL;

   while (job!=NULL)
      {
      flag=FALSE;

      if (job->hfield!=NULL)
         if (job->hfield->isdelayed) flag=TRUE;

      if (job->texture!=NULL)
         if (job->texture->isdelayed) flag=TRUE;

      if (job->fogmap!=NULL)
         if (job->fogmap->isdelayed) flag=TRUE;

      prev=job->prev;

      if (flag) deletejob(job);

      job=prev;
      }
   }

// deliver data that is already available
void datacloud::deliverdata(databuf *hfield,databuf *texture,databuf *fogmap,BOOLINT immediate,int *col,int *row,int *hlod,int *tlod)
   {
   float prio,p;

   jobqueueelem *job,*scan;

   BOOLINT isavailable;

   // start periodical tasks
   if (!immediate)
      {
      if (!DELIVERED)
         {
         // delete delayed jobs
         deletedelayedjobs();

         // delete delayed tiles
         deletedelayedtiles();

         // delete expired tiles
         deleteexpiredtiles();

         // shrink cache to maximum allowed size
         shrinkcache(MAXSIZE*1024*1024);
         }

      // load pending tiles in the background
      startthread();
      }

   // check deferred data delivery
   if (!immediate)
      if (DELIVERED)
         // stop deferred data delivery if time is up
         if (minigettime()-DELIVERYSTART>TIMESLOT)
            {
            DELIVERED=FALSE;
            return;
            }

   // scan jobs
   if (!immediate)
      {
      prio=0.0f;

      job=NULL;
      scan=JOBQUEUETAIL;

      while (scan!=NULL)
         {
         p=0.0f;

         isavailable=TRUE;

         // check the availability of all required maps:

         if (scan->hfield!=NULL)
            if (!scan->hfield->isavailable) isavailable=FALSE;
            else p=getpriority(scan->hfield);

         if (scan->texture!=NULL)
            if (!scan->texture->isavailable) isavailable=FALSE;

         if (scan->fogmap!=NULL)
            if (!scan->fogmap->isavailable) isavailable=FALSE;

         // check the priority of the job
         if (isavailable && p>=prio)
            {
            prio=p;
            job=scan;
            }

         scan=scan->prev;
         }
      }
   else job=JOBQUEUE;

   // deferred data delivery
   if (job!=NULL)
      {
      // return maps
      if (job->hfield!=NULL) *hfield=*(job->hfield->tile);
      if (job->texture!=NULL) *texture=*(job->texture->tile);
      if (job->fogmap!=NULL) *fogmap=*(job->fogmap->tile);

      // return tile position
      if (col!=NULL) *col=job->col;
      if (row!=NULL) *row=job->row;

      // return level of detail
      if (hlod!=NULL) *hlod=job->hlod;
      if (tlod!=NULL) *tlod=job->tlod;

      // check for immediate delivery
      if (immediate) DELIVERED=FALSE;
      // remember starting time of deferred delivery
      else if (!DELIVERED)
         {
         DELIVERED=TRUE;
         DELIVERYSTART=minigettime();
         }

      // remove finished job from queue
      deletejob(job);
      }
   else DELIVERED=FALSE;
   }

// start background thread
void datacloud::startthread()
   {
   int i;

   // check pending tile count
   if (PENDINGTILES==0) return;

   // non-threaded paging
   if (START_CALLBACK==NULL) pager();
   // threaded paging
   else
      for (i=1; i<=MAXTHREADS; i++)
         if (!ISRUNNING[i])
            {
            // wait for background thread to finish
            stopthread(i);

            // start new background thread
            ISRUNNING[i]=TRUE;
            ISREADY[i]=FALSE;
            ISNOTREADY=TRUE;
            START_CALLBACK(triggerpager,&BACKARRAY[i],START_DATA);
            NUMTHREADS++;
            }
   }

// jump to background thread
void *datacloud::triggerpager(void *background)
   {
   backarrayelem *backarray=(backarrayelem *)background;
   backarray->cloud->pager(backarray->background);

   return(NULL);
   }

// background thread
void datacloud::pager(int background)
   {
   double time;

   // load all pending tiles
   while (!SHOULDSTOP[background])
      if (!loadpendingtile(background)) break;

   // cycle until time runs out
   if (START_CALLBACK!=NULL)
      {
      time=minigettime();
      while (!SHOULDSTOP[background] && minigettime()-time<CONFIGURE_KEEPALIVE)
         if (!loadpendingtile(background)) miniwaitfor(CONFIGURE_TIMESLICE); // time runs out
         else time=minigettime(); // keep thread alive
      }

   ISRUNNING[background]=FALSE;
   }

// static callback wrappers:

int datacloud::mystaticrequest(int col,int row,const unsigned char *mapfile,int hlod,const unsigned char *texfile,int tlod,const unsigned char *fogfile,void *data,databuf *hfield,databuf *texture,databuf *fogmap)
   {
   datacloud *mycloud=(datacloud *)data;
   return(mycloud->myrequest(col,row,mapfile,hlod,texfile,tlod,fogfile,hfield,texture,fogmap));
   }

void datacloud::mystaticpreload(int col,int row,const unsigned char *mapfile,int hlod,const unsigned char *texfile,int tlod,const unsigned char *fogfile,void *data)
   {
   datacloud *mycloud=(datacloud *)data;
   mycloud->mypreload(col,row,mapfile,hlod,texfile,tlod,fogfile);
   }

void datacloud::mystaticdeliver(int *col,int *row,databuf *hfield,int *hlod,databuf *texture,int *tlod,databuf *fogmap,void *data)
   {
   datacloud *mycloud=(datacloud *)data;
   mycloud->mydeliver(col,row,hfield,hlod,texture,tlod,fogmap);
   }

// configuring:

void datacloud::configure_dontfree(int dontfree)
   {CONFIGURE_DONTFREE=dontfree;}

void datacloud::configure_autolockio(int autolockio)
   {CONFIGURE_AUTOLOCKIO=autolockio;}

void datacloud::configure_automipmap(int automipmap)
   {CONFIGURE_AUTOMIPMAP=automipmap;}

void datacloud::configure_autocompress(int autocompress)
   {CONFIGURE_AUTOCOMPRESS=autocompress;}

void datacloud::configure_lod0uncompressed(int lod0uncompressed)
   {CONFIGURE_LOD0UNCOMPRESSED=lod0uncompressed;}

void datacloud::configure_keepalive(float keepalive)
   {CONFIGURE_KEEPALIVE=keepalive;}

void datacloud::configure_timeslice(float timeslice)
   {CONFIGURE_TIMESLICE=timeslice;}
