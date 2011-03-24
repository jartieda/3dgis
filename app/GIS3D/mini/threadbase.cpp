// (c) by Stefan Roettger
#      define NOMINMAX
#include <mini/threadbase.h>

#ifndef NOVIEWER

int threadbase::INSTANCES=0;

threadbase::threadbase()
   {
   MULTITHREAD=NULL;

   MAXMULTITHREAD=0;
   NUMMULTITHREAD=0;

   INSTANCES++;

#ifndef USEOPENTH
#ifdef PTW32_STATIC_LIB
   if (INSTANCES==1) pthread_win32_process_attach_np();
#endif
#else
   if (INSTANCES==1) OpenThreads::Thread::Init();
#endif
   }

threadbase::~threadbase()
   {
   INSTANCES--;

#ifndef USEOPENTH
#ifdef PTW32_STATIC_LIB
   if (INSTANCES==0) pthread_win32_process_detach_np();
#endif
#endif
   }

void threadbase::threadinit(int threads,int id,void *data)
   {
   threadbase *obj=(threadbase *)data;
   obj->threadinit_safe(threads,id);
   }

void threadbase::threadexit(int id,void *data)
   {
   threadbase *obj=(threadbase *)data;
   obj->threadexit_safe(id);
   }

void threadbase::startthread(void *(*thread)(void *background),backarrayelem *background,int id,void *data)
   {
   threadbase *obj=(threadbase *)data;
   obj->startthread_safe(thread,background,id);
   }

void threadbase::jointhread(backarrayelem *background,int id,void *data)
   {
   threadbase *obj=(threadbase *)data;
   obj->jointhread_safe(background,id);
   }

void threadbase::lock_cs(int id,void *data)
   {
   threadbase *obj=(threadbase *)data;
   obj->lock_cs_safe(id);
   }

void threadbase::unlock_cs(int id,void *data)
   {
   threadbase *obj=(threadbase *)data;
   obj->unlock_cs_safe(id);
   }

void threadbase::lock_io(int id,void *data)
   {
   threadbase *obj=(threadbase *)data;
   obj->lock_io_safe(id);
   }

void threadbase::unlock_io(int id,void *data)
   {
   threadbase *obj=(threadbase *)data;
   obj->unlock_io_safe(id);
   }

#ifndef USEOPENTH

void threadbase::threadinit_safe(int threads,int id)
   {
   initmultithread(id);

   MULTITHREAD[id]->numthreads=threads;

   MULTITHREAD[id]->pthread=new pthread_t[threads];

   pthread_mutex_init(&MULTITHREAD[id]->mutex,NULL);
   pthread_mutex_init(&MULTITHREAD[id]->iomutex,NULL);

   pthread_attr_init(&MULTITHREAD[id]->attr);
   pthread_attr_setdetachstate(&MULTITHREAD[id]->attr,PTHREAD_CREATE_JOINABLE);
   }

void threadbase::threadexit_safe(int id)
   {
   pthread_mutex_destroy(&MULTITHREAD[id]->mutex);
   pthread_mutex_destroy(&MULTITHREAD[id]->iomutex);

   pthread_attr_destroy(&MULTITHREAD[id]->attr);

   delete[] MULTITHREAD[id]->pthread;

   exitmultithread(id);
   }

void threadbase::startthread_safe(void *(*thread)(void *background),backarrayelem *background,int id)
   {pthread_create(&MULTITHREAD[id]->pthread[background->background-1],&MULTITHREAD[id]->attr,thread,background);}

void threadbase::jointhread_safe(backarrayelem *background,int id)
   {
   void *status;
   pthread_join(MULTITHREAD[id]->pthread[background->background-1],&status);
   }

void threadbase::lock_cs_safe(int id)
   {pthread_mutex_lock(&MULTITHREAD[id]->mutex);}

void threadbase::unlock_cs_safe(int id)
   {pthread_mutex_unlock(&MULTITHREAD[id]->mutex);}

void threadbase::lock_io_safe(int id)
   {pthread_mutex_lock(&MULTITHREAD[id]->iomutex);}

void threadbase::unlock_io_safe(int id)
   {pthread_mutex_unlock(&MULTITHREAD[id]->iomutex);}

#else

void threadbase::threadinit_safe(int threads,int id)
   {
   initmultithread(id);

   MULTITHREAD[id]->numthreads=threads;

   MULTITHREAD[id]->mthread=new MyThread[threads];
   }

void threadbase::threadexit_safe(int id)
   {
   delete[] MULTITHREAD[id]->mthread;

   exitmultithread(id);
   }

void threadbase::startthread_safe(void *(*thread)(void *background),backarrayelem *background,int id)
   {
   MULTITHREAD[id]->mthread[background->background-1].setthread(thread,background);
   MULTITHREAD[id]->mthread[background->background-1].start();
   }

void threadbase::jointhread_safe(backarrayelem *background,int id)
   {MULTITHREAD[id]->mthread[background->background-1].join();}

void threadbase::lock_cs_safe(int id)
   {MULTITHREAD[id]->mutex.lock();}

void threadbase::unlock_cs_safe(int id)
   {MULTITHREAD[id]->mutex.unlock();}

void threadbase::lock_io_safe(int id)
   {MULTITHREAD[id]->iomutex.lock();}

void threadbase::unlock_io_safe(int id)
   {MULTITHREAD[id]->iomutex.unlock();}

#endif

void threadbase::initmultithread(int id)
   {
   if (MAXMULTITHREAD==0)
      {
      MAXMULTITHREAD=id+1;
      if ((MULTITHREAD=(MULTITHREAD_TYPE **)malloc(MAXMULTITHREAD*sizeof(MULTITHREAD_TYPE *)))==NULL) ERRORMSG();
      }

   if (id>=MAXMULTITHREAD)
      {
      MAXMULTITHREAD=id+1;
      if ((MULTITHREAD=(MULTITHREAD_TYPE **)realloc(MULTITHREAD,MAXMULTITHREAD*sizeof(MULTITHREAD_TYPE *)))==NULL) ERRORMSG();
      }

   MULTITHREAD[id]=new MULTITHREAD_TYPE;

   NUMMULTITHREAD++;
   }

void threadbase::exitmultithread(int id)
   {
   delete MULTITHREAD[id];

   NUMMULTITHREAD--;

   if (NUMMULTITHREAD==0)
      {
      free(MULTITHREAD);

      MULTITHREAD=NULL;
      MAXMULTITHREAD=0;
      }
   }

#endif
