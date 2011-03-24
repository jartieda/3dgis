#include "mini/minibase.h"
#include "mini/ministub.h"

int size=3;

float dim=10.0f;
float scale=5.0f;

void mybeginfan()
   {printf("beginfan();\n");}
void myfanvertex(float i,float y,float j)
   {printf("fanvertex(%g,%g,%g); // realvertex=(%g,%g,%g)\n",i,y,j,(i-size/2)*dim,y*scale,(size/2-j)*dim);}
void myprismedge(float x,float y,float yf,float z)
   {printf("prismedge(%g,%g,%g,%g);\n",x,y,yf,z);}

int main(int argc,char *argv[])
   {
   float hfield[]={0,0,0,
                   0,1,0,
                   0,0,0};

   unsigned char ffield[]={2,3,2,
                           3,1,3,
                           2,3,2};

   int fogsize=3;

   float lambda=1.0f;
   float displace=5.0E-3f;
   float attenuation=1.0f;

   ministub *stub;

   stub=new ministub(hfield,
                     &size,&dim,scale,
                     1.0f,0.0f,0.0f,0.0f,
                     mybeginfan,myfanvertex,NULL,NULL,NULL,
                     ffield,fogsize,lambda,displace,attenuation,
                     myprismedge);

   float res=1000.0f;
   float ex=0.0f,ey=10.0f,ez=30.0f;
   float dx=0.0f,dy=-0.25f,dz=-1.0f;
   float ux=0.0f,uy=1.0f,uz=0.0f;
   float fovy=60.0f;
   float aspect=1.0f;
   float nearp=1.0f;
   float farp=100.0f;

   stub->draw(res,
              ex,ey,ez,
              dx,dy,dz,
              ux,uy,uz,
              fovy,aspect,
              nearp,farp);

   delete stub;

   return(0);
   }
