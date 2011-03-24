// (c) by Stefan Roettger

// configuration section:

#define VIEWER_WINWIDTH 768
#define VIEWER_WINHEIGHT 512

#define VIEWER_WINTITLE "libMini Viewer"

#define VIEWER_FPS 25.0f

#define VIEWER_SCALE 100.0f
#define VIEWER_EXAGGER 1.0f

#define VIEWER_FOVY 60.0f
#define VIEWER_NEARP 10.0f
#define VIEWER_FARP 10000.0f

#define VIEWER_RES 1.0E3f
#define VIEWER_RANGE 0.001f

#define VIEWER_SEALEVEL -MAXFLOAT

#define VIEWER_UPLIFT 0.1f

#define VIEWER_MAXSPEED 50.0f
#define VIEWER_GRAVITY 9.81f
#define VIEWER_HOVER 1.81f

#define VIEWER_JUMP 20.0f
#define VIEWER_DAMP 10.0f
#define VIEWER_BOUNCE 5.0f

#define VIEWER_BOOST 50.0f
#define VIEWER_SLOW 10.0f

#define VIEWER_ROTATION 0.1f

#define VIEWER_FOGSTART 0.5f
#define VIEWER_FOGDENSITY 0.5f;

#define VIEWER_VOIDSTART 20000.0f;

#define VIEWER_CONTOURS 10.0f

#define VIEWER_SEABOTTOM -10.0f

#define VIEWER_SIGNPOSTHEIGHT 100.0f
#define VIEWER_SIGNPOSTRANGE 0.25f

#define VIEWER_BRICKSIZE 100.0f
#define VIEWER_BRICKRADIUS 1000.0f

#define VIEWER_BRICKSCROLL 0.5f

#define VIEWER_SAVFILE "settings.sav"

#define VIEWER_BATHYSTART 0.0f
#define VIEWER_BATHYEND -10.0f

#define VIEWER_BATHYWIDTH 256
#define VIEWER_BATHYHEIGHT 2
#define VIEWER_BATHYCOMPS 4

static unsigned char VIEWER_BATHYMAP[VIEWER_BATHYWIDTH*4*2];

#define VIEWER_NPRBATHYSTART 0.0f
#define VIEWER_NPRBATHYEND 1000.0f

#define VIEWER_NPRBATHYWIDTH 1024
#define VIEWER_NPRBATHYHEIGHT 2
#define VIEWER_NPRBATHYCOMPS 4

static unsigned char VIEWER_NPRBATHYMAP[VIEWER_NPRBATHYWIDTH*4*2];

#define VIEWER_NPRCONTOURS 100.0f

#define VIEWER_SBASE 0.5f

// main section:

#include <mini/minibase.h>

#include <mini/miniv3f.h>
#include <mini/miniv3d.h>

#include <mini/minihsv.h>

#include <mini/miniOGL.h>

#include <mini/minilayer.h>
#include <mini/miniterrain.h>

#include <mini/minitile.h>
#include <mini/minitext.h>

#include <mini/pnmbase.h>

#include <mini/viewerbase.h>

#ifndef __APPLE__
#include <GL/glut.h>
#else
#include <GLUT/glut.h>
#endif

// maximum string length
#define MAXSTR 1000

// short usage: url to the tiles and textures
static char shorturl[MAXSTR]="";

// long usage: base url and id-path to the tiles and textures
static char baseurl[MAXSTR]="";
static char baseid[MAXSTR]="";

// sub-path to the tiles and textures
static char basepath1[MAXSTR]=""; // elevation
static char basepath2[MAXSTR]=""; // imagery

// window size
static int winwidth,winheight,winid;

// the viewer object
static viewerbase *viewer=NULL;

// the viewing parameters
static viewerbase::VIEWER_PARAMS *params=NULL;

// the earth parameters
static miniearth::MINIEARTH_PARAMS *eparams=NULL;

// the terrain parameters
static miniterrain::MINITERRAIN_PARAMS *tparams=NULL;

// eye point
static minicoord eye;
static double dez,aez;

// viewing angles
static double angle,turn,pitch,incline;

// viewing direction
static miniv3d dir,up,right;

// gliding parameters
static double maxspeed=VIEWER_MAXSPEED,speedinc=0.1,accel=0.1,gravity=0.0,hover=VIEWER_HOVER;

// jumping parameters
static double jump=VIEWER_JUMP,damp=VIEWER_DAMP,bounce=VIEWER_BOUNCE,earthg=VIEWER_GRAVITY,boost=VIEWER_BOOST,slow=VIEWER_SLOW;

// steering parameters
static double oneturn=5.0,oneincline=5.0;

// gliding speed
static double speed,topspeed;

// stereo base
static const float sbase=VIEWER_SBASE;

// consumed time per frame
static double accu_delta=0.0;
static double accu_idle=0.0;
static double avg_delta=0.0;
static double avg_idle=1.0/VIEWER_FPS;
static int avg_count=0;

// command line switches
static int sw_stereo=0;
static int sw_anaglyph=0;
static int sw_full=0;
static int sw_multi=0;
static int sw_reset=0;
static int sw_autos3tc=0;
static int sw_bricks=0;
static int sw_mpass=0;

// user switches
static int sw_hud=0;
static int sw_hide=0;
static int sw_cross=0;

// initialize the viewing parameters
void initparams()
   {
   viewerbase::VIEWER_PARAMS prms;
   miniearth::MINIEARTH_PARAMS eprms;
   miniterrain::MINITERRAIN_PARAMS tprms;

   viewer->get(prms);

   prms.fps=VIEWER_FPS;

   prms.fovy=VIEWER_FOVY;
   prms.nearp=VIEWER_NEARP;
   prms.farp=VIEWER_FARP;

   viewer->set(prms);

   viewer->getearth()->get(eprms);

   eprms.usefog=TRUE;
   eprms.useshaders=TRUE;
   eprms.usediffuse=FALSE;
   eprms.usevisshader=TRUE;
   eprms.usebathymap=FALSE;
   eprms.usecontours=FALSE;
   eprms.usenprshader=FALSE;
   eprms.useskydome=TRUE;
   eprms.usewaypoints=TRUE;
   eprms.usebricks=(sw_bricks==0)?FALSE:TRUE;
   eprms.useearth=TRUE;
   eprms.useflat=FALSE;

   eprms.fogstart=VIEWER_FOGSTART;
   eprms.fogdensity=VIEWER_FOGDENSITY;

   eprms.voidstart=VIEWER_VOIDSTART;

   viewer->getearth()->set(eprms);

   viewer->getearth()->getterrain()->get(tprms);

   tprms.scale=VIEWER_SCALE;
   tprms.exaggeration=VIEWER_EXAGGER;
   tprms.res=VIEWER_RES;
   tprms.range=VIEWER_RANGE;
   tprms.sealevel=VIEWER_SEALEVEL;

   tprms.genmipmaps=TRUE;
   tprms.automipmap=TRUE;

   tprms.autocompress=(sw_autos3tc==0)?FALSE:TRUE;
   tprms.lod0uncompressed=FALSE;

   tprms.contours=VIEWER_CONTOURS;
   tprms.seabottom=VIEWER_SEABOTTOM;

   tprms.bathystart=VIEWER_BATHYSTART;
   tprms.bathyend=VIEWER_BATHYEND;

   tprms.bathymap=VIEWER_BATHYMAP;
   tprms.bathywidth=VIEWER_BATHYWIDTH;
   tprms.bathyheight=VIEWER_BATHYHEIGHT;
   tprms.bathycomps=VIEWER_BATHYCOMPS;

   tprms.nprbathystart=VIEWER_NPRBATHYSTART;
   tprms.nprbathyend=VIEWER_NPRBATHYEND;

   tprms.nprbathymap=VIEWER_NPRBATHYMAP;
   tprms.nprbathywidth=VIEWER_NPRBATHYWIDTH;
   tprms.nprbathyheight=VIEWER_NPRBATHYHEIGHT;
   tprms.nprbathycomps=VIEWER_NPRBATHYCOMPS;

   tprms.nprcontours=VIEWER_NPRCONTOURS;

   tprms.signpostheight=VIEWER_SIGNPOSTHEIGHT;
   tprms.signpostrange=VIEWER_SIGNPOSTRANGE;

   tprms.bricksize=VIEWER_BRICKSIZE;
   tprms.brickradius=VIEWER_BRICKRADIUS;

   tprms.brickpasses=(sw_mpass==0)?1:2;
   tprms.brickscroll=VIEWER_BRICKSCROLL;

   viewer->getearth()->getterrain()->set(tprms);
   }

// initialize the render window
void initwindow(int width,int height)
   {
   viewerbase::VIEWER_PARAMS prms;

   if (sw_full==0)
      {
      winwidth=width;
      winheight=height;
      }
   else
      {
      viewer->get(prms);

      winwidth=glutGameModeGet(GLUT_GAME_MODE_WIDTH);
      winheight=glutGameModeGet(GLUT_GAME_MODE_HEIGHT);

      prms.fps=glutGameModeGet(GLUT_GAME_MODE_REFRESH_RATE);

      if (winwidth<=0 || winheight<=0 || prms.fps<=0.0f)
         {
         winwidth=width;
         winheight=height;

         prms.fps=VIEWER_FPS;
         }

      viewer->set(prms);
      }

   if (winwidth<1) winwidth=1;
   if (winheight<1) winheight=1;

   glViewport(0,0,winwidth,winheight);
   }

// initialize the view point
void initview(minicoord e,double a,double p,double dh=0.0)
   {
   minicoord el;
   float elev;

   minilayer *ref,*nst;

   initwindow(winwidth,winheight);

   eye=e;

   elev=viewer->getearth()->getterrain()->getheight(eye);

   ref=viewer->getearth()->getreference();
   nst=viewer->getearth()->getnearest(eye);

   el=nst->map_g2l(eye);

   if (elev!=-MAXFLOAT) el.vec.z=fmax(el.vec.z,ref->len_g2l(elev+hover+dh));

   eye=nst->map_l2g(el);

   viewer->initeyepoint(eye);

   dez=aez=0.0;

   angle=turn=a;
   pitch=incline=p;

   speed=topspeed=0.0;
   }

// load settings
void loadsettings()
   {
   FILE *file;

   miniv3f e;
   int type;

   float a,p;

   viewerbase::VIEWER_PARAMS prms;
   miniearth::MINIEARTH_PARAMS eprms;
   miniterrain::MINITERRAIN_PARAMS tprms;

   int flag;

   minilayer *ref=viewer->getearth()->getreference();
   char *savname=ref->getcache()->getfile(VIEWER_SAVFILE);

   if (savname!=NULL)
      {
      viewer->get(prms);
      viewer->getearth()->get(eprms);
      viewer->getearth()->getterrain()->get(tprms);

      if ((file=fopen(savname,"rb"))==NULL) ERRORMSG();
      free(savname);

      // load essential parameters:

      if (fscanf(file,"ex=%f\n",&e.x)!=1) ERRORMSG();
      if (fscanf(file,"ey=%f\n",&e.y)!=1) ERRORMSG();
      if (fscanf(file,"ez=%f\n",&e.z)!=1) ERRORMSG();

      if (fscanf(file,"type=%d\n",&type)!=1) ERRORMSG();

      if (fscanf(file,"angle=%f\n",&a)!=1) ERRORMSG();
      if (fscanf(file,"pitch=%f\n",&p)!=1) ERRORMSG();

      if (fscanf(file,"farp=%f\n",&prms.farp)!=1) ERRORMSG();
      if (fscanf(file,"res=%f\n",&tprms.res)!=1) ERRORMSG();
      if (fscanf(file,"range=%f\n",&tprms.range)!=1) ERRORMSG();

      if (fscanf(file,"fovy=%f\n",&prms.fovy)!=1) ERRORMSG();

      if (fscanf(file,"sealevel=%g\n",&tprms.sealevel)!=1) ERRORMSG();
      if (tprms.sealevel<-MAXFLOAT/2) tprms.sealevel=-MAXFLOAT;

      // load optional parameters:

      if (fscanf(file,"usefog=%d\n",&flag)!=1) ERRORMSG();
      eprms.usefog=flag;

      if (fscanf(file,"usecontours=%d\n",&flag)!=1) ERRORMSG();
      eprms.usecontours=flag;

      if (fscanf(file,"usebathymap=%d\n",&flag)!=1) ERRORMSG();
      eprms.usebathymap=flag;

      if (fscanf(file,"useskydome=%d\n",&flag)!=1) ERRORMSG();
      eprms.useskydome=flag;

      if (fscanf(file,"usewaypoints=%d\n",&flag)!=1) ERRORMSG();
      eprms.usewaypoints=flag;

      if (fscanf(file,"usebricks=%d\n",&flag)!=1) ERRORMSG();
      eprms.usebricks=flag;

      if (fscanf(file,"fogdensity=%f\n",&eprms.fogdensity)!=1) ERRORMSG();

      fclose(file);

      viewer->getearth()->getterrain()->set(tprms);
      viewer->getearth()->set(eprms);
      viewer->set(prms);

      initview(minicoord(miniv4d(e),(minicoord::MINICOORD)type),a,p);
      }
   else
      {
      eye=viewer->getinitial();
      initview(eye,0.0,params->fovy/2,VIEWER_UPLIFT*params->farp);
      }
   }

// save settings
void savesettings()
   {
   FILE *file;

   minilayer *ref=viewer->getearth()->getreference();
   char *savname=ref->getcache()->getfile(VIEWER_SAVFILE);

   if (savname==NULL) savname=strdup(VIEWER_SAVFILE);

   if ((file=fopen(savname,"wb"))==NULL) ERRORMSG();
   free(savname);

   // save essential parameters:

   fprintf(file,"ex=%f\n",eye.vec.x);
   fprintf(file,"ey=%f\n",eye.vec.y);
   fprintf(file,"ez=%f\n",eye.vec.z);

   fprintf(file,"type=%d\n",eye.type);

   fprintf(file,"angle=%f\n",angle);
   fprintf(file,"pitch=%f\n",pitch);

   fprintf(file,"farp=%f\n",params->farp);
   fprintf(file,"res=%f\n",tparams->res);
   fprintf(file,"range=%f\n",tparams->range);

   fprintf(file,"fovy=%f\n",params->fovy);

   fprintf(file,"sealevel=%g\n",tparams->sealevel);

   // save optional parameters:

   fprintf(file,"usefog=%d\n",eparams->usefog);
   fprintf(file,"usecontours=%d\n",eparams->usecontours);
   fprintf(file,"usebathymap=%d\n",eparams->usebathymap);
   fprintf(file,"useskydome=%d\n",eparams->useskydome);
   fprintf(file,"usewaypoints=%d\n",eparams->usewaypoints);
   fprintf(file,"usebricks=%d\n",eparams->usebricks);

   fprintf(file,"fogdensity=%f\n",eparams->fogdensity);

   fclose(file);
   }

// read pixels and save to file
void readrgbpixels()
   {
   int i,j,k;

   unsigned char *pixels;

   unsigned char tmp;

   pixels=readrgbpixels(0,0,winwidth,winheight);

   for (i=0; i<winwidth; i++)
      for (j=0; j<winheight/2; j++)
         for (k=0; k<3; k++)
            {
            tmp=pixels[3*(i+j*winwidth)+k];
            pixels[3*(i+j*winwidth)+k]=pixels[3*(i+(winheight-1-j)*winwidth)+k];
            pixels[3*(i+(winheight-1-j)*winwidth)+k]=tmp;
            }

   writePNMimage("pixels.ppm",pixels,winwidth,winheight,3);
   free(pixels);
   }

// initialize VIS bathy map
void initVISbathymap()
   {
   int i;

   float t;

   float rgba[4];

   static const float hue1=0.0f;
   static const float hue2=240.0f;

   for (i=0; i<VIEWER_BATHYWIDTH; i++)
      {
      t=(float)i/(VIEWER_BATHYWIDTH-1);

      hsv2rgb(hue1+(hue2-hue1)*t,1.0f,1.0f,rgba);

      rgba[3]=fpow(1.0f-fabs(2.0f*(t-0.5f)),1.0f/3);

      VIEWER_BATHYMAP[4*i]=VIEWER_BATHYMAP[4*(i+VIEWER_BATHYWIDTH)]=ftrc(255.0f*rgba[0]+0.5f);
      VIEWER_BATHYMAP[4*i+1]=VIEWER_BATHYMAP[4*(i+VIEWER_BATHYWIDTH)+1]=ftrc(255.0f*rgba[1]+0.5f);
      VIEWER_BATHYMAP[4*i+2]=VIEWER_BATHYMAP[4*(i+VIEWER_BATHYWIDTH)+2]=ftrc(255.0f*rgba[2]+0.5f);
      VIEWER_BATHYMAP[4*i+3]=VIEWER_BATHYMAP[4*(i+VIEWER_BATHYWIDTH)+3]=ftrc(255.0f*rgba[3]+0.5f);
      }
   }

// initialize NPR bathy map
void initNPRbathymap()
   {
   int i;

   float t;
   float alpha;

   float rgba[4];

   static const float hue1=120.0f;
   static const float hue2=60.0f;

   static const float hue_ctr=60.0f;
   static const float sat_ctr=0.75f;
   static const float val_ctr=0.25f;

   for (i=0; i<VIEWER_NPRBATHYWIDTH; i++)
      {
      t=(float)i/(VIEWER_NPRBATHYWIDTH-1);

      alpha=t*fabs(VIEWER_NPRBATHYEND-VIEWER_NPRBATHYSTART)/VIEWER_CONTOURS;
      alpha=alpha-ftrc(alpha);

      if (t<0.5f) hsv2rgb(hue1+(hue2-hue1)*t,t,1.0f,rgba);
      else hsv2rgb(hue1+(hue2-hue1)*t,1.0f-t,1.0f,rgba);

      rgba[3]=0.5f;

      if (alpha>0.9f) hsv2rgb(hue_ctr,sat_ctr,val_ctr,rgba);

      VIEWER_NPRBATHYMAP[4*i]=VIEWER_NPRBATHYMAP[4*(i+VIEWER_NPRBATHYWIDTH)]=ftrc(255.0f*rgba[0]+0.5f);
      VIEWER_NPRBATHYMAP[4*i+1]=VIEWER_NPRBATHYMAP[4*(i+VIEWER_NPRBATHYWIDTH)+1]=ftrc(255.0f*rgba[1]+0.5f);
      VIEWER_NPRBATHYMAP[4*i+2]=VIEWER_NPRBATHYMAP[4*(i+VIEWER_NPRBATHYWIDTH)+2]=ftrc(255.0f*rgba[2]+0.5f);
      VIEWER_NPRBATHYMAP[4*i+3]=VIEWER_NPRBATHYMAP[4*(i+VIEWER_NPRBATHYWIDTH)+3]=ftrc(255.0f*rgba[3]+0.5f);
      }
   }

// render compass
void rendercompass()
   {
   const float sx=0.05f;
   const float sy=0.05f;

   const float xr=0.15f;
   const float yr=0.15f;

   const float alpha=0.5f;

   glLoadIdentity();
   glTranslatef(sx,sy,0.0f);
   glScalef(xr,yr,0.0f);
   glTranslatef(0.5f,0.5f,0.0f);
   glRotatef(turn,0.0f,0.0f,1.0f);

   glColor4f(0.5f,0.75f,1.0f,alpha);

   glBegin(GL_TRIANGLE_FAN);
   glVertex2f(0.0f,0.5f);
   glVertex2f(0.1f,-0.5f);
   glVertex2f(0.0f,-0.4f);
   glVertex2f(-0.1f,-0.5f);
   glEnd();

   glTranslatef(-0.15f,0.55f,0.0f);
   minitext::drawstring(0.3f,120.0f,0.5f,1.0f,1.0f,"North");
   glTranslatef(0.15f,-0.55f,0.0f);
   glRotatef(-45.0f,0.0f,0.0f,1.0f);
   glTranslatef(-0.15f,0.55f,0.0f);
   minitext::drawstring(0.3f,120.0f,0.0f,1.0f,1.0f," ne ");
   glTranslatef(0.15f,-0.55f,0.0f);
   glRotatef(-45.0f,0.0f,0.0f,1.0f);
   glTranslatef(-0.15f,0.55f,0.0f);
   minitext::drawstring(0.3f,120.0f,0.0f,1.0f,1.0f,"East");
   glTranslatef(0.15f,-0.55f,0.0f);
   glRotatef(-45.0f,0.0f,0.0f,1.0f);
   glTranslatef(-0.15f,0.55f,0.0f);
   minitext::drawstring(0.3f,120.0f,0.0f,1.0f,1.0f," se ");
   glTranslatef(0.15f,-0.55f,0.0f);
   glRotatef(-45.0f,0.0f,0.0f,1.0f);
   glTranslatef(-0.15f,0.55f,0.0f);
   minitext::drawstring(0.3f,0.0f,0.5f,1.0f,1.0f,"South");
   glTranslatef(0.15f,-0.55f,0.0f);
   glRotatef(-45.0f,0.0f,0.0f,1.0f);
   glTranslatef(-0.15f,0.55f,0.0f);
   minitext::drawstring(0.3f,120.0f,0.0f,1.0f,1.0f," sw ");
   glTranslatef(0.15f,-0.55f,0.0f);
   glRotatef(-45.0f,0.0f,0.0f,1.0f);
   glTranslatef(-0.15f,0.55f,0.0f);
   minitext::drawstring(0.3f,120.0f,0.0f,1.0f,1.0f,"West");
   glTranslatef(0.15f,-0.55f,0.0f);
   glRotatef(-45.0f,0.0f,0.0f,1.0f);
   glTranslatef(-0.15f,0.55f,0.0f);
   minitext::drawstring(0.3f,120.0f,0.0f,1.0f,1.0f," nw ");
   }

// render tileset info
void renderinfo()
   {
   int i,j,t;

   const float sx=0.6f;
   const float sy=0.02f;

   const float xr=0.38f;
   const float yr=0.38f;

   const float size=0.3f;
   const float alpha=0.5f;

   minilayer *nst=viewer->getearth()->getnearest(eye);
   minitile *mt=nst->getterrain()->getminitile();

   int vcol=mt->getvisibleleft();
   int vrow=mt->getvisiblebottom();

   int vcols=mt->getvisibleright()-mt->getvisibleleft()+1;
   int vrows=mt->getvisibletop()-mt->getvisiblebottom()+1;

   int pcol=mt->getpreloadedleft();
   int prow=mt->getpreloadedbottom();

   int pcols=mt->getpreloadedright()-mt->getpreloadedleft()+1;
   int prows=mt->getpreloadedtop()-mt->getpreloadedbottom()+1;

   glLoadIdentity();
   glTranslatef(sx,sy+yr,0.0f);
   glScalef(xr/pcols,-yr/prows,0.0f);
   glTranslatef(0.5f,0.5f,0.0f);

   glColor4f(1.0f,1.0f,1.0f,0.25f);

   glBegin(GL_QUADS);
   glVertex2f(-size,-size);
   glVertex2f(pcols-1+size,-size);
   glVertex2f(pcols-1+size,prows-1+size);
   glVertex2f(-size,prows-1+size);
   glEnd();

   glColor4f(1.0f,1.0f,1.0f,0.75f);

   glBegin(GL_QUADS);
   glVertex2f(vcol-pcol-size,vrow-prow-size);
   glVertex2f(vcol-pcol+vcols-1+size,vrow-prow-size);
   glVertex2f(vcol-pcol+vcols-1+size,vrow-prow+vrows-1+size);
   glVertex2f(vcol-pcol-size,vrow-prow+vrows-1+size);
   glEnd();

   glBegin(GL_QUADS);

   for (i=0; i<pcols; i++)
      for (j=0; j<prows; j++)
         {
         t=mt->gettexw(pcol+i,prow+j);

         switch (t)
            {
            case 0: glColor4f(0.0f,0.0f,0.0f,alpha); break;
            case 2: glColor4f(0.1f,0.1f,0.1f,alpha); break;
            case 4: glColor4f(0.25f,0.25f,0.25f,alpha); break;
            case 8: glColor4f(0.5f,0.5f,0.5f,alpha); break;
            case 16: glColor4f(0.75f,0.75f,0.75f,alpha); break;
            case 32: glColor4f(1.0f,1.0f,1.0f,alpha); break;
            case 64: glColor4f(0.0f,0.0f,1.0f,alpha); break;
            case 128: glColor4f(0.0f,0.5f,1.0f,alpha); break;
            case 256: glColor4f(0.0f,1.0f,0.0f,alpha); break;
            case 512: glColor4f(0.5f,1.0f,0.0f,alpha); break;
            case 1024: glColor4f(1.0f,1.0f,0.0f,alpha); break;
            case 2048: glColor4f(1.0f,0.5f,0.0f,alpha); break;
            default: glColor4f(1.0f,0.0f,0.0f,alpha); break;
            }

         glVertex2f(i-size,j-size);
         glVertex2f(i+size,j-size);
         glVertex2f(i+size,j+size);
         glVertex2f(i-size,j+size);
         }

   glEnd();
   }

// render head-up display
void renderhud()
   {
   double elev,sea;
   minicoord eye_llh;

   double dist;
   minicoord hit;
   minicoord hit_llh;
   double hit_elev;

   char str[MAXSTR];

   minilayer *nst=viewer->getearth()->getnearest(eye);
   minitile *mt=nst->getterrain()->getminitile();

   minitext::configure_zfight(1.0f);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0.0f,1.0f,0.0f,1.0f,-1.0f,1.0f);

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
   glEnable(GL_BLEND);

   if (sw_hud)
      {
      glTranslatef(0.0f,0.675f,0.0f);

      glColor4f(1.0f,1.0f,1.0f,0.5f);
      glBegin(GL_QUADS);
      glVertex2f(0.0f,0.0f);
      glVertex2f(1.0f,0.0f);
      glVertex2f(1.0f,0.3f);
      glVertex2f(0.0f,0.3f);
      glEnd();

      glTranslatef(0.033f,0.0f,0.0f);

      elev=viewer->getearth()->getterrain()->getheight(eye);
      if (elev==-MAXFLOAT) elev=0.0f;

      sea=tparams->sealevel;
      if (sea==-MAXFLOAT) sea=0.0f;

      eye_llh=nst->map_g2t(eye);
      if (eye_llh.type!=minicoord::MINICOORD_LINEAR) eye_llh.convert2(minicoord::MINICOORD_LLH);

      snprintf(str,MAXSTR,"Position:                \n\n x= %11.1f\n y= %11.1f\n z= %11.1fm (%.1fm)\n\n dir= %.1f\n yon= %.1f\n\nSettings:\n\n farp= %.1fm (f/F)\n\n res=   %.1f (t/T)\n range= %.1fm (r/R)\n\n sea= %.1f (u/U)\n\n gravity= %.1f (g)\n",
               eye_llh.vec.x,eye_llh.vec.y,eye_llh.vec.z,elev/tparams->exaggeration,turn,incline, // position/elevation and direction
               params->farp,tparams->res,tparams->range*params->farp,sea,gravity); // adjustable parameters

      minitext::drawstring(0.3f,240.0f,1.0f,0.25f,1.0f,str);

      glTranslatef(0.3f,0.0f,0.0f);

      snprintf(str,MAXSTR,"Tile Set:                \n\n vis area= [%d-%d]x[%d-%d]\n\n fps= %.1fHz (%.1f%%)\n\n mem= %.1fMB\n tex= %.1fMB\n\nStreaming:\n\n pending= %d\n\n cache= %.1fMB\n\nGeometry:\n\n fans=     %d\n vertices= %d\n",
               mt->getvisibleleft(),mt->getvisibleright(),mt->getvisiblebottom(),mt->getvisibletop(), // visible area of nearest tileset
               1.0/(avg_delta+avg_idle),100*(1.0-avg_idle*params->fps), // actual frame rate and load
               viewer->getearth()->getterrain()->getmem(),viewer->getearth()->getterrain()->gettexmem(), // memory consumed by tilesets
               viewer->getearth()->getterrain()->getpending(), // number of pending tiles
               viewer->getearth()->getterrain()->getcachemem(), // memory foot print of data cache
               viewer->getearth()->getterrain()->getbuffer()->getfancnt(), // rendered triangles fans
               viewer->getearth()->getterrain()->getbuffer()->getvtxcnt()); // rendered vertices

      minitext::drawstring(0.3f,240.0f,1.0f,0.25f,1.0f,str);

      glTranslatef(0.3f,0.0f,0.0f);

      snprintf(str,MAXSTR,"Controls:                \n\n left=       a\n right=      d\n accelerate= w\n decelerate= s\n\n stop=      spc\n jump=      j\n look down= <\n look up=   >\n\n hud/hide=  h/H\n crosshair= c\n waypoints= p\n wireframe= l\n\n load/save= o/O\n quit=      Q esc\n");

      minitext::drawstring(0.3f,240.0f,1.0f,0.25f,1.0f,str);

      if (sw_cross!=0 || incline>89.9f)
         {
         glLoadIdentity();
         glTranslatef(0.5f,0.5f,0.0f);
         glScalef((float)winheight/winwidth,1.0f,1.0f);
         glColor3f(0.0f,0.0f,0.0f);
         glBegin(GL_LINES);
         glVertex2f(-0.025,-0.025f);
         glVertex2f(0.025,0.025f);
         glVertex2f(-0.025,0.025f);
         glVertex2f(0.025,-0.025f);
         glEnd();

         if (sw_cross!=0)
            {
            dist=viewer->shoot(eye,dir);

            if (dist!=MAXFLOAT)
               {
               hit=nst->map_g2l(eye);
               hit+=nst->rot_g2l(dir,eye)*nst->len_g2l(dist);
               hit=nst->map_l2g(hit);

               hit_llh=hit;
               hit_llh.type=minicoord::MINICOORD_ECEF;
               hit_llh.convert2(minicoord::MINICOORD_LLH);

               hit_elev=viewer->getearth()->getterrain()->getheight(hit);
               if (hit_elev==-MAXFLOAT) hit_elev=0.0;

               snprintf(str,MAXSTR,"dist=%3.3f elev=%3.3f\nlat=%3.6f lon=%3.6f",
                        dist,hit_elev,hit_llh.vec.x/3600.0,hit_llh.vec.y/3600.0);

               glTranslatef(0.05f,0.0f,0.0f);
               minitext::drawstring(0.3f,240.0f,1.0f,0.25f,1.0f,str);
               }
            }
         }

      rendercompass();
      renderinfo();
      }
   else
      if (sw_hide==0)
         {
         glTranslatef(0.033f,0.95f,0.0f);
         minitext::drawstring(0.3f,240.0f,1.0f,0.25f,1.0f,"[Press h for HUD]",1.0f,0.25f);

         glLoadIdentity();
         rendercompass();
         }

   glDisable(GL_BLEND);
   }

// GLUT display function
void displayfunc()
   {
   double delta,idle;

   double sina,cosa;
   double sinp,cosp;

   double elev,coef;

   minicoord ep,el,egl;
   miniv3d dgl,ugl,rgl;

   minilayer *ref,*nst;

   double lightdir;
   miniv3d light;

   if (winwidth<=0 || winheight<=0) return;

   // start timer
   viewer->starttimer();

   // update eye point:

   ref=viewer->getearth()->getreference();
   nst=viewer->getearth()->getnearest(eye);

   viewer->getearth()->getterrain()->setreference(viewer->getearth()->getterrain()->getnum(nst));

   el=nst->map_g2l(eye);

   sina=sin(2.0*PI/360.0*turn);
   cosa=cos(2.0*PI/360.0*turn);

   sinp=sin(2.0*PI/360.0*incline);
   cosp=cos(2.0*PI/360.0*incline);

   el.vec.x+=sina*speed/params->fps;
   el.vec.y+=cosa*speed/params->fps;

   ep=nst->map_l2g(el);

   elev=ref->len_g2l(viewer->getearth()->getterrain()->getheight(ep));

   // update eye coordinate system:

   dir.x=sina*cosp;
   dir.y=cosa*cosp;
   dir.z=-sinp;

   up.x=sina*sinp;
   up.y=cosa*sinp;
   up.z=cosp;

   right.x=cosa*sbase;
   right.y=sina*sbase;
   right.z=0.0;

   dir=nst->rot_l2g(dir,el);
   up=nst->rot_l2g(up,el);
   right=nst->rot_l2g(right,el);

   // update eye movement:

   speed+=accel*(topspeed-speed);

   turn+=accel*(angle-turn);
   incline+=accel*(pitch-incline);

   coef=(el.vec.z-elev)/hover-1.0;
   if (coef>1.0) coef=1.0;
   else if (coef<-1.0) coef=-1.0;

   aez=-coef*gravity;
   aez*=fmax(1.0-fabs(dez/maxspeed),0.0);

   dez+=aez/params->fps;
   dez*=pow(1.0/(1.0+damp),1.0/params->fps);

   el.vec.z+=dez/params->fps;

   if (el.vec.z<elev+hover)
      {
      dez=-dez;
      dez*=1.0/(1.0+bounce);
      el.vec.z=elev+hover;
      }

   eye=nst->map_l2g(el);

   egl=nst->map_g2o(eye);
   dgl=nst->rot_g2o(dir,eye);
   ugl=nst->rot_g2o(up,eye);
   rgl=nst->rot_g2o(right,eye);

   tparams->signpostturn=turn;
   tparams->signpostincline=-incline;

   viewer->getearth()->getterrain()->propagate_wp();

   // setup OpenGL state:

   viewer->clear();

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(params->fovy,(float)winwidth/winheight,ref->len_g2o(params->nearp),ref->len_g2o(params->farp));

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   gluLookAt(egl.vec.x,egl.vec.y,egl.vec.z,egl.vec.x+dgl.x,egl.vec.y+dgl.y,egl.vec.z+dgl.z,ugl.x,ugl.y,ugl.z);

   // update scene
   viewer->cache(eye,dir,up,(float)winwidth/winheight);

   // update earth lighting
   if (eparams->usediffuse)
      {
      lightdir=2*PI*VIEWER_ROTATION*viewer->gettime();
      light=miniv3d(sin(lightdir),cos(lightdir),0.0);
      eparams->lightdir=light;
      viewer->propagate();
      }

   // render scene
   if (sw_stereo==0) viewer->render();
   else
      {
      // left channel:

      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      gluLookAt(egl.vec.x-rgl.x,egl.vec.y-rgl.y,egl.vec.z-rgl.z,egl.vec.x+dgl.x-rgl.x,egl.vec.y+dgl.y-rgl.y,egl.vec.z+dgl.z-rgl.z,ugl.x,ugl.y,ugl.z);

      if (sw_anaglyph==0) glDrawBuffer(GL_BACK_LEFT);
      else glColorMask(GL_TRUE,GL_FALSE,GL_FALSE,GL_FALSE);

      viewer->render();

      // right channel:

      glClear(GL_DEPTH_BUFFER_BIT);

      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      gluLookAt(egl.vec.x+rgl.x,egl.vec.y+rgl.y,egl.vec.z+rgl.z,egl.vec.x+dgl.x+rgl.x,egl.vec.y+dgl.y+rgl.y,egl.vec.z+dgl.z+rgl.z,ugl.x,ugl.y,ugl.z);

      if (sw_anaglyph==0) glDrawBuffer(GL_BACK_RIGHT);
      else glColorMask(GL_FALSE,GL_TRUE,GL_TRUE,GL_FALSE);

      viewer->render();

      if (sw_anaglyph==0) glDrawBuffer(GL_BACK);
      else glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_FALSE);
      }

   // render the head-up display
   renderhud();

   // swap buffers and wait for next frame:

   glutSwapBuffers();

   // get time spent
   delta=viewer->gettimer();
   idle=1.0/params->fps-delta;

   // idle for the remainder of the frame
   viewer->idle(delta);

   // update statistics:

   accu_delta+=delta;
   if (idle>0.0) accu_idle+=idle;

   if (++avg_count>params->fps)
      {
      avg_delta=accu_delta/avg_count;
      avg_idle=accu_idle/avg_count;
      accu_delta=0.0;
      accu_idle=0.0f;
      avg_count=0;
      }
   }

// GLUT reshape function
void reshapefunc(int width,int height)
   {initwindow(width,height);}

// GLUT keyboard function
void keyboardfunc(unsigned char key,int x,int y)
   {
   float mousex,mousey;

   mousex=(float)x/(winwidth-1);
   mousey=(float)y/(winwidth-1);

   switch (key)
      {
      case ' ':
         if (topspeed==0.0) topspeed=speedinc*maxspeed;
         else topspeed=0.0;
         break;
      case 'w':
         topspeed+=speedinc*maxspeed;
         if (topspeed>maxspeed) topspeed=maxspeed;
         break;
      case 'W':
         topspeed+=speedinc*maxspeed*boost;
         if (topspeed>maxspeed*boost) topspeed=maxspeed*boost;
         break;
      case 'a':
         angle-=oneturn;
         if (angle<0.0)
            {
            angle+=360.0;
            turn+=360.0;
            }
         break;
      case 'A':
         angle-=oneturn/slow;
         if (angle<0.0)
            {
            angle+=360.0;
            turn+=360.0;
            }
         break;
      case 'd':
         angle+=oneturn;
         if (angle>360.0)
            {
            angle-=360.0;
            turn-=360.0;
            }
         break;
      case 'D':
         angle+=oneturn/slow;
         if (angle>360.0)
            {
            angle-=360.0;
            turn-=360.0;
            }
         break;
      case 's':
         topspeed-=speedinc*maxspeed;
         if (topspeed<-maxspeed) topspeed=-maxspeed;
         break;
      case 'S':
         topspeed-=speedinc*maxspeed*boost;
         if (topspeed<-maxspeed*boost) topspeed=-maxspeed*boost;
         break;
      case '<':
         pitch+=oneincline;
         if (pitch>90.0) pitch=90.0;
         break;
      case '>':
         pitch-=oneincline;
         if (pitch<-90.0) pitch=-90.0;
         break;
      case 'j':
         dez=jump;
         break;
      case 'J':
         dez=jump*boost;
         break;
      case 'g':
         if (gravity==0.0) gravity=earthg;
         else gravity=0.0;
         break;
      case 'G':
         if (gravity==0.0) gravity=earthg*boost;
         else gravity=0.0;
         break;
      case 't':
         tparams->res/=1.1f;
         if (tparams->res<1.0f) tparams->res=1.0f;
         viewer->propagate();
         break;
      case 'T':
         tparams->res*=1.1f;
         if (tparams->res>1.0E10f) tparams->res=1.0E10f;
         viewer->propagate();
         break;
      case 'r':
         tparams->range/=1.1f;
         if (tparams->range<1.0E-5f) tparams->range=1.0E-5f;
         viewer->propagate();
         break;
      case 'R':
         tparams->range*=1.1f;
         if (tparams->range>1.0f) tparams->range=1.0f;
         viewer->propagate();
         break;
      case 'f':
         params->farp/=1.1f;
         if (params->farp<2*params->nearp) params->farp=2*params->nearp;
         viewer->propagate();
         break;
      case 'F':
         params->farp*=1.1f;
         if (params->farp>1.0E10f*params->nearp) params->farp=1.0E10f*params->nearp;
         viewer->propagate();
         break;
      case 'v':
         params->fovy-=5.0f;
         if (params->fovy<10.0f) params->fovy=10.0f;
         viewer->propagate();
         break;
      case 'V':
         params->fovy+=5.0f;
         if (params->fovy>170.0f) params->fovy=170.0f;
         viewer->propagate();
         break;
      case 'u':
         if (tparams->sealevel==-MAXFLOAT) tparams->sealevel=0.0f;
         else
            {
            tparams->sealevel+=0.5f;
            if (tparams->sealevel==0.0f) tparams->sealevel=-MAXFLOAT;
            }
         viewer->propagate();
         viewer->getearth()->getterrain()->update();
         break;
      case 'U':
         if (tparams->sealevel==-MAXFLOAT) tparams->sealevel=0.0f;
         else
            {
            tparams->sealevel-=0.5f;
            if (tparams->sealevel==0.0f) tparams->sealevel=-MAXFLOAT;
            }
         viewer->propagate();
         viewer->getearth()->getterrain()->update();
         break;
      case 'm':
         eparams->fogdensity*=0.9f;
         if (eparams->fogdensity<1.0E-3f) eparams->fogdensity=1.0E-3f;
         viewer->propagate();
         break;
      case 'M':
         eparams->fogdensity*=1.1f;
         if (eparams->fogdensity>10.0f) eparams->fogdensity=10.0f;
         viewer->propagate();
         break;
      case '1':
         if (!eparams->usefog) eparams->usefog=TRUE;
         else eparams->usefog=FALSE;
         viewer->propagate();
         break;
      case '2':
         if (!eparams->usebathymap) eparams->usebathymap=TRUE;
         else eparams->usebathymap=FALSE;
         viewer->propagate();
         break;
      case '3':
         if (!eparams->usecontours) eparams->usecontours=TRUE;
         else eparams->usecontours=FALSE;
         viewer->propagate();
         break;
      case '4':
         if (!eparams->useskydome) eparams->useskydome=TRUE;
         else eparams->useskydome=FALSE;
         break;
      case 'p':
         if (!eparams->usewaypoints) eparams->usewaypoints=TRUE;
         else eparams->usewaypoints=FALSE;
         viewer->propagate();
         break;
      case 'E':
         if (!eparams->useflat) eparams->useflat=TRUE;
         else eparams->useflat=FALSE;
         viewer->propagate();
         break;
      case 'N':
         if (!eparams->usenprshader) eparams->usenprshader=TRUE;
         else eparams->usenprshader=FALSE;
         eparams->usevisshader=!eparams->usenprshader;
         viewer->propagate();
         break;
      case 'l':
         if (!params->usewireframe) params->usewireframe=TRUE;
         else params->usewireframe=FALSE;
         break;
      case 'L':
         if (!eparams->usediffuse) eparams->usediffuse=TRUE;
         else eparams->usediffuse=FALSE;
         viewer->propagate();
         break;
      case 'h':
         if (sw_hud==0) sw_hud=1;
         else sw_hud=0;
         break;
      case 'H':
         if (sw_hide==0) sw_hide=1;
         else sw_hide=0;
         break;
      case 'c':
         if (sw_cross==0) sw_cross=1;
         else sw_cross=0;
         break;
      case 'o':
         loadsettings();
         break;
      case 'O':
         savesettings();
         break;
      case 'P':
         readrgbpixels();
         break;
      case 'Q':
      case 27:
         delete viewer;
         if (sw_full==0) glutDestroyWindow(winid);
         else glutLeaveGameMode();
         exit(0);
      }
   }

// main function
int main(int argc,char *argv[])
   {
   int i;

   int argc_regular;

   // count regular arguments
   for (argc_regular=0,i=1; i<argc; i++)
      if (*argv[i]!='-') argc_regular++;

   // process command line options
   for (i=1; i<argc; i++)
      if (strcmp(argv[i],"-s")==0) sw_stereo=1;
      else if (strcmp(argv[i],"-a")==0) sw_anaglyph=1;
      else if (strcmp(argv[i],"-f")==0) sw_full=1;
      else if (strcmp(argv[i],"-m")==0) sw_multi=1;
      else if (strcmp(argv[i],"-r")==0) sw_reset=1;
      else if (strcmp(argv[i],"-c")==0) sw_autos3tc=1;
      else if (strcmp(argv[i],"-b")==0) sw_bricks=1;
      else if (strcmp(argv[i],"-B")==0) sw_bricks=sw_mpass=1;

   // check arguments
   if ((sw_multi==0 && argc_regular!=1 && argc_regular!=4) ||
       (sw_multi!=0 && argc_regular<1))
      {
      printf("short usage: %s <url> {<options>}\n",argv[0]);
      printf("long usage: %s <url> <tileset.path> <elevation.subpath> <imagery.subpath> {<options>}\n",argv[0]);
      printf("multi usage: %s -m {<url>} {<options>}\n",argv[0]);
      printf("options: -s=stereo -a=anaglyph -f=full-screen -r=reset-cache -c=auto-s3tc\n");
      exit(1);
      }

   // path setup for elevation and imagery
   if (sw_multi==0)
      if (argc_regular!=1)
         {
         if (*argv[1]=='-' || sscanf(argv[1],"%s",baseurl)!=1) exit(1);
         if (*argv[2]=='-' || sscanf(argv[2],"%s",baseid)!=1) exit(1);
         if (*argv[3]=='-' || sscanf(argv[3],"%s",basepath1)!=1) exit(1);
         if (*argv[4]=='-' || sscanf(argv[4],"%s",basepath2)!=1) exit(1);
         }
      else
         if (*argv[1]=='-' || sscanf(argv[1],"%s",shorturl)!=1) exit(1);

   // open window with GLUT:

   winwidth=VIEWER_WINWIDTH;
   winheight=VIEWER_WINHEIGHT;

   glutInit(&argc,argv);
   glutInitWindowSize(winwidth,winheight);
   if (sw_stereo!=0 && sw_anaglyph==0) glutInitDisplayMode(GLUT_RGB|GLUT_ALPHA|GLUT_DEPTH|GLUT_DOUBLE|GLUT_STEREO);
   else glutInitDisplayMode(GLUT_RGB|GLUT_ALPHA|GLUT_DEPTH|GLUT_DOUBLE);
   if (sw_full==0) winid=glutCreateWindow(VIEWER_WINTITLE);
   else glutEnterGameMode();

   glutDisplayFunc(displayfunc);
   glutReshapeFunc(reshapefunc);
   glutMouseFunc(NULL);
   glutMotionFunc(NULL);
   glutKeyboardFunc(keyboardfunc);
   glutSpecialFunc(NULL);
   glutIdleFunc(displayfunc);

   // print unsupported OpenGL extensions
   if (miniOGL::get_unsupported_glexts()>0) miniOGL::print_unsupported_glexts();

   // create the viewer object
   viewer=new viewerbase;

   // initialize the viewing parameters
   initparams();

   // get a reference to the viewing parameters
   params=viewer->get();

   // get a reference to the earth parameters
   eparams=viewer->getearth()->get();

   // get a reference to the terrain parameters
   tparams=viewer->getearth()->getterrain()->get();

   // load tileset (short version)
   if (sw_multi==0 && argc_regular==1)
      if (!viewer->getearth()->load(shorturl,TRUE,sw_reset))
         {
         printf("unable to load tileset at url=%s\n",shorturl);
         exit(1);
         }

   // load tileset (long version)
   if (sw_multi==0 && argc_regular!=1)
      if (!viewer->getearth()->load(baseurl,baseid,basepath1,basepath2,TRUE,sw_reset))
         {
         printf("unable to load tileset at url=%s%s%s (resp. %s)\n",baseurl,baseid,basepath1,basepath2);
         exit(1);
         }

   // load tileset (multi version)
   if (sw_multi!=0)
      for (i=1; i<argc; i++)
         if (*argv[i]!='-')
            if (!viewer->getearth()->load(argv[i],TRUE,sw_reset))
               {
               printf("unable to load tileset at url=%s\n",argv[i]);
               exit(1);
               }

   // load optional features
   viewer->getearth()->loadopts();

   // initialize VIS bathy map
   initVISbathymap();

   // initialize NPR bathy map
   initNPRbathymap();

   // load settings
   loadsettings();

   // enter event loop
   glutMainLoop();

   // never reached
   return(0);
   }
