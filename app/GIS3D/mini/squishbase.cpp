// (c) by Stefan Roettger

#include <mini/minibase.h>

#ifndef NOSQUISH
#include <mini/squishbase.h>
#endif

#include <squish.h> // HINT: do not install the header file in /usr/include on MacOS X

namespace squishbase {

void compressS3TC(int isrgbadata,unsigned char *rawdata,unsigned int bytes,
                  unsigned char **s3tcdata,unsigned int *s3tcbytes,int width,int height,
                  int squishmode)
   {
   if (isrgbadata<0 || rawdata==NULL || bytes==0 ||
       s3tcdata==NULL || s3tcbytes==NULL || width<1 || height<1 ||
       squishmode<0) ERRORMSG();

#ifndef NOSQUISH

   int i;

   int mode;

   unsigned char *rgbadata;

   static const int modefast=squish::kDxt1 | squish::kColourRangeFit; // fast but produces artifacts
   static const int modegood=squish::kDxt1 | squish::kColourClusterFit; // almost no artifacts though much slower
   static const int modeslow=squish::kDxt1 | squish::kColourIterativeClusterFit; // no artifacts but really sluggish

   switch (squishmode)
      {
      default:
      case SQUISHMODE_FAST: mode=modefast; break;
      case SQUISHMODE_GOOD: mode=modegood; break;
      case SQUISHMODE_SLOW: mode=modeslow; break;
      }

   if (isrgbadata==0)
      {
      if (width<1 || height<1) ERRORMSG();
      if (bytes!=(unsigned int)3*width*height) ERRORMSG();

      rgbadata=(unsigned char *)malloc(4*width*height);
      if (rgbadata==NULL) ERRORMSG();

      for (i=0; i<width*height; i++)
         {
         rgbadata[4*i]=rawdata[3*i];
         rgbadata[4*i+1]=rawdata[3*i+1];
         rgbadata[4*i+2]=rawdata[3*i+2];
         rgbadata[4*i+3]=255;
         }

      rawdata=rgbadata;
      }
   else
      {
      if (width<1 || height<1) ERRORMSG();
      if (bytes!=(unsigned int)4*width*height) ERRORMSG();
      }

   *s3tcbytes=squish::GetStorageRequirements(width,height,mode);
   *s3tcdata=(unsigned char *)malloc(*s3tcbytes);
   if (*s3tcdata==NULL) ERRORMSG();

   squish::CompressImage(rawdata,width,height,*s3tcdata,mode);

   if (isrgbadata==0) free(rawdata);

#else

   *s3tcdata=NULL;
   *s3tcbytes=0;

#endif
   }

void decompressS3TC(int isrgbadata,unsigned char *s3tcdata,unsigned int bytes,
                    unsigned char **rawdata,unsigned int *rawbytes,int width,int height)
   {
   if (isrgbadata<0 || s3tcdata==NULL || bytes==0 ||
       rawdata==NULL || rawbytes==NULL || width<1 || height<1) ERRORMSG();

#ifndef NOSQUISH

   int i;

   unsigned char *rgbdata;

   static const int mode=squish::kDxt1;

   if (width<1 || height<1) ERRORMSG();

   *rawbytes=4*width*height;
   *rawdata=(unsigned char *)malloc(*rawbytes);
   if (*rawdata==NULL) ERRORMSG();

   squish::DecompressImage(*rawdata,width,height,s3tcdata,mode);

   if (isrgbadata==0)
      {
      rgbdata=(unsigned char *)malloc(3*width*height);
      if (rgbdata==NULL) ERRORMSG();

      for (i=0; i<width*height; i++)
         {
         rgbdata[3*i]=(*rawdata)[4*i];
         rgbdata[3*i+1]=(*rawdata)[4*i+1];
         rgbdata[3*i+2]=(*rawdata)[4*i+2];
         }

      free(*rawdata);
      *rawdata=rgbdata;
      }

#else

   *rawdata=NULL;
   *rawbytes=0;

#endif
   }

}
