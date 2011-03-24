// (c) by Stefan Roettger

#include "minibase.h"

#include "mini.h"

#include "miniutm.h"
#include "pnmbase.h"

#include "pnmsample.h"

namespace pnmsample {

const int MAX_STR=1024;

int CONFIGURE_SUPERSAMPLING=1;
int CONFIGURE_DOWNSAMPLING=3;
int CONFIGURE_STARTUPFILE=0;

char CONFIGURE_TILESETPATH[MAX_STR]="";
char CONFIGURE_STDPREFIX[MAX_STR]="elev.";
char CONFIGURE_RGBPREFIX[MAX_STR]="imag.";
char CONFIGURE_TILESETNAME[MAX_STR]="tileset.sav";
char CONFIGURE_STARTUPNAME[MAX_STR]="startup.sav";

// get distance of point (x,y) from line starting at (px,py) with direction (dx,dy)
inline float getdistance(float x,float y,
                         float px,float py,
                         float dx,float dy)
   {
   static const float e=1.0E-5f;

   if (fabs(dx)<e)
      if (dy>0.0f) return(x-px);
      else if (dy<0.0f) return(px-x);

   if (fabs(dy)<e)
      if (dx>0.0f) return(py-y);
      else if (dx<0.0f) return(y-py);

   float d=fsqrt(dx*dx+dy*dy);
   if (d>0.0f) return(((x-px)*dy-(y-py)*dx)/d);
   return(MAXFLOAT);
   }

// get elevation at point position (i,j)
float getpoint(unsigned char *image,
               int width,int height,int components,
               float scaling,int missing,
               int i,int j)
   {
   int s;

   if (i<0 || i>width-1 ||
       j<0 || j>height-1) ERRORMSG();

   if (components==1)
      {
      s=image[i+width*j];
      if (s==missing) return(missing);
      return(scaling*s);
      }
   else if (components==2)
      {
      s=(short int)(256*image[2*(i+width*j)]+image[2*(i+width*j)+1]);
      if (s==missing) return(missing);
      return(scaling*s);
      }

   return(missing);
   }

// bilinear resampling at point position (x,y)
float resamplepoint(unsigned char *image,
                    int width,int height,int components,
                    float centerx,float centery,
                    float extentx,float extenty,
                    float scaling,int missing,
                    float x,float y,
                    float *point)
   {
   float cx,cy;

   int pi,pj;
   float rx,ry;

   int s1,s2,s3,s4;
   int mcount;

   float rgb[3];
   int black;

   cx=(x-centerx)/extentx+0.5f;
   cy=(centery-y)/extenty+0.5f;

   if (cx<-0.5f/(width-1) || cx>1.0f+0.5f/(width-1) ||
       cy<-0.5f/(height-1) || cy>1.0f+0.5f/(height-1)) return(missing);

   pi=ftrc(cx*(width-1));
   rx=cx*(width-1)-pi;

   pj=ftrc(cy*(height-1));
   ry=cy*(height-1)-pj;

   if (cx<0.0f)
      {
      pi=0;
      rx=0.0f;
      }

   if (cy<0.0f)
      {
      pj=0;
      ry=0.0f;
      }

   if (pi>=width-1)
      {
      pi=width-2;
      rx=1.0f;
      }

   if (pj>=height-1)
      {
      pj=height-2;
      ry=1.0f;
      }

   if (components==1)
      {
      s1=image[pi+width*pj];
      s2=image[pi+1+width*pj];
      s3=image[pi+width*(pj+1)];
      s4=image[pi+1+width*(pj+1)];

      mcount=0;
      if (s1==missing) mcount++;
      if (s2==missing) mcount++;
      if (s3==missing) mcount++;
      if (s4==missing) mcount++;

      if (mcount==4) return(missing);

      if (mcount==1)
         if (s1==missing)
            if (rx>0.5f || ry>0.5f) s1=(s2+s3+1)/2;
            else return(missing);
         else if (s2==missing)
            if (rx<0.5f || ry>0.5f) s2=(s1+s4+1)/2;
            else return(missing);
         else if (s3==missing)
            if (rx>0.5f || ry<0.5f) s3=(s1+s4+1)/2;
            else return(missing);
         else if (s4==missing)
            if (rx<0.5f || ry<0.5f) s4=(s2+s3+1)/2;
            else return(missing);

      if (mcount==2)
         if (s1==missing && s2==missing)
            if (ry>0.5f) {s1=s3; s2=s4;}
            else return(missing);
         else if (s3==missing && s4==missing)
            if (ry<0.5f) {s3=s1; s4=s2;}
            else return(missing);
         else if (s1==missing && s3==missing)
            if (rx>0.5f) {s1=s2; s3=s4;}
            else return(missing);
         else if (s2==missing && s4==missing)
            if (rx<0.5f) {s2=s1; s4=s3;}
            else return(missing);
         else if (s1==missing && s4==missing)
            if ((rx>0.5f || ry>0.5f) && (rx<0.5f || ry<0.5f)) s1=s4=(s2+s3+1)/2;
            else return(missing);
         else if (s2==missing && s3==missing)
            if ((rx<0.5f || ry>0.5f) && (rx>0.5f || ry<0.5f)) s2=s3=(s1+s4+1)/2;
            else return(missing);

      if (mcount==3)
         if (s1!=missing)
            if (rx<0.5f && ry<0.5f) s2=s3=s4=s1;
            else return(missing);
         else if (s2!=missing)
            if (rx>0.5f && ry<0.5f) s1=s3=s4=s2;
            else return(missing);
         else if (s3!=missing)
            if (rx<0.5f && ry>0.5f) s1=s2=s4=s3;
            else return(missing);
         else if (s4!=missing)
            if (rx>0.5f && ry>0.5f) s1=s2=s3=s4;
            else return(missing);

      return(scaling*((1.0f-ry)*((1.0f-rx)*s1+rx*s2)+ry*((1.0f-rx)*s3+rx*s4)));
      }
   else if (components==2)
      {
      s1=(short int)(256*image[2*(pi+width*pj)]+image[2*(pi+width*pj)+1]);
      s2=(short int)(256*image[2*(pi+1+width*pj)]+image[2*(pi+1+width*pj)+1]);
      s3=(short int)(256*image[2*(pi+width*(pj+1))]+image[2*(pi+width*(pj+1))+1]);
      s4=(short int)(256*image[2*(pi+1+width*(pj+1))]+image[2*(pi+1+width*(pj+1))+1]);

      mcount=0;
      if (s1==missing) mcount++;
      if (s2==missing) mcount++;
      if (s3==missing) mcount++;
      if (s4==missing) mcount++;

      if (mcount==4) return(missing);

      if (mcount==1)
         if (s1==missing)
            if (rx>0.5f || ry>0.5f) s1=(s2+s3+1)/2;
            else return(missing);
         else if (s2==missing)
            if (rx<0.5f || ry>0.5f) s2=(s1+s4+1)/2;
            else return(missing);
         else if (s3==missing)
            if (rx>0.5f || ry<0.5f) s3=(s1+s4+1)/2;
            else return(missing);
         else if (s4==missing)
            if (rx<0.5f || ry<0.5f) s4=(s2+s3+1)/2;
            else return(missing);

      if (mcount==2)
         if (s1==missing && s2==missing)
            if (ry>0.5f) {s1=s3; s2=s4;}
            else return(missing);
         else if (s3==missing && s4==missing)
            if (ry<0.5f) {s3=s1; s4=s2;}
            else return(missing);
         else if (s1==missing && s3==missing)
            if (rx>0.5f) {s1=s2; s3=s4;}
            else return(missing);
         else if (s2==missing && s4==missing)
            if (rx<0.5f) {s2=s1; s4=s3;}
            else return(missing);
         else if (s1==missing && s4==missing)
            if ((rx>0.5f || ry>0.5f) && (rx<0.5f || ry<0.5f)) s1=s4=(s2+s3+1)/2;
            else return(missing);
         else if (s2==missing && s3==missing)
            if ((rx<0.5f || ry>0.5f) && (rx>0.5f || ry<0.5f)) s2=s3=(s1+s4+1)/2;
            else return(missing);

      if (mcount==3)
         if (s1!=missing)
            if (rx<0.5f && ry<0.5f) s2=s3=s4=s1;
            else return(missing);
         else if (s2!=missing)
            if (rx>0.5f && ry<0.5f) s1=s3=s4=s2;
            else return(missing);
         else if (s3!=missing)
            if (rx<0.5f && ry>0.5f) s1=s2=s4=s3;
            else return(missing);
         else if (s4!=missing)
            if (rx>0.5f && ry>0.5f) s1=s2=s3=s4;
            else return(missing);

      return(scaling*((1.0f-ry)*((1.0f-rx)*s1+rx*s2)+ry*((1.0f-rx)*s3+rx*s4)));
      }
   else if (components==3)
      {
      if (point[0]>0.0f || point[1]>0.0f || point[2]>0.0f) return(missing);

      black=15;

      s1=image[3*(pi+width*pj)];
      s2=image[3*(pi+1+width*pj)];
      s3=image[3*(pi+width*(pj+1))];
      s4=image[3*(pi+1+width*(pj+1))];

      if (s1!=0) black&=15-1;
      if (s2!=0) black&=15-2;
      if (s3!=0) black&=15-4;
      if (s4!=0) black&=15-8;

      rgb[0]=(1.0f-ry)*((1.0f-rx)*s1+rx*s2)+ry*((1.0f-rx)*s3+rx*s4);

      s1=image[3*(pi+width*pj)+1];
      s2=image[3*(pi+1+width*pj)+1];
      s3=image[3*(pi+width*(pj+1))+1];
      s4=image[3*(pi+1+width*(pj+1))+1];

      if (s1!=0) black&=15-1;
      if (s2!=0) black&=15-2;
      if (s3!=0) black&=15-4;
      if (s4!=0) black&=15-8;

      rgb[1]=(1.0f-ry)*((1.0f-rx)*s1+rx*s2)+ry*((1.0f-rx)*s3+rx*s4);

      s1=image[3*(pi+width*pj)+2];
      s2=image[3*(pi+1+width*pj)+2];
      s3=image[3*(pi+width*(pj+1))+2];
      s4=image[3*(pi+1+width*(pj+1))+2];

      if (s1!=0) black&=15-1;
      if (s2!=0) black&=15-2;
      if (s3!=0) black&=15-4;
      if (s4!=0) black&=15-8;

      rgb[2]=(1.0f-ry)*((1.0f-rx)*s1+rx*s2)+ry*((1.0f-rx)*s3+rx*s4);

      if (black==0)
         {
         point[0]=rgb[0];
         point[1]=rgb[1];
         point[2]=rgb[2];
         }
      }
   else ERRORMSG();

   return(missing);
   }

// barycentric coordinate transformation
float transformpoint(unsigned char *image,
                     int width,int height,int components,
                     float centerx,float centery,
                     float extentx,float extenty,
                     float *coord,float *utm_coord,int utm_zone,int utm_datum,
                     float scaling,int missing,
                     float x,float y,
                     float *point)
   {
   float dx,dy;
   float s1,s2,t1,t2;

   dx=LONSUB(x,centerx);
   dy=y-centery;

   if (fabs(dx)>extentx/2.0f || fabs(dy)>extenty/2.0f) return(missing);

   x=centerx+dx;

   if (utm_zone==0)
      {
      s1=getdistance(x,y,coord[0],coord[1],LONSUB(coord[2],coord[0]),coord[3]-coord[1]);
      s2=getdistance(x,y,coord[4],coord[5],LONSUB(coord[6],coord[4]),coord[7]-coord[5]);

      t1=getdistance(x,y,coord[6],coord[7],-LONADD(coord[6],-coord[0]),coord[1]-coord[7]);
      t2=getdistance(x,y,coord[2],coord[3],LONADD(coord[4],-coord[2]),coord[5]-coord[3]);
      }
   else
      {
      miniutm::LL2UTM(y,x,utm_zone,utm_datum,&x,&y);

      s1=getdistance(x,y,utm_coord[0],utm_coord[1],utm_coord[2]-utm_coord[0],utm_coord[3]-utm_coord[1]);
      s2=getdistance(x,y,utm_coord[4],utm_coord[5],utm_coord[6]-utm_coord[4],utm_coord[7]-utm_coord[5]);

      t1=getdistance(x,y,utm_coord[6],utm_coord[7],utm_coord[0]-utm_coord[6],utm_coord[1]-utm_coord[7]);
      t2=getdistance(x,y,utm_coord[2],utm_coord[3],utm_coord[4]-utm_coord[2],utm_coord[5]-utm_coord[3]);
      }

   return(resamplepoint(image,
                        width,height,components,
                        0.5f,0.5f,
                        1.0f,1.0f,
                        scaling,missing,
                        s1/(s1+s2),t1/(t1+t2),
                        point));
   }

// resample a collection of heterogeneous grids
void resample(int num,char **grid,
              int tiles,int down,int maxsize,
              char *basepath,
              float *centerx,float *centery,
              float *extentx,float *extenty,
              float *scaling,int missing,
              int *cols,int *rows,
              float *outparams,
              int *outmaxsize,
              float offx,float offy,
              int pyramid)
   {
   int n;

   int i,j;

   BOOLINT done;

   char filename[MAX_STR];

   if (num<1 || tiles<1 || down<0) ERRORMSG();

   if (pyramid<0) ERRORMSG();

   unsigned char **maps=new unsigned char *[num];

   int *widths=new int[num];
   int *heights=new int[num];
   int *comps=new int[num];

   PNMcomment *comms=new PNMcomment[num];

   float *centersx=new float[num];
   float *centersy=new float[num];

   float *extentsx=new float[num];
   float *extentsy=new float[num];

   float *coords=new float[8*num];
   float *cellsizes=new float[2*num];

   float *utm_coords=new float[8*num];
   int *utm_zones=new int[num];
   int *utm_datums=new int[num];

   float *scalings=new float[num];
   BOOLINT *additives=new BOOLINT[num];

   int *missings=new int[num];

   float *maxelevs=new float[num];

   float elev,rgb[3];
   float maxelev,addelev;

   float as2m[2];

   done=TRUE;

   maxelev=addelev=0.0f;

   // for all grids
   for (n=0; n<num; n++)
      {
      if (strrchr(grid[n],'/')==NULL)
         if (basepath==NULL) snprintf(filename,MAX_STR,"stub.%s",grid[n]);
         else snprintf(filename,MAX_STR,"%s%s/stub.%s",CONFIGURE_TILESETPATH,basepath,grid[n]);
      else
         if (basepath==NULL) snprintf(filename,MAX_STR,"stub.%s",strrchr(grid[n],'/')+1);
         else snprintf(filename,MAX_STR,"%s%s/stub.%s",CONFIGURE_TILESETPATH,basepath,strrchr(grid[n],'/')+1);

      // check for stub file
      if ((maps[n]=readPNMfile(filename,
                               &widths[n],&heights[n],&comps[n],
                               &comms[n]))==NULL)
         {
         printf("loading grid[%d]=\"%s\"\n",n+1,grid[n]);

         // load grid file
         if ((maps[n]=readPNMfile(grid[n],
                                  &widths[n],&heights[n],&comps[n],
                                  &comms[n]))==NULL) ERRORMSG();

         if (widths[n]<2 || heights[n]<2) ERRORMSG();
         if (comps[n]!=1 && comps[n]!=2 && comps[n]!=3) ERRORMSG();

         done=FALSE;
         }
      else
         {
         if (widths[n]!=1 || heights[n]!=1) ERRORMSG();
         if (comps[n]!=1 && comps[n]!=2 && comps[n]!=3) ERRORMSG();
         }

      if (n>0)
         {
         if (comps[0]!=3 && comps[n]==3) ERRORMSG();
         if (comps[0]==3 && comps[n]!=3) ERRORMSG();
         }

      // calculate bounding box
      if (centerx==NULL || centery==NULL ||
          extentx==NULL || extenty==NULL ||
          scaling==NULL)
         {
         if (getPNMparams(&comms[n],
                          &coords[8*n],&cellsizes[2*n],
                          &scalings[n],&missings[n],
                          &utm_zones[n],&utm_datums[n])==0) ERRORMSG();

         if (utm_zones[n]!=0)
            {
            float *coord_SW=&coords[8*n];
            float *coord_NW=&coords[8*n+2];
            float *coord_NE=&coords[8*n+4];
            float *coord_SE=&coords[8*n+6];

            float extent_x,extent_y;

            for (i=0; i<8; i++) utm_coords[8*n+i]=coords[8*n+i];

            // calculate original extent:

            extent_x=fsqrt((coord_SE[0]-coord_SW[0])*(coord_SE[0]-coord_SW[0])+
                           (coord_SE[1]-coord_SW[1])*(coord_SE[1]-coord_SW[1]))+
                     fsqrt((coord_NE[0]-coord_NW[0])*(coord_NE[0]-coord_NW[0])+
                           (coord_NE[1]-coord_NW[1])*(coord_NE[1]-coord_NW[1]));

            extent_y=fsqrt((coord_NW[0]-coord_SW[0])*(coord_NW[0]-coord_SW[0])+
                           (coord_NW[1]-coord_SW[1])*(coord_NW[1]-coord_SW[1]))+
                     fsqrt((coord_NE[0]-coord_SE[0])*(coord_NE[0]-coord_SE[0])+
                           (coord_NE[1]-coord_SE[1])*(coord_NE[1]-coord_SE[1]));

            // transform corners
            miniutm::UTM2LL(coord_SW[0],coord_SW[1],utm_zones[n],utm_datums[n],&coord_SW[1],&coord_SW[0]);
            miniutm::UTM2LL(coord_NW[0],coord_NW[1],utm_zones[n],utm_datums[n],&coord_NW[1],&coord_NW[0]);
            miniutm::UTM2LL(coord_NE[0],coord_NE[1],utm_zones[n],utm_datums[n],&coord_NE[1],&coord_NE[0]);
            miniutm::UTM2LL(coord_SE[0],coord_SE[1],utm_zones[n],utm_datums[n],&coord_SE[1],&coord_SE[0]);

            // cell size changes approximately by the same factor as the extent changes:

            cellsizes[2*n]*=(fsqrt(fsqr(LONADD(coord_SE[0],-coord_SW[0]))+
                                   (coord_SE[1]-coord_SW[1])*(coord_SE[1]-coord_SW[1]))+
                             fsqrt(fsqr(LONADD(coord_NE[0],-coord_NW[0]))+
                                   (coord_NE[1]-coord_NW[1])*(coord_NE[1]-coord_NW[1])))/extent_x;

            cellsizes[2*n+1]*=(fsqrt(fsqr(LONSUB(coord_NW[0],coord_SW[0]))+
                                     (coord_NW[1]-coord_SW[1])*(coord_NW[1]-coord_SW[1]))+
                               fsqrt(fsqr(LONSUB(coord_NE[0],coord_SE[0]))+
                                     (coord_NE[1]-coord_SE[1])*(coord_NE[1]-coord_SE[1])))/extent_y;
            }

         centersx[n]=LONLERP(LONMEAN(coords[8*n],coords[8*n+2]),LONMEAN(coords[8*n+4],coords[8*n+6]));
         centersy[n]=(coords[8*n+1]+coords[8*n+3]+coords[8*n+5]+coords[8*n+7])/4.0f;

         extentsx[n]=LONADD(LONRIGHT(coords[8*n+4],coords[8*n+6]),-LONLEFT(coords[8*n],coords[8*n+2]));
         extentsy[n]=fmax(coords[8*n+3],coords[8*n+5])-fmin(coords[8*n+1],coords[8*n+7]);

         if (widths[n]>1 && heights[n]>1)
            {
            cellsizes[2*n]=fmin(cellsizes[2*n],extentsx[n]/(widths[n]-1));
            cellsizes[2*n+1]=fmin(cellsizes[2*n+1],extentsy[n]/(heights[n]-1));
            }

         if (scalings[n]<0.0f)
            {
            scalings[n]*=-1.0f;
            additives[n]=TRUE;
            }
         else additives[n]=FALSE;
         }
      else
         {
         if (widths[n]<2 || heights[n]<2) ERRORMSG();

         centersx[n]=LONSUB(centerx[n]);
         centersy[n]=centery[n];

         if (extentx[n]>360*60*60 || extenty[n]>180*60*60) ERRORMSG();

         extentsx[n]=extentx[n];
         extentsy[n]=extenty[n];

         coords[8*n]=LONSUB(centersx[n],extentsx[n]/2.0f);
         coords[8*n+1]=centersy[n]-extentsy[n]/2.0f;

         coords[8*n+2]=LONSUB(centersx[n],extentsx[n]/2.0f);
         coords[8*n+3]=centersy[n]+extentsy[n]/2.0f;

         coords[8*n+4]=LONSUB(centersx[n],-extentsx[n]/2.0f);
         coords[8*n+5]=centersy[n]+extentsy[n]/2.0f;

         coords[8*n+6]=LONSUB(centersx[n],-extentsx[n]/2.0f);
         coords[8*n+7]=centersy[n]-extentsy[n]/2.0f;

         cellsizes[2*n]=extentsx[n]/(widths[n]-1);
         cellsizes[2*n+1]=extentsy[n]/(heights[n]-1);

         scalings[n]=scaling[n];

         missings[n]=missing;

         if (scalings[n]<0.0f)
            {
            scalings[n]*=-1.0f;
            additives[n]=TRUE;
            }
         else additives[n]=FALSE;
         }

      // compute maximum elevation:

      maxelevs[n]=0.0f;

      if (comps[n]!=3)
         for (i=0; i<widths[n]; i++)
            for (j=0; j<heights[n]; j++)
               {
               elev=getpoint(maps[n],
                             widths[n],heights[n],comps[n],
                             scalings[n],missings[n],
                             i,j);

               if (elev!=missings[n])
                  if (fabs(elev)>maxelevs[n]) maxelevs[n]=fabs(elev);
               }

      if (!additives[n]) maxelev=fmax(maxelev,maxelevs[n]);
      else addelev=fmax(addelev,maxelevs[n]);

      // write stub file
      if (widths[n]>1 && heights[n]>1)
         {
         PNMcomment stub;

         putPNMparams(&stub,
                      "stub generated with libMini",
                      0,0,0,4,
                      coords[8*n],coords[8*n+1],
                      coords[8*n+2],coords[8*n+3],
                      coords[8*n+4],coords[8*n+5],
                      coords[8*n+6],coords[8*n+7],
                      cellsizes[2*n],cellsizes[2*n+1],
                      2,(!additives[n])?scalings[n]:-scalings[n],missings[n]);

         if (comps[n]==1)
            {
            unsigned char maxc;
            maxc=ftrc(maxelevs[n]/scalings[n]+0.5f);
            writePNMimage(filename,&maxc,1,1,comps[n],&stub);
            }
         else if (comps[n]==2)
            {
            unsigned char maxc[2];
            maxc[0]=ftrc(maxelevs[n]/scalings[n]+0.5f)/256;
            maxc[1]=ftrc(maxelevs[n]/scalings[n]+0.5f)%256;
            writePNMimage(filename,maxc,1,1,comps[n],&stub);
            }
         else
            {
            unsigned char maxc[3];
            maxc[0]=maxc[1]=maxc[2]=255;
            writePNMimage(filename,maxc,1,1,comps[n],&stub);
            }
         }

      // adjust bounding box
      if (n>0)
         {
         centersx[n]=LONSUB(centersx[n],-offx);
         centersy[n]+=offy;

         for (i=0; i<4; i++)
            {
            coords[8*n+2*i]=LONSUB(coords[8*n+2*i],-offx);
            coords[8*n+2*i+1]+=offy;
            }

         extentsx[n]+=cellsizes[2*n];
         extentsy[n]+=cellsizes[2*n+1];
         }

      // release grid
      free(maps[n]);
      maps[n]=NULL;
      }

   maxelev+=addelev;
   if (maxelev==0.0f) maxelev=1.0f;

   miniutm::arcsec2meter(centersy[0],as2m);

   float sizex=extentsx[0];
   float sizey=extentsy[0];

   int tilesx=tiles;
   int tilesy=tiles;

   float tilesizex;
   float tilesizey;

   int realmaxsize=0;

   // calculate tile size
   if (sizex*as2m[0]>sizey*as2m[1])
      {
      tilesizex=sizex/tilesx;
      tilesy=max(ftrc(sizey/tilesizex*as2m[1]/as2m[0]+0.5f),1);
      tilesizey=sizey/tilesy;
      }
   else
      {
      tilesizey=sizey/tilesy;
      tilesx=max(ftrc(sizex/tilesizey*as2m[0]/as2m[1]+0.5f),1);
      tilesizex=sizex/tilesx;
      }

   float tileposx,tileposy;
   int width,height;
   BOOLINT smaller;

   int k,l,m;

   int sn;
   float si,sj;

   float posx0,posy0;
   float posx,posy;

   float sample0;
   float rgb0[3];
   float sample;

   int scount;
   int mcount;

   int h;

   FILE *tileset=NULL;
   FILE *startup=NULL;

   // for all tiles
   if (!done)
      {
      // open startup file for appending
      if (CONFIGURE_STARTUPFILE!=0)
         {
         snprintf(filename,MAX_STR,"%s%s",CONFIGURE_TILESETPATH,CONFIGURE_STARTUPNAME);
         if ((startup=fopen(filename,"ab"))==NULL) ERRORMSG();
         }

      for (j=tilesy-1; j>=0; j--)
         for (i=0; i<tilesx; i++)
            {
            // calculate tile position
            tileposx=LONSUB(centersx[0],sizex/2.0f-i*tilesizex);
            tileposy=centersy[0]-sizey/2.0f+j*tilesizey;

            width=2;
            height=2;

            smaller=TRUE;

            // compute tile resolution
            while (smaller)
               {
               width=2*width-1;
               height=2*height-1;

               smaller=FALSE;

               for (n=0; n<num; n++)
                  if (LONSUB(tileposx,centersx[n])<extentsx[n]/2.0f &&
                      LONSUB(tileposx+tilesizex,centersx[n])>-extentsx[n]/2.0f &&
                      tileposy<centersy[n]+extentsy[n]/2.0f &&
                      tileposy+tilesizey>centersy[n]-extentsy[n]/2.0f)
                     if (tilesizex/(width-1)>1.5f*(1<<down)*cellsizes[2*n] ||
                         tilesizey/(height-1)>1.5f*(1<<down)*cellsizes[2*n+1])
                        if (1.5f*width<maxsize && 1.5f*height<maxsize)
                           {
                           smaller=TRUE;
                           break;
                           }
               }

            if (comps[0]==3)
               {
               width--;
               height--;
               }

            // compute maximum size
            if (width>realmaxsize) realmaxsize=width;
            if (height>realmaxsize) realmaxsize=height;

            printf("resampling tile[%d,%d]=%dx%d in [%g,%g]x[%g,%g] with cellsize=%g/%g arc-seconds\n",
                   i+1,tilesy-j,width,height,
                   tileposx,LONSUB(tileposx,-tilesizex),tileposy,tileposy+tilesizey,
                   tilesizex/(width-1),tilesizey/(height-1));

            float rangemin=32767.0f;
            float rangemax=-32768.0f;

            int *valid=new int[num];
            int vnum=0,vn,vm,va;

            // gather relevant grids
            for (n=0; n<num; n++)
               if (LONSUB(tileposx,centersx[n])<extentsx[n]/2.0f &&
                   LONSUB(tileposx+tilesizex,centersx[n])>-extentsx[n]/2.0f &&
                   tileposy<centersy[n]+extentsy[n]/2.0f &&
                   tileposy+tilesizey>centersy[n]-extentsy[n]/2.0f)
                  if (widths[n]>1 && heights[n]>1) valid[vnum++]=n;
                  else ERRORMSG();

            BOOLINT ok;

            // free unnecessary grids
            for (n=0; n<num; n++)
               {
               ok=TRUE;

               for (vn=0; vn<vnum; vn++)
                  if (valid[vn]==n)
                     {
                     ok=FALSE;
                     break;
                     }

               if (ok)
                  if (maps[n]!=NULL)
                     {
                     free(maps[n]);
                     maps[n]=NULL;
                     }
               }

            // reload necessary grids
            for (vn=0; vn<vnum; vn++)
               {
               n=valid[vn];

               if (maps[n]==NULL)
                  if ((maps[n]=readPNMfile(grid[n],
                                           &widths[n],&heights[n],&comps[n],
                                           &comms[n]))==NULL) ERRORMSG();
               }

            // sort relevant grids by cell size
            for (vn=0; vn<vnum-1; vn++)
               for (vm=vnum-1; vm>vn; vm--)
                  if ((cellsizes[2*valid[vm]]<cellsizes[2*valid[vm-1]] &&
                       cellsizes[2*valid[vm]+1]<cellsizes[2*valid[vm-1]+1] &&
                       !additives[valid[vm]] && !additives[valid[vm-1]]) ||
                      (!additives[valid[vm]] && additives[valid[vm-1]]))
                     {
                     int tmp=valid[vm];
                     valid[vm]=valid[vm-1];
                     valid[vm-1]=tmp;
                     }

            // mark first additive grid
            va=vnum-1;
            for (vn=0; vn<vnum; vn++)
               if (additives[valid[vn]])
                  {
                  va=vn-1;
                  break;
                  }

            int s;
            int lod1=0,lod2=0;

            // calculate maximum lod
            if (pyramid>0)
               if (comps[0]!=3)
                  {
                  for (s=width; s>3; s=s/2+1) lod1++;
                  for (s=height; s>3; s=s/2+1) lod2++;
                  }
               else
                  {
                  for (s=width; s>2; s/=2) lod1++;
                  for (s=height; s>2; s/=2) lod2++;
                  }

            int lods=min(min(lod1,lod2),pyramid);

            // for all LODs
            for (m=0; m<lods+2; m++)
               {
               if (m<lods+1)
                  {
                  unsigned char *tileH=new unsigned char[2*width*height];
                  unsigned char *tileRGB=new unsigned char[3*width*height];

                  mcount=0;

                  // for each point
                  for (k=0; k<width; k++)
                     for (l=0; l<height; l++)
                        {
                        // calculate actual sample position
                        posx0=LONSUB(tileposx,-(float)k/(width-1)*tilesizex);
                        posy0=tileposy+(float)l/(height-1)*tilesizey;

                        sample0=0.0f;
                        rgb0[0]=rgb0[1]=rgb0[2]=0.0f;

                        scount=0;

                        // calculate number of samples
                        if (comps[0]!=3) sn=1;
                        else sn=max(CONFIGURE_SUPERSAMPLING,1);

                        // increase number of samples when sampling down textures
                        if (comps[0]==3) sn*=(1<<min(m,CONFIGURE_DOWNSAMPLING));

                        // foreach supersample
                        for (si=-0.5f+0.5f/sn; si<0.5f; si+=1.0f/sn)
                           for (sj=-0.5f+0.5f/sn; sj<0.5f; sj+=1.0f/sn)
                              {
                              posx=LONSUB(posx0,-si*tilesizex/(width-1));
                              posy=posy0+sj*tilesizey/(height-1);

                              if (fabs(posy)>90*60*60) continue;

                              sample=missings[0];
                              rgb[0]=rgb[1]=rgb[2]=0.0f;

                              // for each relevant grid
                              for (vn=0; vn<vnum; vn++)
                                 {
                                 n=valid[vn];

                                 // supersample grid
                                 elev=transformpoint(maps[n],
                                                     widths[n],heights[n],comps[n],
                                                     centersx[n],centersy[n],
                                                     extentsx[n],extentsy[n],
                                                     &coords[8*n],&utm_coords[8*n],utm_zones[n],utm_datums[n],
                                                     scalings[n],missings[n],
                                                     posx,posy,
                                                     rgb);

                                 // check for missing supersamples
                                 if (elev!=missings[n])
                                    if (!additives[n])
                                       {
                                       sample=elev;
                                       vn=va;
                                       }
                                    else
                                       if (sample!=missings[0]) sample+=elev;
                                 }

                              // check for missing elevation
                              if (sample!=missings[0])
                                 {
                                 sample0+=sample;
                                 scount++;
                                 }

                              // check for missing RGB color
                              if (rgb[0]>0.0f || rgb[1]>0.0f || rgb[2]>0.0f)
                                 {
                                 rgb0[0]+=rgb[0];
                                 rgb0[1]+=rgb[1];
                                 rgb0[2]+=rgb[2];
                                 scount++;
                                 }
                              }

                        // check for missing samples
                        if (scount>0)
                           {
                           h=ftrc(32767.0f*sample0/scount/maxelev+0.5f);

                           // calculate elevation range
                           if (m==0)
                              {
                              if (h<rangemin) rangemin=h;
                              if (h>rangemax) rangemax=h;
                              }

                           rgb0[0]/=scount;
                           rgb0[1]/=scount;
                           rgb0[2]/=scount;
                           }
                        else
                           {
                           h=missings[0];
                           mcount++;
                           }

                        // convert to 16 bit
                        if (h>32767) h=32767;
                        else if (h<0)
                           {
                           h=65536+h;
                           if (h<32768) h=32768;
                           }

                        // store elevation
                        tileH[2*(k+(height-1-l)*width)]=h/256;
                        tileH[2*(k+(height-1-l)*width)+1]=h%256;

                        // store color
                        tileRGB[3*(k+(height-1-l)*width)]=ftrc(rgb0[0]+0.5f);
                        tileRGB[3*(k+(height-1-l)*width)+1]=ftrc(rgb0[1]+0.5f);
                        tileRGB[3*(k+(height-1-l)*width)+2]=ftrc(rgb0[2]+0.5f);
                        }

                  if (m==0)
                     if (mcount>0) printf("WARNING: missing values=%d\n",mcount);

                  int maxval=32767;

                  // convert to 8 bit
                  if (comps[0]==1)
                     {
                     unsigned char *tileH8=new unsigned char[width*height];

                     if (m==0)
                        {
                        rangemin=255.0f;
                        rangemax=0.0f;
                        }

                     for (n=0; n<width*height; n++)
                        {
                        h=256*tileH[2*n]+tileH[2*n+1];
                        if (h>32767 || h==missings[0]) h=0;

                        tileH8[n]=ftrc(255.0f/32767.0f*h+0.5f);

                        // recalculate elevation range
                        if (m==0)
                           {
                           if (tileH8[n]<rangemin) rangemin=tileH8[n];
                           if (tileH8[n]>rangemax) rangemax=tileH8[n];
                           }
                        }

                     delete[] tileH;
                     tileH=tileH8;

                     maxval=255;
                     }

                  PNMcomment comment;

                  // construct PNM comment
                  putPNMparams(&comment,
                               "resampled with libMini",
                               0,0,0,4,
                               tileposx,tileposy,
                               tileposx,tileposy+tilesizey,
                               LONSUB(tileposx,-tilesizex),tileposy+tilesizey,
                               LONSUB(tileposx,-tilesizex),tileposy,
                               tilesizex/(width-1),tilesizey/(height-1),
                               2,maxelev/maxval,missings[0]);

                  // write height field
                  if (comps[0]!=3)
                     {
                     if (m==0)
                        if (basepath==NULL) snprintf(filename,MAX_STR,"tile.%d-%d.pgm",i+1,tilesy-j);
                        else snprintf(filename,MAX_STR,"%s%s/tile.%d-%d.pgm",CONFIGURE_TILESETPATH,basepath,i+1,tilesy-j);
                     else
                        if (basepath==NULL) snprintf(filename,MAX_STR,"tile.%d-%d.pgm%d",i+1,tilesy-j,m);
                        else snprintf(filename,MAX_STR,"%s%s/tile.%d-%d.pgm%d",CONFIGURE_TILESETPATH,basepath,i+1,tilesy-j,m);

                     writePNMimage(filename,tileH,width,height,comps[0],&comment);
                     }

                  delete[] tileH;

                  // write texture map
                  if (comps[0]==3)
                     {
                     if (m==0)
                        if (basepath==NULL) snprintf(filename,MAX_STR,"tile.%d-%d.ppm",i+1,tilesy-j);
                        else snprintf(filename,MAX_STR,"%s%s/tile.%d-%d.ppm",CONFIGURE_TILESETPATH,basepath,i+1,tilesy-j);
                     else
                        if (basepath==NULL) snprintf(filename,MAX_STR,"tile.%d-%d.ppm%d",i+1,tilesy-j,m);
                        else snprintf(filename,MAX_STR,"%s%s/tile.%d-%d.ppm%d",CONFIGURE_TILESETPATH,basepath,i+1,tilesy-j,m);

                     writePNMimage(filename,tileRGB,width,height,comps[0],&comment);
                     }

                  delete[] tileRGB;
                  }

               // write startup file
               if (CONFIGURE_STARTUPFILE!=0)
                  {
                  // append height field
                  if (comps[0]!=3)
                     {
                     if (m==0)
                        if (basepath==NULL) snprintf(filename,MAX_STR,"tile.%d-%d.pgm",i+1,tilesy-j);
                        else snprintf(filename,MAX_STR,"%s%s/tile.%d-%d.pgm",CONFIGURE_TILESETPATH,basepath,i+1,tilesy-j);
                     else
                        if (basepath==NULL) snprintf(filename,MAX_STR,"tile.%d-%d.pgm%d",i+1,tilesy-j,m);
                        else snprintf(filename,MAX_STR,"%s%s/tile.%d-%d.pgm%d",CONFIGURE_TILESETPATH,basepath,i+1,tilesy-j,m);

                     if (m<lods+1) fprintf(startup,"%s %d %d %d %d %g %g %d %d\n",filename,1,1,1,0,rangemin,rangemax,width,height);
                     else fprintf(startup,"%s %d %d %d %d %g %g %d %d\n",filename,0,0,1,0,1.0f,0.0f,0,0);
                     }

                  // append texture map
                  if (comps[0]==3)
                     {
                     if (m==0)
                        if (basepath==NULL) snprintf(filename,MAX_STR,"tile.%d-%d.ppm",i+1,tilesy-j);
                        else snprintf(filename,MAX_STR,"%s%s/tile.%d-%d.ppm",CONFIGURE_TILESETPATH,basepath,i+1,tilesy-j);
                     else
                        if (basepath==NULL) snprintf(filename,MAX_STR,"tile.%d-%d.ppm%d",i+1,tilesy-j,m);
                        else snprintf(filename,MAX_STR,"%s%s/tile.%d-%d.ppm%d",CONFIGURE_TILESETPATH,basepath,i+1,tilesy-j,m);

                     if (m<lods+1) fprintf(startup,"%s %d %d %d %d %g %g %d %d\n",filename,1,1,1,1,1.0f,0.0f,width,height);
                     else fprintf(startup,"%s %d %d %d %d %g %g %d %d\n",filename,0,0,1,1,1.0f,0.0f,0,0);
                     }
                  }

               if (comps[0]!=3)
                  {
                  width=width/2+1;
                  height=height/2+1;
                  }
               else
                  {
                  width/=2;
                  height/=2;
                  }
               }

            delete[] valid;
            }

      // close startup file
      if (CONFIGURE_STARTUPFILE!=0) fclose(startup);

      // open tileset file
      if (comps[0]!=3) snprintf(filename,MAX_STR,"%s%s%s",CONFIGURE_TILESETPATH,CONFIGURE_STDPREFIX,CONFIGURE_TILESETNAME);
      else snprintf(filename,MAX_STR,"%s%s%s",CONFIGURE_TILESETPATH,CONFIGURE_RGBPREFIX,CONFIGURE_TILESETNAME);
      if ((tileset=fopen(filename,"wb"))==NULL) ERRORMSG();

      // output number of tiles, bounding box, and maximum texture size
      fprintf(tileset,"tilesx=%d\n",tilesx);
      fprintf(tileset,"tilesy=%d\n",tilesy);
      fprintf(tileset,"centerx=%g arc-seconds\n",centersx[0]);
      fprintf(tileset,"centery=%g arc-seconds\n",centersy[0]);
      fprintf(tileset,"sizex=%g arc-seconds\n",sizex);
      fprintf(tileset,"sizey=%g arc-seconds\n",sizey);
      fprintf(tileset,"maxsize=%d\n",realmaxsize);

      // output maximum elevation
      if (comps[0]!=3) fprintf(tileset,"maxelev=%g meters\n",maxelev);

      // close tileset file
      fclose(tileset);
      }

   // output columns and rows
   if (cols!=NULL) *cols=tilesx;
   if (rows!=NULL) *rows=tilesy;

   // 5 output parameters
   if (outparams!=NULL)
      {
      outparams[0]=tilesizex;
      outparams[1]=tilesizey;

      outparams[2]=centersx[0];
      outparams[3]=centersy[0];

      outparams[4]=maxelev;
      }

   // output maximum size
   if (outmaxsize!=NULL) *outmaxsize=realmaxsize;

   for (n=0; n<num; n++)
      if (maps[n]!=NULL) free(maps[n]);

   delete[] maps;

   delete[] widths;
   delete[] heights;
   delete[] comps;

   delete[] comms;

   delete[] centersx;
   delete[] centersy;

   delete[] extentsx;
   delete[] extentsy;

   delete[] coords;
   delete[] cellsizes;

   delete[] utm_coords;
   delete[] utm_zones;
   delete[] utm_datums;

   delete[] scalings;
   delete[] additives;

   delete[] missings;

   delete[] maxelevs;
   }

// generate the normal maps of a grid collection
void normalize(int num,
               char **grid,
               char *basepath,
               float *centerx,float *centery,
               float *extentx,float *extenty,
               float *scaling,
               int missing,
               int border)
   {
   int i,n;

   char filename[MAX_STR];

   if (num<1) ERRORMSG();

   unsigned char *hmap,*nmap;

   int width,height,components;
   PNMcomment comment;

   float coord[8],cellsize[2];

   float utm_coord[8];
   int utm_zone,utm_datum;

   float centerx0,centery0;
   float extentx0,extenty0;
   float scaling0;
   int missing0;

   float as2m[2];

   // for all grids
   for (n=0; n<num; n++)
      {
      if (strrchr(grid[n],'/')==NULL)
         if (basepath==NULL) snprintf(filename,MAX_STR,"%snormalized.%s",CONFIGURE_TILESETPATH,grid[n]);
         else snprintf(filename,MAX_STR,"%s%s/normalized.%s",CONFIGURE_TILESETPATH,basepath,grid[n]);
      else
         if (basepath==NULL) snprintf(filename,MAX_STR,"%snormalized.%s",CONFIGURE_TILESETPATH,strrchr(grid[n],'/')+1);
         else snprintf(filename,MAX_STR,"%s%s/normalized.%s",CONFIGURE_TILESETPATH,basepath,strrchr(grid[n],'/')+1);

      // check for normalized file
      if ((nmap=readPNMfile(filename,
                            &width,&height,&components,
                            &comment))==NULL)
         {
         // load grid file
         if ((hmap=readPNMfile(grid[n],
                               &width,&height,&components,
                               &comment))==NULL) break;

         if (width<2 || height<2 || (components!=1 && components!=2)) ERRORMSG();

         printf("normalizing grid[%d]=\"%s\"\n",n+1,grid[n]);

         // calculate grid extents
         if (centerx==NULL || centery==NULL ||
             extentx==NULL || extenty==NULL ||
             scaling==NULL)
            {
            if (getPNMparams(&comment,
                             coord,cellsize,
                             &scaling0,&missing0,
                             &utm_zone,&utm_datum)==0) ERRORMSG();

            // transform corners
            if (utm_zone!=0)
               {
               for (i=0; i<8; i++) utm_coord[i]=coord[i];

               miniutm::UTM2LL(coord[0],coord[1],utm_zone,utm_datum,&coord[1],&coord[0]);
               miniutm::UTM2LL(coord[2],coord[3],utm_zone,utm_datum,&coord[3],&coord[2]);
               miniutm::UTM2LL(coord[4],coord[5],utm_zone,utm_datum,&coord[5],&coord[4]);
               miniutm::UTM2LL(coord[6],coord[7],utm_zone,utm_datum,&coord[7],&coord[6]);
               }

            centerx0=LONLERP(LONMEAN(coord[0],coord[2]),LONMEAN(coord[4],coord[6]));
            centery0=(coord[1]+coord[3]+coord[5]+coord[7])/4.0f;

            extentx0=(fsqrt(fsqr(LONADD(coord[6],-coord[0]))+
                            (coord[7]-coord[1])*(coord[7]-coord[1]))+
                      fsqrt(fsqr(LONADD(coord[4],-coord[2]))+
                            (coord[5]-coord[3])*(coord[5]-coord[3])))/2.0f;

            extenty0=(fsqrt(fsqr(LONSUB(coord[2],coord[0]))+
                            (coord[3]-coord[1])*(coord[3]-coord[1]))+
                      fsqrt(fsqr(LONSUB(coord[4],coord[6]))+
                            (coord[5]-coord[7])*(coord[5]-coord[7])))/2.0f;
            }
         else
            {
            if (extentx[n]>360*60*60 || extenty[n]>180*60*60 || scaling[n]<0.0f) ERRORMSG();

            centerx0=centerx[n];
            centery0=centery[n];

            extentx0=extentx[n];
            extenty0=extenty[n];

            scaling0=scaling[n];
            missing0=missing;
            }

         miniutm::arcsec2meter(centery0,as2m);

         nmap=normalizemap(hmap,
                           width,height,components,
                           extentx0*as2m[0],extenty0*as2m[1],
                           scaling0,missing0,
                           (n==0)?1:border);

         free(hmap);

         if (utm_zone!=0)
            for (i=0; i<8; i++) coord[i]=utm_coord[i];

         putPNMparams(&comment,
                      "normalized with libMini",
                      (utm_zone==0)?0:1,
                      utm_zone,utm_datum,
                      (utm_zone==0)?4:2,
                      coord[0],coord[1],
                      coord[2],coord[3],
                      coord[4],coord[5],
                      coord[6],coord[7],
                      cellsize[0],cellsize[1],
                      2,scaling0,missing0);

         writePNMimage(filename,nmap,width,height,3,&comment);
         }

      free(nmap);
      }
   }

// configuring
void configure_supersampling(int supersampling) {CONFIGURE_SUPERSAMPLING=supersampling;}
void configure_downsampling(int downsampling) {CONFIGURE_DOWNSAMPLING=downsampling;}
void configure_startupfile(int startupfile) {CONFIGURE_STARTUPFILE=startupfile;}
void configure_tilesetpath(char *tilesetpath) {strncpy(CONFIGURE_TILESETPATH,tilesetpath,MAX_STR);}
void configure_stdprefix(char *stdprefix) {strncpy(CONFIGURE_STDPREFIX,stdprefix,MAX_STR);}
void configure_rgbprefix(char *rgbprefix) {strncpy(CONFIGURE_RGBPREFIX,rgbprefix,MAX_STR);}
void configure_tilesetname(char *tilesetname) {strncpy(CONFIGURE_TILESETNAME,tilesetname,MAX_STR);}
void configure_startupname(char *startupname) {strncpy(CONFIGURE_STARTUPNAME,startupname,MAX_STR);}

// object data for getelevation wrapper
typedef struct
   {
   unsigned char *hfield;
   int components;
   }
pnmsample_obj;

// getelevation wrapper
short int getelevation(int i,int j,int size,void *data)
   {
   pnmsample_obj *objref=(pnmsample_obj *)data;

   if (objref->components==1) return(objref->hfield[i+j*size]);
   else if (objref->components==2)
      {
      int k=2*(i+j*size);
      int h1=objref->hfield[k];
      int h2=objref->hfield[k+1];

      return((short int)(256*h1+h2));
      }
   else ERRORMSG();

   return(0);
   }

// calculate an RGB normal map
unsigned char *normalizemap(unsigned char *hfield,
                            int width,int height,int components,
                            float extentx,float extenty,
                            float scaling,
                            int missing,
                            int border)
   {
   int i,j;

   short int *hmap;
   unsigned char *nmap;

   int e,e1,e2,e3,e4;
   float dx,dy;

   float nx,ny,nt;
   float l;

   if (width<2 || height<2 || components==3) ERRORMSG();

   if ((hmap=(short int *)malloc(width*height*sizeof(short int)))==NULL) ERRORMSG();
   if ((nmap=(unsigned char *)malloc(3*width*height))==NULL) ERRORMSG();

   if (components==1)
      for (i=0; i<width*height; i++) hmap[i]=hfield[i];
   else
      for (i=0; i<width*height; i++)
         {
         e=256*hfield[2*i]+hfield[2*i+1];
         if (e>32767) e-=65536;
         hmap[i]=e;
         }

   extentx*=2.0f/(width-1);
   extenty*=2.0f/(height-1);

   for (i=0; i<width; i++)
      for (j=0; j<height; j++)
         {
         dx=extentx;
         dy=extenty;

         if (border!=0) e1=e2=e3=e4=hmap[i+j*width];
         else e1=e2=e3=e4=missing;

         if (i>0) e1=hmap[i-1+j*width];
         else dx/=2.0f;

         if (i<width-1) e2=hmap[i+1+j*width];
         else dx/=2.0f;

         if (j>0) e3=hmap[i+(j-1)*width];
         else dy/=2.0f;

         if (j<height-1) e4=hmap[i+(j+1)*width];
         else dy/=2.0f;

         if (e1==missing || e2==missing || e3==missing || e4==missing)
            {
            nx=ny=0.0f;
            nt=0.0f;
            }
         else
            {
            nx=scaling*(e1-e2)/dx;
            ny=scaling*(e4-e3)/dy;
            nt=1.0f;

            l=1.0f/fsqrt(1.0f+nx*nx+ny*ny);

            nx*=l;
            ny*=l;

            nx=nx/2.0f+0.5f;
            ny=ny/2.0f+0.5f;
            }

         nmap[3*(i+j*width)]=ftrc(255.0f*nx+0.5f);
         nmap[3*(i+j*width)+1]=ftrc(255.0f*ny+0.5f);
         nmap[3*(i+j*width)+2]=ftrc(255.0f*nt+0.5f);
         }

   free(hmap);

   return(nmap);
   }

// calculate an RGB normal map from a DEM tile
unsigned char *normalmap(unsigned char *hfield,
                         int size,int components,
                         int width,int height,
                         float extentx,float extenty,
                         float scaling,float sealevel,float snowline)
   {
   int i,j;

   pnmsample_obj obj;
   void *d2map;

   int size2;
   float extent;

   float nx,nz,h;
   unsigned char *nmap;

   if (width<1 || height<1 || components==3 ||
       extentx<=0.0f || extenty<=0.0f || scaling<0.0f ||
       snowline<=sealevel) ERRORMSG();

   obj.components=components;
   obj.hfield=hfield;

   size2=size;
   extent=extentx;

   mini::initmap(NULL,&d2map,&size2,&extent,scaling,extentx/extenty,getelevation,&obj);

   if ((nmap=(unsigned char *)malloc(3*width*height))==NULL) ERRORMSG();

   for (i=0; i<width; i++)
      for (j=0; j<height; j++)
         {
         mini::getnormal((float)i/(width-1),1.0f-(float)j/(height-1),&nx,&nz);

         nmap[3*(i+j*width)]=ftrc(255.0f*(0.5f+0.5f*nx)+0.5f);
         nmap[3*(i+j*width)+1]=ftrc(255.0f*(0.5f-0.5f*nz)+0.5f);

         mini::getheight((float)i/(width-1),1.0f-(float)j/(height-1),&h);

         nmap[3*(i+j*width)+2]=ftrc(255.0f*fsqrt(fmax(fmin((h-sealevel)/(snowline-sealevel),1.0f),0.0f))+0.5f);
         }

   mini::deletemaps();

   return(nmap);
   }

// calculate a texture map from a DEM tile
void texturemap(char *heightfile,
                char *texturefile,
                int twidth,int theight,
                void (*shader)(float nx,float ny,float nz,float elev,float *rgb),
                float sealevel,float snowline)
   {
   int i;

   unsigned char *image;
   int width,height,components;
   PNMcomment comment;

   float coord[8],cellsize[2],scaling;
   int utm_zone,utm_datum,missing;

   float xdim,zdim;
   float centerz;

   float as2m[2];

   unsigned char *nmap,*tmap;

   float nx,ny,nz;
   float elev;

   float rgb[3];

   if (twidth<2 || theight<2) ERRORMSG();

   if ((image=readPNMfile(texturefile,&width,&height,&components))!=NULL)
      {
      if (width!=twidth || height!=theight || components!=3) ERRORMSG();
      free(image);
      return;
      }

   if ((image=readPNMfile(heightfile,&width,&height,&components,&comment))==NULL) ERRORMSG();
   if (width!=height || (components!=1 && components!=2)) ERRORMSG();

   if (getPNMparams(&comment,
                    coord,cellsize,
                    &scaling,&missing,
                    &utm_zone,&utm_datum)==0) ERRORMSG();

   if (utm_zone==0)
      {
      xdim=fsqrt(fsqr(LONADD(coord[6],-coord[0]))+fsqr(coord[7]-coord[1]));
      zdim=fsqrt(fsqr(coord[3]-coord[1])+fsqr(LONSUB(coord[2],coord[0])));

      centerz=(coord[1]+coord[3]+coord[5]+coord[7])/4.0f;
      miniutm::arcsec2meter(centerz,as2m);

      xdim*=as2m[0];
      zdim*=as2m[1];
      }
   else
      {
      xdim=fsqrt(fsqr(coord[6]-coord[0])+fsqr(coord[7]-coord[1]));
      zdim=fsqrt(fsqr(coord[3]-coord[1])+fsqr(coord[2]-coord[0]));
      }

   nmap=normalmap(image,width,components,twidth,theight,
                  xdim/(width-1),zdim/(height-1),
                  scaling,sealevel,snowline);

   free(image);

   if ((tmap=(unsigned char *)malloc(3*twidth*theight))==NULL) ERRORMSG();

   for (i=0; i<twidth*theight; i++)
      {
      nx=nmap[3*i]/127.5f-1.0f;
      nz=nmap[3*i+1]/127.5f-1.0f;
      ny=fsqrt(1.0f-nx*nx-nz*nz);

      elev=fsqr(nmap[3*i+2]/255.0f);

      shader(nx,ny,nz,elev,rgb);

      tmap[3*i]=ftrc(255.0f*rgb[0]+0.5f);
      tmap[3*i+1]=ftrc(255.0f*rgb[1]+0.5f);
      tmap[3*i+2]=ftrc(255.0f*rgb[2]+0.5f);
      }

   free(nmap);

   writePNMimage(texturefile,tmap,twidth,theight,3,&comment);

   free(tmap);
   }

// merge normalized RGB image #2 into gray-scale image #1
void mergeRGBimages(unsigned char *image1,int width1,int height1,
                    unsigned char *image2,int width2,int height2)
   {
   int i,j;

   float x,y;
   int xi,yi;
   float xr,yr;

   unsigned char *ptr1,*ptr2;

   if (width1<2 || height1<2 || width2<2 || height2<2) ERRORMSG();

   for (i=0; i<width1; i++)
      for (j=0; j<height1; j++)
         {
         x=(float)i*(width2-1)/(width1-1);
         y=(float)j*(height2-1)/(height1-1);

         xi=ftrc(x);
         yi=ftrc(y);

         xr=x-xi;
         yr=y-yi;

         if (xi==width2-1)
            {
            xi=width2-2;
            xr=1.0f;
            }

         if (yi==height2-1)
            {
            yi=height2-2;
            yr=1.0f;
            }

         ptr1=&image1[3*(i+j*width1)];
         ptr2=&image2[3*(xi+yi*width2)];

         ptr1[2]=ftrc(0.3f*ptr1[0]+0.5f*ptr1[1]+0.2f*ptr1[2]+0.5f);

         ptr1[0]=ftrc((1.0f-yr)*((1.0f-xr)*ptr2[0]+xr*ptr2[3])+yr*((1.0f-xr)*ptr2[3*width2]+xr*ptr2[3*width2+3])+0.5f);
         ptr1[1]=ftrc((1.0f-yr)*((1.0f-xr)*ptr2[1]+xr*ptr2[4])+yr*((1.0f-xr)*ptr2[3*width2+1]+xr*ptr2[3*width2+4])+0.5f);
         }
   }

}
