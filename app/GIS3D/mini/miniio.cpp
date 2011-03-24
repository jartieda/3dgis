// (c) by Stefan Roettger

#include "minibase.h"

#include "miniio.h"

namespace miniio {

// check a file
int checkfile(const char *filename)
   {
   FILE *file;

   if ((file=fopen(filename,"rb"))==NULL) return(0);
   fclose(file);

   return(1);
   }

// write a raw file
void writefile(const char *filename,unsigned char *data,unsigned int bytes)
   {
   FILE *file;

   if (bytes<1) ERRORMSG();

   if ((file=fopen(filename,"wb"))==NULL) ERRORMSG();
   if (fwrite(data,1,bytes,file)!=bytes) ERRORMSG();

   fclose(file);
   }

// read a raw file
unsigned char *readfile(const char *filename,unsigned int *bytes)
   {
   const unsigned int BLOCKSIZE=1<<20;

   FILE *file;

   unsigned char *data;
   unsigned int cnt,blkcnt;

   if ((file=fopen(filename,"rb"))==NULL) return(NULL);

   data=NULL;
   cnt=0;

   do
      {
      if (data==NULL)
         {if ((data=(unsigned char *)malloc(BLOCKSIZE))==NULL) ERRORMSG();}
      else
         if ((data=(unsigned char *)realloc(data,cnt+BLOCKSIZE))==NULL) ERRORMSG();

      blkcnt=fread(&data[cnt],1,BLOCKSIZE,file);
      cnt+=blkcnt;
      }
   while (blkcnt==BLOCKSIZE);

   if (cnt==0)
      {
      free(data);
      return(NULL);
      }

   if ((data=(unsigned char *)realloc(data,cnt))==NULL) ERRORMSG();

   fclose(file);

   *bytes=cnt;

   return(data);
   }

}
