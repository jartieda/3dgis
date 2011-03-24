// (c) by Stefan Roettger

#include "minibase.h"

#include "minihsv.h"

namespace minihsv {

// rgb to hsv conversion
void rgb2hsv(float r,float g,float b,float hsv[3])
   {
   float h,s,v;
   float maxv,minv,diff,rdist,gdist,bdist;

   if (r<0.0f || r>1.0f || g<0.0f || g>1.0f || b<0.0f || b>1.0f) ERRORMSG();

   maxv=fmax(r,fmax(g,b));
   minv=fmin(r,fmin(g,b));
   diff=maxv-minv;

   v=maxv;

   if (maxv!=0.0f) s=diff/maxv;
   else s=0.0f;

   if (s==0.0f) h=0.0f;
   else
      {
      rdist=(maxv-r)/diff;
      gdist=(maxv-g)/diff;
      bdist=(maxv-b)/diff;

      if (r==maxv) h=bdist-gdist;
      else if (g==maxv) h=2.0f+rdist-bdist;
      else h=4.0f+gdist-rdist;

      h*=60.0f;
      if (h<0.0f) h+=360.0f;
      }

   hsv[0]=h;
   hsv[1]=s;
   hsv[2]=v;
   }

// hsv to rgb conversion
void hsv2rgb(float hue,float sat,float val,float rgb[3])
   {
   float hue6,r,s,t;

   if (hue<0.0f || sat<0.0f || sat>1.0f || val<0.0f || val>1.0f) ERRORMSG();

   hue/=60.0f;
   hue=hue-6.0f*ftrc(hue/6.0f);
   hue6=hue-ftrc(hue);

   r=val*(1.0f-sat);
   s=val*(1.0f-sat*hue6);
   t=val*(1.0f-sat*(1.0f-hue6));

   switch (ftrc(hue))
        {
        case 0: // red -> yellow
           rgb[0] = val;
           rgb[1] = t;
           rgb[2] = r;
           break;
        case 1: // yellow -> green
           rgb[0] = s;
           rgb[1] = val;
           rgb[2] = r;
           break;
        case 2: // green -> cyan
           rgb[0] = r;
           rgb[1] = val;
           rgb[2] = t;
           break;
        case 3: // cyan -> blue
           rgb[0] = r;
           rgb[1] = s;
           rgb[2] = val;
           break;
        case 4: // blue -> magenta
           rgb[0] = t;
           rgb[1] = r;
           rgb[2] = val;
           break;
        case 5: // magenta -> red
           rgb[0] = val;
           rgb[1] = r;
           rgb[2] = s;
           break;
        }
   }

}
