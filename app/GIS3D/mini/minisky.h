// (c) by Stefan Roettger

#ifndef MINISKY_H
#define MINISKY_H

class minisky
   {
   public:

   //! default constructor
   minisky();

   //! destructor
   ~minisky();

   void loadskydome(char *skyfile,
                    float mx=0.0f,float my=0.0f,float mz=0.0f,
                    float diameter=1.0f,
                    float scale=1.0f);

   void setpos(float mx,float my,float mz,
               float diameter,
               float scale=1.0f);

   void drawskydome();

   protected:

   private:

   int ALPHA_STEPS;
   int BETA_STEPS;

   inline float mapalpha(float alpha,int texsize);
   inline float mapbeta(float beta,int texsize);

   inline void drawvertex(float alpha,float beta,int width,int height);

   int texid;
   int width,height,components;

   float centerx,centery,centerz;
   float radius,scaley;

   int loaded;
   };

#endif
