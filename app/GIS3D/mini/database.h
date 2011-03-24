// (c) by Stefan Roettger

#ifndef DATABASE_H
#define DATABASE_H

#include "minibase.h"

/* This class represents a single 1D, 2D, 3D or 4D data buffer with
   additional metadata such as dimension and type. */

class databuf
   {
   public:

   enum DATABUF_TYPE
      {
      DATABUF_TYPE_BYTE=0,
      DATABUF_TYPE_SHORT=1,
      DATABUF_TYPE_FLOAT=2,
      DATABUF_TYPE_RGB=3,
      DATABUF_TYPE_RGBA=4,
      DATABUF_TYPE_RGB_S3TC=5,
      DATABUF_TYPE_RGBA_S3TC=6,
      DATABUF_TYPE_RGB_MM=7,
      DATABUF_TYPE_RGBA_MM=8,
      DATABUF_TYPE_RGB_MM_S3TC=9,
      DATABUF_TYPE_RGBA_MM_S3TC=10
      };

   enum DATABUF_CRS
      {
      DATABUF_CRS_LINEAR=0,
      DATABUF_CRS_LLH=1,
      DATABUF_CRS_UTM=2
      };

   enum DATABUF_EXTFMT
      {
      DATABUF_EXTFMT_PLAIN=0,
      DATABUF_EXTFMT_JPEG=1,
      DATABUF_EXTFMT_PNG=2
      };

   //! default constructor
   databuf();

   //! mandatory metadata
   unsigned int xsize;  // data size along the x-axis of 1D data
   unsigned int ysize;  // optional data size along the y-axis of 2D data
   unsigned int zsize;  // optional data size along the z-axis of 3D data
   unsigned int tsteps; // optional number of frames for time-dependent data
   unsigned int type;   // 0 = unsigned byte, 1 = signed short, 2 = float, 3 = RGB, 4 = RGBA, 5 = compressed RGB (S3TC DXT1), 6 = compressed RGBA (S3TC DXT1 with 1-bit alpha), 7 = mip-mapped RGB, 8 = mip-mapped RGBA, 9 = mip-mapped compressed RGB, 10 = mip-mapped compressed RGBA

   //! optional metadata
   float swx,swy; // SW corner of data brick
   float nwx,nwy; // NW corner of data brick
   float nex,ney; // NE corner of data brick
   float sex,sey; // SE corner of data brick
   float h0,dh;   // base elevation and height of data brick
   float t0,dt;   // time frame start and exposure time

   //! optional scaling
   float scaling; // scale factor of data values, default=1.0f
   float bias;    // bias of data values, default=0.0f

   //! optional minimum and maximum value
   float minvalue;
   float maxvalue;

   //! coordinate system indicator
   int crs; // 0 = none, 1 = LL, 2 = UTM
   int zone; // coordinate system zone
   int datum; // coordinate system datum

   //! no-data indicator
   float nodata;

   //! indicator for external format
   unsigned int extformat; // 0=plain, 1=JPEG, 2=PNG

   //! indicator for implicit format
   unsigned int implformat;

   //! optional corner points in Lat/Lon (WGS84, degrees)
   float LLWGS84_swx,LLWGS84_swy; // SW corner of data brick
   float LLWGS84_nwx,LLWGS84_nwy; // NW corner of data brick
   float LLWGS84_nex,LLWGS84_ney; // NE corner of data brick
   float LLWGS84_sex,LLWGS84_sey; // SE corner of data brick

   //! data chunk
   void *data;         // pointer to raw data, null pointer indicates missing data
   unsigned int bytes; // number of raw data bytes

   //! check for missing data
   int missing() const {return(data==NULL || extformat!=DATABUF_EXTFMT_PLAIN || implformat!=0);}

   //! allocate a new memory chunk
   void alloc(unsigned int xs,unsigned int ys,unsigned int zs,unsigned int ts=1,unsigned int ty=0);

   //! set data to memory chunk
   void set(void *chunk,unsigned int length,
            unsigned int xs,unsigned int ys,unsigned int zs,unsigned int ts=1,unsigned int ty=0);

   //! copy data from memory chunk
   void copy(void *chunk,unsigned int length,
             unsigned int xs,unsigned int ys,unsigned int zs,unsigned int ts=1,unsigned int ty=0);

   //! copy data from buffer
   void copy(databuf *buf);

   //! duplicate data from buffer
   void duplicate(databuf *buf);

   //! clear buffer
   void clear(float value=0.0f);

   //! reset buffer
   void reset();

   //! release buffer
   void release();

   //! set native extents
   void set_extents(float left,float right,float bottom,float top);

   //! set LLWGS84 extents
   void set_LLWGS84extents(float left,float right,float bottom,float top);

   //! set native extents
   void set_corners(float sw_corner_x,float sw_corner_y,
                    float se_corner_x,float se_corner_y,
                    float nw_corner_x,float nw_corner_y,
                    float ne_corner_x,float ne_corner_y);

   //! set LLWGS84 corners
   void set_LLWGS84corners(float sw_corner_x,float sw_corner_y,
                           float se_corner_x,float se_corner_y,
                           float nw_corner_x,float nw_corner_y,
                           float ne_corner_x,float ne_corner_y);

   //! set height extent
   void set_height(float bottom,float height);

   //! set time range
   void set_time(float time,float range);

   //! set coordinate system
   void set_crs(int crs_type,int crs_zone=0,int crs_datum=0);

   //! native input/output
   void savedata(const char *filename,unsigned int extfmt=DATABUF_EXTFMT_PLAIN); // data is saved in MSB byte order
   int loaddata(const char *filename,int stub=0,unsigned int tstart=0,unsigned int tstop=0); // data is converted from MSB to native byte order

   //! convert byte order
   void swap2(int msb=1);

   //! set conversion hook for external formats
   static void setconversion(int (*conversion)(int israwdata,unsigned char *srcdata,unsigned int bytes,unsigned int extformat,unsigned char **newdata,unsigned int *newbytes,databuf *obj,void *data),void *data);

   //! check conversion hook
   static int check_conversion() {return((CONVERSION_HOOK==NULL)?0:1);}

   //! set automatic mip-mapping
   static void setautomipmap(int autos3tcmipmap=0);

   //! check automatic mip-mapping
   static int check_automipmap() {return(AUTOS3TCMIPMAP);}

   //! automatic mip-mapping
   void automipmap();

   //! set hook for automatic s3tc compression
   static void setautocompress(void (*autocompress)(int isrgbadata,unsigned char *rawdata,unsigned int bytes,unsigned char **s3tcdata,unsigned int *s3tcbytes,int width,int height,void *data),void *data);

   //! check autocompress hook
   static int check_autocompress() {return((AUTOCOMPRESS_HOOK==NULL)?0:1);}

   //! automatic s3tc compression
   void autocompress();

   //! set hook for automatic s3tc decompression
   static void setautodecompress(void (*autodecompress)(int isrgbadata,unsigned char *s3tcdata,unsigned int bytes,unsigned char **rawdata,unsigned int *rawbytes,int width,int height,void *data),void *data);

   //! check autodecompress hook
   static int check_autodecompress() {return((AUTODECOMPRESS_HOOK==NULL)?0:1);}

   //! automatic s3tc decompression
   void autodecompress();

   //! set interpreter hook for implicit format
   static void setinterpreter(void (*parser)(unsigned int implformat,char *code,int bytes,databuf *obj,void *data),void *data,
                              void (*interpreter)(float *value,int comps,float x,float y,float z,float t,databuf *obj,void *data));

   //! check interpreter hook
   static int check_interpreter() {return((INTERPRETER_HOOK==NULL)?0:1);}

   //! data is loaded from PNM file
   int loadPNMdata(const char *filename);

   //! data is converted from PPM into compressed native format
   int loadPPMcompressed(const char *filename);

   //! data is converted from normalized PPM into compressed native format
   int loadPPMnormalized(const char *filename,const char *normalizedpath);

   //! data is loaded from PVM file
   int loadPVMdata(const char *filename,
                   float midx=0.0f,float midy=0.0f,float basez=0.0f,
                   float dx=1.0f,float dy=1.0f,float dz=1.0f);

   //! data is loaded from PVM time series
   //! the actual time step n is appended to the file name, e.g. filename.pvm-t01
   int loadPVMdata(const char *filename,
                   unsigned int t, unsigned int n,
                   float timestart,float timestep,
                   float midx,float midy,float basez,
                   float dx,float dy,float dz);

   //! data is loaded from MOE file
   int loadMOEdata(const char *filename,float *useful_smallest=0,float *useful_greatest=0);

   //! data is saved as plain PNM image
   void savePNMdata(const char *filename);

   //! data is saved as PVM volume
   void savePVMdata(const char *filename);

   //! data is generated from plane equation
   void generateplane(int size, // grid size
                      float px,float py,float pz, // point on plane
                      float nx,float ny,float nz, // plane normal
                      float dx,float dy,float dz); // dimension of box

   //! convert data from one type to another
   void convertdata(unsigned int newtype);

   //! resample data at a higher resolution
   void resampledata(unsigned int xs,unsigned int ys,unsigned int zs);

   //! set the no-data indicator
   void setnodata(float value);

   //! get the minimum and maximum scalar value
   void getminmax(float *minval=0,float *maxval=0);

   //! get the minimum and maximum scalar value within a useful range
   void getminmax(float usefs,float usefg, // useful range
                  float *minval=0,float *maxval=0); // result range

   //! check for no-data values
   int checknodata();

   //! replace no-data values
   unsigned int replacenodata(float value);

   //! fill-in no-data values
   unsigned int fillnodata(int radius=5);

   //! replace invalid values
   unsigned int replaceinvalid(float usefs,float usefg,float useful);

   //! clamp to range
   unsigned int clamp(float usefs,float usefg);

   //! compute absolute values
   void computeabsolute();

   //! set a single scalar value
   void setval(const unsigned int i,const unsigned int j,const unsigned int k,const float value);
   void setval(const unsigned int i,const unsigned int j,const unsigned int k,const unsigned int t,const float value);

   //! get a single scalar value
   float getval(const unsigned int i,const unsigned int j,const unsigned int k);
   float getval(const unsigned int i,const unsigned int j,const unsigned int k,const unsigned int t);

   //! get an interpolated scalar value
   float getvalue(float x,float y,float z,unsigned int t);

   //! set rgb[a] color
   void setrgb(const unsigned int i,const unsigned int j,const unsigned int k,const float *value);
   void setrgb(const unsigned int i,const unsigned int j,const unsigned int k,const unsigned int t,const float *value);
   void setrgba(const unsigned int i,const unsigned int j,const unsigned int k,const float *value);
   void setrgba(const unsigned int i,const unsigned int j,const unsigned int k,const unsigned int t,const float *value);

   //! print values
   void print();

   protected:

   static unsigned int MAGIC1;
   static unsigned int MAGIC2;
   static unsigned int MAGIC3;
   static unsigned int MAGIC4;
   static unsigned int MAGIC5;

   private:

   static unsigned short int INTEL_CHECK;

   static int AUTOS3TCMIPMAP;

   static int (*CONVERSION_HOOK)(int israwdata,unsigned char *srcdata,unsigned int bytes,unsigned int extformat,unsigned char **newdata,unsigned int *newbytes,databuf *obj,void *data);
   static void *CONVERSION_DATA;

   static void (*AUTOCOMPRESS_HOOK)(int isrgbadata,unsigned char *rawdata,unsigned int bytes,unsigned char **s3tcdata,unsigned int *s3tcbytes,int width,int height,void *data);
   static void *AUTOCOMPRESS_DATA;

   static void (*AUTODECOMPRESS_HOOK)(int isrgbadata,unsigned char *s3tcdata,unsigned int bytes,unsigned char **rawdata,unsigned int *rawbytes,int width,int height,void *data);
   static void *AUTODECOMPRESS_DATA;

   static void (*INTERPRETER_INIT)(unsigned int implformat,char *code,int bytes,databuf *obj,void *data);
   static void (*INTERPRETER_HOOK)(float *value,int comps,float x,float y,float z,float t,databuf *obj,void *data);
   static void *INTERPRETER_DATA;

   void writeparam(const char *tag,float v,FILE *file,int digits=8);
   int readparam(const char *tag,float *v,FILE *file);

   int readparami(const char *tag,int *v,FILE *file);
   int readparamu(const char *tag,unsigned int *v,FILE *file);

   void loadblock(FILE *file);

   char *readoneline(FILE *file);

   void convertchunk(int israw,unsigned int extfmt);
   void interpretechunk(unsigned int implfmt);

   void autocompress_mipmaps(int isrgbadata,unsigned char **s3tcdata,unsigned int *s3tcbytes);

   void swapbytes();
   };

#endif
