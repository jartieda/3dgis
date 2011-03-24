#include "minibase.h"

#include "database.h"

#include "imgbase.h"

static const int fillin_radius=5;

int main(int argc,char *argv[])
   {
   databuf buf;

   float jpgquality=0.9f;

   if (argc!=3)
      {
      printf("usage: %s <input.db> <output.db>\n",argv[0]);
      exit(1);
      }

   // load buffer
   if (imgbase::loadimg(buf,argv[1])!=0)
      {
      // fill-in no-data values
      buf.fillnodata(fillin_radius);

      // save buffer
      imgbase::saveimg(buf,argv[2],jpgquality);

      // release buffer
      buf.release();
      }

   return(0);
   }
