// (c) by Stefan Roettger

#ifndef LUNASCAN_H
#define LUNASCAN_H

#include "minibase.h"

class lunascan
   {
   public:

   enum
      {
      LUNA_UNKNOWN=-1,
      LUNA_VALUE=-2,
      LUNA_STRING=-3,
      LUNA_END=-4
      };

   //! default constructor
   lunascan();

   //! destructor
   ~lunascan();

   int addtoken(const char *string,unsigned int id,int info=0);

   void setcode(const char *code);
   void setcode(const char *code,int bytes);
   void pushcode(const char *code);
   char *getcode();
   void freecode();

   void push();
   void pop();

   int popserials();
   int popline();

   int gettoken();
   int getserial();
   float getvalue();
   char *getstring();
   int getinfo();

   BOOLINT check_identifier();
   BOOLINT check_special();

   void next();

   int getserials();

   int gettoken(int serial);
   int getinfo(int serial);

   int getcol();
   int getline();

   void setcomment(char comment);

   void setcodestacksize(int size);
   void sethashsize(int size);
   void setstringsize(int size);
   void setscopestacksize(int size);

   protected:

   char **CODE;
   int CODESTACKSIZE,CODESTACKMAX;
   int *POINTER;

   char CH;
   int COL,LINE;
   int PCOL,PLINE;
   int PPCOL,PPLINE;

   int TOKEN,SERIAL;
   float VALUE;

   BOOLINT CHECKIDNT,CHECKSPCL;

   private:

   struct LUNA_TOKEN
      {
      int id;
      char *string;

      int info;
      };

   struct LUNA_HASH
      {
      int num,maxnum;
      int *serial;
      };

   LUNA_TOKEN *POOL;
   int POOLSIZE,POOLMAX;

   LUNA_HASH *HASH;
   int HASHSIZE;

   char *STRING;
   int STRINGSIZE,STRINGMAX;

   int *SCOPESTACK;
   int SCOPESTACKSIZE,SCOPESTACKMAX;

   char COMMENT;

   void init();

   int getstring(const char *string);
   int addstring(const char *string,unsigned int id);

   void addhash(int serial);

   void scanspace();
   void scanvalue();
   void scanminus();
   void scanchar();
   void scanstring();
   void scanidentifier();
   void scanspecial();

   void clear();
   void pushback(char ch);

   inline void getmychar();
   inline void getrawchar();
   inline void ungetmychar();

   inline unsigned int gethash(const char *string);

   void SCANNERMSG(const char *msg);
   };

#endif
