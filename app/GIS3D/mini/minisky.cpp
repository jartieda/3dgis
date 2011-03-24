// (c) by Stefan Roettger

#include "minibase.h"

#include "pnmbase.h"
#include "miniOGL.h"

#include "minisky.h"

// default constructor
minisky::minisky()
   {
   loaded=FALSE;

   ALPHA_STEPS=30;
   BETA_STEPS=15;
   }

// destructor
minisky::~minisky()
   {if (loaded) deletetexmap(texid);}

inline float minisky::mapalpha(float alpha,int texsize)
   {
   float a=alpha/PI/2.0f;
   return((1.0f-a)*0.5f/texsize+a*(1.0f+0.5f/texsize));
   }

inline float minisky::mapbeta(float beta,int texsize)
   {
   float b=2.0f*beta/PI;
   if (b>1.0f) return(1.0f-0.5f/texsize);
   return((1.0f-b)*0.5f/texsize+b*(1.0f-0.5f/texsize));
   }

inline void minisky::drawvertex(float alpha,float beta,int width,int height)
   {
   texcoord(mapalpha(alpha,width),mapbeta(beta,height));
   fanvertex(fsin(alpha)*fsin(beta),fcos(beta),fcos(alpha)*fsin(beta));
   }

void minisky::loadskydome(char *skyfile,
                          float mx,float my,float mz,
                          float diameter,
                          float scale)
   {
   unsigned char *image;

   if ((image=readPNMfile(skyfile,&width,&height,&components))==NULL) return;
   if (width<2 || height<2 || components!=3) ERRORMSG();

   if (loaded) deletetexmap(texid);
   texid=buildRGBtexmap(image,&width,&height);
   free(image);

   centerx=mx;
   centery=my;
   centerz=mz;

   radius=diameter/2.0f;
   scaley=scale;

   loaded=TRUE;
   }

void minisky::setpos(float mx,float my,float mz,
                     float diameter,
                     float scale)
   {
   centerx=mx;
   centery=my;
   centerz=mz;

   radius=diameter/2.0f;
   scaley=scale;
   }

void minisky::drawskydome()
   {
   int i,j;

   float alpha,beta;

   if (loaded)
      {
      initstate();

      mtxpush();
      mtxtranslate(centerx,centery,centerz);
      mtxscale(radius,radius*scaley,radius);

      disableculling();

      bindtexmap(texid);

      beginfans();
      color(1.0f,1.0f,1.0f);

      alpha=0.0f;
      for (i=0; i<ALPHA_STEPS; i++)
         {
         beta=0.0f;
         for (j=0; j<BETA_STEPS; j++)
            {
            beginfan();
            drawvertex(alpha,beta,width,height);
            drawvertex(alpha+2.0f*PI/ALPHA_STEPS,beta,width,height);
            drawvertex(alpha+2.0f*PI/ALPHA_STEPS,beta+PI/BETA_STEPS,width,height);
            drawvertex(alpha,beta+PI/BETA_STEPS,width,height);
            beta+=PI/BETA_STEPS;
            }
         alpha+=2.0f*PI/ALPHA_STEPS;
         }

      endfans();
      bindtexmap(0);

      enableBFculling();
      mtxpop();

      exitstate();
      }
   }
