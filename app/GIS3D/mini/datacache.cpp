// (c) by Stefan Roettger

#include "minibase.h"

#include "miniio.h"

#include "datacache.h"

// default constructor
datacache::datacache(miniload *terrain)
   {
   if (terrain==NULL) CLOUD=NULL;
   else CLOUD=new datacloud(terrain);

   REQUEST_CALLBACK=NULL;
   REQUEST_DATA=NULL;

   RECEIVE_CALLBACK=NULL;
   CHECK_CALLBACK=NULL;
   RECEIVE_DATA=NULL;

   RURL=RID=NULL;
   LPATH=NULL;

   LOCAL=TRUE;

   HASHSIZE=100000;
   HASHTABLE=NULL;

   ELEVTILESETFILE=NULL;
   IMAGTILESETFILE=NULL;
   VTBELEVINIFILE=NULL;
   VTBIMAGINIFILE=NULL;
   VTBELEVPATH=NULL;
   VTBIMAGPATH=NULL;
   STARTUPFILE=NULL;

   HAS_ELEVINFO=FALSE;

   ELEVINFO_TILESX=0;
   ELEVINFO_TILESY=0;
   ELEVINFO_CENTERX=0.0f;
   ELEVINFO_CENTERY=0.0f;
   ELEVINFO_SIZEX=0.0f;
   ELEVINFO_SIZEY=0.0f;
   ELEVINFO_MAXDEMSIZE=0;
   ELEVINFO_MAXELEV=0.0f;

   HAS_IMAGINFO=FALSE;

   IMAGINFO_TILESX=0;
   IMAGINFO_TILESY=0;
   IMAGINFO_CENTERX=0.0f;
   IMAGINFO_CENTERY=0.0f;
   IMAGINFO_SIZEX=0.0f;
   IMAGINFO_SIZEY=0.0f;
   IMAGINFO_MAXTEXSIZE=0;

   HAS_ELEVINI=HAS_ELEVINI_GEO=HAS_ELEVINI_COORDSYS=FALSE;

   ELEVINI_TILESX=0;
   ELEVINI_TILESY=0;
   ELEVINI_CENTERX=0.0f;
   ELEVINI_CENTERY=0.0f;
   ELEVINI_CENTERX_LLWGS84=0.0f;
   ELEVINI_CENTERY_LLWGS84=0.0f;
   ELEVINI_NORTHX_LLWGS84=0.0f;
   ELEVINI_NORTHY_LLWGS84=0.0f;
   ELEVINI_SIZEX=0.0f;
   ELEVINI_SIZEY=0.0f;
   ELEVINI_MAXDEMSIZE=0;
   ELEVINI_MINELEV=0.0f;
   ELEVINI_MAXELEV=0.0f;
   ELEVINI_COORDSYS_LL=0;
   ELEVINI_COORDSYS_LLDATUM=0;
   ELEVINI_COORDSYS_UTMZONE=0;
   ELEVINI_COORDSYS_UTMDATUM=0;

   HAS_IMAGINI=HAS_IMAGINI_GEO=HAS_IMAGINI_COORDSYS=FALSE;

   IMAGINI_TILESX=0;
   IMAGINI_TILESY=0;
   IMAGINI_CENTERX=0.0f;
   IMAGINI_CENTERY=0.0f;
   IMAGINI_CENTERX_LLWGS84=0.0f;
   IMAGINI_CENTERY_LLWGS84=0.0f;
   IMAGINI_NORTHX_LLWGS84=0.0f;
   IMAGINI_NORTHY_LLWGS84=0.0f;
   IMAGINI_SIZEX=0.0f;
   IMAGINI_SIZEY=0.0f;
   IMAGINI_MAXTEXSIZE=0;
   IMAGINI_COORDSYS_LL=0;
   IMAGINI_COORDSYS_LLDATUM=0;
   IMAGINI_COORDSYS_UTMZONE=0;
   IMAGINI_COORDSYS_UTMDATUM=0;

   CONFIGURE_DONTFREE=0;
   CONFIGURE_LOCTHREADS=1;
   CONFIGURE_NETTHREADS=10;
   CONFIGURE_AUTOLOCKIO=0;

   if (CLOUD!=NULL)
      {
      CLOUD->configure_dontfree(CONFIGURE_DONTFREE);
      CLOUD->configure_autolockio(0);
      }
   }

// destructor
datacache::~datacache()
   {
   if (CLOUD!=NULL) delete CLOUD;

   if (RURL!=NULL) free(RURL);
   if (RID!=NULL) free(RID);
   if (LPATH!=NULL) free(LPATH);

   reset();

   if (ELEVTILESETFILE!=NULL) free(ELEVTILESETFILE);
   if (IMAGTILESETFILE!=NULL) free(IMAGTILESETFILE);
   if (VTBELEVINIFILE!=NULL) free(VTBELEVINIFILE);
   if (VTBIMAGINIFILE!=NULL) free(VTBIMAGINIFILE);
   if (VTBELEVPATH!=NULL) free(VTBELEVPATH);
   if (VTBIMAGPATH!=NULL) free(VTBIMAGPATH);
   if (STARTUPFILE!=NULL) free(STARTUPFILE);
   }

// set callback for requesting tiles
void datacache::setloader(void (*request)(const char *file,int istexture,databuf *buf,void *data),void *data,
                          int paging,
                          float pfarp,
                          float prange,int pbasesize,
                          int plazyness,int pupdate,
                          int expire)
   {
   if (CLOUD==NULL) ERRORMSG();
   if (request==NULL) ERRORMSG();

   // propagate request and check callbacks to the wrapped datacloud object
   CLOUD->setloader(mystaticrequest,this,mystaticcheck,
                    paging,pfarp,prange,pbasesize,plazyness,pupdate,expire);

   // propagate optional inquiry callback
   CLOUD->setinquiry(mystaticinquiry,this);

   // propagate optional query callback
   CLOUD->setquery(mystaticquery,this);

   REQUEST_CALLBACK=request;
   REQUEST_DATA=data;
   }

// set callback for requesting tiles from a server
void datacache::setreceiver(void (*receive)(const char *src_url,const char *src_id,const char *src_file,const char *dst_file,int background,void *data),void *data,
                            int (*check)(const char *src_url,const char *src_id,const char *src_file,void *data))
   {
   RECEIVE_CALLBACK=receive;
   CHECK_CALLBACK=check;
   RECEIVE_DATA=data;
   }

// set remote url
void datacache::setremoteurl(const char *url)
   {
   if (RURL!=NULL) free(RURL);

   if (url==NULL) RURL=NULL;
   else if (strstr(url,"file://")!=NULL) RURL=strdup(url+7);
   else RURL=strdup(url);

   LOCAL=TRUE;

   if (RURL!=NULL)
      {
      if (strstr(RURL,"http://")!=NULL) LOCAL=FALSE;
      if (strstr(RURL,"https://")!=NULL) LOCAL=FALSE;
      if (strstr(RURL,"ftp://")!=NULL) LOCAL=FALSE;
      if (strstr(RURL,"ftps://")!=NULL) LOCAL=FALSE;
      }

   if (CLOUD!=NULL)
      if (LOCAL) CLOUD->setmulti(CONFIGURE_LOCTHREADS);
      else CLOUD->setmulti(CONFIGURE_NETTHREADS);
   }

// set remote world id
void datacache::setremoteid(const char *id)
   {
   if (RID!=NULL) free(RID);

   if (id==NULL) RID=NULL;
   else RID=strdup(id);
   }

// set local path to file cache
void datacache::setlocalpath(const char *path)
   {
   if (LPATH!=NULL) free(LPATH);

   if (path==NULL) LPATH=NULL;
   else LPATH=strdup(path);
   }

// set size of hash table
void datacache::sethashsize(int size)
   {
   if (HASHSIZE<1) ERRORMSG();

   if (HASHTABLE==NULL) HASHSIZE=size;
   }

// get file
char *datacache::getfile(const char *src_file,const char *altpath)
   {
   char *srcfile;
   char *filename;
   char *localname;
   char *altname;

   fileinfoelem *info;

   BOOLINT isavailable=FALSE;
   BOOLINT islocal=FALSE;

   srcfile=strdup(src_file);

   filename=sourcefilename(RID,srcfile);

   // check standard path:

   if (!LOCAL)
      {
      if (CHECK_CALLBACK==NULL || RECEIVE_CALLBACK==NULL) ERRORMSG();

      info=checkfilename(filename);

      if (info!=NULL)
         {
         isavailable=info->isavailable;
         islocal=info->islocal;
         }
      else
         {
         if (localfilecheck(filename)) isavailable=islocal=TRUE;
         else if (CHECK_CALLBACK(RURL,RID,srcfile,REQUEST_DATA)!=0) isavailable=TRUE;
         }

      if (isavailable && !islocal)
         {
         localname=localfilename(filename);
         RECEIVE_CALLBACK(RURL,RID,srcfile,localname,0,REQUEST_DATA);
         free(localname);
         islocal=TRUE;
         }
      }
   else isavailable=islocal=localfilecheck(filename);

   insertfilename(filename,isavailable,islocal,FALSE,FALSE);
   free(srcfile);

   localname=localfilename(filename);
   free(filename);

   if (islocal) return(localname);

   free(localname);

   // check alternative path:

   if (altpath!=NULL)
      {
      srcfile=strdup(src_file);
      localname=strdup(altpath);

      altname=strcct(localname,srcfile);

      free(srcfile);
      free(localname);

      if (checkfile(altname)!=0) return(altname);

      free(altname);
      }

   return(NULL);
   }

// set name of elev tileset file
void datacache::setelevtilesetfile(const char *filename)
   {
   if (ELEVTILESETFILE!=NULL) free(ELEVTILESETFILE);

   if (filename==NULL) ELEVTILESETFILE=NULL;
   else ELEVTILESETFILE=strdup(filename);
   }

// set name of imag tileset file
void datacache::setimagtilesetfile(const char *filename)
   {
   if (IMAGTILESETFILE!=NULL) free(IMAGTILESETFILE);

   if (filename==NULL) IMAGTILESETFILE=NULL;
   else IMAGTILESETFILE=strdup(filename);
   }

// set name of vtb ini file for elevation tileset
void datacache::setvtbelevinifile(const char *filename)
   {
   if (VTBELEVINIFILE!=NULL) free(VTBELEVINIFILE);

   if (filename==NULL) VTBELEVINIFILE=NULL;
   else VTBELEVINIFILE=strdup(filename);
   }

// set name of vtb ini file for imagery tileset
void datacache::setvtbimaginifile(const char *filename)
   {
   if (VTBIMAGINIFILE!=NULL) free(VTBIMAGINIFILE);

   if (filename==NULL) VTBIMAGINIFILE=NULL;
   else VTBIMAGINIFILE=strdup(filename);
   }

// set path to vtb elevation tileset
void datacache::setvtbelevpath(const char *elev)
   {
   if (VTBELEVPATH!=NULL) free(VTBELEVPATH);

   if (elev==NULL) VTBELEVPATH=NULL;
   else VTBELEVPATH=strdup(elev);
   }

// set path to vtb imagery tileset
void datacache::setvtbimagpath(const char *imag)
   {
   if (VTBIMAGPATH!=NULL) free(VTBIMAGPATH);

   if (imag==NULL) VTBIMAGPATH=NULL;
   else VTBIMAGPATH=strdup(imag);
   }

// set name of startup file
void datacache::setstartupfile(const char *filename)
   {
   if (STARTUPFILE!=NULL) free(STARTUPFILE);

   if (filename==NULL) STARTUPFILE=NULL;
   else STARTUPFILE=strdup(filename);
   }

// load elev tileset info file
void datacache::loadelevtilesetinfo()
   {
   FILE *file;

   HAS_ELEVINFO=FALSE;

   if (ELEVTILESETFILE==NULL) return;

   char *tilesetname=getfile(ELEVTILESETFILE);

   if (tilesetname!=NULL)
      {
      HAS_ELEVINFO=TRUE;

      if ((file=fopen(tilesetname,"rb"))==NULL) ERRORMSG();

      if (fscanf(file,"tilesx=%d\n",&ELEVINFO_TILESX)!=1) HAS_ELEVINFO=FALSE;
      if (fscanf(file,"tilesy=%d\n",&ELEVINFO_TILESY)!=1) HAS_ELEVINFO=FALSE;
      if (fscanf(file,"centerx=%g arc-seconds\n",&ELEVINFO_CENTERX)!=1) HAS_ELEVINFO=FALSE;
      if (fscanf(file,"centery=%g arc-seconds\n",&ELEVINFO_CENTERY)!=1) HAS_ELEVINFO=FALSE;
      if (fscanf(file,"sizex=%g arc-seconds\n",&ELEVINFO_SIZEX)!=1) HAS_ELEVINFO=FALSE;
      if (fscanf(file,"sizey=%g arc-seconds\n",&ELEVINFO_SIZEY)!=1) HAS_ELEVINFO=FALSE;
      if (fscanf(file,"maxsize=%d\n",&ELEVINFO_MAXDEMSIZE)!=1) HAS_ELEVINFO=FALSE;
      if (fscanf(file,"maxelev=%g meters\n",&ELEVINFO_MAXELEV)!=1) HAS_ELEVINFO=FALSE;

      fclose(file);

      free(tilesetname);
      }
   }

// load imag tileset info file
void datacache::loadimagtilesetinfo()
   {
   FILE *file;

   HAS_IMAGINFO=FALSE;

   if (IMAGTILESETFILE==NULL) return;

   char *tilesetname=getfile(IMAGTILESETFILE);

   if (tilesetname!=NULL)
      {
      HAS_IMAGINFO=TRUE;

      if ((file=fopen(tilesetname,"rb"))==NULL) ERRORMSG();

      if (fscanf(file,"tilesx=%d\n",&IMAGINFO_TILESX)!=1) HAS_IMAGINFO=FALSE;
      if (fscanf(file,"tilesy=%d\n",&IMAGINFO_TILESY)!=1) HAS_IMAGINFO=FALSE;
      if (fscanf(file,"centerx=%g arc-seconds\n",&IMAGINFO_CENTERX)!=1) HAS_IMAGINFO=FALSE;
      if (fscanf(file,"centery=%g arc-seconds\n",&IMAGINFO_CENTERY)!=1) HAS_IMAGINFO=FALSE;
      if (fscanf(file,"sizex=%g arc-seconds\n",&IMAGINFO_SIZEX)!=1) HAS_IMAGINFO=FALSE;
      if (fscanf(file,"sizey=%g arc-seconds\n",&IMAGINFO_SIZEY)!=1) HAS_IMAGINFO=FALSE;
      if (fscanf(file,"maxsize=%d\n",&IMAGINFO_MAXTEXSIZE)!=1) HAS_IMAGINFO=FALSE;
      if (fscanf(file,"maxelev=%g\n",&IMAGINFO_MAXELEV)!=1) HAS_IMAGINFO=FALSE;

      fclose(file);

      free(tilesetname);
      }
   }

// load vtb ini file for elevation tileset
void datacache::loadvtbelevini()
   {
   int i;

   FILE *file;

   float left,right,bottom,top;

   char ch;

   int lod0,size0;

   int row,col,lod;
   int minlod,maxlod;

   static const int maxstr=1024;
   char *pathname,filename[maxstr],*fullname;

   HAS_ELEVINI=HAS_ELEVINI_GEO=FALSE;

   if (VTBELEVINIFILE==NULL) return;

   char *vtbininame=getfile(VTBELEVINIFILE);

   if (vtbininame!=NULL)
      {
      HAS_ELEVINI=TRUE;

      if ((file=fopen(vtbininame,"rb"))==NULL) ERRORMSG();

      if (fscanf(file,"[TilesetDescription]\nColumns=%d\n",&ELEVINI_TILESX)!=1) HAS_ELEVINI=FALSE;
      if (fscanf(file,"Rows=%d\n",&ELEVINI_TILESY)!=1) HAS_ELEVINI=FALSE;
      if (fscanf(file,"LOD0_Size=%d\n",&ELEVINI_MAXDEMSIZE)!=1) HAS_ELEVINI=FALSE;

      if (fscanf(file,"Extent_Left=%g\n",&left)!=1) HAS_ELEVINI=FALSE;
      if (fscanf(file,"Extent_Right=%g\n",&right)!=1) HAS_ELEVINI=FALSE;
      if (fscanf(file,"Extent_Bottom=%g\n",&bottom)!=1) HAS_ELEVINI=FALSE;
      if (fscanf(file,"Extent_Top=%g\n",&top)!=1) HAS_ELEVINI=FALSE;

      if (HAS_ELEVINI)
         {
         ELEVINI_CENTERX=(left+right)/2.0f;
         ELEVINI_CENTERY=(bottom+top)/2.0f;
         ELEVINI_SIZEX=right-left;
         ELEVINI_SIZEY=top-bottom;

         ELEVINI_MAXDEMSIZE++;

         // skip over CRS
         ch=fgetc(file);
         while (ch!='\n' && ch!='\0') ch=fgetc(file);

         // read minimum elevation
         if (fscanf(file,"Elevation_Min=%g\n",&ELEVINI_MINELEV)!=1) HAS_ELEVINI=FALSE;

         // read maximum elevation
         if (fscanf(file,"Elevation_Max=%g\n",&ELEVINI_MAXELEV)!=1) HAS_ELEVINI=FALSE;

         // calculate LOD0
         for (lod0=1,size0=3; size0<ELEVINI_MAXDEMSIZE; lod0++) size0=2*size0-1;

         // read LODs from each row
         for (i=0; i<ELEVINI_TILESY; i++)
            {
            // read actual row number
            if (fscanf(file,"RowLODs %d:",&row)!=1) HAS_ELEVINI=FALSE;

            // process columns
            for (col=0; col<ELEVINI_TILESX; col++)
               {
               // read maximum and minimum LOD in vtb notation
               if (col<ELEVINI_TILESX-1)
                  {if (fscanf(file," %d/%d",&maxlod,&minlod)!=2) HAS_ELEVINI=FALSE;}
               else
                  {if (fscanf(file," %d/%d\n",&maxlod,&minlod)!=2) HAS_ELEVINI=FALSE;}

               // concatenate path to elev tileset
               pathname=strcct(RID,VTBELEVPATH);

               // register each existent lod with data cache
               if (minlod>0) // tile exists
                  for (lod=minlod; lod<=maxlod; lod++)
                     {
                     if (lod==maxlod) snprintf(filename,maxstr,"/tile.%d-%d.db",col,row);
                     else snprintf(filename,maxstr,"/tile.%d-%d.db%d",col,row,maxlod-lod); // write actual lod in mini notation

                     fullname=strcct(pathname,filename);
                     insertfilename(fullname,TRUE,LOCAL,TRUE,FALSE,ELEVINI_MINELEV,ELEVINI_MAXELEV,(1<<lod)+1,(1<<lod)+1,TRUE,TRUE);
                     free(fullname);
                     }

               // register first non-existent lod with data cache
               if (minlod==0 || minlod>1)
                  {
                  lod=minlod-1;

                  if (lod>=0) snprintf(filename,maxstr,"/tile.%d-%d.db%d",col,row,maxlod-lod);
                  else snprintf(filename,maxstr,"/tile.%d-%d.db",col,row);

                  lod=max(lod,0);

                  fullname=strcct(pathname,filename);
                  insertfilename(fullname,FALSE,FALSE,TRUE,FALSE,ELEVINI_MINELEV,ELEVINI_MAXELEV,(1<<lod)+1,(1<<lod)+1,TRUE,TRUE);
                  free(fullname);
                  }

               // release path to elev tileset
               free(pathname);
               }
            }
         }

      if (HAS_ELEVINI)
         {
         HAS_ELEVINI_GEO=TRUE;

         // read center point of the tileset in Lat/Lon WGS84
         if (fscanf(file,"CenterPoint_LLWGS84=(%g,%g)\n",&ELEVINI_CENTERX_LLWGS84,&ELEVINI_CENTERY_LLWGS84)!=2) HAS_ELEVINI_GEO=FALSE;

         // read north point of the tileset in Lat/Lon WGS84
         if (fscanf(file,"NorthPoint_LLWGS84=(%g,%g)\n",&ELEVINI_NORTHX_LLWGS84,&ELEVINI_NORTHY_LLWGS84)!=2) HAS_ELEVINI_GEO=FALSE;

         if (HAS_ELEVINI_GEO)
            {
            HAS_ELEVINI_COORDSYS=TRUE;

            // read LL coord sys info
            if (fscanf(file,"CoordSys_LL=(%d,%d)\n",&ELEVINI_COORDSYS_LL,&ELEVINI_COORDSYS_LLDATUM)!=2) HAS_ELEVINI_COORDSYS=FALSE;

            // read UTM coord sys info
            if (fscanf(file,"CoordSys_UTM=(%d,%d)\n",&ELEVINI_COORDSYS_UTMZONE,&ELEVINI_COORDSYS_UTMDATUM)!=2) HAS_ELEVINI_COORDSYS=FALSE;
            }
         }

      fclose(file);

      free(vtbininame);
      }
   }

// load vtb ini file for imagery tileset
void datacache::loadvtbimagini()
   {
   int i;

   FILE *file;

   float left,right,bottom,top;

   char ch;

   int lod0,size0;

   int row,col,lod;
   int minlod,maxlod;

   static const int maxstr=1024;
   char *pathname,filename[maxstr],*fullname;

   HAS_IMAGINI=HAS_IMAGINI_GEO=FALSE;

   if (VTBIMAGINIFILE==NULL) return;

   char *vtbininame=getfile(VTBIMAGINIFILE);

   if (vtbininame!=NULL)
      {
      HAS_IMAGINI=TRUE;

      if ((file=fopen(vtbininame,"rb"))==NULL) ERRORMSG();

      if (fscanf(file,"[TilesetDescription]\nColumns=%d\n",&IMAGINI_TILESX)!=1) HAS_IMAGINI=FALSE;
      if (fscanf(file,"Rows=%d\n",&IMAGINI_TILESY)!=1) HAS_IMAGINI=FALSE;
      if (fscanf(file,"LOD0_Size=%d\n",&IMAGINI_MAXTEXSIZE)!=1) HAS_IMAGINI=FALSE;

      if (fscanf(file,"Extent_Left=%g\n",&left)!=1) HAS_IMAGINI=FALSE;
      if (fscanf(file,"Extent_Right=%g\n",&right)!=1) HAS_IMAGINI=FALSE;
      if (fscanf(file,"Extent_Bottom=%g\n",&bottom)!=1) HAS_IMAGINI=FALSE;
      if (fscanf(file,"Extent_Top=%g\n",&top)!=1) HAS_IMAGINI=FALSE;

      if (HAS_IMAGINI)
         {
         IMAGINI_CENTERX=(left+right)/2.0f;
         IMAGINI_CENTERY=(bottom+top)/2.0f;
         IMAGINI_SIZEX=right-left;
         IMAGINI_SIZEY=top-bottom;

         // skip over CRS
         ch=fgetc(file);
         while (ch!='\n' && ch!='\0') ch=fgetc(file);

         // calculate LOD0
         for (lod0=0,size0=1; size0<IMAGINI_MAXTEXSIZE; lod0++) size0*=2;

         // read LODs from each row
         for (i=0; i<IMAGINI_TILESY; i++)
            {
            // read actual row number
            if (fscanf(file,"RowLODs %d:",&row)!=1) HAS_IMAGINI=FALSE;

            // process columns
            for (col=0; col<IMAGINI_TILESX; col++)
               {
               // read maximum and minimum LOD in vtb notation
               if (col<IMAGINI_TILESX-1)
                  {if (fscanf(file," %d/%d",&maxlod,&minlod)!=2) HAS_IMAGINI=FALSE;}
               else
                  {if (fscanf(file," %d/%d\n",&maxlod,&minlod)!=2) HAS_IMAGINI=FALSE;}

               // concatenate path to imag tileset
               pathname=strcct(RID,VTBIMAGPATH);

               // register each existent lod with data cache
               if (minlod>0) // tile exists
                  for (lod=minlod; lod<=maxlod; lod++)
                     {
                     if (lod==maxlod) snprintf(filename,maxstr,"/tile.%d-%d.db",col,row);
                     else snprintf(filename,maxstr,"/tile.%d-%d.db%d",col,row,maxlod-lod); // write actual lod in mini notation

                     fullname=strcct(pathname,filename);
                     insertfilename(fullname,TRUE,LOCAL,FALSE,TRUE,1.0f,0.0f,1<<lod,1<<lod,TRUE,TRUE);
                     free(fullname);
                     }

               // register first non-existent lod with data cache
               if (minlod==0 || minlod>1)
                  {
                  lod=minlod-1;

                  if (lod>=0) snprintf(filename,maxstr,"/tile.%d-%d.db%d",col,row,maxlod-lod);
                  else snprintf(filename,maxstr,"/tile.%d-%d.db",col,row);

                  lod=max(lod,0);

                  fullname=strcct(pathname,filename);
                  insertfilename(fullname,FALSE,FALSE,FALSE,TRUE,1.0f,0.0f,1<<lod,1<<lod,TRUE,TRUE);
                  free(fullname);
                  }

               // release path to imag tileset
               free(pathname);
               }
            }
         }

      if (HAS_IMAGINI)
         {
         HAS_IMAGINI_GEO=TRUE;

         // read center point of the tileset in Lat/Lon WGS84
         if (fscanf(file,"CenterPoint_LLWGS84=(%g,%g)\n",&IMAGINI_CENTERX_LLWGS84,&IMAGINI_CENTERY_LLWGS84)!=2) HAS_IMAGINI_GEO=FALSE;

         // read north point of the tileset in Lat/Lon WGS84
         if (fscanf(file,"NorthPoint_LLWGS84=(%g,%g)\n",&IMAGINI_NORTHX_LLWGS84,&IMAGINI_NORTHY_LLWGS84)!=2) HAS_IMAGINI_GEO=FALSE;

         if (HAS_IMAGINI_GEO)
            {
            HAS_IMAGINI_COORDSYS=TRUE;

            // read LL coord sys info
            if (fscanf(file,"CoordSys_LL=(%d,%d)\n",&IMAGINI_COORDSYS_LL,&IMAGINI_COORDSYS_LLDATUM)!=2) HAS_IMAGINI_COORDSYS=FALSE;

            // read UTM coord sys info
            if (fscanf(file,"CoordSys_UTM=(%d,%d)\n",&IMAGINI_COORDSYS_UTMZONE,&IMAGINI_COORDSYS_UTMDATUM)!=2) HAS_IMAGINI_COORDSYS=FALSE;
            }
         }

      fclose(file);

      free(vtbininame);
      }
   }

// load persistent startup file
void datacache::load(BOOLINT reset)
   {
   int c;

   FILE *file;
   char ch;

   char *name;
   int strsize;

   int v1,v2,v3,v4;
   float minvalue,maxvalue;
   int width,height;

   char *filename;
   char *localname;

   BOOLINT remote=FALSE;

   loadelevtilesetinfo();
   loadimagtilesetinfo();

   if (HAS_ELEVINFO && HAS_IMAGINFO)
      {
      if (IMAGINFO_TILESX!=ELEVINFO_TILESX) HAS_ELEVINFO=HAS_IMAGINFO=FALSE;
      if (IMAGINFO_TILESY!=ELEVINFO_TILESY) HAS_ELEVINFO=HAS_IMAGINFO=FALSE;
      }

   loadvtbelevini();
   loadvtbimagini();

   if (HAS_ELEVINI && HAS_IMAGINI)
      {
      if (IMAGINI_TILESX!=ELEVINI_TILESX) HAS_ELEVINI=HAS_IMAGINI=FALSE;
      if (IMAGINI_TILESY!=ELEVINI_TILESY) HAS_ELEVINI=HAS_IMAGINI=FALSE;
      }

   if (STARTUPFILE==NULL || reset) return;

   filename=sourcefilename(RID,STARTUPFILE);
   localname=localfilename(filename);
   free(filename);

   if ((file=fopen(localname,"rb"))==NULL)
      if (LOCAL)
         {
         free(localname);
         return;
         }
      else
         {
         if (CHECK_CALLBACK==NULL || RECEIVE_CALLBACK==NULL) ERRORMSG();

         if (CHECK_CALLBACK(RURL,RID,STARTUPFILE,REQUEST_DATA)==0)
            {
            free(localname);
            return;
            }
         else
            {
            RECEIVE_CALLBACK(RURL,RID,STARTUPFILE,localname,0,REQUEST_DATA);

            if ((file=fopen(localname,"rb"))==NULL)
               {
               free(localname);
               return;
               }

            remote=TRUE;
            }
         }

   free(localname);

   strsize=1000;

   if ((name=(char *)malloc(strsize))==NULL) ERRORMSG();

   ch=getc(file);

   while (ch!=EOF)
      {
      c=0;

      while (ch!=' ' && ch!=EOF)
         {
         if (c>strsize-2)
            {
            strsize*=2;
            if ((name=(char *)realloc(name,strsize))==NULL) ERRORMSG();
            }

         name[c++]=ch;

         ch=getc(file);
         }

      name[c]='\0';

      fscanf(file,"%d %d %d %d %g %g %d %d\n",&v1,&v2,&v3,&v4,&minvalue,&maxvalue,&width,&height);

      ch=getc(file);

      while (ch=='\n' && ch!=EOF) ch=getc(file);

      if (remote) v2=0;

      insertfilename(name,v1!=0,v2!=0,v3!=0,v4!=0,minvalue,maxvalue,width,height,TRUE);
      }

   free(name);

   fclose(file);

   if (remote) save();
   }

// save persistent startup file
void datacache::save()
   {
   int i;

   FILE *file;

   fileinfoelem *info;

   char *filename;
   char *localname;

   if (STARTUPFILE==NULL) return;

   if (HASHTABLE==NULL) return;

   filename=sourcefilename(RID,STARTUPFILE);
   localname=localfilename(filename);

   if ((file=fopen(localname,"wb"))==NULL) ERRORMSG();

   free(filename);
   free(localname);

   for (i=0; i<HASHSIZE; i++)
      {
      info=HASHTABLE[i];

      while (info!=NULL)
         {
         fprintf(file,"%s %d %d %d %d %g %g %d %d\n",
                 info->filename,
                 info->isavailable,
                 info->islocal,
                 info->istile,
                 info->istexture,
                 info->minvalue,
                 info->maxvalue,
                 info->width,
                 info->height);

         info=info->next;
         }
      }

   fclose(file);
   }

// reset startup state
void datacache::reset()
   {
   int i;

   fileinfoelem *info,*next;

   if (HASHTABLE==NULL) return;

   for (i=0; i<HASHSIZE; i++)
      {
      info=HASHTABLE[i];

      while (info!=NULL)
         {
         next=info->next;

         free(info->filename);
         delete info;

         info=next;
         }
      }

   delete[] HASHTABLE;

   HASHTABLE=NULL;
   }

// private callbacks:

void datacache::myrequest(const unsigned char *mapfile,databuf *map,int istexture,int background)
   {
   char *filename;
   char *localname;

   if (REQUEST_CALLBACK==NULL) ERRORMSG();

   filename=sourcefilename(RID,(char *)mapfile);

   if (!LOCAL)
      {
      if (RECEIVE_CALLBACK==NULL) ERRORMSG();

      if (!localfilecheck(filename))
         {
         localname=localfilename(filename);
         RECEIVE_CALLBACK(RURL,RID,(char *)mapfile,localname,background,REQUEST_DATA);
         free(localname);
         }
      }

   localname=localfilename(filename);
   if (CONFIGURE_AUTOLOCKIO!=0) CLOUD->lockio();
   REQUEST_CALLBACK(localname,istexture,map,REQUEST_DATA);
   if (CONFIGURE_AUTOLOCKIO!=0) CLOUD->unlockio();
   free(localname);

   insertfilename(filename,TRUE,TRUE,TRUE,istexture!=0);

   free(filename);
   }

int datacache::mycheck(const unsigned char *mapfile,int istexture)
   {
   char *filename;

   fileinfoelem *info;

   BOOLINT isavailable=FALSE;
   BOOLINT islocal=FALSE;

   filename=sourcefilename(RID,(char *)mapfile);

   if (!LOCAL)
      {
      if (CHECK_CALLBACK==NULL) ERRORMSG();

      info=checkfilename(filename);

      if (info!=NULL)
         {
         isavailable=info->isavailable;
         islocal=info->islocal;
         }
      else
         {
         if (localfilecheck(filename)) isavailable=islocal=TRUE;
         else if (CHECK_CALLBACK(RURL,RID,(char *)mapfile,REQUEST_DATA)!=0) isavailable=TRUE;
         }
      }
   else isavailable=islocal=localfilecheck(filename);

   insertfilename(filename,isavailable,islocal,TRUE,istexture!=0);

   free(filename);

   return(isavailable?1:0);
   }

int datacache::myinquiry(int col,int row,const unsigned char *mapfile,int hlod,float *minvalue,float *maxvalue)
   {
   char *filename;

   fileinfoelem *info;

   databuf map;

   if (col<0 || row<0 || hlod<0) ERRORMSG();

   mycheck(mapfile,TRUE);

   filename=sourcefilename(RID,(char *)mapfile);

   info=checkfilename(filename);

   free(filename);

   if (info==NULL) ERRORMSG();

   if (!info->isavailable) return(0);

   if (info->minvalue>info->maxvalue)
      {
      myrequest(mapfile,&map,FALSE,0);

      map.getminmax(&info->minvalue,&info->maxvalue);

      if (CONFIGURE_DONTFREE==0) map.release();
      }

   *minvalue=info->minvalue;
   *maxvalue=info->maxvalue;

   return(1);
   }

void datacache::myquery(int col,int row,const unsigned char *texfile,int tlod,int *tsizex,int *tsizey)
   {
   char *filename;

   fileinfoelem *info;

   databuf map;

   if (col<0 || row<0 || tlod<0) ERRORMSG();

   mycheck(texfile,TRUE);

   filename=sourcefilename(RID,(char *)texfile);

   info=checkfilename(filename);

   free(filename);

   if (info==NULL) ERRORMSG();

   if (!info->isavailable) ERRORMSG();

   if (info->width==0 || info->height==0)
      {
      myrequest(texfile,&map,TRUE,0);

      info->width=map.xsize;
      info->height=map.ysize;

      if (CONFIGURE_DONTFREE==0) map.release();
      }

   *tsizex=info->width;
   *tsizey=info->height;
   }

// concatenate source file name
char *datacache::sourcefilename(const char *id,const char *filename)
   {
   char *name;

   if (filename==NULL) ERRORMSG();
   if (strlen(filename)==0) ERRORMSG();

   // concat id and filename
   if (id==NULL) name=strdup(filename);
   else name=strcct(id,filename);

   return(name);
   }

// construct a local filename for a remote file
char *datacache::localfilename(const char *filename)
   {
   unsigned int i;

   char *name,*path;

   name=strdup(filename);

   if (!LOCAL)
      {
      // replace bad characters
      for (i=0; i<strlen(name); i++)
         if (name[i]=='/' || name[i]=='\\' || name[i]==':' || name[i]=='~') name[i]='_';

      // concat local destination path
      if (LPATH!=NULL)
         {
         path=strcct(LPATH,name);
         free(name);
         name=path;
         }
      }
   else
      // concat local source path
      if (RURL!=NULL)
         {
         path=strcct(RURL,name);
         free(name);
         name=path;
         }

   return(name);
   }

// check whether or not a remote file is already stored in the local file cache
BOOLINT datacache::localfilecheck(const char *filename)
   {
   char *localname;

   fileinfoelem *info;

   BOOLINT islocal;

   info=checkfilename(filename);

   if (info!=NULL) return(info->islocal);
   else
      {
      localname=localfilename(filename);
      islocal=(checkfile(localname)!=0);
      free(localname);

      return(islocal);
      }
   }

// insert a filename into the hash table
void datacache::insertfilename(const char *filename,
                               BOOLINT isavailable,BOOLINT islocal,
                               BOOLINT istile,BOOLINT istexture,
                               float minvalue,float maxvalue,
                               int width,int height,
                               BOOLINT tailinsert,
                               BOOLINT dontupdate)
   {
   int i;

   unsigned int hash,index;

   fileinfoelem *info,*last;

   if (CLOUD!=NULL) CLOUD->lockthread();

   if (HASHTABLE==NULL)
      {
      HASHTABLE=new fileinfoelem *[HASHSIZE];
      for (i=0; i<HASHSIZE; i++) HASHTABLE[i]=NULL;
      }

   hash=hashsum(filename);
   index=hash%HASHSIZE;

   info=HASHTABLE[index];

   while (info!=NULL)
      {
      if (hash==info->hash)
         if (strcmp(filename,info->filename)==0)
            {
            if (!dontupdate)
               {
               info->isavailable=isavailable;
               info->islocal=islocal;
               }

            if (CLOUD!=NULL) CLOUD->unlockthread();

            return;
            }

      info=info->next;
      }

   info=new fileinfoelem;

   info->filename=strdup(filename);
   info->hash=hash;

   info->isavailable=isavailable;
   info->islocal=islocal;

   info->istile=istile;
   info->istexture=istexture;

   info->minvalue=minvalue;
   info->maxvalue=maxvalue;

   info->width=width;
   info->height=height;

   if (!tailinsert || HASHTABLE[index]==NULL)
      {
      info->next=HASHTABLE[index];
      HASHTABLE[index]=info;
      }
   else
      {
      last=HASHTABLE[index];
      while (last->next!=NULL) last=last->next;

      last->next=info;
      info->next=NULL;
      }

   if (CLOUD!=NULL) CLOUD->unlockthread();
   }

// check for a filename in the hash table
fileinfoelem *datacache::checkfilename(const char *filename)
   {
   unsigned int hash,index;

   fileinfoelem *info;

   if (HASHTABLE==NULL) return(NULL);

   if (CLOUD!=NULL) CLOUD->lockthread();

   hash=hashsum(filename);
   index=hash%HASHSIZE;

   info=HASHTABLE[index];

   while (info!=NULL)
      {
      if (hash==info->hash)
         if (strcmp(filename,info->filename)==0)
            {
            if (CLOUD!=NULL) CLOUD->unlockthread();

            return(info);
            }

      info=info->next;
      }

   if (CLOUD!=NULL) CLOUD->unlockthread();

   return(NULL);
   }

// compute hash sum of a string
unsigned int datacache::hashsum(const char *str) const
   {
   static const unsigned int hashconst=271;

   const char *ptr;

   unsigned int hash;

   hash=0;

   for (ptr=str; *ptr!='\0'; ptr++) hash=hashconst*(hash+*ptr)+hash/HASHSIZE;

   return(hash);
   }

// static callback wrappers:

void datacache::mystaticrequest(const unsigned char *mapfile,databuf *map,int istexture,int background,void *data)
   {
   datacache *mycache=(datacache *)data;
   mycache->myrequest(mapfile,map,istexture,background);
   }

int datacache::mystaticcheck(const unsigned char *mapfile,int istexture,void *data)
   {
   datacache *mycache=(datacache *)data;
   return(mycache->mycheck(mapfile,istexture));
   }

int datacache::mystaticinquiry(int col,int row,const unsigned char *mapfile,int hlod,void *data,float *minvalue,float *maxvalue)
   {
   datacache *mycache=(datacache *)data;
   return(mycache->myinquiry(col,row,mapfile,hlod,minvalue,maxvalue));
   }

void datacache::mystaticquery(int col,int row,const unsigned char *texfile,int tlod,void *data,int *tsizex,int *tsizey)
   {
   datacache *mycache=(datacache *)data;
   mycache->myquery(col,row,texfile,tlod,tsizex,tsizey);
   }

// configuring:

void datacache::configure_dontfree(int dontfree)
   {
   CONFIGURE_DONTFREE=dontfree;
   if (CLOUD!=NULL) CLOUD->configure_dontfree(dontfree);
   }

void datacache::configure_locthreads(int locthreads)
   {
   if (locthreads>CONFIGURE_NETTHREADS) locthreads=CONFIGURE_NETTHREADS;
   CONFIGURE_LOCTHREADS=locthreads;
   }

void datacache::configure_netthreads(int netthreads)
   {
   CONFIGURE_NETTHREADS=netthreads;
   if (CONFIGURE_LOCTHREADS>CONFIGURE_NETTHREADS) CONFIGURE_LOCTHREADS=CONFIGURE_NETTHREADS;
   }

void datacache::configure_autolockio(int autolockio)
   {CONFIGURE_AUTOLOCKIO=autolockio;}
