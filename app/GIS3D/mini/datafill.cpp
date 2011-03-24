// (c) by Stefan Roettger

#include "minibase.h"

#include "datafill.h"

// fill-in no-data values by region growing
unsigned int datafill::fillin_by_regiongrowing(int radius_stop,int radius_start)
   {
   int r;

   unsigned int count;

   count=0;

   for (r=radius_start; r<=radius_stop; r++) count+=fillin(r);

   return(count);
   }

// fill-in algorithm
// replaces no-data values by repeated region growing
// smoothly extrapolates the filled-in value via partial derivatives
// restricts the fill-in operation to concavities with a diameter of less than radius^2+1 pixels
unsigned int datafill::fillin(int radius)
   {
   unsigned int count;

   int i,j,k,t;
   int m,n,o;

   databuf buf;
   databuf cnt;
   databuf tmp;

   int size;
   int sizex,sizey,sizez;
   int thres;

   BOOLINT done;

   int cells;

   float v1,v2;
   float dx,dy,dz;
   int dxnum,dynum,dznum;
   float val,weight,sum;

   if (type!=DATABUF_TYPE_FLOAT) ERRORMSG();

   count=0;

   if (checknodata()!=0)
      {
      // copy working buffer
      buf.duplicate(this);

      // allocate counting buffer
      cnt.alloc(xsize,ysize,zsize,tsteps,1);
      tmp.alloc(xsize,ysize,zsize,tsteps,1);

      // calculate foot print size
      size=2*radius+1;
      if (size<3) size=3;

      done=FALSE;

      while (!done)
         {
         done=TRUE;

         // calculate foot print size in x/y/z-direction
         if (xsize<2)
            {
            sizex=1;
            sizey=1;
            sizez=1;
            }
         else if (ysize<2)
            {
            sizex=size;
            sizey=1;
            sizez=1;
            }
         else if (zsize<2)
            {
            sizex=size;
            sizey=size;
            sizez=1;
            }
         else
            {
            sizex=size;
            sizey=size;
            sizez=size;
            }

         // calculate growing threshold
         thres=(sizex*sizey*sizez+1)/2;

         // clear counting buffer
         cnt.clear();

         // search for no-data values
         for (t=0; t<(int)tsteps; t++)
            for (i=0; i<(int)xsize; i++)
               for (j=0; j<(int)ysize; j++)
                  for (k=0; k<(int)zsize; k++)
                     if (getval(i,j,k,t)!=nodata) cnt.setval(i,j,k,t,1);

         // accumulate no-data values in x-direction
         if (xsize>1)
            for (t=0; t<(int)tsteps; t++)
               for (j=0; j<(int)ysize; j++)
                  for (k=0; k<(int)zsize; k++)
                     {
                     cells=0;

                     for (i=-sizex/2; i<(int)xsize; i++)
                        {
                        if (i-sizex/2-1>=0) cells-=ftrc(cnt.getval(i-sizex/2-1,j,k,t)+0.5f);
                        if (i+sizex/2<(int)xsize) cells+=ftrc(cnt.getval(i+sizex/2,j,k,t)+0.5f);

                        if (i>=0) tmp.setval(i,j,k,t,cells);
                        }
                     }

         // copy counting buffer back
         cnt.copy(&tmp);

         // accumulate no-data values in y-direction
         if (ysize>1)
            for (t=0; t<(int)tsteps; t++)
               for (i=0; i<(int)xsize; i++)
                  for (k=0; k<(int)zsize; k++)
                     {
                     cells=0;

                     for (j=-sizey/2; j<(int)ysize; j++)
                        {
                        if (j-sizey/2-1>=0) cells-=ftrc(cnt.getval(i,j-sizey/2-1,k,t)+0.5f);
                        if (j+sizey/2<(int)ysize) cells+=ftrc(cnt.getval(i,j+sizey/2,k,t)+0.5f);

                        if (j>=0) tmp.setval(i,j,k,t,cells);
                        }
                     }

         // copy counting buffer back
         cnt.copy(&tmp);

         // accumulate no-data values in z-direction
         if (zsize>1)
            for (t=0; t<(int)tsteps; t++)
               for (i=0; i<(int)xsize; i++)
                  for (j=0; j<(int)ysize; j++)
                     {
                     cells=0;

                     for (k=-sizez/2; k<(int)zsize; k++)
                        {
                        if (k-sizez/2-1>=0) cells-=ftrc(cnt.getval(i,j,k-sizez/2-1,t)+0.5f);
                        if (k+sizez/2<(int)zsize) cells+=ftrc(cnt.getval(i,j,k+sizez/2,t)+0.5f);

                        if (k>=0) tmp.setval(i,j,k,t,cells);
                        }
                     }

         // copy counting buffer back
         cnt.copy(&tmp);

         // search for no-data values
         for (t=0; t<(int)tsteps; t++)
            for (i=0; i<(int)xsize; i++)
               for (j=0; j<(int)ysize; j++)
                  for (k=0; k<(int)zsize; k++)
                     if (getval(i,j,k,t)==nodata)
                        // check number of foot print cells against growing threshold
                        if (ftrc(cnt.getval(i,j,k,t)+0.5f)>=thres)
                           {
                           dx=dy=dz=0.0f;
                           dxnum=dynum=dznum=0;

                           // average partial derivatives
                           for (m=-sizex/2; m<=sizex/2; m++)
                              for (n=-sizey/2; n<=sizey/2; n++)
                                 for (o=-sizez/2; o<=sizez/2; o++)
                                    if (i+m>=0 && i+m<(int)xsize && j+n>=0 && j+n<(int)ysize && k+o>=0 && k+o<(int)zsize)
                                       {
                                       v1=getval(i+m,j+n,k+o,t);

                                       if (v1!=nodata)
                                          {
                                          if (i+m-1>=0 && m>-sizex/2)
                                             {
                                             v2=getval(i+m-1,j+n,k+o,t);

                                             if (v2!=nodata)
                                                {
                                                dx+=v1-v2;
                                                dxnum++;
                                                }
                                             }

                                          if (j+n-1>=0 && n>-sizey/2)
                                             {
                                             v2=getval(i+m,j+n-1,k+o,t);

                                             if (v2!=nodata)
                                                {
                                                dy+=v1-v2;
                                                dynum++;
                                                }
                                             }

                                          if (k+o-1>=0 && o>-sizez/2)
                                             {
                                             v2=getval(i+m,j+n,k+o-1,t);

                                             if (v2!=nodata)
                                                {
                                                dz+=v1-v2;
                                                dznum++;
                                                }
                                             }
                                          }
                                       }

                           if (dxnum>0) dx/=dxnum;
                           if (dynum>0) dy/=dynum;
                           if (dznum>0) dz/=dznum;

                           val=0.0f;
                           sum=0.0f;

                           // extrapolate partial derivatives
                           for (m=-sizex/2; m<=sizex/2; m++)
                              for (n=-sizey/2; n<=sizey/2; n++)
                                 for (o=-sizez/2; o<=sizez/2; o++)
                                    if (i+m>=0 && i+m<(int)xsize && j+n>=0 && j+n<(int)ysize && k+o>=0 && k+o<(int)zsize)
                                       {
                                       v1=getval(i+m,j+n,k+o,t);

                                       if (v1!=nodata)
                                          {
                                          v2=v1-m*dx-n*dy-o*dz;
                                          weight=m*m+n*n+o*o;

                                          if (weight>0.0f)
                                             {
                                             val+=v2/weight;
                                             sum+=1.0f/weight;
                                             }
                                          }
                                       }

                           // fill-in extrapolated value
                           if (sum>0.0f)
                              {
                              val/=sum;
                              if ((val-bias)/scaling==nodata) val+=scaling;

                              buf.setval(i,j,k,t,val);
                              count++;

                              done=FALSE;
                              }
                           }

         // copy working buffer back
         copy(&buf);
         }

      // free working buffer
      buf.release();

      // free counting buffer
      cnt.release();
      tmp.release();
      }

   return(count);
   }
