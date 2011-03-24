// (c) by Stefan Roettger

#ifndef MINIGLOBE_H
#define MINIGLOBE_H

#include "database.h"

#include "ministrip.h"

#define MINIGLOBE_ONE_RENDER_PHASE miniglobe::ONE_RENDER_PHASE
#define MINIGLOBE_FIRST_RENDER_PHASE miniglobe::FIRST_RENDER_PHASE
#define MINIGLOBE_LAST_RENDER_PHASE miniglobe::LAST_RENDER_PHASE

class miniglobe
   {
   public:

   //! exposed globe types
   enum
      {
      SHAPE_SUN=0,
      SHAPE_MERCURY=1,
      SHAPE_VENUS=2,
      SHAPE_EARTH=3,
      SHAPE_JUPITER=4,
      SHAPE_SATURN=5,
      SHAPE_URANUS=6,
      SHAPE_NEPTUNE=7,
      SHAPE_CERES=8,
      SHAPE_PLUTO=9,
      SHAPE_ERIS=10,
      SHAPE_MOON=42
      }
   SHAPE_ENUM;

   //! exposed render phases
   static const int ONE_RENDER_PHASE=0;    // render all phases in one pass
   static const int FIRST_RENDER_PHASE=1;  // render pass without updating Z
   static const int LAST_RENDER_PHASE=2;   // render pass without updating RGB

   //! default constructor
   miniglobe();

   //! destructor
   ~miniglobe();

   //! set shape of globe
   void setshape(int shape=SHAPE_EARTH);

   //! set scaling of globe
   void setscale(float scale);

   //! set dynamic scaling of globe
   void setdynscale(float scale);

   //! set tesselation of globe
   void settess(int stripes);

   //! set matrix
   void setmatrix(double mtx[16]);

   //! set direct shading parameters
   void setshadedirectparams(float lightdir[3],
                             float lightbias=0.5f,float lightoffset=0.5f);

   //! set direct texturing parameters
   void settexturedirectparams(float lightdir[3],
                               float transbias=4.0f,float transoffset=0.0f);

   //! set fog parameters
   void setfogparams(float fogstart,float fogend,
                     float fogdensity,
                     float *fogcolor);

   //! render the globe
   void render(int phase=ONE_RENDER_PHASE);

   //! configuring
   void configure_frontname(const char *frontname);
   void configure_backname(const char *backname);
   void configure_frontbuf(databuf *frontbuf);
   void configure_backbuf(databuf *backbuf);

   protected:

   int SHAPE;
   float SCALE;
   int STRIPES;

   ministrip *STRIP;
   int SLOT;

   int SHADE;

   int DONE;

   char *CONFIGURE_FRONTNAME;
   char *CONFIGURE_BACKNAME;

   databuf *CONFIGURE_FRONTBUF;
   databuf *CONFIGURE_BACKBUF;

   private:

   void create_globe(float radius,const float color[3]);
   void create_earth(const float color[3]);

   void create_sun();
   void create_earth();
   void create_moon();

   void create_shader(const char *frontame,const char *backname,
                      databuf *frontbuf,databuf *backbuf);
   };

#endif
