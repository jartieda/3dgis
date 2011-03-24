// (c) by Stefan Roettger

#ifndef THREADBASE_H
#define THREADBASE_H

#ifndef NOVIEWER

#ifndef USEOPENTH
#   include <pthread.h>
#else
#   include <OpenThreads/Thread>
#   include <OpenThreads/Mutex>
#endif

#include <mini/datacloud.h>

#ifdef USEOPENTH

class MyThread: public OpenThreads::Thread
   {
   public:

   MyThread(): OpenThreads::Thread()
      {
      THREAD=NULL;
      BACKGROUND=NULL;
      }

   virtual ~MyThread() {}

   void setthread(void *(*thread)(void *background),backarrayelem *background)
      {
      THREAD=thread;
      BACKGROUND=background;
      }

   virtual void run() {THREAD(BACKGROUND);}

   private:

   void *(*THREAD)(void *background);
   backarrayelem *BACKGROUND;
   };

#endif

class threadbase
   {
   public:

   threadbase();
   ~threadbase();

   static void threadinit(int threads,int id,void *data);
   static void threadexit(int id,void *data);

   static void startthread(void *(*thread)(void *background),backarrayelem *background,int id,void *data);
   static void jointhread(backarrayelem *background,int id,void *data);

   static void lock_cs(int id,void *data);
   static void unlock_cs(int id,void *data);

   static void lock_io(int id,void *data);
   static void unlock_io(int id,void *data);

   private:

#ifndef USEOPENTH
   typedef pthread_t *PTHREADPTR;
#else
   typedef MyThread *MTHREADPTR;
#endif

   struct MULTITHREAD_STRUCT
      {
      int numthreads;

#ifndef USEOPENTH
      PTHREADPTR pthread;
      pthread_mutex_t mutex,iomutex;
      pthread_attr_t attr;
#else
      MTHREADPTR mthread;
      OpenThreads::Mutex mutex,iomutex;
#endif
      };

   typedef MULTITHREAD_STRUCT MULTITHREAD_TYPE;

   MULTITHREAD_TYPE **MULTITHREAD;
   int MAXMULTITHREAD,NUMMULTITHREAD;

   static int INSTANCES;

   void threadinit_safe(int threads,int id);
   void threadexit_safe(int id);

   void startthread_safe(void *(*thread)(void *background),backarrayelem *background,int id);
   void jointhread_safe(backarrayelem *background,int id);

   void lock_cs_safe(int id);
   void unlock_cs_safe(int id);

   void lock_io_safe(int id);
   void unlock_io_safe(int id);

   void initmultithread(int id);
   void exitmultithread(int id);
   };

#endif

#endif
