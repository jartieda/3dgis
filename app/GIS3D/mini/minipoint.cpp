// (c) by Stefan Roettger

#include "miniio.h"

#include "miniutm.h"

#include "miniOGL.h"
#include "minitext.h"

#include "minisort.h"

#include "lunascan.h"

#include "minipoint.h"

float minipoint::SHELLSORT_X,minipoint::SHELLSORT_Y,minipoint::SHELLSORT_ELEV;
float minipoint::SHELLSORT_DX,minipoint::SHELLSORT_DY,minipoint::SHELLSORT_DE;

minipointrndr minipoint::RNDR_NONE;
minipointrndr_signpost minipoint::RNDR_SIGNPOST;

minipointrndr_brick minipoint::RNDR_BRICK[4]=
   {
   minipointrndr_brick(minipointopts::OPTION_TYPE_BRICK1),
   minipointrndr_brick(minipointopts::OPTION_TYPE_BRICK2),
   minipointrndr_brick(minipointopts::OPTION_TYPE_BRICK3),
   minipointrndr_brick(minipointopts::OPTION_TYPE_BRICK4)
   };

// default constructor
minipointopts::minipointopts()
   {
   type=minipointopts::OPTION_TYPE_ANY;

   signpostsize=0.0f;
   signpostheight=0.0f;
   signpostnoauto=FALSE;
   signpostturn=0.0f;
   signpostincline=0.0f;
   signpostalpha=0.0f;
   signpostrange=0.0f;

   brickfile=NULL;
   bricksize=0.0f;
   brickturn=0.0f;
   brickincline=0.0f;
   brickcolor_red=0.0f;
   brickcolor_green=0.0f;
   brickcolor_blue=0.0f;
   brickalpha=0.0f;
   brickradius=0.0f;
   brickceiling=0.0f;
   bricklods=0;
   brickstagger=0.0f;
   brickstripes=0.0f;
   brickloaded=FALSE;
   brickindex=-1;

   datafile=NULL;
   datasize=0.0f;
   dataturn=0.0f;
   dataincline=0.0f;
   datacolor_red=0.0f;
   datacolor_green=0.0f;
   datacolor_blue=0.0f;
   dataalpha=0.0f;
   datarange=0.0f;
   dataloaded=FALSE;
   dataindex=-1;

   data=NULL;
   }

// destructor
minipointopts::~minipointopts()
   {
   if (brickfile!=NULL) free(brickfile);
   if (datafile!=NULL) free(datafile);
   if (data!=NULL) free(data);
   }

// default constructor
minipoint::minipoint(minitile *tile)
   {
   TILE=tile;

   POINTS=NULL;
   PNUM=0;

   VPOINTS=NULL;
   MAXVNUM=0;
   VNUM=0;

   RNDRS=NULL;
   MAXRNUM=0;
   RNUM=0;

   LOOKUP=FALSE;

   CACHE=NULL;
   ALTPATH=NULL;

   TAKEN=TRANS=FALSE;

   OFFSETLAT=OFFSETLON=0.0f;
   SCALEX=SCALEY=SCALEELEV=1.0f;

   BRICKNAME=NULL;

   registerrndr(&RNDR_NONE);
   registerrndr(&RNDR_SIGNPOST);

   registerrndr(&RNDR_BRICK[0]);
   registerrndr(&RNDR_BRICK[1]);
   registerrndr(&RNDR_BRICK[2]);
   registerrndr(&RNDR_BRICK[3]);

   CONFIGURE_SRCDATUM=3; // WGS84
   CONFIGURE_DSTZONE=0; // LatLon
   CONFIGURE_DSTDATUM=3; // WGS84
   CONFIGURE_AUTOMAP=0;

   CONFIGURE_SIGNPOSTALPHA=0.5f;

   CONFIGURE_BRICKALPHA=0.5f;
   CONFIGURE_BRICKCEILING=0.0f;
   CONFIGURE_BRICKLODS=16;
   CONFIGURE_BRICKSTAGGER=1.25f;
   CONFIGURE_BRICKPASSES=1;
   CONFIGURE_BRICKSTRIPES=0.0f;
   }

// destructor
minipoint::~minipoint()
   {
   int i,j;

   if (POINTS!=NULL)
      {
      for (i=0; i<COLS*ROWS; i++)
         {
         for (j=0; j<NUM[i]; j++)
            {
            if (POINTS[i][j].desc!=NULL) free(POINTS[i][j].desc);
            if (POINTS[i][j].meta!=NULL) free(POINTS[i][j].meta);
            if (POINTS[i][j].comment!=NULL) free(POINTS[i][j].comment);
            if (POINTS[i][j].system!=NULL) free(POINTS[i][j].system);
            if (POINTS[i][j].latitude!=NULL) free(POINTS[i][j].latitude);
            if (POINTS[i][j].longitude!=NULL) free(POINTS[i][j].longitude);
            if (POINTS[i][j].elevation!=NULL) free(POINTS[i][j].elevation);

            if (POINTS[i][j].opts!=NULL) delete POINTS[i][j].opts;
            }

         free(POINTS[i]);
         }

      free(POINTS);

      free(MAXNUM);
      free(NUM);
      }

   if (VPOINTS!=NULL) free(VPOINTS);

   if (RNDRS!=NULL) free(RNDRS);

   if (ALTPATH!=NULL) free(ALTPATH);

   if (BRICKNAME!=NULL) free(BRICKNAME);
   }

// register renderer
void minipoint::registerrndr(minipointrndr *rndr)
   {
   if (RNUM>=MAXRNUM)
      {
      MAXRNUM=2*MAXRNUM+1;

      if (RNDRS==NULL)
         {if ((RNDRS=(minipointrndr **)malloc(MAXRNUM*sizeof(minipointrndr *)))==NULL) ERRORMSG();}
      else
         {if ((RNDRS=(minipointrndr **)realloc(RNDRS,MAXRNUM*sizeof(minipointrndr *)))==NULL) ERRORMSG();}
      }

   RNDRS[RNUM]=rndr;

   RNUM++;
   }

// add waypoint
BOOLINT minipoint::add(minipointdata *point)
   {
   int i;

   float posx,posy;
   int col,row;

   if (TILE==NULL) ERRORMSG();

   if (POINTS==NULL)
      {
      COLS=TILE->getcols();
      ROWS=TILE->getrows();

      if ((POINTS=(minipointdata **)malloc(COLS*ROWS*sizeof(minipointdata *)))==NULL) ERRORMSG();

      if ((NUM=(int *)malloc(COLS*ROWS*sizeof(int)))==NULL) ERRORMSG();
      if ((MAXNUM=(int *)malloc(COLS*ROWS*sizeof(int)))==NULL) ERRORMSG();

      for (i=0; i<COLS*ROWS; i++)
         {
         POINTS[i]=NULL;
         MAXNUM[i]=NUM[i]=0;
         }
      }

   posx=(point->x-TILE->getcenterx())/TILE->getcoldim()+COLS/2.0f;
   posy=(-point->y-TILE->getcenterz())/TILE->getrowdim()+ROWS/2.0f;

   if (posx<0.0f || posx>COLS || posy<0.0f || posy>ROWS) return(FALSE);

   col=ftrc(posx);
   row=ftrc(posy);

   if (col==COLS) col--;
   if (row==ROWS) row--;

   if (NUM[col+row*COLS]>=MAXNUM[col+row*COLS])
      {
      MAXNUM[col+row*COLS]=2*MAXNUM[col+row*COLS]+1;

      if (POINTS[col+row*COLS]==NULL)
         {if ((POINTS[col+row*COLS]=(minipointdata *)malloc(MAXNUM[col+row*COLS]*sizeof(minipointdata)))==NULL) ERRORMSG();}
      else
         {if ((POINTS[col+row*COLS]=(minipointdata *)realloc(POINTS[col+row*COLS],MAXNUM[col+row*COLS]*sizeof(minipointdata)))==NULL) ERRORMSG();}
      }

   point->number=PNUM++;

   if (strlen(point->comment)>0) parsecomment(point);

   POINTS[col+row*COLS][NUM[col+row*COLS]++]=*point;

   return(TRUE);
   }

// add character to string
char *minipoint::addch(char *str,char ch)
   {
   const int space=8;
   char *str2;

   if (str==NULL)
      {
      if ((str2=(char *)malloc(space))==NULL) ERRORMSG();
      str2[0]='\0';
      }
   else if (strlen(str)%space>=space-1)
      {
      if ((str2=(char *)malloc(strlen(str)+1+space))==NULL) ERRORMSG();
      strcpy(str2,str);
      free(str);
      }
   else str2=str;

   str2[strlen(str2)+1]='\0';
   str2[strlen(str2)]=ch;

   return(str2);
   }

// parse comment
void minipoint::parsecomment(minipointdata *point)
   {
   lunascan scanner;

   scanner.addtoken("$",minipointopts::OPTION_DELIMITER);

   scanner.addtoken("=",minipointopts::OPTION_EQ);
   scanner.addtoken(":=",minipointopts::OPTION_EQ);

   scanner.addtoken("type",minipointopts::OPTION_TYPE);

   scanner.addtoken("signpostsize",minipointopts::OPTION_SIGNPOSTSIZE);
   scanner.addtoken("signpostheight",minipointopts::OPTION_SIGNPOSTHEIGHT);
   scanner.addtoken("signpostnoauto",minipointopts::OPTION_SIGNPOSTNOAUTO);
   scanner.addtoken("signpostturn",minipointopts::OPTION_SIGNPOSTTURN);
   scanner.addtoken("signpostincline",minipointopts::OPTION_SIGNPOSTINCLINE);
   scanner.addtoken("signpostalpha",minipointopts::OPTION_SIGNPOSTALPHA);

   scanner.addtoken("brickfile",minipointopts::OPTION_BRICKFILE);
   scanner.addtoken("bricksize",minipointopts::OPTION_BRICKSIZE);
   scanner.addtoken("brickturn",minipointopts::OPTION_BRICKTURN);
   scanner.addtoken("brickincline",minipointopts::OPTION_BRICKINCLINE);
   scanner.addtoken("brickcolor_red",minipointopts::OPTION_BRICKCOLOR_RED);
   scanner.addtoken("brickcolor_green",minipointopts::OPTION_BRICKCOLOR_GREEN);
   scanner.addtoken("brickcolor_blue",minipointopts::OPTION_BRICKCOLOR_BLUE);
   scanner.addtoken("brickalpha",minipointopts::OPTION_BRICKALPHA);

   scanner.addtoken("datafile",minipointopts::OPTION_DATAFILE);
   scanner.addtoken("datasize",minipointopts::OPTION_DATASIZE);
   scanner.addtoken("dataturn",minipointopts::OPTION_DATATURN);
   scanner.addtoken("dataincline",minipointopts::OPTION_DATAINCLINE);
   scanner.addtoken("datacolor_red",minipointopts::OPTION_DATACOLOR_RED);
   scanner.addtoken("datacolor_green",minipointopts::OPTION_DATACOLOR_GREEN);
   scanner.addtoken("datacolor_blue",minipointopts::OPTION_DATACOLOR_BLUE);
   scanner.addtoken("dataalpha",minipointopts::OPTION_DATAALPHA);
   scanner.addtoken("datarange",minipointopts::OPTION_DATARANGE);

   scanner.setcode(point->comment);

   // options are of the form $option=value
   while (scanner.gettoken()!=lunascan::LUNA_END)
      if (scanner.gettoken()==minipointopts::OPTION_DELIMITER)
         {
         scanner.next();
         parseoption(point,&scanner);
         }
      else scanner.next();
   }

// parse option
void minipoint::parseoption(minipointdata *point,lunascan *scanner)
   {
   int option;
   float value;
   char *name;

   option=scanner->gettoken();
   scanner->next();

   if (scanner->gettoken()==minipointopts::OPTION_EQ)
      {
      scanner->next();

      if (scanner->gettoken()==lunascan::LUNA_VALUE ||
          scanner->gettoken()==lunascan::LUNA_STRING)
         {
         value=scanner->getvalue();

         name=scanner->getstring();
         if (strcmp(name,"")==0) name=NULL;

         if (point->opts==NULL) point->opts=new minipointopts;

         switch (option)
            {
            case minipointopts::OPTION_TYPE: point->opts->type=ftrc(value+0.5f); break;
            case minipointopts::OPTION_SIGNPOSTSIZE: point->opts->signpostsize=value; break;
            case minipointopts::OPTION_SIGNPOSTHEIGHT: point->opts->signpostheight=value; break;
            case minipointopts::OPTION_SIGNPOSTNOAUTO: point->opts->signpostnoauto=ftrc(value+0.5f); break;
            case minipointopts::OPTION_SIGNPOSTTURN: point->opts->signpostturn=value; break;
            case minipointopts::OPTION_SIGNPOSTINCLINE: point->opts->signpostincline=value; break;
            case minipointopts::OPTION_SIGNPOSTALPHA: point->opts->signpostalpha=value; break;
            case minipointopts::OPTION_BRICKFILE: if (name!=NULL) point->opts->brickfile=strdup(name); break;
            case minipointopts::OPTION_BRICKSIZE: point->opts->bricksize=value; break;
            case minipointopts::OPTION_BRICKTURN: point->opts->brickturn=value; break;
            case minipointopts::OPTION_BRICKINCLINE: point->opts->brickincline=value; break;
            case minipointopts::OPTION_BRICKCOLOR_RED: point->opts->brickcolor_red=value; break;
            case minipointopts::OPTION_BRICKCOLOR_GREEN: point->opts->brickcolor_green=value; break;
            case minipointopts::OPTION_BRICKCOLOR_BLUE: point->opts->brickcolor_blue=value; break;
            case minipointopts::OPTION_BRICKALPHA: point->opts->brickalpha=value; break;
            case minipointopts::OPTION_DATAFILE: if (name!=NULL) point->opts->datafile=strdup(name); break;
            case minipointopts::OPTION_DATASIZE: point->opts->datasize=value; break;
            case minipointopts::OPTION_DATATURN: point->opts->dataturn=value; break;
            case minipointopts::OPTION_DATAINCLINE: point->opts->dataincline=value; break;
            case minipointopts::OPTION_DATACOLOR_RED: point->opts->datacolor_red=value; break;
            case minipointopts::OPTION_DATACOLOR_GREEN: point->opts->datacolor_green=value; break;
            case minipointopts::OPTION_DATACOLOR_BLUE: point->opts->datacolor_blue=value; break;
            case minipointopts::OPTION_DATAALPHA: point->opts->dataalpha=value; break;
            case minipointopts::OPTION_DATARANGE: point->opts->datarange=value; break;
            }
         }

      scanner->next();
      }
   }

// set file cache
void minipoint::setcache(datacache *cache,const char *altpath)
   {
   CACHE=cache;

   if (ALTPATH!=NULL) free(ALTPATH);
   ALTPATH=NULL;

   if (altpath!=NULL) ALTPATH=strdup(altpath);
   }

// get file
char *minipoint::getfile(const char *filename,const char *altpath)
   {
   if (checkfile(filename)!=0) return(strdup(filename));

   if (CACHE!=NULL) return(CACHE->getfile(filename,altpath));

   return(NULL);
   }

// load waypoints
void minipoint::load(const char *filename,
                     float offsetlat,float offsetlon,
                     float scalex,float scaley,float scaleelev,
                     minitile *tile,
                     char delimiter)
   {
   unsigned int i;

   char *wpname;

   FILE *file;

   int ch;

   minipointdata point;

   int zone;

   if (tile!=NULL)
      if (TILE==NULL) TILE=tile;
      else if (tile!=TILE) ERRORMSG();

   if (TILE==NULL) ERRORMSG();

   if (TILE->getwarp()!=NULL)
      {
      if (TILE->getwarp()->getdat()==minicoord::MINICOORD_LLH)
         {
         CONFIGURE_DSTZONE=0;
         TAKEN=TRUE;
         }
      else if (TILE->getwarp()->getdat()==minicoord::MINICOORD_UTM)
         {
         CONFIGURE_DSTZONE=TILE->getwarp()->getutmzone();
         CONFIGURE_DSTDATUM=TILE->getwarp()->getutmdatum();

         TAKEN=TRUE;
         }
      }

   wpname=getfile(filename,ALTPATH);

   if (wpname==NULL) return;

   if ((file=fopen(wpname,"rb"))==NULL)
      {
      free(wpname);
      return;
      }

   free(wpname);

   ch=fgetc(file);

   while (ch=='\n' || ch=='\r') ch=fgetc(file);

   while (ch!=EOF)
      {
      point.x=point.y=0.0f;
      point.elev=point.height=0.0f;

      point.desc=point.meta=point.comment=NULL;
      point.system=point.latitude=point.longitude=point.elevation=NULL;

      point.zone=point.datum=0;

      point.opts=NULL;
      point.rndr=NULL;

      // read full description
      while (ch!='\n' && ch!='\r' && ch!=EOF)
         {
         point.meta=addch(point.meta,ch);
         ch=fgetc(file);
         }

      if (point.meta==NULL) ERRORMSG();

      // skip end of line
      while (ch=='\n' || ch=='\r') ch=fgetc(file);

      // copy description without comment
      for (i=0; i<strlen(point.meta); i++)
         {
         if (point.meta[i]==delimiter) break;
         point.desc=addch(point.desc,point.meta[i]);
         }

      if (point.desc==NULL) point.desc=addch(point.desc,'\0');

      // read lines starting with comment delimiter
      while (ch==delimiter)
         {
         ch=fgetc(file);

         while (ch!='\n' && ch!='\r' && ch!=EOF)
            {
            point.comment=addch(point.comment,ch);
            ch=fgetc(file);
            }

         while (ch=='\n' || ch=='\r')
            {
            if (ch=='\n') point.comment=addch(point.comment,ch);
            ch=fgetc(file);
            }
         }

      if (point.comment==NULL) point.comment=addch(point.comment,'\0');

      // read coordinate system
      while (ch!='\n' && ch!='\r' && ch!=EOF)
         {
         point.system=addch(point.system,ch);
         ch=fgetc(file);
         }

      // skip end of line
      while (ch=='\n' || ch=='\r') ch=fgetc(file);

      if (point.system==NULL) ERRORMSG();

      // read Lat/Lon coordinates
      if (strcmp(point.system,"LL")==0)
         {
         while (ch!='\n' && ch!='\r' && ch!=EOF)
            {
            point.latitude=addch(point.latitude,ch);
            ch=fgetc(file);
            }

         while (ch=='\n' || ch=='\r') ch=fgetc(file);

         if (point.latitude==NULL) ERRORMSG();

         while (ch!='\n' && ch!='\r' && ch!=EOF)
            {
            point.longitude=addch(point.longitude,ch);
            ch=fgetc(file);
            }

         while (ch=='\n' || ch=='\r') ch=fgetc(file);

         if (point.longitude==NULL) ERRORMSG();

         if (sscanf(point.latitude,"%g",&point.y)!=1) ERRORMSG();
         if (sscanf(point.longitude,"%g",&point.x)!=1) ERRORMSG();

         point.x*=60*60;
         point.y*=60*60;

         point.x=LONSUB(point.x);
         if (point.y<-90*60*60 || point.y>90*60*60) ERRORMSG();

         if (CONFIGURE_AUTOMAP!=0 && !TAKEN)
            {
            CONFIGURE_DSTZONE=0;
            TAKEN=TRUE;
            }
         }
      // read UTM coordinates
      else if (sscanf(point.system,"UTM %d",&zone)==1)
         {
         if (zone==0 || zone<-60 || zone>60) ERRORMSG();

         point.zone=zone;
         point.datum=CONFIGURE_SRCDATUM;

         while (ch!='\n' && ch!='\r' && ch!=EOF)
            {
            point.longitude=addch(point.longitude,ch);
            ch=fgetc(file);
            }

         while (ch=='\n' || ch=='\r') ch=fgetc(file);

         if (point.longitude==NULL) ERRORMSG();

         while (ch!='\n' && ch!='\r' && ch!=EOF)
            {
            point.latitude=addch(point.latitude,ch);
            ch=fgetc(file);
            }

         while (ch=='\n' || ch=='\r') ch=fgetc(file);

         if (point.latitude==NULL) ERRORMSG();

         if (sscanf(point.longitude,"%g",&point.x)!=1) ERRORMSG();
         if (sscanf(point.latitude,"%g",&point.y)!=1) ERRORMSG();

         miniutm::UTM2LL(point.x,point.y,zone,CONFIGURE_SRCDATUM,&point.y,&point.x);

         if (CONFIGURE_AUTOMAP!=0 && !TAKEN)
            {
            CONFIGURE_DSTZONE=zone;
            CONFIGURE_DSTDATUM=CONFIGURE_SRCDATUM;

            TAKEN=TRUE;
            }
         }
      else ERRORMSG();

      // read elevation
      while (ch!='\n' && ch!='\r' && ch!=EOF)
         {
         point.elevation=addch(point.elevation,ch);
         ch=fgetc(file);
         }

      // skip end of line
      while (ch=='\n' || ch=='\r') ch=fgetc(file);

      if (point.elevation==NULL) ERRORMSG();

      if (sscanf(point.elevation,"%gm",&point.elev)!=1)
         if (sscanf(point.elevation,"%gf",&point.elev)!=1) ERRORMSG();
         else point.elev*=0.3048f; // feet to meters

      if (CONFIGURE_DSTZONE==0)
         {
         point.x=LONSUB(point.x,-offsetlon);
         point.y+=offsetlat;

         if (point.y<-90*60*60 || point.y>90*60*60) ERRORMSG();
         }
      else
         {
         miniutm::LL2UTM(point.y,point.x,CONFIGURE_DSTZONE,CONFIGURE_DSTDATUM,&point.x,&point.y);

         point.x+=offsetlon;
         point.y+=offsetlat;
         }

      point.x*=scalex;
      point.y*=scaley;

      point.elev*=scaleelev;
      point.height=-MAXFLOAT;

      if (!add(&point))
         {
         if (point.desc!=NULL) free(point.desc);
         if (point.meta!=NULL) free(point.meta);
         if (point.comment!=NULL) free(point.comment);
         if (point.system!=NULL) free(point.system);
         if (point.latitude!=NULL) free(point.latitude);
         if (point.longitude!=NULL) free(point.longitude);
         if (point.elevation!=NULL) free(point.elevation);

         if (point.opts!=NULL) delete point.opts;
         }
      }

   fclose(file);

   if (!TRANS)
      {
      OFFSETLAT=offsetlat;
      OFFSETLON=offsetlon;

      SCALEX=scalex;
      SCALEY=scaley;
      SCALEELEV=scaleelev;

      TRANS=TRUE;
      }
   }

// get waypoint
minipointdata *minipoint::getpoint(int p)
   {
   int i,j,k;

   if (TILE==NULL) return(NULL);

   for (i=0; i<COLS; i++)
      for (j=0; j<ROWS; j++)
         for (k=0; k<NUM[i+j*COLS]; k++)
            if (p==POINTS[i+j*COLS][k].number) return(&POINTS[i+j*COLS][k]);

   return(NULL);
   }

// lookup renderers
void minipoint::lookuprndrs()
   {
   int i,j,k,l;

   minipointdata *point;

   int type;

   if (TILE==NULL) return;

   if (!LOOKUP)
      {
      for (i=0; i<COLS; i++)
         for (j=0; j<ROWS; j++)
            for (k=0; k<NUM[i+j*COLS]; k++)
               {
               point=&POINTS[i+j*COLS][k];

               if (point->opts==NULL) type=minipointopts::OPTION_TYPE_ANY;
               else type=point->opts->type;

               for (l=0; l<RNUM; l++)
                  if (RNDRS[l]->gettype()==type)
                     {
                     point->rndr=RNDRS[l];
                     break;
                     }
               }

      LOOKUP=TRUE;
      }
   }

// calculate visible waypoints
void minipoint::calcvdata()
   {
   int i,j,k;

   minipointdata *point;

   if (TILE==NULL) return;

   VNUM=0;

   for (i=TILE->getvisibleleft(); i<=TILE->getvisibleright(); i++)
      for (j=TILE->getvisiblebottom(); j<=TILE->getvisibletop(); j++)
         for (k=0; k<NUM[i+j*COLS]; k++)
            {
            point=&POINTS[i+j*COLS][k];

            point->height=TILE->getheight(point->x,-point->y);

            if (VNUM>=MAXVNUM)
               {
               MAXVNUM=2*MAXVNUM+1;

               if (VPOINTS==NULL)
                  {if ((VPOINTS=(minipointdata **)malloc(MAXVNUM*sizeof(minipointdata *)))==NULL) ERRORMSG();}
               else
                  {if ((VPOINTS=(minipointdata **)realloc(VPOINTS,MAXVNUM*sizeof(minipointdata *)))==NULL) ERRORMSG();}
               }

            VPOINTS[VNUM]=point;

            VNUM++;
            }
   }

// sort visible waypoints along viewing axis
void minipoint::sortvdata(float x,float y,float elev,
                          float dx,float dy,float de)
   {
   if (VPOINTS!=NULL)
      {
      SHELLSORT_X=x;
      SHELLSORT_Y=y;
      SHELLSORT_ELEV=elev;

      SHELLSORT_DX=dx;
      SHELLSORT_DY=dy;
      SHELLSORT_DE=de;

      shellsort<minipointdata>(VPOINTS,VNUM);
      }
   }

// comparison operator of waypoint data
inline int operator < (const minipointdata &a,const minipointdata &b)
   {
   float d1,d2;
   int type1,type2;

   if (a.opts==NULL) type1=minipointopts::OPTION_TYPE_ANY;
   else type1=a.opts->type;

   if (b.opts==NULL) type2=minipointopts::OPTION_TYPE_ANY;
   else type2=b.opts->type;

   if (type1==type2)
      {
      d1=minipoint::SHELLSORT_DX*(a.x-minipoint::SHELLSORT_X)+minipoint::SHELLSORT_DY*(a.y-minipoint::SHELLSORT_Y)+minipoint::SHELLSORT_DE*(a.height-minipoint::SHELLSORT_ELEV);
      d2=minipoint::SHELLSORT_DX*(b.x-minipoint::SHELLSORT_X)+minipoint::SHELLSORT_DY*(b.y-minipoint::SHELLSORT_Y)+minipoint::SHELLSORT_DE*(b.height-minipoint::SHELLSORT_ELEV);

      return(d1>d2);
      }
   else return(type1<type2);
   }

// get nearest waypoint
minipointdata *minipoint::getnearest(float x,float y,float elev)
   {
   int i;

   minipointdata **vpoint,*nearest;

   if (VPOINTS==NULL) return(NULL);

   vpoint=VPOINTS;
   nearest=NULL;

   for (i=0; i<VNUM; i++,vpoint++)
      if (nearest==NULL) nearest=*vpoint;
      else if (getdistance2(x,y,elev,*vpoint)<getdistance2(x,y,elev,nearest)) nearest=*vpoint;

   return(nearest);
   }

// get squared distance to waypoint
float minipoint::getdistance2(float x,float y,float elev,minipointdata *point)
   {return(fsqr(point->x-x)+fsqr(point->y-y)+fsqr(point->height-elev));}

// render waypoints
void minipoint::draw(float ex,float ey,float ez,
                     float dx,float dy,float dz,
                     float nearp,float farp,float fovy,float aspect,
                     double time,minipointopts *global,
                     minipointrndr *fallback)
   {
   int i,j;

   int p1,p2;
   minipointrndr *rndr;

   // lookup renderers
   lookuprndrs();

   // calculate visible points
   calcvdata();

   // check if any points were found
   if (VNUM==0) return;

   // sort visible points
   sortvdata(ex,ez,ey,dx,dz,dy);

   // process sorted points
   for (p1=0; p1<VNUM; p1=p2)
      {
      // get renderer
      rndr=VPOINTS[p1]->rndr;

      // search points until renderer differs
      for (p2=p1+1; p2<VNUM; p2++)
         if (VPOINTS[p2]->rndr!=rndr) break;

      // use fallback renderer if necessary
      if (rndr==NULL) rndr=fallback;

      // call renderer for each point and pass
      if (rndr!=NULL)
         {
         rndr->init(this,
                    ex,ey,ez,
                    dx,dy,dz,
                    nearp,farp,fovy,aspect,
                    time,global);

         for (i=1; i<=rndr->getpasses(); i++)
            {
            rndr->pre(i);
            for (j=p1; j<p2; j++) rndr->render(VPOINTS[j],i);
            rndr->post(i);
            }

         rndr->exit();
         }
      }
   }

// render waypoints with signposts
void minipoint::drawsignposts(float ex,float ey,float ez,
                              float height,float range,
                              float turn,float yon)
   {
   float dx,dy,dz;

   float sint,cost;
   float siny,cosy;

   minipointopts global;

   sint=sin(2.0f*PI/360.0f*turn);
   cost=cos(2.0f*PI/360.0f*turn);

   siny=sin(2.0f*PI/360.0f*yon);
   cosy=cos(2.0f*PI/360.0f*yon);

   dx=cosy*sint;
   dz=cosy*cost;
   dy=siny;

   global.signpostsize=height;
   global.signpostheight=height;
   global.signpostrange=range;
   global.signpostturn=turn;
   global.signpostincline=yon;

   global.signpostalpha=CONFIGURE_SIGNPOSTALPHA;

   draw(ex,ey,ez,
        dx,dy,dz,
        0.0f,MAXFLOAT,90.0f,1.0f,
        0.0,&global,
        &RNDR_SIGNPOST);
   }

// set default brick file name
void minipoint::setbrick(const char *filename)
   {
   if (filename==NULL) ERRORMSG();

   if (BRICKNAME!=NULL) free(BRICKNAME);
   BRICKNAME=strdup(filename);
   }

// get default brick file name
char *minipoint::getbrick()
   {return(BRICKNAME);}

// render waypoints with bricks
void minipoint::drawbricks(float ex,float ey,float ez,
                           float brad,float farp,
                           float fovy,float aspect,
                           float size)
   {
   minipointopts global;

   global.brickradius=brad;
   global.bricksize=size;

   global.brickfile=strdup(BRICKNAME);
   global.brickalpha=CONFIGURE_BRICKALPHA;
   global.brickceiling=CONFIGURE_BRICKCEILING;
   global.bricklods=CONFIGURE_BRICKLODS;
   global.brickstagger=CONFIGURE_BRICKSTAGGER;
   global.brickstripes=CONFIGURE_BRICKSTRIPES;

   draw(ex,ey,ez,
        0.0f,0.0f,0.0f,
        0.0f,farp,fovy,aspect,
        0.0,&global,
        &RNDR_BRICK[CONFIGURE_BRICKPASSES-1]);
   }

// signpost init method
void minipointrndr_signpost::init(minipoint *points,
                                  float ex,float ey,float ez,
                                  float dx,float dy,float dz,
                                  float nearp,float farp,float fovy,float aspect,
                                  double time,minipointopts *global)
   {
   if (dx==MAXFLOAT || dy==MAXFLOAT || dz==MAXFLOAT ||
       nearp<=0.0f || farp<=0.0f || fovy<=0.0f || aspect<=0.0f ||
       time<0.0) ERRORMSG();

   POINTS=points;

   EX=ex;
   EY=ey;
   EZ=ez;

   GLOBAL=global;

   SCALEELEV=points->getscaleelev();

   NEAREST=points->getnearest(ex,ez,ey);

   initstate();
   disableculling();
   enableblending();
   }

// signpost pre-render method
void minipointrndr_signpost::pre(int pass)
   {
   if (pass==1)
      {
      linewidth(2);
      enablelinesmooth();

      color(0.25f,0.25f,0.5f);
      }
   else if (pass==2)
      minitext::configure_zfight(0.975f);
   }

// signpost rendering method
void minipointrndr_signpost::render(minipointdata *vpoint,int pass)
   {
   const int maxinfo=1000;
   static char info[maxinfo];

   float sheight,ssize;
   float sturn,syon;
   float salpha;

   // mark waypoint with a post
   if (pass==1)
      {
      sheight=GLOBAL->signpostheight;

      if (vpoint->opts!=NULL)
         if (vpoint->opts->signpostheight>0.0f) sheight=vpoint->opts->signpostheight*SCALEELEV;

      drawline(vpoint->x,vpoint->height,-vpoint->y,vpoint->x,vpoint->height+sheight,-vpoint->y);
      }
   // label waypoint within range
   else if (pass==2)
      {
      // check distance
      if (POINTS->getdistance2(EX,EZ,EY,vpoint)>fsqr(GLOBAL->signpostrange)) return;

      // get global waypoint parameters
      ssize=GLOBAL->signpostsize;
      sheight=GLOBAL->signpostheight;
      sturn=GLOBAL->signpostturn;
      syon=GLOBAL->signpostincline;
      salpha=GLOBAL->signpostalpha;

      // get optional waypoint parameters
      if (vpoint->opts!=NULL)
         {
         if (vpoint->opts->signpostsize>0.0f) ssize=vpoint->opts->signpostsize*SCALEELEV;
         if (vpoint->opts->signpostheight>0.0f) sheight=vpoint->opts->signpostheight*SCALEELEV;

         if (vpoint->opts->signpostnoauto)
            {
            sturn=vpoint->opts->signpostturn;
            syon=vpoint->opts->signpostincline;
            }

         if (vpoint->opts->signpostalpha>0.0f) salpha=vpoint->opts->signpostalpha;
         }

      // compile label information of waypoint within range
      if (vpoint==NEAREST)
         {
         if (vpoint->zone==0)
            snprintf(info,maxinfo,"\n %s \n\n Lat=%s Lon=%s \n Elev=%s \n",
                     vpoint->desc,
                     vpoint->latitude,vpoint->longitude,
                     vpoint->elevation);
         else
            snprintf(info,maxinfo,"\n %s \n\n %s East=%s North=%s \n Elev=%s \n",
                     vpoint->desc,
                     vpoint->system,vpoint->longitude,vpoint->latitude,
                     vpoint->elevation);

         salpha=1.0f;
         }
      else snprintf(info,maxinfo,"\n %s \n",vpoint->desc);

      // label waypoint within range
      mtxpush();
      mtxtranslate(0.0f,sheight,0.0f);
      mtxtranslate(vpoint->x,vpoint->height,-vpoint->y);
      mtxrotate(-sturn,0.0f,1.0f,0.0f);
      mtxrotate(syon,1.0f,0.0f,0.0f);
      mtxscale(2.0f*ssize,2.0f*ssize,2.0f*ssize);
      mtxtranslate(-0.5f,0.0f,0.0f);
      minitext::drawstring(1.0f,240.0f,0.5f,0.5f,1.0f,info,1.0f,salpha);
      mtxpop();
      }
   }

// signpost post-render method
void minipointrndr_signpost::post(int pass)
   {
   if (pass==1)
      linewidth(1);
   else if (pass==2)
      disablelinesmooth();
   }

// signpost exit method
void minipointrndr_signpost::exit()
   {
   disableblending();
   enableBFculling();
   exitstate();
   }

// default constructor
minipointrndr_brick::minipointrndr_brick(int type):
   minipointrndr(type,1)
   {LODS=NULL;}

// destructor
minipointrndr_brick::~minipointrndr_brick()
   {if (LODS!=NULL) delete LODS;}

// brick init method
void minipointrndr_brick::init(minipoint *points,
                               float ex,float ey,float ez,
                               float dx,float dy,float dz,
                               float nearp,float farp,float fovy,float aspect,
                               double time,minipointopts *global)
   {
   if (dx==MAXFLOAT || dy==MAXFLOAT || dz==MAXFLOAT ||
       time<0.0) ERRORMSG();

   POINTS=points;

   EX=ex;
   EY=ey;
   EZ=ez;

   NEARP=nearp;
   FARP=farp;
   FOVY=fovy;
   ASPECT=aspect;

   GLOBAL=global;

   OFFSETLAT=points->getoffsetlat();
   OFFSETLON=points->getoffsetlon();
   SCALEX=points->getscalex();
   SCALEY=points->getscaley();
   SCALEELEV=points->getscaleelev();

   // initialize brick LOD renderer
   if (LODS==NULL)
      {
      // create new brick LOD renderer
      LODS=new minilod(OFFSETLAT,OFFSETLON,SCALEX,SCALEY,SCALEELEV);

      // check if a default brick file name was set
      if (global->brickfile!=NULL)
         LODS->addbrick(global->brickfile,global->brickradius,global->bricklods,global->brickstagger);
      }

   // clear all volumes
   LODS->clearvolumes();
   }

// brick rendering method
void minipointrndr_brick::render(minipointdata *vpoint,int pass)
   {
   float bsize;

   int bindex,vindex;
   char *bname;

   float bred,bgreen,bblue,balpha;

   float midx,midy,basez;
   float color,r,g,b,a;

   float alpha,beta;
   float sa,ca,sb,cb;

   if (pass==1)
      {
      // get brick size
      if (vpoint->opts==NULL) bsize=GLOBAL->bricksize;
      else if (vpoint->opts->bricksize>0.0f) bsize=vpoint->opts->bricksize*SCALEELEV;
      else bsize=GLOBAL->bricksize;

      // get brick index
      if (vpoint->opts!=NULL) bindex=vpoint->opts->brickindex;
      else if (GLOBAL->brickfile!=NULL) bindex=0;
      else bindex=-1;

      // check for individual brick file
      if (vpoint->opts!=NULL)
         {
         if (vpoint->opts->brickfile!=NULL)
            if (!vpoint->opts->brickloaded)
               {
               bname=POINTS->getfile(vpoint->opts->brickfile);

               if (bname!=NULL)
                  {
                  bindex=vpoint->opts->brickindex=LODS->addbrick(bname,GLOBAL->brickradius,GLOBAL->bricklods,GLOBAL->brickstagger);
                  free(bname);
                  }

               vpoint->opts->brickloaded=TRUE;
               }

         if (!vpoint->opts->brickloaded)
            if (GLOBAL->brickfile!=NULL) bindex=vpoint->opts->brickindex=0;
         }

      // check for valid brick index
      if (bindex<0) return;

      // calculate position
      midx=vpoint->x/SCALEX-OFFSETLON;
      midy=vpoint->y/SCALEY-OFFSETLAT;
      basez=(vpoint->elev-0.25f*bsize)/SCALEELEV;

      // get individual brick color
      if (vpoint->opts==NULL) bred=bgreen=bblue=balpha=0.0f;
      else
         {
         bred=vpoint->opts->brickcolor_red;
         bgreen=vpoint->opts->brickcolor_green;
         bblue=vpoint->opts->brickcolor_blue;
         balpha=vpoint->opts->brickalpha;
         }

      // check for individual brick color
      if (bred>0.0f || bgreen>0.0f || bblue>0.0f)
         {
         r=bred;
         g=bgreen;
         b=bblue;
         a=balpha;

         if (a==0.0f) a=GLOBAL->brickalpha;
         }
      else
         {
         // check elevation
         if (GLOBAL->brickceiling==0.0f) color=0.0f;
         else color=basez/GLOBAL->brickceiling;

         // calculate color
         if (color<0.5f)
            {
            r=0.0f;
            g=2.0f*color;
            b=1.0f-2.0f*color;
            }
         else
            {
            r=2.0f*color-1.0f;
            g=2.0f-2.0f*color;
            b=0.0f;
            }

         a=GLOBAL->brickalpha;
         }

      // set position and color
      vindex=LODS->addvolume(bindex,
                             midx,midy,basez,
                             bsize/SCALEX,bsize/SCALEY,bsize/SCALEELEV,
                             r,g,b,a);

      // set orientation
      if (vpoint->opts!=NULL)
         {
         alpha=-vpoint->opts->brickturn*RAD;
         beta=vpoint->opts->brickincline*RAD;

         if (alpha!=0.0f || beta!=0.0f)
            {
            sa=fsin(alpha);
            ca=fcos(alpha);

            sb=fsin(beta);
            cb=fcos(beta);

            LODS->addorientation(vindex,
                                 ca,0.0f,sa,
                                 -sa*sb,cb,ca*sb,
                                 -sa*cb,-sb,ca*cb);
            }
         }
      }
   }

// brick exit method
void minipointrndr_brick::exit()
   {
   // set rendering passes
   LODS->configure_brickpasses(TYPE-minipointopts::OPTION_TYPE_BRICK1+1);

   // set stripe pattern
   LODS->configure_brickoffset(GLOBAL->brickstripes);

   // render visible bricks
   LODS->render(EX,EY,EZ,FARP,FOVY,ASPECT);
   }

// configuring:

void minipoint::configure_srcdatum(int datum)
   {CONFIGURE_SRCDATUM=datum;}

void minipoint::configure_dstzone(int zone)
   {CONFIGURE_DSTZONE=zone;}

void minipoint::configure_dstdatum(int datum)
   {CONFIGURE_DSTDATUM=datum;}

void minipoint::configure_automap(int automap)
   {CONFIGURE_AUTOMAP=automap;}

// configuring of signpost renderer:

void minipoint::configure_signpostalpha(float signpostalpha)
   {CONFIGURE_SIGNPOSTALPHA=signpostalpha;}

// configuring of brick renderer:

void minipoint::configure_brickalpha(float brickalpha)
   {CONFIGURE_BRICKALPHA=brickalpha;}

void minipoint::configure_brickceiling(float brickceiling)
   {CONFIGURE_BRICKCEILING=brickceiling;}

void minipoint::configure_bricklods(int bricklods)
   {CONFIGURE_BRICKLODS=bricklods;}

void minipoint::configure_brickstagger(float brickstagger)
   {CONFIGURE_BRICKSTAGGER=brickstagger;}

void minipoint::configure_brickpasses(float brickpasses)
   {CONFIGURE_BRICKPASSES=brickpasses;}

void minipoint::configure_brickstripes(float brickstripes)
   {CONFIGURE_BRICKSTRIPES=brickstripes;}
