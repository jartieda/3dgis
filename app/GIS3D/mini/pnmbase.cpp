// (c) by Stefan Roettger

#include "minibase.h"

#include "miniutm.h"

#include "pnmbase.h"

namespace pnmbase {

// PNMcomment implementation:

PNMcomment::PNMcomment()
   {
   maxc=MAX_COM;
   if ((com=(char *)malloc(maxc))==NULL) ERRORMSG();
   reset();
   }

PNMcomment::~PNMcomment()
   {free(com);}

void PNMcomment::reset()
   {
   c=0;
   com[c]='\0';
   }

char PNMcomment::addchar(char ch)
   {
   if (ch==' ' && c>0)
      if (com[c-1]==' ') return(ch);

   if (c>=maxc-1)
      {
      maxc*=2;
      if ((com=(char *)realloc(com,maxc))==NULL) ERRORMSG();
      }

   com[c++]=ch;
   com[c]='\0';

   return(ch);
   }

void PNMcomment::addstring(const char *str)
   {while (*str!='\0') addchar(*str++);}

void PNMcomment::addfloat(float v,int leading)
   {
   char str[MAX_STR];
   if (leading==0) snprintf(str,MAX_STR,"%g",v);
   else snprintf(str,MAX_STR,"%12f",v);
   addstring(str);
   }

void PNMcomment::addunits(int units)
   {
   if (units==0) addstring("radians");
   else if (units==1) addstring("feet");
   else if (units==2) addstring("meters");
   else if (units==3) addstring("decimeters");
   else if (units==4) addstring("arc-seconds");
   }

// write a PNM image
void writePNMimage(const char *pnmfilename,
                   unsigned char *image,
                   int width,int height,int components,
                   PNMcomment *comment)
   {
   FILE *file;

   char *com;

   if (width<1 || height<1) ERRORMSG();

   if ((file=fopen(pnmfilename,"wb"))==NULL) ERRORMSG();

   if (components==1 || components==2) fprintf(file,"P5");
   else if (components==3) fprintf(file,"P6");
   else if (components==4) fprintf(file,"P8");
   else ERRORMSG();

   if (comment!=NULL)
      {
      com=comment->str();
      fprintf(file,"\n#");
      while (*com!='\0')
         if (*com!='\n') fputc(*com++,file);
         else if (*++com!='\0') fprintf(file,"\n#");
      }

   fprintf(file,"\n%d %d\n",width,height);

   if (components==1 || components==3 || components==4) fprintf(file,"255\n");
   else fprintf(file,"32767\n");

   if (fwrite(image,width*height*components,1,file)!=1) ERRORMSG();
   fclose(file);
   }

// read a PNM image
unsigned char *readPNMimage(unsigned char *pnmimage,
                            int *width,int *height,int *components,
                            int loaded,unsigned char **ptr,
                            PNMcomment *comment)
   {
   FILE *file;

   int pnmtype,maxval;
   unsigned char *image;

   char ch;
   int val;

   PNMcomment com;

   if (loaded==0)
      {
      if ((file=fopen((char *)pnmimage,"rb"))==NULL) return(NULL);

      if (fscanf(file,"P%1d\n",&pnmtype)!=1)
         {
         fclose(file);
         return(NULL);
         }

      while ((ch=fgetc(file))=='#')
         while (com.addchar(fgetc(file))!='\n');
      ungetc(ch,file);

      if (fscanf(file,"%d %d\n",width,height)!=2) ERRORMSG();
      if (fscanf(file,"%d",&maxval)!=1) ERRORMSG();
      if (fgetc(file)!='\n') ERRORMSG();

      if (*width<1 || *height<1) ERRORMSG();

      if (pnmtype==5 && maxval==255) *components=1;
      else if (pnmtype==5 && (maxval==32767 || maxval==65535)) *components=2;
      else if (pnmtype==6 && maxval==255) *components=3;
      else if (pnmtype==8 && maxval==255) *components=4;
      else ERRORMSG();

      if ((image=(unsigned char *)malloc((*width)*(*height)*(*components)))==NULL) ERRORMSG();

      if (fread(image,(*width)*(*height)*(*components),1,file)!=1) ERRORMSG();
      fclose(file);
      }
   else
      {
      image=pnmimage;

      ch=*image++;

      if (ch!='P') return(NULL);

      ch=*image++;

      pnmtype=ch-'0';

      ch=*image++;

      if (ch!='\n') return(NULL);

      ch=*image++;

      while (ch=='#')
         {
         ch=*image++;
         while (com.addchar(ch)!='\n') ch=*image++;
         ch=*image++;
         }

      val=0;

      while (ch>='0' && ch<='9')
         {
         val=10*val+ch-'0';
         ch=*image++;
         }

      *width=val;

      if (ch!=' ') ERRORMSG();

      ch=*image++;

      val=0;

      while (ch>='0' && ch<='9')
         {
         val=10*val+ch-'0';
         ch=*image++;
         }

      *height=val;

      if (ch!='\n') ERRORMSG();

      ch=*image++;

      maxval=0;

      while (ch>='0' && ch<='9')
         {
         maxval=10*maxval+ch-'0';
         ch=*image++;
         }

      if (ch!='\n') ERRORMSG();

      if (pnmtype==5 && maxval==255) *components=1;
      else if (pnmtype==5 && (maxval==32767 || maxval==65535)) *components=2;
      else if (pnmtype==6 && maxval==255) *components=3;
      else if (pnmtype==8 && maxval==255) *components=4;
      else ERRORMSG();

      if (ptr!=NULL) *ptr=image+(*width)*(*height)*(*components);
      }

   if (comment!=NULL)
      {
      comment->reset();
      comment->addstring(com.str());
      }

   return(image);
   }

// read a PNM file
unsigned char *readPNMfile(const char *pnmfilename,
                           int *width,int *height,int *components,
                           PNMcomment *comment)
   {
   return(readPNMimage((unsigned char *)pnmfilename,
                       width,height,components,
                       0,NULL,comment));
   }

// write a PVM volume
void writePVMvolume(const char *filename,unsigned char *volume,
                    int width,int height,int depth,int components)
   {
   FILE *file;

   if (width<1 || height<1 || depth<1 || components<1) ERRORMSG();

   if ((file=fopen(filename,"wb"))==NULL) ERRORMSG();

   fprintf(file,"PVM\n%d %d %d\n%d\n",width,height,depth,components);

   if (fwrite(volume,width*height*depth*components,1,file)!=1) ERRORMSG();
   fclose(file);
   }

// read a PVM volume
unsigned char *readPVMvolume(const char *filename,
                             int *width,int *height,int *depth,int *components)
   {
   FILE *file;

   unsigned char header[5],*volume;

   if ((file=fopen(filename,"rb"))==NULL) return(NULL);

   if (fread(header,1,4,file)!=4) return(NULL);

   header[4]='\0';

   if (strcmp((char *)header,"PVM\n")!=0)
      {
      fclose(file);
      return(NULL);
      }

   if (fscanf(file,"%d %d %d\n",width,height,depth)!=3) ERRORMSG();
   if (fscanf(file,"%d",components)!=1) ERRORMSG();
   if (fgetc(file)!='\n') ERRORMSG();

   if (*width<1 || *height<1 || *depth<1 || *components<1) ERRORMSG();

   if ((volume=(unsigned char *)malloc((*width)*(*height)*(*depth)*(*components)))==NULL) ERRORMSG();

   if (fread(volume,(*width)*(*height)*(*depth)*(*components),1,file)!=1) ERRORMSG();
   fclose(file);

   return(volume);
   }

// put a geographic grid descriptor into a PNM comment
// default values correspond to USGS 1:250,000 DEM quads
void putPNMparams(PNMcomment *comment,
                  const char *pnm_description,
                  int coord_sys,
                  int coord_zone,
                  int coord_datum,
                  int coord_units,
                  float coord_SW_x,float coord_SW_y,
                  float coord_NW_x,float coord_NW_y,
                  float coord_NE_x,float coord_NE_y,
                  float coord_SE_x,float coord_SE_y,
                  float cell_size_x,float cell_size_y,
                  int scaling_units,
                  float vertical_scaling,
                  int missing_value)
   {
   if (coord_sys<0 || coord_sys>1) ERRORMSG();
   if (coord_zone<-60 || coord_zone>60) ERRORMSG();
   if (coord_units<0 || coord_units>4) ERRORMSG();
   if (cell_size_x<=0.0f || cell_size_y<=0.0f) ERRORMSG();
   if (scaling_units<1 || scaling_units>3) ERRORMSG();

   if (coord_sys==0 && coord_units==3)
      {
      coord_SW_x=LONSUB(coord_SW_x);
      if (fabs(coord_SW_y)>90*60*60) ERRORMSG();

      coord_NW_x=LONSUB(coord_NW_x);
      if (fabs(coord_NW_y)>90*60*60) ERRORMSG();

      coord_NE_x=LONSUB(coord_NE_x);
      if (fabs(coord_NE_y)>90*60*60) ERRORMSG();

      coord_SE_x=LONSUB(coord_SE_x);
      if (fabs(coord_SE_y)>90*60*60) ERRORMSG();

      if (coord_SW_y>=coord_NW_y || coord_SE_y>=coord_NE_y) ERRORMSG();
      }

   comment->reset();

   // magic descriptor
   comment->addstring(" BOX\n");

   // data description
   comment->addstring(" description=");
   if (pnm_description!=NULL) comment->addstring(pnm_description);
   comment->addchar('\n');

   // reference coordinate system
   comment->addstring(" coordinate system=");
   switch (coord_sys)
      {
      case 0: comment->addstring("LL\n"); break; // LL=Lat/Lon=Latitude/Longitude=geographic
      case 1: comment->addstring("UTM\n"); break; // UTM=Universal Transverse Mercator
      }
   comment->addstring(" coordinate zone=");
   comment->addfloat(coord_zone);
   comment->addchar('\n');
   comment->addstring(" coordinate datum=");
   comment->addfloat(coord_datum);
   comment->addchar('\n');

   // coordinates of corners
   comment->addstring(" SW corner=");
   comment->addfloat(coord_SW_x,1);
   comment->addchar('/');
   comment->addfloat(coord_SW_y,1);
   comment->addchar(' ');
   comment->addunits(coord_units);
   comment->addchar('\n');
   comment->addstring(" NW corner=");
   comment->addfloat(coord_NW_x,1);
   comment->addchar('/');
   comment->addfloat(coord_NW_y,1);
   comment->addchar(' ');
   comment->addunits(coord_units);
   comment->addchar('\n');
   comment->addstring(" NE corner=");
   comment->addfloat(coord_NE_x,1);
   comment->addchar('/');
   comment->addfloat(coord_NE_y,1);
   comment->addchar(' ');
   comment->addunits(coord_units);
   comment->addchar('\n');
   comment->addstring(" SE corner=");
   comment->addfloat(coord_SE_x,1);
   comment->addchar('/');
   comment->addfloat(coord_SE_y,1);
   comment->addchar(' ');
   comment->addunits(coord_units);
   comment->addchar('\n');

   // spatial resolution
   comment->addstring(" cell size=");
   comment->addfloat(cell_size_x);
   comment->addchar('/');
   comment->addfloat(cell_size_y);
   comment->addchar(' ');
   comment->addunits(coord_units);
   comment->addchar('\n');
   comment->addstring(" vertical scaling=");
   comment->addfloat(vertical_scaling);
   comment->addchar(' ');
   comment->addunits(scaling_units);
   comment->addchar('\n');

   // missing value
   comment->addstring(" missing value=");
   comment->addfloat(missing_value);
   comment->addchar('\n');
   }

// calculate the grid parameters in geographic coordinates
// returns 1 if PNM comment contains a valid descriptor, 0 otherwise
int getPNMparams(PNMcomment *comment,
                 float *coord,
                 float *cell_size,
                 float *vertical_scaling,
                 int *missing_value,
                 int *utm_zone,
                 int *utm_datum)
   {
   int line;
   char *ptr;

   PNMcomment com;
   char str[PNMcomment::MAX_STR];

   BOOLINT texflag=FALSE;
   int coord_sys=0,coord_zone=0,coord_datum=0;
   float coord_SW_x=0.0f,coord_SW_y=0.0f;
   float coord_NW_x=0.0f,coord_NW_y=0.0f;
   float coord_NE_x=0.0f,coord_NE_y=0.0f;
   float coord_SE_x=0.0f,coord_SE_y=0.0f;
   int coord_units=0;
   float cell_size_x=0.0f,cell_size_y=0.0f;
   float extent_x=0.0f,extent_y=0.0f;
   int scaling_units=0;

   float dxxSW,dxySW,dyxSW,dyySW;
   float dxxNW,dxyNW,dyxNW,dyyNW;
   float dxxNE,dxyNE,dyxNE,dyyNE;
   float dxxSE,dxySE,dyxSE,dyySE;
   float length;

   BOOLINT check;

   ptr=comment->str();

   check=TRUE;

   for (line=1; *ptr!='\0' && check; line++)
      {
      com.reset();

      while (*ptr==' ') ptr++;
      while (com.addchar(*ptr++)!='\n');

      switch (line)
         {
         case 1: // magic descriptor
            if (sscanf(com.str(),"%3s",
                       str)!=1) {check=FALSE; break;}

            check=FALSE;

            if (strcmp(str,"BOX")==0 || strcmp(str,"DEM")==0)
               {
               texflag=FALSE;
               check=TRUE;
               }

            if (strcmp(str,"TEX")==0)
               {
               texflag=TRUE;
               check=TRUE;
               }
            break;
         case 2: // data description
            if (sscanf(com.str(),"%11s=",
                       str)!=1) {check=FALSE; break;}

            if (strcmp(str,"description")!=0) check=FALSE;
            break;
         case 3: // coordinate system
            if (sscanf(com.str(),"coordinate system=%s",
                       str)!=1) {check=FALSE; break;}

            if (strcmp(str,"LL")==0) coord_sys=0;
            else if (strcmp(str,"UTM")==0) coord_sys=1;
            else check=FALSE;
            break;
         case 4: // coordinate zone
            if (sscanf(com.str(),"coordinate zone=%d",
                       &coord_zone)!=1) {check=FALSE; break;}

            if (coord_sys==0 && coord_zone!=0) check=FALSE;
            if (coord_sys==1 && coord_zone==0) check=FALSE;
            if (coord_zone<-60 || coord_zone>60) check=FALSE;
            break;
         case 5: // coordinate datum
            if (sscanf(com.str(),"coordinate datum=%d",
                       &coord_datum)!=1) {check=FALSE; break;}

            if (coord_sys==0 && coord_datum!=0) check=FALSE;
            break;
         case 6: // SW corner
            if (sscanf(com.str(),"SW corner=%g/%g %s",
                       &coord_SW_x,&coord_SW_y,str)!=3) {check=FALSE; break;}

            if (strcmp(str,"radians")==0) coord_units=0;
            else if (strcmp(str,"feet")==0) coord_units=1;
            else if (strcmp(str,"meters")==0) coord_units=2;
            else if (strcmp(str,"decimeters")==0) coord_units=3;
            else if (strcmp(str,"arc-seconds")==0) coord_units=4;
            else {check=FALSE; break;}

            if (coord_units==0)
               {
               coord_SW_x*=360*60*60/(2*PI);
               coord_SW_y*=360*60*60/(2*PI);
               coord_units=4;
               }

            if (coord_units==1)
               {
               coord_SW_x*=0.3048f;
               coord_SW_y*=0.3048f;
               coord_units=2;
               }

            if (coord_units==3)
               {
               coord_SW_x*=0.1f;
               coord_SW_y*=0.1f;
               coord_units=2;
               }

            if (coord_sys==0 && coord_units!=4) check=FALSE;
            if (coord_sys==1 && coord_units!=2) check=FALSE;
            break;
         case 7: // NW corner
            if (sscanf(com.str(),"NW corner=%g/%g %s",
                       &coord_NW_x,&coord_NW_y,str)!=3) {check=FALSE; break;}

            if (strcmp(str,"radians")==0) coord_units=0;
            else if (strcmp(str,"feet")==0) coord_units=1;
            else if (strcmp(str,"meters")==0) coord_units=2;
            else if (strcmp(str,"decimeters")==0) coord_units=3;
            else if (strcmp(str,"arc-seconds")==0) coord_units=4;
            else {check=FALSE; break;}

            if (coord_units==0)
               {
               coord_NW_x*=360*60*60/(2*PI);
               coord_NW_y*=360*60*60/(2*PI);
               coord_units=4;
               }

            if (coord_units==1)
               {
               coord_NW_x*=0.3048f;
               coord_NW_y*=0.3048f;
               coord_units=2;
               }

            if (coord_units==3)
               {
               coord_NW_x*=0.1f;
               coord_NW_y*=0.1f;
               coord_units=2;
               }

            if (coord_sys==0 && coord_units!=4) check=FALSE;
            if (coord_sys==1 && coord_units!=2) check=FALSE;
            break;
         case 8: // NE corner
            if (sscanf(com.str(),"NE corner=%g/%g %s",
                       &coord_NE_x,&coord_NE_y,str)!=3) {check=FALSE; break;}

            if (strcmp(str,"radians")==0) coord_units=0;
            else if (strcmp(str,"feet")==0) coord_units=1;
            else if (strcmp(str,"meters")==0) coord_units=2;
            else if (strcmp(str,"decimeters")==0) coord_units=3;
            else if (strcmp(str,"arc-seconds")==0) coord_units=4;
            else {check=FALSE; break;}

            if (coord_units==0)
               {
               coord_NE_x*=360*60*60/(2*PI);
               coord_NE_y*=360*60*60/(2*PI);
               coord_units=4;
               }

            if (coord_units==1)
               {
               coord_NE_x*=0.3048f;
               coord_NE_y*=0.3048f;
               coord_units=2;
               }

            if (coord_units==3)
               {
               coord_NE_x*=0.1f;
               coord_NE_y*=0.1f;
               coord_units=2;
               }

            if (coord_sys==0 && coord_units!=4) check=FALSE;
            if (coord_sys==1 && coord_units!=2) check=FALSE;
            break;
         case 9: // SE corner
            if (sscanf(com.str(),"SE corner=%g/%g %s",
                       &coord_SE_x,&coord_SE_y,str)!=3) {check=FALSE; break;}

            if (strcmp(str,"radians")==0) coord_units=0;
            else if (strcmp(str,"feet")==0) coord_units=1;
            else if (strcmp(str,"meters")==0) coord_units=2;
            else if (strcmp(str,"decimeters")==0) coord_units=3;
            else if (strcmp(str,"arc-seconds")==0) coord_units=4;
            else {check=FALSE; break;}

            if (coord_units==0)
               {
               coord_SE_x*=360*60*60/(2*PI);
               coord_SE_y*=360*60*60/(2*PI);
               coord_units=4;
               }

            if (coord_units==1)
               {
               coord_SE_x*=0.3048f;
               coord_SE_y*=0.3048f;
               coord_units=2;
               }

            if (coord_units==3)
               {
               coord_SE_x*=0.1f;
               coord_SE_y*=0.1f;
               coord_units=2;
               }

            if (coord_sys==0 && coord_units!=4) check=FALSE;
            if (coord_sys==1 && coord_units!=2) check=FALSE;
            break;
         case 10: // cell size
            if (sscanf(com.str(),"cell size=%g/%g %s",
                       &cell_size_x,&cell_size_y,str)!=3) {check=FALSE; break;}

            if (strcmp(str,"radians")==0) coord_units=0;
            else if (strcmp(str,"feet")==0) coord_units=1;
            else if (strcmp(str,"meters")==0) coord_units=2;
            else if (strcmp(str,"decimeters")==0) coord_units=3;
            else if (strcmp(str,"arc-seconds")==0) coord_units=4;
            else {check=FALSE; break;}

            if (coord_units==0)
               {
               cell_size_x*=360*60*60/(2*PI);
               cell_size_y*=360*60*60/(2*PI);
               coord_units=4;
               }

            if (coord_units==1)
               {
               cell_size_x*=0.3048f;
               cell_size_y*=0.3048f;
               coord_units=2;
               }

            if (coord_units==3)
               {
               cell_size_x*=0.1f;
               cell_size_y*=0.1f;
               coord_units=2;
               }

            if (coord_sys==0 && coord_units!=4) check=FALSE;
            if (coord_sys==1 && coord_units!=2) check=FALSE;
            break;
         case 11: // vertical scaling
            if (sscanf(com.str(),"vertical scaling=%g %s",
                       vertical_scaling,str)!=2) {check=FALSE; break;}

            if (strcmp(str,"feet")==0) scaling_units=1;
            else if (strcmp(str,"meters")==0) scaling_units=2;
            else if (strcmp(str,"decimeters")==0) scaling_units=3;
            else {check=FALSE; break;}

            if (scaling_units==1)
               {
               *vertical_scaling*=0.3048f;
               scaling_units=2;
               }

            if (scaling_units==3)
               {
               *vertical_scaling*=0.1f;
               scaling_units=2;
               }
            break;
         case 12: // missing value
            if (sscanf(com.str(),"missing value=%d",
                       missing_value)!=1) check=FALSE;
            break;
         }
      }

   if (line<13 || !check) return(0);

   // x coordinates extend from -180*60*60 arc-seconds (-180 degrees) to 180*60*60 arc-seconds (180 degrees)
   // y coordinates extend from -90*60*60 arc-seconds (-90 degrees) to 90*60*60 arc-seconds (90 degrees)
   if (coord_sys==0)
      {
      coord_SW_x=LONSUB(coord_SW_x);
      if (fabs(coord_SW_y)>90*60*60) ERRORMSG();

      coord_NW_x=LONSUB(coord_NW_x);
      if (fabs(coord_NW_y)>90*60*60) ERRORMSG();

      coord_NE_x=LONSUB(coord_NE_x);
      if (fabs(coord_NE_y)>90*60*60) ERRORMSG();

      coord_SE_x=LONSUB(coord_SE_x);
      if (fabs(coord_SE_y)>90*60*60) ERRORMSG();

      if (coord_SW_y>=coord_NW_y || coord_SE_y>=coord_NE_y) ERRORMSG();

      if (utm_zone!=NULL && utm_datum!=NULL)
         {
         *utm_zone=0;
         *utm_datum=0;
         }
      }
   else if (coord_sys==1)
      {
      if (coord_SW_x>=coord_SE_x || coord_NW_x>=coord_NE_x) ERRORMSG();
      if (coord_SW_y>=coord_NW_y || coord_SE_y>=coord_NE_y) ERRORMSG();

      if (utm_zone!=NULL && utm_datum!=NULL)
         {
         *utm_zone=coord_zone;
         *utm_datum=coord_datum;
         }
      else
         {
         // calculate extent:

         extent_x=fsqrt((coord_SE_x-coord_SW_x)*(coord_SE_x-coord_SW_x)+
                        (coord_SE_y-coord_SW_y)*(coord_SE_y-coord_SW_y))+
                  fsqrt((coord_NE_x-coord_NW_x)*(coord_NE_x-coord_NW_x)+
                        (coord_NE_y-coord_NW_y)*(coord_NE_y-coord_NW_y));

         extent_y=fsqrt((coord_NW_x-coord_SW_x)*(coord_NW_x-coord_SW_x)+
                        (coord_NW_y-coord_SW_y)*(coord_NW_y-coord_SW_y))+
                  fsqrt((coord_NE_x-coord_SE_x)*(coord_NE_x-coord_SE_x)+
                        (coord_NE_y-coord_SE_y)*(coord_NE_y-coord_SE_y));

         // transform corners
         miniutm::UTM2LL(coord_SW_x,coord_SW_y,coord_zone,coord_datum,&coord_SW_y,&coord_SW_x);
         miniutm::UTM2LL(coord_NW_x,coord_NW_y,coord_zone,coord_datum,&coord_NW_y,&coord_NW_x);
         miniutm::UTM2LL(coord_NE_x,coord_NE_y,coord_zone,coord_datum,&coord_NE_y,&coord_NE_x);
         miniutm::UTM2LL(coord_SE_x,coord_SE_y,coord_zone,coord_datum,&coord_SE_y,&coord_SE_x);

         // cell size changes approximately by the same factor as the extent changes:

         cell_size_x*=(fsqrt(fsqr(LONADD(coord_SE_x,-coord_SW_x))+
                             (coord_SE_y-coord_SW_y)*(coord_SE_y-coord_SW_y))+
                       fsqrt(fsqr(LONADD(coord_NE_x,-coord_NW_x))+
                             (coord_NE_y-coord_NW_y)*(coord_NE_y-coord_NW_y)))/extent_x;

         cell_size_y*=(fsqrt(fsqr(LONSUB(coord_NW_x,coord_SW_x))+
                             (coord_NW_y-coord_SW_y)*(coord_NW_y-coord_SW_y))+
                       fsqrt(fsqr(LONSUB(coord_NE_x,coord_SE_x))+
                             (coord_NE_y-coord_SE_y)*(coord_NE_y-coord_SE_y)))/extent_y;

         coord_zone=coord_datum=0;
         coord_units=4;
         coord_sys=0;
         }
      }

   // textures are usually provided using a cell centric data representation
   // so we need to shrink them by 0.5 texels on each side
   // to convert from a cell centric to a corner centric data representation
   if (texflag)
      if (coord_sys==1)
         {
         // SW corner:

         dxxSW=coord_SE_x-coord_SW_x;
         dxySW=coord_SE_y-coord_SW_y;

         if ((length=fsqrt(dxxSW*dxxSW+dxySW*dxySW))==0.0f) ERRORMSG();
         dxxSW/=length;
         dxySW/=length;

         dyxSW=coord_NW_x-coord_SW_x;
         dyySW=coord_NW_y-coord_SW_y;

         if ((length=fsqrt(dyxSW*dyxSW+dyySW*dyySW))==0.0f) ERRORMSG();
         dyxSW/=length;
         dyySW/=length;

         // NW corner:

         dxxNW=coord_NE_x-coord_NW_x;
         dxyNW=coord_NE_y-coord_NW_y;

         if ((length=fsqrt(dxxNW*dxxNW+dxyNW*dxyNW))==0.0f) ERRORMSG();
         dxxNW/=length;
         dxyNW/=length;

         dyxNW=coord_SW_x-coord_NW_x;
         dyyNW=coord_SW_y-coord_NW_y;

         if ((length=fsqrt(dyxNW*dyxNW+dyyNW*dyyNW))==0.0f) ERRORMSG();
         dyxNW/=length;
         dyyNW/=length;

         // NE corner:

         dxxNE=coord_NW_x-coord_NE_x;
         dxyNE=coord_NW_y-coord_NE_y;

         if ((length=fsqrt(dxxNE*dxxNE+dxyNE*dxyNE))==0.0f) ERRORMSG();
         dxxNE/=length;
         dxyNE/=length;

         dyxNE=coord_SE_x-coord_NE_x;
         dyyNE=coord_SE_y-coord_NE_y;

         if ((length=fsqrt(dyxNE*dyxNE+dyyNE*dyyNE))==0.0f) ERRORMSG();
         dyxNE/=length;
         dyyNE/=length;

         // SE corner:

         dxxSE=coord_SW_x-coord_SE_x;
         dxySE=coord_SW_y-coord_SE_y;

         if ((length=fsqrt(dxxSE*dxxSE+dxySE*dxySE))==0.0f) ERRORMSG();
         dxxSE/=length;
         dxySE/=length;

         dyxSE=coord_NE_x-coord_SE_x;
         dyySE=coord_NE_y-coord_SE_y;

         if ((length=fsqrt(dyxSE*dyxSE+dyySE*dyySE))==0.0f) ERRORMSG();
         dyxSE/=length;
         dyySE/=length;

         // shrink by 0.5 texel:

         coord_SW_x+=0.5f*(cell_size_x*dxxSW+cell_size_y*dyxSW);
         coord_SW_y+=0.5f*(cell_size_x*dxySW+cell_size_y*dyySW);

         coord_NW_x+=0.5f*(cell_size_x*dxxNW+cell_size_y*dyxNW);
         coord_NW_y+=0.5f*(cell_size_x*dxyNW+cell_size_y*dyyNW);

         coord_NE_x+=0.5f*(cell_size_x*dxxNE+cell_size_y*dyxNE);
         coord_NE_y+=0.5f*(cell_size_x*dxyNE+cell_size_y*dyyNE);

         coord_SE_x+=0.5f*(cell_size_x*dxxSE+cell_size_y*dyxSE);
         coord_SE_y+=0.5f*(cell_size_x*dxySE+cell_size_y*dyySE);
         }
      else
         {
         // SW corner:

         dxxSW=LONADD(coord_SE_x,-coord_SW_x);
         dxySW=coord_SE_y-coord_SW_y;

         if ((length=fsqrt(dxxSW*dxxSW+dxySW*dxySW))==0.0f) ERRORMSG();
         dxxSW/=length;
         dxySW/=length;

         dyxSW=LONSUB(coord_NW_x,coord_SW_x);
         dyySW=coord_NW_y-coord_SW_y;

         if ((length=fsqrt(dyxSW*dyxSW+dyySW*dyySW))==0.0f) ERRORMSG();
         dyxSW/=length;
         dyySW/=length;

         // NW corner:

         dxxNW=LONADD(coord_NE_x,-coord_NW_x);
         dxyNW=coord_NE_y-coord_NW_y;

         if ((length=fsqrt(dxxNW*dxxNW+dxyNW*dxyNW))==0.0f) ERRORMSG();
         dxxNW/=length;
         dxyNW/=length;

         dyxNW=LONSUB(coord_SW_x,coord_NW_x);
         dyyNW=coord_SW_y-coord_NW_y;

         if ((length=fsqrt(dyxNW*dyxNW+dyyNW*dyyNW))==0.0f) ERRORMSG();
         dyxNW/=length;
         dyyNW/=length;

         // NE corner:

         dxxNE=-LONADD(coord_NE_x,-coord_NW_x);
         dxyNE=coord_NW_y-coord_NE_y;

         if ((length=fsqrt(dxxNE*dxxNE+dxyNE*dxyNE))==0.0f) ERRORMSG();
         dxxNE/=length;
         dxyNE/=length;

         dyxNE=LONSUB(coord_SE_x,coord_NE_x);
         dyyNE=coord_SE_y-coord_NE_y;

         if ((length=fsqrt(dyxNE*dyxNE+dyyNE*dyyNE))==0.0f) ERRORMSG();
         dyxNE/=length;
         dyyNE/=length;

         // SE corner:

         dxxSE=-LONADD(coord_SE_x,-coord_SW_x);
         dxySE=coord_SW_y-coord_SE_y;

         if ((length=fsqrt(dxxSE*dxxSE+dxySE*dxySE))==0.0f) ERRORMSG();
         dxxSE/=length;
         dxySE/=length;

         dyxSE=LONSUB(coord_NE_x,coord_SE_x);
         dyySE=coord_NE_y-coord_SE_y;

         if ((length=fsqrt(dyxSE*dyxSE+dyySE*dyySE))==0.0f) ERRORMSG();
         dyxSE/=length;
         dyySE/=length;

         // shrink by 0.5 texel:

         coord_SW_x=LONSUB(coord_SW_x,-0.5f*(cell_size_x*dxxSW+cell_size_y*dyxSW));
         coord_SW_y+=0.5f*(cell_size_x*dxySW+cell_size_y*dyySW);

         coord_NW_x=LONSUB(coord_NW_x,-0.5f*(cell_size_x*dxxNW+cell_size_y*dyxNW));
         coord_NW_y+=0.5f*(cell_size_x*dxyNW+cell_size_y*dyyNW);

         coord_NE_x=LONSUB(coord_NE_x,-0.5f*(cell_size_x*dxxNE+cell_size_y*dyxNE));
         coord_NE_y+=0.5f*(cell_size_x*dxyNE+cell_size_y*dyyNE);

         coord_SE_x=LONSUB(coord_SE_x,-0.5f*(cell_size_x*dxxSE+cell_size_y*dyxSE));
         coord_SE_y+=0.5f*(cell_size_x*dxySE+cell_size_y*dyySE);
         }

   coord[0]=coord_SW_x;
   coord[1]=coord_SW_y;
   coord[2]=coord_NW_x;
   coord[3]=coord_NW_y;
   coord[4]=coord_NE_x;
   coord[5]=coord_NE_y;
   coord[6]=coord_SE_x;
   coord[7]=coord_SE_y;

   cell_size[0]=cell_size_x;
   cell_size[1]=cell_size_y;

   return(1);
   }

}
