// (c) by Stefan Roettger

#include <mini/curlbase.h>

#ifndef NOVIEWER

int curlbase::INSTANCES=0;

curlbase::curlbase()
   {
   MULTICURL=NULL;

   MAXMULTICURL=0;
   NUMMULTICURL=0;

   INSTANCES++;

   // init curl
   if (INSTANCES==1) curl_global_init(CURL_GLOBAL_ALL);
   }

curlbase::~curlbase()
   {
   INSTANCES--;

   // clean-up curl
   if (INSTANCES==0) curl_global_cleanup();
   }

void *curlbase::myrealloc(void *ptr,size_t size)
   {
   if (ptr) return realloc(ptr,size);
   else return malloc(size);
   }

size_t curlbase::WriteMemoryCallback(void *ptr,size_t size,size_t nmemb,void *data)
   {
   size_t realsize=size*nmemb;
   struct MemoryStruct *mem=(struct MemoryStruct *)data;

   mem->memory=(char *)myrealloc(mem->memory,mem->size+realsize+1);

   if (mem->memory)
      {
      memcpy(&(mem->memory[mem->size]),ptr,realsize);
      mem->size+=realsize;
      mem->memory[mem->size]=0;
      }

   return(realsize);
   }

void curlbase::curlinit(int threads,int id,const char *proxyname,const char *proxyport,void *data)
   {
   curlbase *obj=(curlbase *)data;
   obj->curlinit_safe(threads,id,proxyname,proxyport);
   }

void curlbase::curlexit(int id,void *data)
   {
   curlbase *obj=(curlbase *)data;
   obj->curlexit_safe(id);
   }

void curlbase::getURL(const char *src_url,const char *src_id,const char *src_file,const char *dst_file,int background,int id,void *data)
   {
   curlbase *obj=(curlbase *)data;
   obj->getURL_safe(src_url,src_id,src_file,dst_file,background,id);
   }

int curlbase::checkURL(const char *src_url,const char *src_id,const char *src_file,int id,void *data)
   {
   curlbase *obj=(curlbase *)data;
   return(obj->checkURL_safe(src_url,src_id,src_file,id));
   }

void curlbase::curlinit_safe(int threads,int id,const char *proxyname,const char *proxyport)
   {
   int i;

   initmulticurl(id);

   MULTICURL[id]->numthreads=threads;

   // allocate curl handles
   MULTICURL[id]->curl_handle=new CURLPTR[threads+2];

   for (i=0; i<threads+2; i++)
      {
      // init the curl session
      MULTICURL[id]->curl_handle[i]=curl_easy_init();

      // send all data to this function
      curl_easy_setopt(MULTICURL[id]->curl_handle[i],CURLOPT_WRITEFUNCTION,WriteMemoryCallback);

      // optionally set the proxy server address
      if (proxyname!=NULL)
         {
         curl_easy_setopt(MULTICURL[id]->curl_handle[i],CURLOPT_PROXY,proxyname);
         if (proxyport!=NULL) curl_easy_setopt(MULTICURL[id]->curl_handle[i],CURLOPT_PROXYPORT,proxyport);
         }

      // some servers don't like requests that are made without a user-agent
      curl_easy_setopt(MULTICURL[id]->curl_handle[i],CURLOPT_USERAGENT,"libMini-agent/1.0");

      if (i<threads+1)
         {
         // request zlib decompression
         curl_easy_setopt(MULTICURL[id]->curl_handle[i],CURLOPT_ENCODING,"deflate");
         }
      else
         {
         // request header only
         curl_easy_setopt(MULTICURL[id]->curl_handle[i],CURLOPT_NOBODY,1);
         }
      }
   }

void curlbase::curlexit_safe(int id)
   {
   int i;

   // clean-up curl sessions
   for (i=0; i<MULTICURL[id]->numthreads+2; i++) curl_easy_cleanup(MULTICURL[id]->curl_handle[i]);

   // free curl handles
   delete[] MULTICURL[id]->curl_handle;

   exitmulticurl(id);
   }

void curlbase::getURL_safe(const char *src_url,const char *src_id,const char *src_file,const char *dst_file,int background,int id)
   {
   char *url;

   struct MemoryStruct chunk;

   chunk.memory=NULL;
   chunk.size=0;

   url=strcct(src_url,strcct(src_id,src_file));

   // pass the chunk struct to the callback function
   curl_easy_setopt(MULTICURL[id]->curl_handle[background],CURLOPT_WRITEDATA,(void *)&chunk);

   // specify URL to get
   curl_easy_setopt(MULTICURL[id]->curl_handle[background],CURLOPT_URL,url);

   // get it!
   curl_easy_perform(MULTICURL[id]->curl_handle[background]);

   // write memory chunk to file
   if (chunk.memory)
      {
      FILE *file;

      if ((file=fopen(dst_file,"wb"))==NULL) exit(1);
      fwrite(chunk.memory,1,chunk.size,file);
      fclose(file);

      free(chunk.memory);
      }

   free(url);
   }

int curlbase::checkURL_safe(const char *src_url,const char *src_id,const char *src_file,int id)
   {
   char *url;
   int threads;
   long response;

   struct MemoryStruct chunk;

   chunk.memory=NULL;
   chunk.size=0;

   threads=MULTICURL[id]->numthreads;

   url=strcct(src_url,strcct(src_id,src_file));

   // pass the chunk struct to the callback function
   curl_easy_setopt(MULTICURL[id]->curl_handle[threads+1],CURLOPT_WRITEDATA,(void *)&chunk);

   // specify URL to get
   curl_easy_setopt(MULTICURL[id]->curl_handle[threads+1],CURLOPT_URL,url);

   // get it!
   curl_easy_perform(MULTICURL[id]->curl_handle[threads+1]);

   // query response code
   curl_easy_getinfo(MULTICURL[id]->curl_handle[threads+1],CURLINFO_RESPONSE_CODE,&response);

   // free memory chunk
   if (chunk.memory) free(chunk.memory);

   free(url);

   return(response==200);
   }

void curlbase::initmulticurl(int id)
   {
   if (MAXMULTICURL==0)
      {
      MAXMULTICURL=id+1;
      if ((MULTICURL=(MULTICURL_TYPE **)malloc(MAXMULTICURL*sizeof(MULTICURL_TYPE *)))==NULL) ERRORMSG();
      }

   if (id>=MAXMULTICURL)
      {
      MAXMULTICURL=id+1;
      if ((MULTICURL=(MULTICURL_TYPE **)realloc(MULTICURL,MAXMULTICURL*sizeof(MULTICURL_TYPE *)))==NULL) ERRORMSG();
      }

   MULTICURL[id]=new MULTICURL_TYPE;

   NUMMULTICURL++;
   }

void curlbase::exitmulticurl(int id)
   {
   delete MULTICURL[id];

   NUMMULTICURL--;

   if (NUMMULTICURL==0)
      {
      free(MULTICURL);

      MULTICURL=NULL;
      MAXMULTICURL=0;
      }
   }

#endif
