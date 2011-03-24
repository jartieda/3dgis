// (c) by Stefan Roettger

#ifndef NOVIEWER

#include <mini/minitime.h>

#include <mini/database.h>

#ifndef NOSQUISH
#include <mini/squishbase.h>
#endif

#include <mini/miniOGL.h>

#include <mini/viewerbase.h>

// default constructor
viewerbase::viewerbase()
   {
   // configurable parameters:

   PARAMS.fps=25.0f;     // frames per second (target frame rate)

   PARAMS.fovy=60.0f;    // field of view (degrees)
   PARAMS.nearp=10.0f;   // near plane (meters)
   PARAMS.farp=10000.0f; // far plane (meters)

   // feature switches:

   PARAMS.usewireframe=FALSE;

   // image conversion parameters:

   // parameter set for "better-quality" compressed images: quality=75 denoising=OFF -> ratio 1:20
   // parameter set for "less-size" compressed images: quality=50 denoising=ON -> ratio 1:40

   PARAMS.conversion_params.jpeg_quality=75.0f; // jpeg quality in percent

   PARAMS.conversion_params.usegreycstoration=FALSE; // use greycstoration for image denoising

   PARAMS.conversion_params.greyc_p=0.8f; // greycstoration sharpness, useful range=[0.7-0.9]
   PARAMS.conversion_params.greyc_a=0.4f; // greycstoration anisotropy, useful range=[0.1-0.5]

   // initialize state:

   EARTH=new miniearth();

   START=minigettime();
   TIMER=0.0;

   THREADBASE=new threadbase();
   CURLBASE=new curlbase();

   inithooks();
   }

// destructor
viewerbase::~viewerbase()
   {
   delete EARTH;

   delete THREADBASE;
   delete CURLBASE;
   }

// get parameters
void viewerbase::get(VIEWER_PARAMS &params)
   {params=PARAMS;}

// set parameters
void viewerbase::set(VIEWER_PARAMS &params)
   {
   miniearth::MINIEARTH_PARAMS eparams;

   // set new state
   PARAMS=params;

   // get the actual earth state
   EARTH->get(eparams);

   // update the terrain state:

   eparams.fps=PARAMS.fps;

   eparams.fovy=PARAMS.fovy;
   eparams.nearp=PARAMS.nearp;
   eparams.farp=PARAMS.farp;

   // finally pass the updated earth state
   EARTH->set(eparams);
   }

// propagate parameters
void viewerbase::propagate()
   {set(PARAMS);}

#ifndef NOSQUISH

// S3TC auto-compression hook
void viewerbase::autocompress(int isrgbadata,unsigned char *rawdata,unsigned int bytes,
                              unsigned char **s3tcdata,unsigned int *s3tcbytes,int width,int height,
                              void *data)
   {
   if (data!=NULL) ERRORMSG();

   squishbase::compressS3TC(isrgbadata,rawdata,bytes,
                            s3tcdata,s3tcbytes,width,height);
   }

#endif

// initialize the terrain hooks
void viewerbase::inithooks()
   {
   // register callbacks
   EARTH->getterrain()->setcallbacks(THREADBASE,
                                     threadbase::threadinit,threadbase::threadexit,
                                     threadbase::startthread,threadbase::jointhread,
                                     threadbase::lock_cs,threadbase::unlock_cs,
                                     threadbase::lock_io,threadbase::unlock_io,
                                     CURLBASE,
                                     curlbase::curlinit,curlbase::curlexit,
                                     curlbase::getURL,curlbase::checkURL);

   // register libMini conversion hook (JPEG/PNG)
   convbase::setconversion(&PARAMS.conversion_params);

#ifndef NOSQUISH

   // register auto-compression hook
   databuf::setautocompress(autocompress,NULL);

#endif
   }

// get initial view point
minicoord viewerbase::getinitial()
   {return(EARTH->getterrain()->getinitial());}

// set initial eye point
void viewerbase::initeyepoint(const minicoord &e)
   {EARTH->getterrain()->initeyepoint(e);}

// clear scene
void viewerbase::clear()
   {EARTH->clear();}

// enable a specific focus point
void viewerbase::enablefocus(const minicoord &f)
   {EARTH->getterrain()->enablefocus(f);}

// disable the focus point
void viewerbase::disablefocus()
   {EARTH->getterrain()->disablefocus();}

// generate and cache scene for a particular eye point
void viewerbase::cache(const minicoord &e,const miniv3d &d,const miniv3d &u,float aspect)
   {EARTH->getterrain()->cache(e,d,u,aspect,gettime());}

// render cached scene
void viewerbase::render()
   {
   // enable wireframe mode
   if (PARAMS.usewireframe) glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);

   // render earth
   EARTH->render();

   // disable wireframe mode
   if (PARAMS.usewireframe) glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
   }

// get time
double viewerbase::gettime()
   {return(minigettime()-START);}

// start timer
void viewerbase::starttimer()
   {TIMER=minigettime()-START;}

// measure timer
double viewerbase::gettimer()
   {return(minigettime()-START-TIMER);}

// idle for the remainder of the frame
void viewerbase::idle(double dt)
   {miniwaitfor(1.0/PARAMS.fps-dt);}

// shoot a ray at the scene
double viewerbase::shoot(const minicoord &o,const miniv3d &d)
   {return(EARTH->shoot(o,d));}

#endif
