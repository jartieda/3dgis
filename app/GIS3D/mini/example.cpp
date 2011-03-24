#include <mini/minibase.h>
#include <mini/mini.h>

#ifndef __APPLE__
#include <GL/glut.h>
#else
#include <GLUT/glut.h>
#endif

static int winwidth,winheight,winid;

void displayfunc()
   {
   float res=1000.0f;
   float ex=0.0f,ey=10.0f,ez=30.0f;
   float fx=0.0f,fy=10.0f,fz=30.0f;
   float dx=0.0f,dy=-0.25f,dz=-1.0f;
   float ux=0.0f,uy=1.0f,uz=0.0f;
   float fovy=60.0f;
   float aspect=1.0f;
   float nearp=1.0f;
   float farp=100.0f;

   aspect=(float)winwidth/winheight;

   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(fovy,aspect,nearp,farp);

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   gluLookAt(ex,ey,ez,ex+dx,ey+dy,ez+dz,ux,uy,uz);

   mini::drawlandscape(res,
                       ex,ey,ez,
                       fx,fy,fz,
                       dx,dy,dz,
                       ux,uy,uz,
                       fovy,aspect,
                       nearp,farp);

   glutSwapBuffers();
   }

void reshapefunc(int width,int height)
   {
   winwidth=width;
   winheight=height;

   glViewport(0,0,width,height);

   displayfunc();
   }

void keyboardfunc(unsigned char key,int x,int y)
   {
   float mousex,mousey;

   mousex=(float)x/(winwidth-1);
   mousey=(float)y/(winwidth-1);

   if (key=='q' || key==27)
      {
      mini::deletemaps();
      glutDestroyWindow(winid);
      exit(0);
      }
   }

int main(int argc,char *argv[])
   {
   short int hfield[]={0,0,0,
                       0,5,0,
                       0,0,0};

   int size=3;

   float dim=10.0f;
   float scale=1.0f;

   void *map,*d2map;

   unsigned char texture[]={255,63,63, 255,63,63, 255,63,63, 255,63,63,
                            255,63,63, 63,63,255, 63,63,255, 255,63,63,
                            255,63,63, 63,63,255, 63,63,255, 255,63,63,
                            255,63,63, 255,63,63, 255,63,63, 255,63,63};

   int width=4,height=4;
   int mipmaps=1;

   int texid;

   unsigned char ffield[]={2,3,2,
                           3,1,3,
                           2,3,2};

   int fogsize=3;

   float lambda=1.0f;
   float displace=5.0E-3f;
   float emission=0.05f;
   float attenuation=1.0f;

   void *fogmap;

   winwidth=winheight=512;

   glutInit(&argc,argv);
   glutInitWindowSize(winwidth,winheight);
   glutInitDisplayMode(GLUT_RGB|GLUT_ALPHA|GLUT_DEPTH|GLUT_DOUBLE);
   winid=glutCreateWindow("libMini Example");

   glutDisplayFunc(displayfunc);
   glutReshapeFunc(reshapefunc);
   glutMouseFunc(NULL);
   glutMotionFunc(NULL);
   glutKeyboardFunc(keyboardfunc);
   glutSpecialFunc(NULL);
   glutIdleFunc(NULL);

   map=mini::initmap(hfield,&d2map,&size,&dim,scale);

   texid=mini::inittexmap(texture,&width,&height);

   fogmap=mini::initfogmap(ffield,fogsize,lambda,displace,emission,attenuation);

   mini::setmaps(map,d2map,size,dim,scale,
                 texid,width,height,mipmaps,
                 1.0f,0.0f,0.0f,0.0f,NULL,NULL,
                 fogmap,lambda,displace,
                 emission);

   glutMainLoop();

   return(0);
   }
