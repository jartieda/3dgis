// (c) by Stefan Roettger

#ifndef MINISHADER_H
#define MINISHADER_H

#include "minicache.h"

class minishader
   {
   public:

   static void setVISshader(minicache *cache,
                            float scale,float exaggeration,
                            float fogstart,float fogend,
                            float fogdensity,
                            float fogcolor[3],
                            float bathystart,float bathyend,
                            float contours,
                            float sealevel,float seabottom,
                            float seacolor[3],
                            float seatrans,float bottomtrans,
                            float bottomcolor[3],
                            float seamodulate);

   static void setVISbathymap(unsigned char *bathymap,
                              int bathywidth,int bathyheight,int bathycomps);

   static void setNPRshader(minicache *cache,
                            float scale,float exaggeration,
                            float fogstart,float fogend,
                            float fogdensity,
                            float fogcolor[3],
                            float bathystart,float bathyend,
                            float contours,
                            float fadefactor,
                            float sealevel,
                            float seacolor[3],float seatrans,
                            float seagray);

   static void setNPRbathymap(unsigned char *bathymap,
                              int bathywidth,int bathyheight,int bathycomps);

   static void unsetshaders(minicache *cache);

   protected:

   private:

   static unsigned char *VISBATHYMAP;
   static int VISBATHYWIDTH,VISBATHYHEIGHT,VISBATHYCOMPS;
   static int VISBATHYMOD;

   static unsigned char *NPRBATHYMAP;
   static int NPRBATHYWIDTH,NPRBATHYHEIGHT,NPRBATHYCOMPS;
   static int NPRBATHYMOD;
   };

#endif
