// (c) by Stefan Roettger

#ifndef MINILOAD_H
#define MINILOAD_H

#include "database.h"
#include "minitile.h"

class miniload
   {
   public:

   //! default constructor
   miniload();

   //! constructor
   //! loads a regularly tiled terrain
   //! each tile is assumed to have a corner centric data representation
   miniload(unsigned char **hfields, // 2D array of filenames for height fields
            unsigned char **textures, // 2D array of filenames for texture maps
            int cols,int rows, // number of tile columns and rows
            float coldim,float rowdim, // dimension of a column or a row
            float scale, // vertical scaling of elevations
            float centerx=0.0f,float centery=0.0f,float centerz=0.0f, // map center
            unsigned char **fogmaps=0, // optional 2D array of filenames for ground fog maps
            float lambda=0.0f, // vertical scaling of fog elevations
            float displace=0.0f, // vertical fog displacement above ground
            float emission=0.0f, // fog appearance control
            float attenuation=1.0f, // relative importance of fog
            float fogR=1.0f,float fogG=1.0f,float fogB=1.0f); // fog color

   //! destructor
   ~miniload();

   //! draw scene
   void draw(float res, // resolution
             float ex,float ey,float ez, // eye point
             float dx,float dy,float dz, // view direction
             float ux,float uy,float uz, // up vector
             float fovy,float aspect, // field of view and aspect
             float nearp,float farp, // near and far plane
             int update=0); // optional incremental update

   //! set focus of interest (equal to eye point by default)
   void setfocus(int on=0,float fx=0.0f,float fy=0.0f,float fz=0.0f);

   //! set relative scaling (0<=scale<=1)
   void setrelscale(float scale=1.0f);

   //! get relative scaling
   float getrelscale();

   //! set sea level
   void setsealevel(float level);

   //! get sea level
   float getsealevel();

   //! set rendering modes
   void settexmode(int texmode=0);
   void setfogmode(int fogmode=1);

   //! get-functions for geometric properties
   float getheight(float x,float z,int approx=0);
   float getfogheight(float x,float z,int approx=0);
   void getnormal(float x,float z,float *nx,float *ny,float *nz);
   int getdim(float x,float z,float *dimx,float *dimz,float *radius2=0);

   //! get-functions for fog properties
   float getlambda();
   float getdisplace();

   //! reload a specific tile
   void reload(int col,int row, // tile to reload
               const unsigned char *hmap, // filename of height map
               const unsigned char *tmap, // filename of texture map
               const unsigned char *fmap=0); // optional filename of ground fog map

   //! loader for paged tile sets
   int load(int cols,int rows,
            const char *basepath1=0,const char *basepath2=0,const char *basepath3=0,
            float offsetx=0.0f,float offsety=0.0f,float offseth=0.0f,
            float stretch=1.0f,float stretchx=1.0f,float stretchy=1.0f,
            float exaggeration=1.0f,float scale=1.0f,
            float lambda=1.0f,float attenuation=1.0f,
            float minres=0.0f,float minoff=0.0f,
            float outparams[5]=0,
            float outscale[3]=0);

   //! set callbacks for requesting and preloading tiles
   void setloader(int (*request)(int col,int row,const unsigned char *mapfile,int hlod,const unsigned char *texfile,int tlod,const unsigned char *fogfile,void *data,databuf *hfield,databuf *texture,databuf *fogmap),void *data,
                  void (*preload)(int col,int row,const unsigned char *mapfile,int hlod,const unsigned char *texfile,int tlod,const unsigned char *fogfile,void *data)=0,
                  void (*deliver)(int *col,int *row,databuf *hfield,int *hlod,databuf *texture,int *tlod,databuf *fogmap,void *data)=0,
                  int paging=0,
                  float pfarp=0.0f,
                  float prange=0.0f,int pbasesize=0,
                  int plazyness=0,int pupdate=0,
                  int expire=0);

   //! set callback for inquiry of height map elevation range
   void setinquiry(int (*inquiry)(int col,int row,const unsigned char *mapfile,int hlod,void *data,float *minvalue,float *maxvalue),void *data);

   //! set callback for query of texture map base size
   void setquery(void (*query)(int col,int row,const unsigned char *texfile,int tlod,void *data,int *tsizex,int *tsizey),void *data);

   //! compute texture paging range from screen space error
   float calcrange(float dim, // size of one texel in meters
                   int height, // height of the view port in pixels
                   float fovy, // vertical field of view in degrees
                   float thres=1.5f); // screen space error in pixels

   //! set preloading
   void setpreload(float pfarp,int pupdate);

   //! set expiration
   void setexpire(int expire);

   //! set fast initialization
   void setfastinit(int fast=1,float avgd2=0.1f) {TILE->setfastinit(fast,avgd2);}

   //! set resolution reduction of invisible tiles
   void setreduction(float reduction=2.0f,float ratio=3.0f) {TILE->setreduction(reduction,ratio);}

   //! set texture range
   void setrange(float prange); // texture lod drops by one level beyond range

   //! define radius at which the non-linear texture LOD term kicks in
   void setradius(float radius, // non-linear texture LOD radius beyond which the drop-off takes effect
                  float dropoff=1.0f); // number of levels by which the texture LOD drops off

   //! define texture size of LOD0
   void setbasesize(int basesize); // should be equal to maximum texture size of entire tile set

   //! define restricted roi in which tiles are mandatory at startup
   void restrictroi(float rx,float rz, // roi center
                    float rrad); // roi radius

   //! update roi so that missing tiles are requested immediately
   void updateroi(float res, // resolution
                  float ex,float ey,float ez, // eye point
                  float rx,float rz, // roi center
                  float rrad, // roi radius
                  float farp=0.0f, // far plane
                  float fovy=0.0f,float aspect=0.0f); // field of view and aspect

   //! update entire tile set
   void updateall();

   //! return wrapped minitile object
   minitile *getminitile() {return(TILE);}

   //! get texture paging range
   float getrange() {return(PRANGE0);}

   //! get texture dropoff radius
   float getradius() {return(PRADIUS0);}

   //! return memory consumption
   float getmem();

   //! return raw texture memory consumption
   float gettexmem();

   //! configure core parameters
   void configure_minres(float minres=9.0f); // default minimum resolution
   void configure_maxd2(float maxd2=100.0f); // default maximum d2-value
   void configure_sead2(float sead2=0.1f); // default sea level d2-value
   void configure_minoff(float minoff=0.1f); // default minimum base offset
   void configure_maxcull(int maxcull=8); // default maximum culling depth

   //! configuring
   void configure_seamin(float seamin); // set minimum sea level
   void configure_seamax(float seamax); // set maximum sea level
   void configure_mipmaps(int mipmaps=1); // turn mipmaps on or off

   //! configuring the pager
   void configure_szone(float szone=0.01f); // safety zone for texture paging
   void configure_tsafety(float tsafety=0.5f); // safety offset for texture paging
   void configure_minlazy(float minlazy=0.25f); // minimum lazyness for texture paging
   void configure_maxtexlod(int maxtexlod=10); // maximum allowed lod for texture paging
   void configure_maxmaplod(int maxmaplod=10); // maximum allowed lod for height map paging
   void configure_dontfree(int dontfree=0); // free memory chunks returned by callbacks

   //! configuring the loader
   void configure_usezeronumbering(int usezeronumbering=0); // col/row numbering starts with zero
   void configure_tilesetpath(char *tilesetpath); // define tile set path
   void configure_usepnm(int usepnm); // use either PNM or DB

   unsigned char **HFIELDS,**TEXTURES;

   int COLS,ROWS;
   float COLDIM,ROWDIM,SCALE;
   float CENTERX,CENTERY,CENTERZ;
   float RELSCALE,SEALEVEL;

   unsigned char **FOGMAPS;

   int *MANDATORY;

   unsigned char **LODS;
   int *MAXLOD,*ACTLOD,*NEWLOD;

   unsigned char **TEXS;
   int *MAXTEX,*ACTTEX,*NEWTEX,*MINTEX;

   unsigned char **FOGS;
   int *MAXFOG;

   int *LRU,TIME;

   protected:

   minitile *TILE;

   private:

   int MAX_STR;

   void init();
   void cleanup();

   unsigned char *updatelod(int col,int row,int lodinc);
   void updatefog(int col,int row);
   void checklods(int col,int row,int prelod);

   unsigned char *updatetex(int col,int row,int texinc);
   void checktexs(int col,int row,int pretex);

   static void request(int col,int row,int needtex,void *data);
   static void preload(int col,int row,void *data);

   void deliver();
   void expire();

   int (*REQUEST_CALLBACK)(int col,int row,const unsigned char *mapfile,int hlod,const unsigned char *texfile,int tlod,const unsigned char *fogfile,void *data,databuf *hfield,databuf *texture,databuf *fogmap);
   void (*PRELOAD_CALLBACK)(int col,int row,const unsigned char *mapfile,int hlod,const unsigned char *texfile,int tlod,const unsigned char *fogfile,void *data);
   void (*DELIVER_CALLBACK)(int *col,int *row,databuf *hfield,int *hlod,databuf *texture,int *tlod,databuf *fogmap,void *data);
   void *REQUEST_DATA;

   int (*INQUIRY_CALLBACK)(int col,int row,const unsigned char *mapfile,int hlod,void *data,float *minvalue,float *maxvalue);
   void *INQUIRY_DATA;

   void (*QUERY_CALLBACK)(int col,int row,const unsigned char *texfile,int tlod,void *data,int *tsizex,int *tsizey);
   void *QUERY_DATA;

   int PAGING;

   float PFARP;
   float PRANGE,PRANGE0;
   int PBASESIZE;
   int PUPDATE;

   float PRADIUS,PRADIUS0;
   float PDROPOFF;

   int PRELOD;
   int PRETEX;

   int LAZYNESS;
   int LAZYTEX;

   int RELOADED;

   float CONFIGURE_MINRES;
   float CONFIGURE_MAXD2;
   float CONFIGURE_SEAD2;
   float CONFIGURE_MINOFF;
   int CONFIGURE_MAXCULL;

   float CONFIGURE_SEAMIN;
   float CONFIGURE_SEAMAX;
   int CONFIGURE_MIPMAPS;

   float CONFIGURE_SZONE;
   float CONFIGURE_TSAFETY;
   float CONFIGURE_MINLAZY;
   int CONFIGURE_MAXTEXLOD;
   int CONFIGURE_MAXMAPLOD;
   int CONFIGURE_DONTFREE;

   int CONFIGURE_USEZERONUMBERING;
   char *CONFIGURE_TILESETPATH;
   int CONFIGURE_USEPNM;

   int EXPIRE;
   int ECOL,EROW;

   float RES;

   int FOCUS;
   float FX,FY,FZ;
   };

#endif
