// (c) by Stefan Roettger

#include <mini/minibase.h>

#include <mini/miniio.h>

#include <mini/database.h>
#include <mini/datacalc.h>

#include <mini/squishbase.h>

#include <mini/jpegbase.h>
#include <mini/pngbase.h>

#include <mini/imgbase.h>

datacalc imgbase::CALC;

// S3TC auto-compression hook
void imgbase::autocompress(int isrgbadata,unsigned char *rawdata,unsigned int bytes,
                           unsigned char **s3tcdata,unsigned int *s3tcbytes,int width,int height,
                           void *data)
   {
   if (data!=NULL) ERRORMSG();

   squishbase::compressS3TC(isrgbadata,rawdata,bytes,
                            s3tcdata,s3tcbytes,width,height,
                            squishbase::SQUISHMODE_GOOD);
   }

// S3TC auto-decompression hook
void imgbase::autodecompress(int isrgbadata,unsigned char *s3tcdata,unsigned int bytes,
                             unsigned char **rawdata,unsigned int *rawbytes,int width,int height,
                             void *data)
   {
   if (data!=NULL) ERRORMSG();

   squishbase::decompressS3TC(isrgbadata,s3tcdata,bytes,
                              rawdata,rawbytes,width,height);
   }

// load image based on extension
int imgbase::loadimg(databuf &buf,char *filename)
   {
   FILE_TYPE type;
   char *ext;

   unsigned char *jpgdata;
   unsigned int jpgbytes;

   int jpgwidth,jpgheight,jpgcomponents;

   unsigned char *pngdata;
   unsigned int pngbytes;

   int pngwidth,pngheight,pngcomponents;

   unsigned char *rawdata;

   if (checkfile(filename)==0) return(0);

   ext=strrchr(filename,'.');

   type=FILE_TYPE_ETC;

   if (ext!=NULL)
      if (strcmp(ext,".db")==0) type=FILE_TYPE_DB;
      else if (strcmp(ext,".pgm")==0) type=FILE_TYPE_PNM;
      else if (strcmp(ext,".ppm")==0) type=FILE_TYPE_PNM;
      else if (strcmp(ext,".pvm")==0) type=FILE_TYPE_PVM;
      else if (strcmp(ext,".jpg")==0) type=FILE_TYPE_JPG;
      else if (strcmp(ext,".png")==0) type=FILE_TYPE_PNG;

   // register implicit calculator
   if (databuf::check_interpreter()==0) CALC.doregister();

   // load buffer
   if (type==FILE_TYPE_DB) buf.loaddata(filename);
   else if (type==FILE_TYPE_PNM) buf.loadPNMdata(filename);
   else if (type==FILE_TYPE_PVM) buf.loadPVMdata(filename);
   else if (type==FILE_TYPE_JPG)
      {
      jpgdata=readfile(filename,&jpgbytes);
      rawdata=decompressJPEGimage(jpgdata,jpgbytes,&jpgwidth,&jpgheight,&jpgcomponents);

      if (jpgcomponents==1) buf.set(rawdata,jpgwidth*jpgheight*jpgcomponents,jpgwidth,jpgheight,1,1,0);
      else if (jpgcomponents==3) buf.set(rawdata,jpgwidth*jpgheight*jpgcomponents,jpgwidth,jpgheight,1,1,3);
      else if (jpgcomponents==4) buf.set(rawdata,jpgwidth*jpgheight*jpgcomponents,jpgwidth,jpgheight,1,1,4);
      }
   else if (type==FILE_TYPE_PNG)
      {
      pngdata=readfile(filename,&pngbytes);
      rawdata=decompressPNGimage(pngdata,pngbytes,&pngwidth,&pngheight,&pngcomponents);

      if (pngcomponents==1) buf.set(rawdata,pngwidth*pngheight*pngcomponents,pngwidth,pngheight,1,1,0);
      else if (pngcomponents==2)
         {
         buf.set(rawdata,pngwidth*pngheight*pngcomponents,pngwidth,pngheight,1,1,1);
         buf.swap2();
         }
      else if (pngcomponents==3) buf.set(rawdata,pngwidth*pngheight*pngcomponents,pngwidth,pngheight,1,1,3);
      else if (pngcomponents==4) buf.set(rawdata,pngwidth*pngheight*pngcomponents,pngwidth,pngheight,1,1,4);
      }
   else return(0);

   return(1);
   }

// save image based on extension
int imgbase::saveimg(databuf &buf,char *filename,float jpgquality)
   {
   FILE_TYPE type;
   char *ext;

   unsigned char *jpgdata;
   unsigned int jpgbytes;

   unsigned char *pngdata;
   unsigned int pngbytes;

   ext=strrchr(filename,'.');

   type=FILE_TYPE_ETC;

   if (ext!=NULL)
      if (strcmp(ext,".db")==0) type=FILE_TYPE_DB;
      else if (strcmp(ext,".pgm")==0) type=FILE_TYPE_PNM;
      else if (strcmp(ext,".ppm")==0) type=FILE_TYPE_PNM;
      else if (strcmp(ext,".pvm")==0) type=FILE_TYPE_PVM;
      else if (strcmp(ext,".jpg")==0) type=FILE_TYPE_JPG;
      else if (strcmp(ext,".png")==0) type=FILE_TYPE_PNG;

   // automatic mip-mapping
   if (type==FILE_TYPE_DB) buf.automipmap();

   // register auto-compression hook
   if (databuf::check_autocompress()==0) databuf::setautocompress(autocompress,NULL);

   // register auto-decompression hook
   if (databuf::check_autodecompress()==0) databuf::setautodecompress(autodecompress,NULL);

   // compress to or decompress from s3tc
   if (type==FILE_TYPE_DB) buf.autocompress();
   else buf.autodecompress();

   // save buffer
   if (type==FILE_TYPE_DB) buf.savedata(filename);
   else if (type==FILE_TYPE_PNM) buf.savePNMdata(filename);
   else if (type==FILE_TYPE_PVM) buf.savePVMdata(filename);
   else if (type==FILE_TYPE_JPG && buf.zsize==1 && buf.tsteps==1)
      {
      if (buf.type==0) compressJPEGimage((unsigned char *)buf.data,buf.xsize,buf.ysize,1,jpgquality,&jpgdata,&jpgbytes);
      else if (buf.type==3) compressJPEGimage((unsigned char *)buf.data,buf.xsize,buf.ysize,3,jpgquality,&jpgdata,&jpgbytes);
      else if (buf.type==4) compressJPEGimage((unsigned char *)buf.data,buf.xsize,buf.ysize,4,jpgquality,&jpgdata,&jpgbytes);

      writefile(filename,jpgdata,jpgbytes);
      }
   else if (type==FILE_TYPE_PNG && buf.zsize==1 && buf.tsteps==1)
      {
      if (buf.type==0) compressPNGimage((unsigned char *)buf.data,buf.xsize,buf.ysize,1,&pngdata,&pngbytes);
      else if (buf.type==1)
         {
         buf.swap2();
         compressPNGimage((unsigned char *)buf.data,buf.xsize,buf.ysize,2,&pngdata,&pngbytes);
         }
      else if (buf.type==3) compressPNGimage((unsigned char *)buf.data,buf.xsize,buf.ysize,3,&pngdata,&pngbytes);
      else if (buf.type==4) compressPNGimage((unsigned char *)buf.data,buf.xsize,buf.ysize,4,&pngdata,&pngbytes);

      writefile(filename,pngdata,pngbytes);
      }
   else return(0);

   return(1);
   }
