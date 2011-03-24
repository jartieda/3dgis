// (c) by Stefan Roettger

#include "lunascan.h"

// default constructor
lunascan::lunascan()
   {
   CODE=NULL;
   POINTER=NULL;

   CODESTACKMAX=100;
   CODESTACKSIZE=0;

   POOL=NULL;
   HASH=NULL;

   HASHSIZE=10000;

   STRING=NULL;
   SCOPESTACK=NULL;

   STRINGMAX=1000;
   SCOPESTACKMAX=100;

   COL=PCOL=PPCOL=0;
   LINE=PLINE=PPLINE=0;

   COMMENT='#';
   }

// destructor
lunascan::~lunascan()
   {
   int i;

   if (CODE!=NULL)
      {
      for (i=0; i<CODESTACKSIZE; i++) free(CODE[i]);

      free(CODE);
      }

   if (POINTER!=NULL) free(POINTER);

   if (POOL!=NULL)
      {
      for (i=0; i<POOLMAX; i++)
         if (POOL[i].string!=NULL) free(POOL[i].string);

      free(POOL);
      }

   if (HASH!=NULL)
      {
      for (i=0; i<HASHSIZE; i++)
         if (HASH[i].serial!=NULL) free(HASH[i].serial);

      free(HASH);
      }

   if (STRING!=NULL) free(STRING);
   if (SCOPESTACK!=NULL) free(SCOPESTACK);
   }

void lunascan::init()
   {
   int i;

   if (CODE==NULL)
      if ((CODE=(char **)malloc(CODESTACKMAX*sizeof(char *)))==NULL) ERRORMSG();

   if (POINTER==NULL)
      if ((POINTER=(int *)malloc(CODESTACKMAX*sizeof(int)))==NULL) ERRORMSG();

   if (HASH==NULL)
      {
      if ((HASH=(LUNA_HASH *)malloc(HASHSIZE*sizeof(LUNA_HASH)))==NULL) ERRORMSG();

      for (i=0; i<HASHSIZE; i++)
         {
         HASH[i].num=HASH[i].maxnum=0;
         HASH[i].serial=NULL;
         }
      }

   if (STRING==NULL)
      {
      if ((STRING=(char *)malloc(STRINGMAX*sizeof(char)))==NULL) ERRORMSG();
      STRINGSIZE=0;
      }

   if (SCOPESTACK==NULL)
      {
      if ((SCOPESTACK=(int *)malloc(SCOPESTACKMAX*sizeof(int)))==NULL) ERRORMSG();
      SCOPESTACKSIZE=0;
      }
   }

int lunascan::addtoken(const char *string,unsigned int id,int info)
   {
   int serial;

   init();

   serial=getstring(string);

   if (serial!=LUNA_UNKNOWN)
      {
      SCANNERMSG("identifier already defined");
      return(LUNA_UNKNOWN);
      }

   serial=addstring(string,id);

   POOL[serial].info=info;

   return(serial);
   }

int lunascan::getstring(const char *string)
   {
   int i;

   unsigned int hash;
   int serial;

   hash=gethash(string)%HASHSIZE;

   for (i=0; i<HASH[hash].num; i++)
      {
      serial=HASH[hash].serial[i];

      if (serial>=POOLSIZE) continue;

      if (strcmp(POOL[serial].string,string)==0) return(serial);
      }

   return(LUNA_UNKNOWN);
   }

int lunascan::addstring(const char *string,unsigned int id)
   {
   int i;

   int serial;

   if (POOL==NULL)
      {
      POOLMAX=2;

      if ((POOL=(LUNA_TOKEN *)malloc(POOLMAX*sizeof(LUNA_TOKEN)))==NULL) ERRORMSG();

      for (i=0; i<POOLMAX; i++)
         {
         POOL[i].string=NULL;
         POOL[i].id=LUNA_UNKNOWN;
         }

      POOLSIZE=0;
      }

   if (POOLSIZE>=POOLMAX)
      {
      if ((POOL=(LUNA_TOKEN *)realloc(POOL,2*POOLMAX*sizeof(LUNA_TOKEN)))==NULL) ERRORMSG();

      for (i=POOLMAX; i<2*POOLMAX; i++)
         {
         POOL[i].string=NULL;
         POOL[i].id=LUNA_UNKNOWN;
         }

      POOLMAX*=2;
      }

   serial=POOLSIZE;

   if (POOL[serial].string!=NULL) free(POOL[serial].string);

   POOL[serial].string=strdup(string);
   POOL[serial].id=id;

   addhash(serial);

   POOLSIZE++;

   return(serial);
   }

void lunascan::addhash(int serial)
   {
   int i;

   unsigned int hash;

   hash=gethash(POOL[serial].string)%HASHSIZE;

   for (i=0; i<HASH[hash].num; i++)
      if (HASH[hash].serial[i]>=POOLSIZE)
         {
         HASH[hash].serial[i]=serial;
         return;
         }

   if (HASH[hash].num>=HASH[hash].maxnum)
      if (HASH[hash].serial==NULL)
         {
         if ((HASH[hash].serial=(int *)malloc(2*sizeof(int)))==NULL) ERRORMSG();
         HASH[hash].maxnum=2;
         }
      else
         {
         if ((HASH[hash].serial=(int *)realloc(HASH[hash].serial,2*HASH[hash].maxnum*sizeof(int)))==NULL) ERRORMSG();
         HASH[hash].maxnum*=2;
         }

   HASH[hash].serial[HASH[hash].num]=serial;
   HASH[hash].num++;
   }

void lunascan::setcode(const char *code)
   {
   if (CODESTACKSIZE>0) ERRORMSG();
   pushcode(code);
   }

void lunascan::setcode(const char *code,int bytes)
   {
   char *code0;

   if (CODESTACKSIZE>0) ERRORMSG();

   if ((code0=(char *)malloc(bytes+1))==NULL) ERRORMSG();

   memcpy(code0,code,bytes);
   code0[bytes]='\0';

   pushcode(code0);
   free(code0);
   }

void lunascan::pushcode(const char *code)
   {
   init();

   if (CODESTACKSIZE>=CODESTACKMAX)
      {
      SCANNERMSG("code nesting too deep");
      return;
      }

   ungetmychar();

   CODE[CODESTACKSIZE]=strdup(code);
   POINTER[CODESTACKSIZE]=0;

   CODESTACKSIZE++;

   getmychar();
   next();
   }

char *lunascan::getcode()
   {
   if (CODESTACKSIZE<1) return(NULL);
   else return(CODE[0]);
   }

void lunascan::freecode()
   {
   int i;

   if (CODESTACKSIZE<1) ERRORMSG();

   for (i=0; i<CODESTACKSIZE; i++) free(CODE[i]);

   free(CODE);
   CODE=NULL;

   CODESTACKSIZE=0;
   }

void lunascan::push()
   {
   if (CODESTACKSIZE<1) ERRORMSG();

   if (SCOPESTACKSIZE>=SCOPESTACKMAX-1)
      {
      SCANNERMSG("scope stack overrun");
      return;
      }

   SCOPESTACK[SCOPESTACKSIZE++]=POOLSIZE;
   SCOPESTACK[SCOPESTACKSIZE++]=PLINE;
   }

void lunascan::pop()
   {
   if (CODESTACKSIZE<1) ERRORMSG();

   if (SCOPESTACKSIZE<=1)
      {
      SCANNERMSG("scope stack underrun");
      return;
      }

   SCOPESTACKSIZE--;
   POOLSIZE=SCOPESTACK[--SCOPESTACKSIZE];
   }

int lunascan::popserials()
   {
   if (CODESTACKSIZE<1) ERRORMSG();

   if (SCOPESTACKSIZE<=1) return(0);
   else return(SCOPESTACK[SCOPESTACKSIZE-2]);
   }

int lunascan::popline()
   {
   if (CODESTACKSIZE<1) ERRORMSG();

   if (SCOPESTACKSIZE<=1) return(0);
   else return(SCOPESTACK[SCOPESTACKSIZE-1]);
   }

int lunascan::gettoken()
   {
   if (CODESTACKSIZE<1) ERRORMSG();
   return(TOKEN);
   }

int lunascan::getserial()
   {
   if (CODESTACKSIZE<1) ERRORMSG();
   return(SERIAL);
   }

float lunascan::getvalue()
   {
   if (CODESTACKSIZE<1) ERRORMSG();
   return(VALUE);
   }

char *lunascan::getstring()
   {
   if (CODESTACKSIZE<1) ERRORMSG();
   return(STRING);
   }

int lunascan::getinfo()
   {
   if (CODESTACKSIZE<1) ERRORMSG();

   if (SERIAL==LUNA_UNKNOWN) ERRORMSG();

   return(POOL[SERIAL].info);
   }

BOOLINT lunascan::check_identifier()
   {
   if (CODESTACKSIZE<1) ERRORMSG();
   return(CHECKIDNT);
   }

BOOLINT lunascan::check_special()
   {
   if (CODESTACKSIZE<1) ERRORMSG();
   return(CHECKSPCL);
   }

void lunascan::next()
   {
   if (CODESTACKSIZE<1) ERRORMSG();

   clear();

   scanspace();

   if (CH!='\0')
      if (CH>='0' && CH<='9') scanvalue();
      else if (CH=='-') scanminus();
      else if (CH=='\'') scanchar();
      else if (CH=='"') scanstring();
      else if ((CH>='a' && CH<='z') || (CH>='A' && CH<='Z') || CH=='_') scanidentifier();
      else scanspecial();
   }

int lunascan::getserials()
   {
   if (CODESTACKSIZE<1) ERRORMSG();
   return(POOLSIZE);
   }

int lunascan::gettoken(int serial)
   {
   if (CODESTACKSIZE<1) ERRORMSG();

   if (serial<0 || serial>=POOLSIZE) ERRORMSG();

   return(POOL[serial].id);
   }

int lunascan::getinfo(int serial)
   {
   if (CODESTACKSIZE<1) ERRORMSG();

   if (serial<0 || serial>=POOLSIZE) ERRORMSG();

   return(POOL[serial].info);
   }

int lunascan::getcol()
   {
   if (CODESTACKSIZE<1) ERRORMSG();
   return(PPCOL);
   }

int lunascan::getline()
   {
   if (CODESTACKSIZE<1) ERRORMSG();
   return(PPLINE);
   }

void lunascan::scanspace()
   {while (CH==' ' || CH=='\n' || CH=='\r' || CH=='\t') getmychar();}

void lunascan::scanvalue()
   {
   double value;
   double point,xpnt;

   value=0.0;

   while (CH>='0' && CH<='9')
      {
      value=10.0*value+CH-'0';
      getmychar();
      }

   if (CH=='.')
       {
       getmychar();

       point=1.0;

       while (CH>='0' && CH<='9')
          {
          value=10.0*value+CH-'0';
          point*=0.1;
          getmychar();
          }

       if (CH=='e' || CH=='E')
          {
          getmychar();

          xpnt=0.0;

          if (CH=='-')
             {
             getmychar();

             while (CH>='0' && CH<='9')
                {
                xpnt=10.0*xpnt+CH-'0';
                getmychar();
                }

             xpnt*=-1.0;
             }
          else
             while (CH>='0' && CH<='9')
                {
                xpnt=10.0*xpnt+CH-'0';
                getmychar();
                }

          point*=pow(10.0,xpnt);
          }

       value*=point;
       }

   TOKEN=LUNA_VALUE;
   VALUE=value;
   }

void lunascan::scanminus()
   {
   getmychar();

   if (CH>='0' && CH<='9')
      {
      scanvalue();
      VALUE*=-1.0f;
      }
   else
      {
      if (CH=='-' || CH=='+' || CH=='*' || CH=='/')
         {
         pushback('-');
         pushback(CH);
         getmychar();
         }
      else pushback('-');

      SERIAL=getstring(STRING);

      if (SERIAL!=LUNA_UNKNOWN) TOKEN=POOL[SERIAL].id;
      else TOKEN=LUNA_UNKNOWN;

      CHECKSPCL=TRUE;
      }
   }

void lunascan::scanchar()
   {
   getmychar();

   if (CH=='\\')
      {
      getmychar();

      switch (CH)
         {
         case 'n': pushback('\n'); break;
         case 'r': pushback('\r'); break;
         case 't': pushback('\t'); break;
         case '\'': pushback('\''); break;
         case '"': pushback('"'); break;
         case '\\': pushback('\\'); break;
         case '0': pushback('\0'); break;
         default:
            SCANNERMSG("unknown escape sequence");
            pushback(CH);
            break;
         }

      getmychar();
      }
   else
      {
      pushback(CH);
      getmychar();
      }

   if (CH!='\'') SCANNERMSG("unterminated char constant");

   getmychar();

   TOKEN=LUNA_VALUE;
   VALUE=STRING[0];
   }

void lunascan::scanstring()
   {
   getmychar();

   while (CH!='"' && CH!='\n' && CH!='\r' && CH!='\0')
      if (CH=='\\')
         {
         getmychar();

         switch (CH)
            {
            case 'n': pushback('\n'); break;
            case 'r': pushback('\r'); break;
            case 't': pushback('\t'); break;
            case '\'': pushback('\''); break;
            case '"': pushback('"'); break;
            case '\\': pushback('\\'); break;
            case '0': pushback('\0'); break;
            default:
               SCANNERMSG("unknown escape sequence");
               pushback(CH);
               break;
            }

         getmychar();
         }
      else
         {
         pushback(CH);
         getmychar();
         }

   if (CH!='"') SCANNERMSG("unterminated string constant");

   getmychar();

   TOKEN=LUNA_STRING;
   }

void lunascan::scanidentifier()
   {
   while ((CH>='a' && CH<='z') || (CH>='A' && CH<='Z') || CH=='_' || (CH>='0' && CH<='9'))
      {
      pushback(CH);
      getmychar();
      }

   SERIAL=getstring(STRING);

   if (SERIAL!=LUNA_UNKNOWN) TOKEN=POOL[SERIAL].id;
   else TOKEN=LUNA_UNKNOWN;

   CHECKIDNT=TRUE;
   }

void lunascan::scanspecial()
   {
   if (CH=='-' || CH=='+' || CH=='*' || CH=='/')
      {
      pushback(CH);
      getmychar();

      if (CH=='-' || CH=='+' || CH=='*' || CH=='/')
         {
         pushback(CH);
         getmychar();
         }
      }
   else
      if  (CH==':' || CH=='=' || CH=='<' || CH=='>')
         {
         pushback(CH);
         getmychar();

         if  (CH==':' || CH=='=' || CH=='<' || CH=='>')
            {
            pushback(CH);
            getmychar();
            }
         }
      else
         {
         pushback(CH);
         getmychar();
         }

   SERIAL=getstring(STRING);

   if (SERIAL!=LUNA_UNKNOWN) TOKEN=POOL[SERIAL].id;
   else TOKEN=LUNA_UNKNOWN;

   CHECKSPCL=TRUE;
   }

void lunascan::clear()
   {
   TOKEN=LUNA_END;
   SERIAL=LUNA_UNKNOWN;
   VALUE=0.0f;

   CHECKIDNT=CHECKSPCL=FALSE;

   STRINGSIZE=0;
   STRING[0]='\0';
   }

void lunascan::pushback(char ch)
   {
   if (STRINGSIZE>=STRINGMAX-1)
      {
      SCANNERMSG("string too long");
      return;
      }

   STRING[STRINGSIZE++]=ch;
   STRING[STRINGSIZE]='\0';
   }

void lunascan::getmychar()
   {
   PPCOL=PCOL;
   PPLINE=PLINE;

   getrawchar();

   if (CH==COMMENT)
      {
      getrawchar();
      while (CH!='\n' && CH!='\r' && CH!='\0') getrawchar();
      }

   if (CH=='\t') SCANNERMSG("tab should be replaced with spaces");
   }

void lunascan::getrawchar()
   {
   CH=CODE[CODESTACKSIZE-1][POINTER[CODESTACKSIZE-1]];

   while (CH=='\0' && CODESTACKSIZE>1)
      {
      CODESTACKSIZE--;
      free(CODE[CODESTACKSIZE]);

      CH=CODE[CODESTACKSIZE-1][POINTER[CODESTACKSIZE-1]];
      }

   if (CH!='\0')
      {
      POINTER[CODESTACKSIZE-1]++;

      PCOL=COL;
      PLINE=LINE;

      if (CODESTACKSIZE<=1)
         {
         COL++;
         if (LINE==0) LINE++;

         if (CH=='\n')
            {
            COL=1;
            LINE++;
            }
         }
      }
   }

void lunascan::ungetmychar()
   {
   if (CODESTACKSIZE>0)
      if (POINTER[CODESTACKSIZE-1]>0)
         {
         POINTER[CODESTACKSIZE-1]--;

         if (CODESTACKSIZE<=1)
            {
            COL=PCOL;
            LINE=PLINE;
            }
         }
   }

unsigned int lunascan::gethash(const char *string)
   {
   static const unsigned int hashconst=271;

   unsigned int hash;

   const char *ptr;

   hash=0;

   for (ptr=string; *ptr!='\0'; ptr++) hash=hashconst*(hash+*ptr)+hash/HASHSIZE;

   return(hash);
   }

void lunascan::setcomment(char comment)
   {
   if (CODESTACKSIZE>0) ERRORMSG();

   if (comment==' ' || comment=='\n' || comment=='\r' || comment=='\0') ERRORMSG();

   COMMENT=comment;
   }

void lunascan::setcodestacksize(int size)
   {
   if (CODE!=NULL) ERRORMSG();

   if (size<1) ERRORMSG();

   CODESTACKMAX=size;
   }

void lunascan::sethashsize(int size)
   {
   if (HASH!=NULL) ERRORMSG();

   if (size<1) ERRORMSG();

   HASHSIZE=size;
   }

void lunascan::setstringsize(int size)
   {
   if (STRING!=NULL) ERRORMSG();

   if (size<1) ERRORMSG();

   STRINGSIZE=size;
   }

void lunascan::setscopestacksize(int size)
   {
   if (SCOPESTACK!=NULL) ERRORMSG();

   if (size<1) ERRORMSG();

   SCOPESTACKSIZE=size;
   }

void lunascan::SCANNERMSG(const char *msg)
   {
   if (PLINE>0) fprintf(stderr,"scanner error in column %d of line %d: %s\n",PCOL,PLINE,msg);
   else fprintf(stderr,"scanner error: %s\n",msg);
   }
