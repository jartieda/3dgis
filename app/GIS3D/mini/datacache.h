// (c) by Stefan Roettger

#ifndef DATACACHE_H
#define DATACACHE_H

/* This class caches data that is arriving from a server and stores the
   downloaded data persistently on disk. The data transfer is delegated
   to an external library like libcurl. */

#include "minibase.h"

#include "database.h"
#include "miniload.h"
#include "datacloud.h"

// one element of file information
struct fileinfoelem
   {
   char *filename;
   unsigned int hash;

   BOOLINT isavailable,islocal;

   BOOLINT istile,istexture;
   float minvalue,maxvalue;
   int width,height;

   fileinfoelem *next;
   };

// a cache for terrain data tile sets and related information
class datacache
   {
   public:

   //! default constructor
   datacache(miniload *terrain);

   //! destructor
   ~datacache();

   //! cloud getter
   datacloud *getcloud() {return(CLOUD);}

   //! set callback for requesting tiles
   void setloader(void (*request)(const char *file,int istexture,databuf *buf,void *data),void *data,
                  int paging=0,
                  float pfarp=0.0f,
                  float prange=0.0f,int pbasesize=0,
                  int plazyness=0,int pupdate=0,
                  int expire=0);

   //! set optional callback for requesting tiles from a server
   void setreceiver(void (*receive)(const char *src_url,const char *src_id,const char *src_file,const char *dst_file,int background,void *data),void *data,
                    int (*check)(const char *src_url,const char *src_id,const char *src_file,void *data));

   //! set remote url
   void setremoteurl(const char *url);

   //! set remote world id
   void setremoteid(const char *id);

   //! set local path to file cache
   void setlocalpath(const char *path);

   //! set size of hash table
   void sethashsize(int size);

   //! get file
   char *getfile(const char *src_file,const char *altpath=NULL);

   //! set name of elev tileset file
   void setelevtilesetfile(const char *filename);

   //! set name of imag tileset file
   void setimagtilesetfile(const char *filename);

   //! set name of vtb ini file for elevation tileset
   void setvtbelevinifile(const char *filename);

   //! set name of vtb ini file for imagery tileset
   void setvtbimaginifile(const char *filename);

   //! set path to vtb elevation tileset
   void setvtbelevpath(const char *elev);

   //! set path to vtb imagery tileset
   void setvtbimagpath(const char *imag);

   //! set name of startup file
   void setstartupfile(const char *filename);

   //! load persistent startup file
   void load(BOOLINT reset=FALSE);

   //! save persistent startup file
   void save();

   //! getters for elev tileset info
   BOOLINT haselevinfo() {return(HAS_ELEVINFO);}
   int getelevinfo_tilesx() {return(ELEVINFO_TILESX);}
   int getelevinfo_tilesy() {return(ELEVINFO_TILESY);}
   float getelevinfo_centerx() {return(ELEVINFO_CENTERX);}
   float getelevinfo_centery() {return(ELEVINFO_CENTERY);}
   float getelevinfo_sizex() {return(ELEVINFO_SIZEX);}
   float getelevinfo_sizey() {return(ELEVINFO_SIZEY);}
   int getelevinfo_maxdemsize() {return(ELEVINFO_MAXDEMSIZE);}
   float getelevinfo_maxelev() {return(ELEVINFO_MAXELEV);}

   //! getters for imag tileset info
   BOOLINT hasimaginfo() {return(HAS_IMAGINFO);}
   int getimaginfo_tilesx() {return(IMAGINFO_TILESX);}
   int getimaginfo_tilesy() {return(IMAGINFO_TILESY);}
   float getimaginfo_centerx() {return(IMAGINFO_CENTERX);}
   float getimaginfo_centery() {return(IMAGINFO_CENTERY);}
   float getimaginfo_sizex() {return(IMAGINFO_SIZEX);}
   float getimaginfo_sizey() {return(IMAGINFO_SIZEY);}
   int getimaginfo_maxtexsize() {return(IMAGINFO_MAXTEXSIZE);}

   //! getters for vtb elev ini file
   BOOLINT haselevini() {return(HAS_ELEVINI);}
   BOOLINT haselevini_geo() {return(HAS_ELEVINI_GEO);}
   BOOLINT haselevini_coordsys() {return(HAS_ELEVINI_COORDSYS);}
   int getelevini_tilesx() {return(ELEVINI_TILESX);}
   int getelevini_tilesy() {return(ELEVINI_TILESY);}
   float getelevini_centerx() {return(ELEVINI_CENTERX);}
   float getelevini_centery() {return(ELEVINI_CENTERY);}
   float getelevini_centerx_llwgs84() {return(ELEVINI_CENTERX_LLWGS84);}
   float getelevini_centery_llwgs84() {return(ELEVINI_CENTERY_LLWGS84);}
   float getelevini_northx_llwgs84() {return(ELEVINI_NORTHX_LLWGS84);}
   float getelevini_northy_llwgs84() {return(ELEVINI_NORTHY_LLWGS84);}
   float getelevini_sizex() {return(ELEVINI_SIZEX);}
   float getelevini_sizey() {return(ELEVINI_SIZEY);}
   int getelevini_maxdemsize() {return(ELEVINI_MAXDEMSIZE);}
   float getelevini_minelev() {return(ELEVINI_MINELEV);}
   float getelevini_maxelev() {return(ELEVINI_MAXELEV);}
   int getelevini_coordsys_ll() {return(ELEVINI_COORDSYS_LL);}
   int getelevini_coordsys_lldatum() {return(ELEVINI_COORDSYS_LLDATUM);}
   int getelevini_coordsys_utmzone() {return(ELEVINI_COORDSYS_UTMZONE);}
   int getelevini_coordsys_utmdatum() {return(ELEVINI_COORDSYS_UTMDATUM);}

   //! getters for vtb imag ini file
   BOOLINT hasimagini() {return(HAS_IMAGINI);}
   BOOLINT hasimagini_geo() {return(HAS_IMAGINI_GEO);}
   BOOLINT hasimagini_coordsys() {return(HAS_IMAGINI_COORDSYS);}
   int getimagini_tilesx() {return(IMAGINI_TILESX);}
   int getimagini_tilesy() {return(IMAGINI_TILESY);}
   float getimagini_centerx() {return(IMAGINI_CENTERX);}
   float getimagini_centery() {return(IMAGINI_CENTERY);}
   float getimagini_centerx_llwgs84() {return(IMAGINI_CENTERX_LLWGS84);}
   float getimagini_centery_llwgs84() {return(IMAGINI_CENTERY_LLWGS84);}
   float getimagini_northx_llwgs84() {return(IMAGINI_NORTHX_LLWGS84);}
   float getimagini_northy_llwgs84() {return(IMAGINI_NORTHY_LLWGS84);}
   float getimagini_sizex() {return(IMAGINI_SIZEX);}
   float getimagini_sizey() {return(IMAGINI_SIZEY);}
   int getimagini_maxtexsize() {return(IMAGINI_MAXTEXSIZE);}
   int getimagini_coordsys_ll() {return(IMAGINI_COORDSYS_LL);}
   int getimagini_coordsys_lldatum() {return(IMAGINI_COORDSYS_LLDATUM);}
   int getimagini_coordsys_utmzone() {return(IMAGINI_COORDSYS_UTMZONE);}
   int getimagini_coordsys_utmdatum() {return(IMAGINI_COORDSYS_UTMDATUM);}

   //! query total number of tiles in the cache
   int gettotal() {return(CLOUD->gettotal());}

   //! query total number of pending tiles and jobs
   int getpending() {return(CLOUD->getpending());}

   //! query total amount of memory allocated in the cache (in MB)
   double getmem() {return(CLOUD->getmem());}

   //! configuring
   void configure_dontfree(int dontfree=0); // free memory chunks returned by callbacks
   void configure_locthreads(int locthreads=1); // number of background threads for local access
   void configure_netthreads(int netthreads=10); // number of background threads for network access
   void configure_autolockio(int autolockio=0); // automatically lock io operations

   protected:

   void reset();

   void loadelevtilesetinfo();
   void loadimagtilesetinfo();

   void loadvtbelevini();
   void loadvtbimagini();

   void myrequest(const unsigned char *mapfile,databuf *map,int istexture,int background);
   int mycheck(const unsigned char *mapfile,int istexture);
   int myinquiry(int col,int row,const unsigned char *mapfile,int hlod,float *minvalue,float *maxvalue);
   void myquery(int col,int row,const unsigned char *texfile,int tlod,int *tsizex,int *tsizey);

   char *sourcefilename(const char *id,const char *filename);
   char *localfilename(const char *filename);
   BOOLINT localfilecheck(const char *filename);

   void insertfilename(const char *filename,BOOLINT isavailable,BOOLINT islocal,BOOLINT istile,BOOLINT istexture,float minvalue=1.0f,float maxvalue=0.0f,int width=0,int height=0,BOOLINT tailinsert=FALSE,BOOLINT dontupdate=FALSE);
   fileinfoelem *checkfilename(const char *filename);

   inline unsigned int hashsum(const char *str) const;

   private:

   datacloud *CLOUD;

   void (*REQUEST_CALLBACK)(const char *file,int istexture,databuf *buf,void *data);
   void *REQUEST_DATA;

   void (*RECEIVE_CALLBACK)(const char *src_url,const char *src_id,const char *src_file,const char *dst_file,int background,void *data);
   int (*CHECK_CALLBACK)(const char *src_url,const char *src_id,const char *src_file,void *data);
   void *RECEIVE_DATA;

   char *RURL,*RID;
   char *LPATH;

   BOOLINT LOCAL;

   fileinfoelem **HASHTABLE;
   int HASHSIZE;

   char *ELEVTILESETFILE;
   char *IMAGTILESETFILE;
   char *VTBELEVINIFILE;
   char *VTBIMAGINIFILE;
   char *VTBELEVPATH;
   char *VTBIMAGPATH;
   char *STARTUPFILE;

   BOOLINT HAS_ELEVINFO;
   int ELEVINFO_TILESX;
   int ELEVINFO_TILESY;
   float ELEVINFO_CENTERX;
   float ELEVINFO_CENTERY;
   float ELEVINFO_SIZEX;
   float ELEVINFO_SIZEY;
   int ELEVINFO_MAXDEMSIZE;
   float ELEVINFO_MAXELEV;

   BOOLINT HAS_IMAGINFO;
   int IMAGINFO_TILESX;
   int IMAGINFO_TILESY;
   float IMAGINFO_CENTERX;
   float IMAGINFO_CENTERY;
   float IMAGINFO_SIZEX;
   float IMAGINFO_SIZEY;
   int IMAGINFO_MAXTEXSIZE;
   float IMAGINFO_MAXELEV;

   BOOLINT HAS_ELEVINI;
   BOOLINT HAS_ELEVINI_GEO;
   BOOLINT HAS_ELEVINI_COORDSYS;
   int ELEVINI_TILESX;
   int ELEVINI_TILESY;
   float ELEVINI_CENTERX;
   float ELEVINI_CENTERY;
   float ELEVINI_CENTERX_LLWGS84;
   float ELEVINI_CENTERY_LLWGS84;
   float ELEVINI_NORTHX_LLWGS84;
   float ELEVINI_NORTHY_LLWGS84;
   float ELEVINI_SIZEX;
   float ELEVINI_SIZEY;
   int ELEVINI_MAXDEMSIZE;
   float ELEVINI_MINELEV;
   float ELEVINI_MAXELEV;
   int ELEVINI_COORDSYS_LL;
   int ELEVINI_COORDSYS_LLDATUM;
   int ELEVINI_COORDSYS_UTMZONE;
   int ELEVINI_COORDSYS_UTMDATUM;

   BOOLINT HAS_IMAGINI;
   BOOLINT HAS_IMAGINI_GEO;
   BOOLINT HAS_IMAGINI_COORDSYS;
   int IMAGINI_TILESX;
   int IMAGINI_TILESY;
   float IMAGINI_CENTERX;
   float IMAGINI_CENTERY;
   float IMAGINI_CENTERX_LLWGS84;
   float IMAGINI_CENTERY_LLWGS84;
   float IMAGINI_NORTHX_LLWGS84;
   float IMAGINI_NORTHY_LLWGS84;
   float IMAGINI_SIZEX;
   float IMAGINI_SIZEY;
   int IMAGINI_MAXTEXSIZE;
   int IMAGINI_COORDSYS_LL;
   int IMAGINI_COORDSYS_LLDATUM;
   int IMAGINI_COORDSYS_UTMZONE;
   int IMAGINI_COORDSYS_UTMDATUM;

   int CONFIGURE_DONTFREE;
   int CONFIGURE_LOCTHREADS;
   int CONFIGURE_NETTHREADS;
   int CONFIGURE_AUTOLOCKIO;

   static void mystaticrequest(const unsigned char *mapfile,databuf *map,int istexture,int background,void *data);
   static int mystaticcheck(const unsigned char *mapfile,int istexture,void *data);
   static int mystaticinquiry(int col,int row,const unsigned char *mapfile,int hlod,void *data,float *minvalue,float *maxvalue);
   static void mystaticquery(int col,int row,const unsigned char *texfile,int tlod,void *data,int *tsizex,int *tsizey);
   };

#endif
