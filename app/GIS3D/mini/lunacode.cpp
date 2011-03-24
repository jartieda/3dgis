// (c) by Stefan Roettger

#include "lunacode.h"

// default constructor
lunacode::lunacode()
   {
   CODEMAX=2;

   if ((CODE=(unsigned char *)malloc(CODEMAX*sizeof(unsigned char *)))==NULL) ERRORMSG();

   CODESIZE=0;

   RETSTACK=NULL;
   RETSTACKMAX=100;

   VALSTACK=NULL;
   VALSTACKMAX=100;

   GLBVAR=NULL;
   LOCVAR=NULL;

   GLBVARMAX=1000;
   LOCVARMAX=10000;

   LUNADEBUG=FALSE;
   }

// destructor
lunacode::~lunacode()
   {
   free(CODE);

   if (RETSTACK!=NULL) free(RETSTACK);
   if (VALSTACK!=NULL) free(VALSTACK);

   if (GLBVAR!=NULL) free(GLBVAR);
   if (LOCVAR!=NULL) free(LOCVAR);
   }

void lunacode::addcode(int code,int mode,int ival,float fval)
   {addcodeat(getaddr(),code,mode,ival,fval);}

void lunacode::addcodeat(int addr,int code,int mode,int ival,float fval)
   {
   addbyte(addr,code);

   if (code==CODE_JMP || code==CODE_JIF || code==CODE_JSR) ival-=addr;

   if (mode==MODE_ANY)
      if (ival>=-128 && ival<=127) mode=MODE_BYTE;
      else if (ival>=-32768 && ival<=32767) mode=MODE_SHORT;
      else mode=MODE_INT;

   if (mode==MODE_NONE) addbyte(addr+1,MODE_NONE);
   else if (mode==MODE_BYTE)
      {
      addbyte(addr+1,MODE_BYTE);
      addbyte(addr+2,((unsigned int)ival)&255);
      }
   else if (mode==MODE_SHORT)
      {
      addbyte(addr+1,MODE_SHORT);
      addbyte(addr+2,(((unsigned int)ival)>>8)&255);
      addbyte(addr+3,((unsigned int)ival)&255);
      }
   else if (mode==MODE_INT)
      {
      addbyte(addr+1,MODE_INT);
      addbyte(addr+2,(((unsigned int)ival)>>24)&255);
      addbyte(addr+3,(((unsigned int)ival)>>16)&255);
      addbyte(addr+4,(((unsigned int)ival)>>8)&255);
      addbyte(addr+5,((unsigned int)ival)&255);
      }
   else if (mode==MODE_FLOAT)
      {
      addbyte(addr+1,MODE_FLOAT);
      addbyte(addr+2,((unsigned char *)&fval)[0]);
      addbyte(addr+3,((unsigned char *)&fval)[1]);
      addbyte(addr+4,((unsigned char *)&fval)[2]);
      addbyte(addr+5,((unsigned char *)&fval)[3]);
      }
   }

void lunacode::addbyte(int addr,int val)
   {
   if (addr<0 || addr>CODEMAX) ERRORMSG();

   if (addr>=CODEMAX)
      {
      CODEMAX*=2;

      if ((CODE=(unsigned char *)realloc(CODE,CODEMAX*sizeof(unsigned char *)))==NULL) ERRORMSG();
      }

   CODE[addr]=val;

   if (addr==CODESIZE) CODESIZE++;
   }

int lunacode::getcode(int addr,int *code,int *mode,int *ival,float *fval)
   {
   int val;

   *code=CODE[addr++];
   *mode=CODE[addr];

   if (*mode!=MODE_NONE)
      if (*mode==MODE_FLOAT)
         {
         *fval=*((float *)&CODE[++addr]);
         addr+=3;
         }
      else if (*mode==MODE_BYTE)
         {
         val=CODE[++addr];
         if (val>127) val=val-256;

         if (*code==CODE_JMP || *code==CODE_JIF || *code==CODE_JSR) val+=addr-2;

         *ival=val;
         *fval=val;
         }
      else if (*mode==MODE_SHORT)
         {
         val=CODE[++addr];
         val=256*(unsigned int)val+CODE[++addr];
         if (val>32767) val=val-65536;

         if (*code==CODE_JMP || *code==CODE_JIF || *code==CODE_JSR) val+=addr-3;

         *ival=val;
         *fval=val;
         }
      else if (*mode==MODE_INT)
         {
         val=CODE[++addr];
         val=(int)(256*(unsigned int)val+CODE[++addr]);
         val=(int)(256*(unsigned int)val+CODE[++addr]);
         val=(int)(256*(unsigned int)val+CODE[++addr]);

         if (*code==CODE_JMP || *code==CODE_JIF || *code==CODE_JSR) val+=addr-5;

         *ival=val;
         *fval=val;
         }

   return(addr+1);
   }

int lunacode::getaddr()
   {return(CODESIZE);}

void lunacode::init(BOOLINT init_static)
   {
   RETSTACKSIZE=1;
   VALSTACKSIZE=0;

   if (GLBVAR==NULL || init_static) GLBVARSIZE=0;
   LOCVARSIZE=0;

   allocate_stacks();
   allocate_vars();

   execmd(CODE_JMP,0);
   }

void lunacode::allocate_stacks()
   {
   int i;

   if (RETSTACK==NULL)
      {
      if ((RETSTACK=(int *)malloc(RETSTACKMAX*sizeof(int)))==NULL) ERRORMSG();
      for (i=0; i<RETSTACKMAX; i++) RETSTACK[i]=-1;
      }

   if (VALSTACK==NULL)
      {
      if ((VALSTACK=(LUNA_ITEM *)malloc(VALSTACKMAX*sizeof(LUNA_ITEM)))==NULL) ERRORMSG();
      for (i=0; i<VALSTACKMAX; i++) VALSTACK[i].item=ITEM_NONE;
      }

   if (RETSTACKSIZE>=RETSTACKMAX)
      {
      if ((RETSTACK=(int *)realloc(RETSTACK,2*RETSTACKMAX*sizeof(int)))==NULL) ERRORMSG();
      for (i=RETSTACKMAX; i<2*RETSTACKMAX; i++) RETSTACK[i]=-1;
      RETSTACKMAX*=2;
      }

   if (VALSTACKSIZE>=VALSTACKMAX)
      {
      if ((VALSTACK=(LUNA_ITEM *)realloc(VALSTACK,2*VALSTACKMAX*sizeof(LUNA_ITEM)))==NULL) ERRORMSG();
      for (i=VALSTACKMAX; i<2*VALSTACKMAX; i++) VALSTACK[i].item=ITEM_NONE;
      VALSTACKMAX*=2;
      }
   }

void lunacode::allocate_vars()
   {
   int i;

   if (GLBVAR==NULL)
      {
      if ((GLBVAR=(LUNA_ITEM *)malloc(GLBVARMAX*sizeof(LUNA_ITEM)))==NULL) ERRORMSG();
      for (i=0; i<GLBVARMAX; i++) GLBVAR[i].item=ITEM_NONE;
      }

   if (LOCVAR==NULL)
      {
      if ((LOCVAR=(LUNA_ITEM *)malloc(LOCVARMAX*sizeof(LUNA_ITEM)))==NULL) ERRORMSG();
      for (i=0; i<LOCVARMAX; i++) LOCVAR[i].item=ITEM_NONE;
      for (i=0; i<LOCVARMAX; i++) LOCVAR[i].timeloc=0;
      }

   if (GLBVARSIZE>=GLBVARMAX)
      {
      if ((GLBVAR=(LUNA_ITEM *)realloc(GLBVAR,2*GLBVARMAX*sizeof(LUNA_ITEM)))==NULL) ERRORMSG();
      for (i=GLBVARMAX; i<2*GLBVARMAX; i++) GLBVAR[i].item=ITEM_NONE;
      GLBVARMAX*=2;
      }

   if (LOCVARSIZE>=LOCVARMAX)
      {
      if ((LOCVAR=(LUNA_ITEM *)realloc(LOCVAR,2*LOCVARMAX*sizeof(LUNA_ITEM)))==NULL) ERRORMSG();
      for (i=LOCVARMAX; i<2*LOCVARMAX; i++) LOCVAR[i].item=ITEM_NONE;
      for (i=LOCVARMAX; i<2*LOCVARMAX; i++) LOCVAR[i].timeloc=0;
      LOCVARMAX*=2;
      }
   }

void lunacode::execute()
   {
   int addr;

   int code;
   int mode;
   int ival;
   float fval;

   addr=0;

   while (addr>=0 && addr<CODESIZE)
      {
      code=CODE_NOP;
      mode=MODE_NONE;
      ival=0;
      fval=0.0f;

      addr=getcode(RETSTACK[RETSTACKSIZE-1],&code,&mode,&ival,&fval);
      RETSTACK[RETSTACKSIZE-1]=addr;

      execmd(code,ival,fval);
      addr=RETSTACK[RETSTACKSIZE-1];
      }
   }

void *lunacode::mallocarray(int item,unsigned int size)
   {
   unsigned int i;

   void *ptr;

   float *ptrfloat;
   unsigned char *ptruchar;

   if (item==ITEM_ARRAY_FLOAT)
      {
      ptr=malloc(size*sizeof(float));
      if (ptr!=NULL)
         {
         ptrfloat=(float *)ptr;
         for (i=0; i<size; i++) *ptrfloat++=0.0f;
         }
      }
   else if (item==ITEM_ARRAY_BYTE)
      {
      ptr=malloc(size*sizeof(unsigned char));
      if (ptr!=NULL)
         {
         ptruchar=(unsigned char *)ptr;
         for (i=0; i<size; i++) *ptruchar++=0;
         }
      }
   else ptr=NULL;

   return(ptr);
   }

void *lunacode::reallocarray(void *array,int item,unsigned int size,unsigned int orig)
   {
   unsigned int i;

   void *ptr;

   float *ptrfloat;
   unsigned char *ptruchar;

   if (item==ITEM_ARRAY_FLOAT)
      {
      ptr=realloc(array,size*sizeof(float));
      if (ptr!=NULL)
         {
         ptrfloat=(float *)ptr+orig;
         for (i=orig; i<size; i++) *ptrfloat++=0.0f;
         }
      }
   else if (item==ITEM_ARRAY_BYTE)
      {
      ptr=realloc(array,size*sizeof(unsigned char));
      if (ptr!=NULL)
         {
         ptruchar=(unsigned char *)ptr+orig;
         for (i=orig; i<size; i++) *ptruchar++=0;
         }
      }
   else ptr=NULL;

   return(ptr);
   }

void lunacode::checkarray(int ref,unsigned int idx)
   {
   if (GLBVAR[ref].maxsize>0 && idx>=GLBVAR[ref].maxsize) return;

   if (GLBVAR[ref].array==NULL)
      if (GLBVAR[ref].maxsize>0)
         if ((GLBVAR[ref].array=mallocarray(GLBVAR[ref].item,GLBVAR[ref].maxsize))==NULL) CODEMSG("insufficient resources");
         else GLBVAR[ref].size=GLBVAR[ref].maxsize;
      else
         if ((GLBVAR[ref].array=mallocarray(GLBVAR[ref].item,(idx+1)))==NULL) CODEMSG("insufficient resources");
         else GLBVAR[ref].size=idx+1;
   else
      if (GLBVAR[ref].maxsize==0 && idx>=GLBVAR[ref].size)
         if ((GLBVAR[ref].array=reallocarray(GLBVAR[ref].array,GLBVAR[ref].item,idx+1,GLBVAR[ref].size))==NULL) CODEMSG("insufficient resources");
         else GLBVAR[ref].size=idx+1;
   }

void lunacode::checkarrayloc(int refloc,unsigned int idx)
   {
   if (LOCVAR[refloc].maxsize>0 && idx>=LOCVAR[refloc].maxsize) return;

   if (LOCVAR[refloc].array==NULL)
      {
      if (LOCVAR[refloc].item==ITEM_ARRAY_FLOAT)
         if (LOCVAR[refloc].maxsize>0)
            if ((LOCVAR[refloc].array=malloc(LOCVAR[refloc].maxsize*sizeof(float)))==NULL) CODEMSG("insufficient resources");
            else LOCVAR[refloc].size=LOCVAR[refloc].maxsize;
         else
            if ((LOCVAR[refloc].array=malloc((idx+1)*sizeof(float)))==NULL) CODEMSG("insufficient resources");
            else LOCVAR[refloc].size=idx+1;

      if (LOCVAR[refloc].item==ITEM_ARRAY_BYTE)
         if (LOCVAR[refloc].maxsize>0)
            if ((LOCVAR[refloc].array=malloc(LOCVAR[refloc].maxsize*sizeof(unsigned char)))==NULL) CODEMSG("insufficient resources");
            else LOCVAR[refloc].size=LOCVAR[refloc].maxsize;
         else
            if ((LOCVAR[refloc].array=malloc((idx+1)*sizeof(unsigned char)))==NULL) CODEMSG("insufficient resources");
            else LOCVAR[refloc].size=idx+1;
      }
   else
      {
      if (LOCVAR[refloc].item==ITEM_ARRAY_FLOAT)
         if (LOCVAR[refloc].maxsize==0 && idx>=LOCVAR[refloc].size)
            if ((LOCVAR[refloc].array=realloc(LOCVAR[refloc].array,(idx+1)*sizeof(float)))==NULL) CODEMSG("insufficient resources");
            else LOCVAR[refloc].size=idx+1;

      if (LOCVAR[refloc].item==ITEM_ARRAY_BYTE)
         if (LOCVAR[refloc].maxsize==0 && idx>=LOCVAR[refloc].size)
            if ((LOCVAR[refloc].array=realloc(LOCVAR[refloc].array,(idx+1)*sizeof(unsigned char)))==NULL) CODEMSG("insufficient resources");
            else LOCVAR[refloc].size=idx+1;
      }
   }

double lunacode::mod(double a,double b)
   {return(a-floor(a/b)*b);}

void lunacode::execmd(int code,int ival,float fval)
   {
   int ref,refloc;
   unsigned int idx;

   if (LUNADEBUG)
      {
      printf("cmd=");
      printcode(code);

      printf(" ival=%d fval=%f",ival,fval);

      if (VALSTACKSIZE>0) printf(" pre=%f",VALSTACK[VALSTACKSIZE-1].val);
      else printf(" pre=none");
      }

   switch (code)
      {
      case CODE_NOP:
         break;
      case CODE_ADD:
         if (VALSTACKSIZE<2) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT || VALSTACK[VALSTACKSIZE-2].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else {VALSTACK[VALSTACKSIZE-2].val+=VALSTACK[VALSTACKSIZE-1].val; VALSTACKSIZE--;}
         break;
      case CODE_SUB:
         if (VALSTACKSIZE<2) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT || VALSTACK[VALSTACKSIZE-2].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else {VALSTACK[VALSTACKSIZE-2].val-=VALSTACK[VALSTACKSIZE-1].val; VALSTACKSIZE--;}
         break;
      case CODE_MUL:
         if (VALSTACKSIZE<2) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT || VALSTACK[VALSTACKSIZE-2].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else {VALSTACK[VALSTACKSIZE-2].val*=VALSTACK[VALSTACKSIZE-1].val; VALSTACKSIZE--;}
         break;
      case CODE_DIV:
         if (VALSTACKSIZE<2) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT || VALSTACK[VALSTACKSIZE-2].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else {VALSTACK[VALSTACKSIZE-2].val/=VALSTACK[VALSTACKSIZE-1].val; VALSTACKSIZE--;}
         break;
      case CODE_MOD:
         if (VALSTACKSIZE<2) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT || VALSTACK[VALSTACKSIZE-2].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else {VALSTACK[VALSTACKSIZE-2].val=mod(VALSTACK[VALSTACKSIZE-2].val,VALSTACK[VALSTACKSIZE-1].val); VALSTACKSIZE--;}
         break;
      case CODE_EQ:
         if (VALSTACKSIZE<2) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT || VALSTACK[VALSTACKSIZE-2].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else {VALSTACK[VALSTACKSIZE-2].val=(VALSTACK[VALSTACKSIZE-2].val==VALSTACK[VALSTACKSIZE-1].val)?1.0:0.0; VALSTACKSIZE--;}
         break;
      case CODE_NEQ:
         if (VALSTACKSIZE<2) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT || VALSTACK[VALSTACKSIZE-2].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else {VALSTACK[VALSTACKSIZE-2].val=(VALSTACK[VALSTACKSIZE-2].val!=VALSTACK[VALSTACKSIZE-1].val)?1.0:0.0; VALSTACKSIZE--;}
         break;
      case CODE_LT:
         if (VALSTACKSIZE<2) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT || VALSTACK[VALSTACKSIZE-2].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else {VALSTACK[VALSTACKSIZE-2].val=(VALSTACK[VALSTACKSIZE-2].val<VALSTACK[VALSTACKSIZE-1].val)?1.0:0.0; VALSTACKSIZE--;}
         break;
      case CODE_GT:
         if (VALSTACKSIZE<2) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT || VALSTACK[VALSTACKSIZE-2].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else {VALSTACK[VALSTACKSIZE-2].val=(VALSTACK[VALSTACKSIZE-2].val>VALSTACK[VALSTACKSIZE-1].val)?1.0:0.0; VALSTACKSIZE--;}
         break;
      case CODE_LE:
         if (VALSTACKSIZE<2) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT || VALSTACK[VALSTACKSIZE-2].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else {VALSTACK[VALSTACKSIZE-2].val=(VALSTACK[VALSTACKSIZE-2].val<=VALSTACK[VALSTACKSIZE-1].val)?1.0:0.0; VALSTACKSIZE--;}
         break;
      case CODE_GE:
         if (VALSTACKSIZE<2) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT || VALSTACK[VALSTACKSIZE-2].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else {VALSTACK[VALSTACKSIZE-2].val=(VALSTACK[VALSTACKSIZE-2].val>=VALSTACK[VALSTACKSIZE-1].val)?1.0:0.0; VALSTACKSIZE--;}
         break;
      case CODE_AND:
         if (VALSTACKSIZE<2) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT || VALSTACK[VALSTACKSIZE-2].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else {VALSTACK[VALSTACKSIZE-2].val=(VALSTACK[VALSTACKSIZE-2].val!=0.0 && VALSTACK[VALSTACKSIZE-1].val!=0.0)?1.0:0.0; VALSTACKSIZE--;}
         break;
      case CODE_OR:
         if (VALSTACKSIZE<2) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT || VALSTACK[VALSTACKSIZE-2].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else {VALSTACK[VALSTACKSIZE-2].val=(VALSTACK[VALSTACKSIZE-2].val!=0.0 || VALSTACK[VALSTACKSIZE-1].val!=0.0)?1.0:0.0; VALSTACKSIZE--;}
         break;
      case CODE_NOT:
         if (VALSTACKSIZE<1) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else VALSTACK[VALSTACKSIZE-1].val=(VALSTACK[VALSTACKSIZE-1].val==0.0)?1.0:0.0;
         break;
      case CODE_MIN:
         if (VALSTACKSIZE<2) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT || VALSTACK[VALSTACKSIZE-2].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else {VALSTACK[VALSTACKSIZE-2].val=(VALSTACK[VALSTACKSIZE-2].val<VALSTACK[VALSTACKSIZE-1].val)?VALSTACK[VALSTACKSIZE-2].val:VALSTACK[VALSTACKSIZE-1].val; VALSTACKSIZE--;}
         break;
      case CODE_MAX:
         if (VALSTACKSIZE<2) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT || VALSTACK[VALSTACKSIZE].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else {VALSTACK[VALSTACKSIZE-2].val=(VALSTACK[VALSTACKSIZE-2].val>VALSTACK[VALSTACKSIZE-1].val)?VALSTACK[VALSTACKSIZE-2].val:VALSTACK[VALSTACKSIZE-1].val; VALSTACKSIZE--;}
         break;
      case CODE_ABS:
         if (VALSTACKSIZE<1) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else VALSTACK[VALSTACKSIZE-1].val=(VALSTACK[VALSTACKSIZE-1].val>0.0)?VALSTACK[VALSTACKSIZE-1].val:-VALSTACK[VALSTACKSIZE-1].val;
         break;
      case CODE_SQR:
         if (VALSTACKSIZE<1) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else VALSTACK[VALSTACKSIZE-1].val=VALSTACK[VALSTACKSIZE-1].val*VALSTACK[VALSTACKSIZE-1].val;
         break;
      case CODE_SQRT:
         if (VALSTACKSIZE<1) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else VALSTACK[VALSTACKSIZE-1].val=sqrt(VALSTACK[VALSTACKSIZE-1].val);
         break;
      case CODE_EXP:
         if (VALSTACKSIZE<1) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else VALSTACK[VALSTACKSIZE-1].val=exp(VALSTACK[VALSTACKSIZE-1].val);
         break;
      case CODE_LOG:
         if (VALSTACKSIZE<1) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else VALSTACK[VALSTACKSIZE-1].val=log(VALSTACK[VALSTACKSIZE-1].val);
         break;
      case CODE_POW:
         if (VALSTACKSIZE<2) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT || VALSTACK[VALSTACKSIZE-2].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else {VALSTACK[VALSTACKSIZE-2].val=pow(VALSTACK[VALSTACKSIZE-2].val,VALSTACK[VALSTACKSIZE-1].val); VALSTACKSIZE--;}
         break;
      case CODE_SIN:
         if (VALSTACKSIZE<1) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else VALSTACK[VALSTACKSIZE-1].val=sin(VALSTACK[VALSTACKSIZE-1].val);
         break;
      case CODE_COS:
         if (VALSTACKSIZE<1) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else VALSTACK[VALSTACKSIZE-1].val=cos(VALSTACK[VALSTACKSIZE-1].val);
         break;
      case CODE_TAN:
         if (VALSTACKSIZE<1) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else VALSTACK[VALSTACKSIZE-1].val=tan(VALSTACK[VALSTACKSIZE-1].val);
         break;
      case CODE_ATAN:
         if (VALSTACKSIZE<1) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else VALSTACK[VALSTACKSIZE-1].val=atan(VALSTACK[VALSTACKSIZE-1].val);
         break;
      case CODE_ATAN2:
         if (VALSTACKSIZE<2) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT || VALSTACK[VALSTACKSIZE-2].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else {VALSTACK[VALSTACKSIZE-2].val=atan2(VALSTACK[VALSTACKSIZE-2].val,VALSTACK[VALSTACKSIZE-1].val); VALSTACKSIZE--;}
         break;
      case CODE_PUSH:
         VALSTACKSIZE++;
         allocate_stacks();
         VALSTACK[VALSTACKSIZE-1].item=ITEM_FLOAT;
         VALSTACK[VALSTACKSIZE-1].val=fval;
         break;
      case CODE_RESERVE_VAR:
         if (ival>GLBVARSIZE) GLBVARSIZE=ival;
         allocate_vars();
         break;
      case CODE_RESERVE_VAR_LOC:
         LOCVARSIZE+=ival;
         allocate_vars();
         break;
      case CODE_REMOVE_VAR_LOC:
         if (LOCVARSIZE<ival) CODEMSG("heap underrun");
         else LOCVARSIZE-=ival;
         break;
      case CODE_PUSH_VAR:
         VALSTACKSIZE++;
         allocate_stacks();
         VALSTACK[VALSTACKSIZE-1].item=ITEM_FLOAT;
         VALSTACK[VALSTACKSIZE-1].val=GLBVAR[ival].val;
         break;
      case CODE_PUSH_VAR_LOC:
         VALSTACKSIZE++;
         allocate_stacks();
         VALSTACK[VALSTACKSIZE-1].item=ITEM_FLOAT;
         VALSTACK[VALSTACKSIZE-1].val=LOCVAR[LOCVARSIZE-1-ival].val;
         break;
      case CODE_POP_VAR:
         if (VALSTACKSIZE<1) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else if (GLBVAR[ival].item!=ITEM_NONE && GLBVAR[ival].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else
            {
            GLBVAR[ival].item=ITEM_FLOAT;
            GLBVAR[ival].val=VALSTACK[--VALSTACKSIZE].val;
            }
         break;
      case CODE_POP_VAR_STAT:
         if (VALSTACKSIZE<1) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else if (GLBVAR[ival].item!=ITEM_NONE && GLBVAR[ival].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else if (GLBVAR[ival].item==ITEM_NONE)
            {
            GLBVAR[ival].item=ITEM_FLOAT;
            GLBVAR[ival].val=VALSTACK[--VALSTACKSIZE].val;
            }
         else VALSTACKSIZE--;
         break;
      case CODE_POP_VAR_LOC:
         if (VALSTACKSIZE<1) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else if (LOCVAR[LOCVARSIZE-1-ival].item!=ITEM_NONE && LOCVAR[LOCVARSIZE-1-ival].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else
            {
            LOCVAR[LOCVARSIZE-1-ival].item=ITEM_FLOAT;
            LOCVAR[LOCVARSIZE-1-ival].val=VALSTACK[--VALSTACKSIZE].val;
            }
         break;
      case CODE_INC_VAR:
         if (GLBVAR[ival].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else GLBVAR[ival].val++;
         break;
      case CODE_INC_VAR_LOC:
         if (LOCVAR[LOCVARSIZE-1-ival].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else LOCVAR[LOCVARSIZE-1-ival].val++;
         break;
      case CODE_DEC_VAR:
         if (GLBVAR[ival].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else GLBVAR[ival].val--;
         break;
      case CODE_DEC_VAR_LOC:
         if (LOCVAR[LOCVARSIZE-1-ival].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else LOCVAR[LOCVARSIZE-1-ival].val--;
         break;
      case CODE_ALLOC_ARRAY:
         if (VALSTACKSIZE<1) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else if (GLBVAR[ival].item!=ITEM_NONE) CODEMSG("invalid operation");
         else
            {
            GLBVAR[ival].item=ITEM_ARRAY_FLOAT;
            GLBVAR[ival].array=NULL;
            if (VALSTACK[VALSTACKSIZE-1].val<1.0) VALSTACK[VALSTACKSIZE-1].val=1.0;
            GLBVAR[ival].maxsize=(unsigned int)floor(VALSTACK[--VALSTACKSIZE].val+0.5);
            GLBVAR[ival].size=0;
            }
         break;
      case CODE_ALLOC_ARRAY_LOC:
         if (VALSTACKSIZE<1) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else if (LOCVAR[LOCVARSIZE-1-ival].item!=ITEM_NONE) CODEMSG("invalid operation");
         else
            {
            LOCVAR[LOCVARSIZE-1-ival].item=ITEM_ARRAY_FLOAT;
            LOCVAR[LOCVARSIZE-1-ival].timeloc++;
            LOCVAR[LOCVARSIZE-1-ival].array=NULL;
            if (VALSTACK[VALSTACKSIZE-1].val<1.0) VALSTACK[VALSTACKSIZE-1].val=1.0;
            LOCVAR[LOCVARSIZE-1-ival].maxsize=(unsigned int)floor(VALSTACK[--VALSTACKSIZE].val+0.5);
            LOCVAR[LOCVARSIZE-1-ival].size=0;
            }
         break;
      case CODE_ALLOC_ARRAY_BYT:
         if (VALSTACKSIZE<1) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else if (GLBVAR[ival].item!=ITEM_NONE) CODEMSG("invalid operation");
         else
            {
            GLBVAR[ival].item=ITEM_ARRAY_BYTE;
            GLBVAR[ival].array=NULL;
            if (VALSTACK[VALSTACKSIZE-1].val<1.0) VALSTACK[VALSTACKSIZE-1].val=1.0;
            GLBVAR[ival].maxsize=(unsigned int)floor(VALSTACK[--VALSTACKSIZE].val+0.5);
            GLBVAR[ival].size=0;
            }
         break;
      case CODE_ALLOC_ARRAY_BYT_LOC:
         if (VALSTACKSIZE<1) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else if (LOCVAR[LOCVARSIZE-1-ival].item!=ITEM_NONE) CODEMSG("invalid operation");
         else
            {
            LOCVAR[LOCVARSIZE-1-ival].item=ITEM_ARRAY_BYTE;
            LOCVAR[LOCVARSIZE-1-ival].timeloc++;
            LOCVAR[LOCVARSIZE-1-ival].array=NULL;
            if (VALSTACK[VALSTACKSIZE-1].val<1.0) VALSTACK[VALSTACKSIZE-1].val=1.0;
            LOCVAR[LOCVARSIZE-1-ival].maxsize=(unsigned int)floor(VALSTACK[--VALSTACKSIZE].val+0.5);
            LOCVAR[LOCVARSIZE-1-ival].size=0;
            }
         break;
      case CODE_INIT_ARRAY:
         if (GLBVAR[ival].item!=ITEM_NONE) CODEMSG("invalid operation");
         else
            {
            GLBVAR[ival].item=ITEM_ARRAY_FLOAT;
            GLBVAR[ival].array=NULL;
            GLBVAR[ival].maxsize=0;
            GLBVAR[ival].size=0;
            }
         break;
      case CODE_INIT_ARRAY_LOC:
         if (LOCVAR[LOCVARSIZE-1-ival].item!=ITEM_NONE) CODEMSG("invalid operation");
         else
            {
            LOCVAR[LOCVARSIZE-1-ival].item=ITEM_ARRAY_FLOAT;
            LOCVAR[LOCVARSIZE-1-ival].timeloc++;
            LOCVAR[LOCVARSIZE-1-ival].array=NULL;
            LOCVAR[LOCVARSIZE-1-ival].maxsize=0;
            LOCVAR[LOCVARSIZE-1-ival].size=0;
            }
         break;
      case CODE_INIT_ARRAY_BYT:
         if (GLBVAR[ival].item!=ITEM_NONE) CODEMSG("invalid operation");
         else
            {
            GLBVAR[ival].item=ITEM_ARRAY_BYTE;
            GLBVAR[ival].array=NULL;
            GLBVAR[ival].maxsize=0;
            GLBVAR[ival].size=0;
            }
         break;
      case CODE_INIT_ARRAY_BYT_LOC:
         if (LOCVAR[LOCVARSIZE-1-ival].item!=ITEM_NONE) CODEMSG("invalid operation");
         else
            {
            LOCVAR[LOCVARSIZE-1-ival].item=ITEM_ARRAY_BYTE;
            LOCVAR[LOCVARSIZE-1-ival].timeloc++;
            LOCVAR[LOCVARSIZE-1-ival].array=NULL;
            LOCVAR[LOCVARSIZE-1-ival].maxsize=0;
            LOCVAR[LOCVARSIZE-1-ival].size=0;
            }
         break;
      case CODE_FREE_ARRAY_LOC:
         if (LOCVAR[LOCVARSIZE-1-ival].item!=ITEM_ARRAY_FLOAT && LOCVAR[LOCVARSIZE-1-ival].item!=ITEM_ARRAY_BYTE) CODEMSG("invalid operation");
         else
            {
            if (LOCVAR[LOCVARSIZE-1-ival].array!=NULL) free(LOCVAR[LOCVARSIZE-1-ival].array);
            LOCVAR[LOCVARSIZE-1-ival].item=ITEM_NONE;
            }
         break;
      case CODE_SIZE_ARRAY:
         if (GLBVAR[ival].item!=ITEM_ARRAY_FLOAT && GLBVAR[ival].item!=ITEM_ARRAY_BYTE) CODEMSG("invalid operation");
         else
            {
            VALSTACKSIZE++;
            allocate_stacks();
            VALSTACK[VALSTACKSIZE-1].item=ITEM_FLOAT;
            VALSTACK[VALSTACKSIZE-1].val=GLBVAR[ival].size;
            }
         break;
      case CODE_SIZE_ARRAY_LOC:
         if (LOCVAR[LOCVARSIZE-1-ival].item!=ITEM_ARRAY_FLOAT && LOCVAR[LOCVARSIZE-1-ival].item!=ITEM_ARRAY_BYTE) CODEMSG("invalid operation");
         else
            {
            VALSTACKSIZE++;
            allocate_stacks();
            VALSTACK[VALSTACKSIZE-1].item=ITEM_FLOAT;
            VALSTACK[VALSTACKSIZE-1].val=LOCVAR[LOCVARSIZE-1-ival].size;
            }
         break;
      case CODE_INIT_REF:
         if (GLBVAR[ival].item!=ITEM_NONE) CODEMSG("invalid operation");
         else
            {
            GLBVAR[ival].item=ITEM_REF_FLOAT;
            GLBVAR[ival].ref=GLBVAR[ival].refloc=-1;
            }
         break;
      case CODE_INIT_REF_LOC:
         if (LOCVAR[LOCVARSIZE-1-ival].item!=ITEM_NONE) CODEMSG("invalid operation");
         else
            {
            LOCVAR[LOCVARSIZE-1-ival].item=ITEM_REF_FLOAT;
            LOCVAR[LOCVARSIZE-1-ival].ref=LOCVAR[LOCVARSIZE-1-ival].refloc=-1;
            }
         break;
      case CODE_INIT_REF_BYT:
         if (GLBVAR[ival].item!=ITEM_NONE) CODEMSG("invalid operation");
         else
            {
            GLBVAR[ival].item=ITEM_REF_BYTE;
            GLBVAR[ival].ref=GLBVAR[ival].refloc=-1;
            }
         break;
      case CODE_INIT_REF_BYT_LOC:
         if (LOCVAR[LOCVARSIZE-1-ival].item!=ITEM_NONE) CODEMSG("invalid operation");
         else
            {
            LOCVAR[LOCVARSIZE-1-ival].item=ITEM_REF_BYTE;
            LOCVAR[LOCVARSIZE-1-ival].ref=LOCVAR[LOCVARSIZE-1-ival].refloc=-1;
            }
         break;
      case CODE_SIZE_REF:
         if (GLBVAR[ival].item!=ITEM_REF_FLOAT && GLBVAR[ival].item!=ITEM_REF_BYTE) CODEMSG("invalid operation");
         else
            if (GLBVAR[ival].ref>=0)
               {
               ref=GLBVAR[ival].ref;
               if (GLBVAR[ref].item!=ITEM_ARRAY_FLOAT && GLBVAR[ref].item!=ITEM_ARRAY_BYTE) CODEMSG("invalid operation");
               else
                  {
                  VALSTACKSIZE++;
                  allocate_stacks();
                  VALSTACK[VALSTACKSIZE-1].item=ITEM_FLOAT;
                  VALSTACK[VALSTACKSIZE-1].val=GLBVAR[ref].size;
                  }
               }
            else if (GLBVAR[ival].refloc>=0)
               {
               refloc=GLBVAR[ival].refloc;
               if (LOCVAR[refloc].item!=ITEM_ARRAY_FLOAT && LOCVAR[refloc].item!=ITEM_ARRAY_BYTE) CODEMSG("invalid operation");
               else
                  {
                  VALSTACKSIZE++;
                  allocate_stacks();
                  VALSTACK[VALSTACKSIZE-1].item=ITEM_FLOAT;
                  VALSTACK[VALSTACKSIZE-1].val=LOCVAR[refloc].size;
                  }
               }
            else CODEMSG("invalid reference");
         break;
      case CODE_SIZE_REF_LOC:
         if (LOCVAR[LOCVARSIZE-1-ival].item!=ITEM_REF_FLOAT && LOCVAR[LOCVARSIZE-1-ival].item!=ITEM_REF_BYTE) CODEMSG("invalid operation");
         else
            if (LOCVAR[LOCVARSIZE-1-ival].ref>=0)
               {
               ref=LOCVAR[LOCVARSIZE-1-ival].ref;
               if (GLBVAR[ref].item!=ITEM_ARRAY_FLOAT && GLBVAR[ref].item!=ITEM_ARRAY_BYTE) CODEMSG("invalid operation");
               else
                  {
                  VALSTACKSIZE++;
                  allocate_stacks();
                  VALSTACK[VALSTACKSIZE-1].item=ITEM_FLOAT;
                  VALSTACK[VALSTACKSIZE-1].val=GLBVAR[ref].size;
                  }
               }
            else if (LOCVAR[LOCVARSIZE-1-ival].refloc>=0)
               {
               refloc=LOCVAR[LOCVARSIZE-1-ival].refloc;
               if (LOCVAR[refloc].item!=ITEM_ARRAY_FLOAT && LOCVAR[refloc].item!=ITEM_ARRAY_BYTE) CODEMSG("invalid operation");
               else
                  {
                  VALSTACKSIZE++;
                  allocate_stacks();
                  VALSTACK[VALSTACKSIZE-1].item=ITEM_FLOAT;
                  VALSTACK[VALSTACKSIZE-1].val=LOCVAR[refloc].size;
                  }
               }
            else CODEMSG("invalid reference");
         break;
      case CODE_PUSH_ARRAY:
         if (GLBVAR[ival].item==ITEM_ARRAY_FLOAT)
            {
            VALSTACKSIZE++;
            allocate_stacks();
            VALSTACK[VALSTACKSIZE-1].item=ITEM_REF_FLOAT;
            VALSTACK[VALSTACKSIZE-1].val=0.0f;
            VALSTACK[VALSTACKSIZE-1].ref=ival;
            VALSTACK[VALSTACKSIZE-1].refloc=-1;
            }
         else if (GLBVAR[ival].item==ITEM_ARRAY_BYTE)
            {
            VALSTACKSIZE++;
            allocate_stacks();
            VALSTACK[VALSTACKSIZE-1].item=ITEM_REF_BYTE;
            VALSTACK[VALSTACKSIZE-1].val=0.0f;
            VALSTACK[VALSTACKSIZE-1].ref=ival;
            VALSTACK[VALSTACKSIZE-1].refloc=-1;
            }
         else CODEMSG("invalid operation");
         break;
      case CODE_PUSH_ARRAY_LOC:
         if (LOCVAR[LOCVARSIZE-1-ival].item==ITEM_ARRAY_FLOAT)
            {
            VALSTACKSIZE++;
            allocate_stacks();
            VALSTACK[VALSTACKSIZE-1].item=ITEM_REF_FLOAT;
            VALSTACK[VALSTACKSIZE-1].val=0.0f;
            VALSTACK[VALSTACKSIZE-1].timeloc=LOCVAR[LOCVARSIZE-1-ival].timeloc;
            VALSTACK[VALSTACKSIZE-1].refloc=LOCVARSIZE-1-ival;
            VALSTACK[VALSTACKSIZE-1].ref=-1;
            }
         else if (LOCVAR[LOCVARSIZE-1-ival].item==ITEM_ARRAY_BYTE)
            {
            VALSTACKSIZE++;
            allocate_stacks();
            VALSTACK[VALSTACKSIZE-1].item=ITEM_REF_BYTE;
            VALSTACK[VALSTACKSIZE-1].val=0.0f;
            VALSTACK[VALSTACKSIZE-1].timeloc=LOCVAR[LOCVARSIZE-1-ival].timeloc;
            VALSTACK[VALSTACKSIZE-1].refloc=LOCVARSIZE-1-ival;
            VALSTACK[VALSTACKSIZE-1].ref=-1;
            }
         else CODEMSG("invalid operation");
         break;
      case CODE_PUSH_ARRAY_IDX:
         if (VALSTACKSIZE<1) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else if (GLBVAR[ival].item==ITEM_ARRAY_FLOAT)
            if (VALSTACK[VALSTACKSIZE-1].val<0.0) CODEMSG("invalid index");
            else
               {
               idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE-1].val+0.5);
               checkarray(ival,idx);
               if (idx>=GLBVAR[ival].size) CODEMSG("invalid index");
               else VALSTACK[VALSTACKSIZE-1].val=((float *)GLBVAR[ival].array)[idx];
               }
         else if (GLBVAR[ival].item==ITEM_ARRAY_BYTE)
            if (VALSTACK[VALSTACKSIZE-1].val<0.0) CODEMSG("invalid index");
            else
               {
               idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE-1].val+0.5);
               checkarray(ival,idx);
               if (idx>=GLBVAR[ival].size) CODEMSG("invalid index");
               else VALSTACK[VALSTACKSIZE-1].val=((unsigned char *)GLBVAR[ival].array)[idx];
               }
         else CODEMSG("invalid operation");
         break;
      case CODE_PUSH_ARRAY_LOC_IDX:
         if (VALSTACKSIZE<1) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else if (LOCVAR[LOCVARSIZE-1-ival].item==ITEM_ARRAY_FLOAT)
            if (VALSTACK[VALSTACKSIZE-1].val<0.0) CODEMSG("invalid index");
            else
               {
               idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE-1].val+0.5);
               checkarrayloc(LOCVARSIZE-1-ival,idx);
               if (idx>=LOCVAR[LOCVARSIZE-1-ival].size) CODEMSG("invalid index");
               else VALSTACK[VALSTACKSIZE-1].val=((float *)LOCVAR[LOCVARSIZE-1-ival].array)[idx];
               }
         else if (LOCVAR[LOCVARSIZE-1-ival].item==ITEM_ARRAY_BYTE)
            if (VALSTACK[VALSTACKSIZE-1].val<0.0) CODEMSG("invalid index");
            else
               {
               idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE-1].val+0.5);
               checkarrayloc(LOCVARSIZE-1-ival,idx);
               if (idx>=LOCVAR[LOCVARSIZE-1-ival].size) CODEMSG("invalid index");
               else VALSTACK[VALSTACKSIZE-1].val=((unsigned char *)LOCVAR[LOCVARSIZE-1-ival].array)[idx];
               }
         else CODEMSG("invalid operation");
         break;
      case CODE_POP_ARRAY_IDX:
         if (VALSTACKSIZE<2) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT || VALSTACK[VALSTACKSIZE-2].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else if (GLBVAR[ival].item==ITEM_ARRAY_FLOAT)
            {
            if (VALSTACK[VALSTACKSIZE-2].val<0.0) CODEMSG("invalid index");
            else
               {
               idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE-2].val+0.5);
               checkarray(ival,idx);
               if (idx>=GLBVAR[ival].size) CODEMSG("invalid index");
               else ((float *)GLBVAR[ival].array)[idx]=VALSTACK[VALSTACKSIZE-1].val;
               }
            VALSTACKSIZE-=2;
            }
         else if (GLBVAR[ival].item==ITEM_ARRAY_BYTE)
            {
            if (VALSTACK[VALSTACKSIZE-2].val<0.0) CODEMSG("invalid index");
            else
               {
               idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE-2].val+0.5);
               checkarray(ival,idx);
               if (idx>=GLBVAR[ival].size) CODEMSG("invalid index");
               else ((unsigned char *)GLBVAR[ival].array)[idx]=(int)floor(VALSTACK[VALSTACKSIZE-1].val+0.5);
               }
            VALSTACKSIZE-=2;
            }
         else CODEMSG("invalid operation");
         break;
      case CODE_POP_ARRAY_LOC_IDX:
         if (VALSTACKSIZE<2) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT || VALSTACK[VALSTACKSIZE-2].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else if (LOCVAR[LOCVARSIZE-1-ival].item==ITEM_ARRAY_FLOAT)
            {
            if (VALSTACK[VALSTACKSIZE-2].val<0.0) CODEMSG("invalid index");
            else
               {
               idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE-2].val+0.5);
               checkarrayloc(LOCVARSIZE-1-ival,idx);
               if (idx>=LOCVAR[LOCVARSIZE-1-ival].size) CODEMSG("invalid index");
               else ((float *)LOCVAR[LOCVARSIZE-1-ival].array)[idx]=VALSTACK[VALSTACKSIZE-1].val;
               }
            VALSTACKSIZE-=2;
            }
         else if (LOCVAR[LOCVARSIZE-1-ival].item==ITEM_ARRAY_BYTE)
            {
            if (VALSTACK[VALSTACKSIZE-2].val<0.0) CODEMSG("invalid index");
            else
               {
               idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE-2].val+0.5);
               checkarrayloc(LOCVARSIZE-1-ival,idx);
               if (idx>=LOCVAR[LOCVARSIZE-1-ival].size) CODEMSG("invalid index");
               else ((unsigned char *)LOCVAR[LOCVARSIZE-1-ival].array)[idx]=(int)floor(VALSTACK[VALSTACKSIZE-1].val+0.5);
               }
            VALSTACKSIZE-=2;
            }
         else CODEMSG("invalid operation");
         break;
      case CODE_INC_ARRAY_IDX:
         if (VALSTACKSIZE<1) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else if (GLBVAR[ival].item==ITEM_ARRAY_FLOAT)
            if (VALSTACK[--VALSTACKSIZE].val<0.0) CODEMSG("invalid index");
            else
               {
               idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE].val+0.5);
               checkarray(ival,idx);
               if (idx>=GLBVAR[ival].size) CODEMSG("invalid index");
               else ((float *)GLBVAR[ival].array)[idx]++;
               }
         else if (GLBVAR[ival].item==ITEM_ARRAY_BYTE)
            if (VALSTACK[--VALSTACKSIZE].val<0.0) CODEMSG("invalid index");
            else
               {
               idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE].val+0.5);
               checkarray(ival,idx);
               if (idx>=GLBVAR[ival].size) CODEMSG("invalid index");
               else ((unsigned char *)GLBVAR[ival].array)[idx]++;
               }
         else CODEMSG("invalid operation");
         break;
      case CODE_INC_ARRAY_LOC_IDX:
         if (VALSTACKSIZE<1) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else if (LOCVAR[LOCVARSIZE-1-ival].item==ITEM_ARRAY_FLOAT)
            if (VALSTACK[--VALSTACKSIZE].val<0.0) CODEMSG("invalid index");
            else
               {
               idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE].val+0.5);
               checkarrayloc(LOCVARSIZE-1-ival,idx);
               if (idx>=LOCVAR[LOCVARSIZE-1-ival].size) CODEMSG("invalid index");
               else ((float *)LOCVAR[LOCVARSIZE-1-ival].array)[idx]++;
               }
         else if (LOCVAR[LOCVARSIZE-1-ival].item==ITEM_ARRAY_BYTE)
            if (VALSTACK[--VALSTACKSIZE].val<0.0) CODEMSG("invalid index");
            else
               {
               idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE].val+0.5);
               checkarrayloc(LOCVARSIZE-1-ival,idx);
               if (idx>=LOCVAR[LOCVARSIZE-1-ival].size) CODEMSG("invalid index");
               else ((unsigned char *)LOCVAR[LOCVARSIZE-1-ival].array)[idx]++;
               }
         else CODEMSG("invalid operation");
         break;
      case CODE_DEC_ARRAY_IDX:
         if (VALSTACKSIZE<1) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else if (GLBVAR[ival].item==ITEM_ARRAY_FLOAT)
            if (VALSTACK[--VALSTACKSIZE].val<0.0) CODEMSG("invalid index");
            else
               {
               idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE].val+0.5);
               checkarray(ival,idx);
               if (idx>=GLBVAR[ival].size) CODEMSG("invalid index");
               else ((float *)GLBVAR[ival].array)[idx]--;
               }
         else if (GLBVAR[ival].item==ITEM_ARRAY_BYTE)
            if (VALSTACK[--VALSTACKSIZE].val<0.0) CODEMSG("invalid index");
            else
               {
               idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE].val+0.5);
               checkarray(ival,idx);
               if (idx>=GLBVAR[ival].size) CODEMSG("invalid index");
               else ((unsigned char *)GLBVAR[ival].array)[idx]--;
               }
         else CODEMSG("invalid operation");
         break;
      case CODE_DEC_ARRAY_LOC_IDX:
         if (VALSTACKSIZE<1) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else if (LOCVAR[LOCVARSIZE-1-ival].item==ITEM_ARRAY_FLOAT)
            if (VALSTACK[--VALSTACKSIZE].val<0.0) CODEMSG("invalid index");
            else
               {
               idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE].val+0.5);
               checkarrayloc(LOCVARSIZE-1-ival,idx);
               if (idx>=LOCVAR[LOCVARSIZE-1-ival].size) CODEMSG("invalid index");
               else ((float *)LOCVAR[LOCVARSIZE-1-ival].array)[idx]--;
               }
         else if (LOCVAR[LOCVARSIZE-1-ival].item==ITEM_ARRAY_BYTE)
            if (VALSTACK[--VALSTACKSIZE].val<0.0) CODEMSG("invalid index");
            else
               {
               idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE].val+0.5);
               checkarrayloc(LOCVARSIZE-1-ival,idx);
               if (idx>=LOCVAR[LOCVARSIZE-1-ival].size) CODEMSG("invalid index");
               else ((unsigned char *)LOCVAR[LOCVARSIZE-1-ival].array)[idx]--;
               }
         else CODEMSG("invalid operation");
         break;
      case CODE_PUSH_REF:
         if (GLBVAR[ival].item==ITEM_REF_FLOAT)
            {
            VALSTACKSIZE++;
            allocate_stacks();
            VALSTACK[VALSTACKSIZE-1].item=ITEM_REF_FLOAT;
            VALSTACK[VALSTACKSIZE-1].val=0.0f;
            VALSTACK[VALSTACKSIZE-1].ref=GLBVAR[ival].ref;
            VALSTACK[VALSTACKSIZE-1].refloc=GLBVAR[ival].refloc;
            }
         else if (GLBVAR[ival].item==ITEM_REF_BYTE)
            {
            VALSTACKSIZE++;
            allocate_stacks();
            VALSTACK[VALSTACKSIZE-1].item=ITEM_REF_BYTE;
            VALSTACK[VALSTACKSIZE-1].val=0.0f;
            VALSTACK[VALSTACKSIZE-1].ref=GLBVAR[ival].ref;
            VALSTACK[VALSTACKSIZE-1].refloc=GLBVAR[ival].refloc;
            }
         else CODEMSG("invalid operation");
         break;
      case CODE_PUSH_REF_LOC:
         if (LOCVAR[LOCVARSIZE-1-ival].item==ITEM_REF_FLOAT)
            {
            VALSTACKSIZE++;
            allocate_stacks();
            VALSTACK[VALSTACKSIZE-1].item=ITEM_REF_FLOAT;
            VALSTACK[VALSTACKSIZE-1].val=0.0f;
            VALSTACK[VALSTACKSIZE-1].timeloc=LOCVAR[LOCVARSIZE-1-ival].timeloc;
            VALSTACK[VALSTACKSIZE-1].ref=LOCVAR[LOCVARSIZE-1-ival].ref;
            VALSTACK[VALSTACKSIZE-1].refloc=LOCVAR[LOCVARSIZE-1-ival].refloc;
            }
         else if (LOCVAR[LOCVARSIZE-1-ival].item==ITEM_REF_BYTE)
            {
            VALSTACKSIZE++;
            allocate_stacks();
            VALSTACK[VALSTACKSIZE-1].item=ITEM_REF_BYTE;
            VALSTACK[VALSTACKSIZE-1].val=0.0f;
            VALSTACK[VALSTACKSIZE-1].timeloc=LOCVAR[LOCVARSIZE-1-ival].timeloc;
            VALSTACK[VALSTACKSIZE-1].ref=LOCVAR[LOCVARSIZE-1-ival].ref;
            VALSTACK[VALSTACKSIZE-1].refloc=LOCVAR[LOCVARSIZE-1-ival].refloc;
            }
         break;
      case CODE_PUSH_REF_IDX:
         if (VALSTACKSIZE<1) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else if (GLBVAR[ival].item==ITEM_REF_FLOAT)
            if (GLBVAR[ival].ref>=0)
               {
               ref=GLBVAR[ival].ref;
               if (GLBVAR[ref].item!=ITEM_ARRAY_FLOAT) CODEMSG("invalid operation");
               else
                  if (VALSTACK[VALSTACKSIZE-1].val<0.0) CODEMSG("invalid index");
                  else
                     {
                     idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE-1].val+0.5);
                     checkarray(ref,idx);
                     if (idx>=GLBVAR[ref].size) CODEMSG("invalid index");
                     else VALSTACK[VALSTACKSIZE-1].val=((float *)GLBVAR[ref].array)[idx];
                     }
               }
            else if (GLBVAR[ival].refloc>=0)
               {
               refloc=GLBVAR[ival].refloc;
               if (LOCVAR[refloc].item!=ITEM_ARRAY_FLOAT) CODEMSG("invalid operation");
               else
                  if (VALSTACK[VALSTACKSIZE-1].val<0.0) CODEMSG("invalid index");
                  else
                     {
                     idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE-1].val+0.5);
                     checkarrayloc(refloc,idx);
                     if (idx>=LOCVAR[refloc].size) CODEMSG("invalid index");
                     else VALSTACK[VALSTACKSIZE-1].val=((float *)LOCVAR[refloc].array)[idx];
                     }
               }
            else CODEMSG("invalid reference");
         else if (GLBVAR[ival].item==ITEM_REF_BYTE)
            if (GLBVAR[ival].ref>=0)
               {
               ref=GLBVAR[ival].ref;
               if (GLBVAR[ref].item!=ITEM_ARRAY_BYTE) CODEMSG("invalid operation");
               else
                  if (VALSTACK[VALSTACKSIZE-1].val<0.0) CODEMSG("invalid index");
                  else
                     {
                     idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE-1].val+0.5);
                     checkarray(ref,idx);
                     if (idx>=GLBVAR[ref].size) CODEMSG("invalid index");
                     else VALSTACK[VALSTACKSIZE-1].val=((unsigned char *)GLBVAR[ref].array)[idx];
                     }
               }
            else if (GLBVAR[ival].refloc>=0)
               {
               refloc=GLBVAR[ival].refloc;
               if (LOCVAR[refloc].item!=ITEM_ARRAY_BYTE) CODEMSG("invalid operation");
               else
                  if (VALSTACK[VALSTACKSIZE-1].val<0.0) CODEMSG("invalid index");
                  else
                     {
                     idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE-1].val+0.5);
                     checkarrayloc(refloc,idx);
                     if (idx>=LOCVAR[refloc].size) CODEMSG("invalid index");
                     else VALSTACK[VALSTACKSIZE-1].val=((unsigned char *)LOCVAR[refloc].array)[idx];
                     }
               }
            else CODEMSG("invalid reference");
         else CODEMSG("invalid operation");
         break;
      case CODE_PUSH_REF_LOC_IDX:
         if (VALSTACKSIZE<1) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else if (LOCVAR[LOCVARSIZE-1-ival].item==ITEM_REF_FLOAT)
            if (LOCVAR[LOCVARSIZE-1-ival].ref>=0)
               {
               ref=LOCVAR[LOCVARSIZE-1-ival].ref;
               if (GLBVAR[ref].item!=ITEM_ARRAY_FLOAT) CODEMSG("invalid operation");
               else
                  if (VALSTACK[VALSTACKSIZE-1].val<0.0) CODEMSG("invalid index");
                  else
                     {
                     idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE-1].val+0.5);
                     checkarray(ref,idx);
                     if (idx>=GLBVAR[ref].size) CODEMSG("invalid index");
                     else VALSTACK[VALSTACKSIZE-1].val=((float *)GLBVAR[ref].array)[idx];
                     }
               }
            else if (LOCVAR[LOCVARSIZE-1-ival].refloc>=0)
               {
               refloc=LOCVAR[LOCVARSIZE-1-ival].refloc;
               if (LOCVAR[refloc].item!=ITEM_ARRAY_FLOAT) CODEMSG("invalid operation");
               else
                  if (VALSTACK[VALSTACKSIZE-1].val<0.0) CODEMSG("invalid index");
                  else
                     {
                     idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE-1].val+0.5);
                     checkarrayloc(refloc,idx);
                     if (idx>=LOCVAR[refloc].size) CODEMSG("invalid index");
                     else VALSTACK[VALSTACKSIZE-1].val=((float *)LOCVAR[refloc].array)[idx];
                     }
               }
            else CODEMSG("invalid reference");
         else if (LOCVAR[LOCVARSIZE-1-ival].item==ITEM_REF_BYTE)
            if (LOCVAR[LOCVARSIZE-1-ival].ref>=0)
               {
               ref=LOCVAR[LOCVARSIZE-1-ival].ref;
               if (GLBVAR[ref].item!=ITEM_ARRAY_BYTE) CODEMSG("invalid operation");
               else
                  if (VALSTACK[VALSTACKSIZE-1].val<0.0) CODEMSG("invalid index");
                  else
                     {
                     idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE-1].val+0.5);
                     checkarray(ref,idx);
                     if (idx>=GLBVAR[ref].size) CODEMSG("invalid index");
                     else VALSTACK[VALSTACKSIZE-1].val=((unsigned char *)GLBVAR[ref].array)[idx];
                     }
               }
            else if (LOCVAR[LOCVARSIZE-1-ival].refloc>=0)
               {
               refloc=LOCVAR[LOCVARSIZE-1-ival].refloc;
               if (LOCVAR[refloc].item!=ITEM_ARRAY_BYTE) CODEMSG("invalid operation");
               else
                  if (VALSTACK[VALSTACKSIZE-1].val<0.0) CODEMSG("invalid index");
                  else
                     {
                     idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE-1].val+0.5);
                     checkarrayloc(refloc,idx);
                     if (idx>=LOCVAR[refloc].size) CODEMSG("invalid index");
                     else VALSTACK[VALSTACKSIZE-1].val=((unsigned char *)LOCVAR[refloc].array)[idx];
                     }
               }
            else CODEMSG("invalid reference");
         else CODEMSG("invalid operation");
         break;
      case CODE_POP_REF:
         if (VALSTACKSIZE<1) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_REF_FLOAT && VALSTACK[VALSTACKSIZE-1].item!=ITEM_REF_BYTE) CODEMSG("invalid operation");
         else if (VALSTACK[VALSTACKSIZE-1].item!=GLBVAR[ival].item) CODEMSG("invalid operation");
         else if (VALSTACK[VALSTACKSIZE-1].ref>=0)
            {
            GLBVAR[ival].ref=VALSTACK[VALSTACKSIZE-1].ref;
            GLBVAR[ival].refloc=-1;
            VALSTACKSIZE--;
            }
         else if (VALSTACK[VALSTACKSIZE-1].refloc>=0) CODEMSG("invalid assignment");
         else CODEMSG("invalid reference");
         break;
      case CODE_POP_REF_LOC:
         if (VALSTACKSIZE<1) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_REF_FLOAT && VALSTACK[VALSTACKSIZE-1].item!=ITEM_REF_BYTE) CODEMSG("invalid operation");
         else if (VALSTACK[VALSTACKSIZE-1].item!=LOCVAR[LOCVARSIZE-1-ival].item) CODEMSG("invalid operation");
         else if (VALSTACK[VALSTACKSIZE-1].ref>=0)
            {
            LOCVAR[LOCVARSIZE-1-ival].ref=VALSTACK[VALSTACKSIZE-1].ref;
            LOCVAR[LOCVARSIZE-1-ival].refloc=-1;
            VALSTACKSIZE--;
            }
         else if (VALSTACK[VALSTACKSIZE-1].refloc>=0)
            if (VALSTACK[VALSTACKSIZE-1].refloc>LOCVARSIZE-1) CODEMSG("invalid assignment");
            else if (VALSTACK[VALSTACKSIZE-1].item==ITEM_REF_FLOAT && LOCVAR[VALSTACK[VALSTACKSIZE-1].refloc].item!=ITEM_ARRAY_FLOAT) CODEMSG("invalid assignment");
            else if (VALSTACK[VALSTACKSIZE-1].item==ITEM_REF_BYTE && LOCVAR[VALSTACK[VALSTACKSIZE-1].refloc].item!=ITEM_ARRAY_BYTE) CODEMSG("invalid assignment");
            else if (VALSTACK[VALSTACKSIZE-1].timeloc!=LOCVAR[VALSTACK[VALSTACKSIZE-1].refloc].timeloc) CODEMSG("invalid assignment");
            else
               {
               LOCVAR[LOCVARSIZE-1-ival].timeloc=VALSTACK[VALSTACKSIZE-1].timeloc;
               LOCVAR[LOCVARSIZE-1-ival].refloc=VALSTACK[VALSTACKSIZE-1].refloc;
               LOCVAR[LOCVARSIZE-1-ival].ref=-1;
               VALSTACKSIZE--;
               }
         else CODEMSG("invalid reference");
         break;
      case CODE_POP_REF_IDX:
         if (VALSTACKSIZE<2) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT || VALSTACK[VALSTACKSIZE-2].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else if (GLBVAR[ival].item==ITEM_REF_FLOAT)
            if (GLBVAR[ival].ref>=0)
               {
               ref=GLBVAR[ival].ref;
               if (GLBVAR[ref].item!=ITEM_ARRAY_FLOAT) CODEMSG("invalid operation");
               else
                  {
                  if (VALSTACK[VALSTACKSIZE-2].val<0.0) CODEMSG("invalid index");
                  else
                     {
                     idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE-2].val+0.5);
                     checkarray(ref,idx);
                     if (idx>=GLBVAR[ref].size) CODEMSG("invalid index");
                     else ((float *)GLBVAR[ref].array)[idx]=VALSTACK[VALSTACKSIZE-1].val;
                     }
                  VALSTACKSIZE-=2;
                  }
               }
            else if (GLBVAR[ival].refloc>=0)
               {
               refloc=GLBVAR[ival].refloc;
               if (LOCVAR[refloc].item!=ITEM_ARRAY_FLOAT) CODEMSG("invalid operation");
               else
                  {
                  if (VALSTACK[VALSTACKSIZE-2].val<0.0) CODEMSG("invalid index");
                  else
                     {
                     idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE-2].val+0.5);
                     checkarrayloc(refloc,idx);
                     if (idx>=LOCVAR[refloc].size) CODEMSG("invalid index");
                     else ((float *)LOCVAR[refloc].array)[idx]=VALSTACK[VALSTACKSIZE-1].val;
                     }
                  VALSTACKSIZE-=2;
                  }
               }
            else CODEMSG("invalid reference");
         else if (GLBVAR[ival].item==ITEM_REF_BYTE)
            if (GLBVAR[ival].ref>=0)
               {
               ref=GLBVAR[ival].ref;
               if (GLBVAR[ref].item!=ITEM_ARRAY_BYTE) CODEMSG("invalid operation");
               else
                  {
                  if (VALSTACK[VALSTACKSIZE-2].val<0.0) CODEMSG("invalid index");
                  else
                     {
                     idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE-2].val+0.5);
                     checkarray(ref,idx);
                     if (idx>=GLBVAR[ref].size) CODEMSG("invalid index");
                     else ((unsigned char *)GLBVAR[ref].array)[idx]=(int)floor(VALSTACK[VALSTACKSIZE-1].val+0.5);
                     }
                  VALSTACKSIZE-=2;
                  }
               }
            else if (GLBVAR[ival].refloc>=0)
               {
               refloc=GLBVAR[ival].refloc;
               if (LOCVAR[refloc].item!=ITEM_ARRAY_BYTE) CODEMSG("invalid operation");
               else
                  {
                  if (VALSTACK[VALSTACKSIZE-2].val<0.0) CODEMSG("invalid index");
                  else
                     {
                     idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE-2].val+0.5);
                     checkarrayloc(refloc,idx);
                     if (idx>=LOCVAR[refloc].size) CODEMSG("invalid index");
                     else ((unsigned char *)LOCVAR[refloc].array)[idx]=(int)floor(VALSTACK[VALSTACKSIZE-1].val+0.5);
                     }
                  VALSTACKSIZE-=2;
                  }
               }
            else CODEMSG("invalid reference");
         else CODEMSG("invalid operation");
         break;
      case CODE_POP_REF_LOC_IDX:
         if (VALSTACKSIZE<2) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT || VALSTACK[VALSTACKSIZE-2].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else if (LOCVAR[LOCVARSIZE-1-ival].item==ITEM_REF_FLOAT)
            if (LOCVAR[LOCVARSIZE-1-ival].ref>=0)
               {
               ref=LOCVAR[LOCVARSIZE-1-ival].ref;
               if (GLBVAR[ref].item!=ITEM_ARRAY_FLOAT) CODEMSG("invalid operation");
               else
                  {
                  if (VALSTACK[VALSTACKSIZE-2].val<0.0) CODEMSG("invalid index");
                  else
                     {
                     idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE-2].val+0.5);
                     checkarray(ref,idx);
                     if (idx>=GLBVAR[ref].size) CODEMSG("invalid index");
                     else ((float *)GLBVAR[ref].array)[idx]=VALSTACK[VALSTACKSIZE-1].val;
                     }
                  VALSTACKSIZE-=2;
                  }
               }
            else if (LOCVAR[LOCVARSIZE-1-ival].refloc>=0)
               {
               refloc=LOCVAR[LOCVARSIZE-1-ival].refloc;
               if (LOCVAR[refloc].item!=ITEM_ARRAY_FLOAT) CODEMSG("invalid operation");
               else
                  {
                  if (VALSTACK[VALSTACKSIZE-2].val<0.0) CODEMSG("invalid index");
                  else
                     {
                     idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE-2].val+0.5);
                     checkarrayloc(refloc,idx);
                     if (idx>=LOCVAR[refloc].size) CODEMSG("invalid index");
                     else ((float *)LOCVAR[refloc].array)[idx]=VALSTACK[VALSTACKSIZE-1].val;
                     }
                  VALSTACKSIZE-=2;
                  }
               }
            else CODEMSG("invalid reference");
         else if (LOCVAR[LOCVARSIZE-1-ival].item==ITEM_REF_BYTE)
            if (LOCVAR[LOCVARSIZE-1-ival].ref>=0)
               {
               ref=LOCVAR[LOCVARSIZE-1-ival].ref;
               if (GLBVAR[ref].item!=ITEM_ARRAY_BYTE) CODEMSG("invalid operation");
               else
                  {
                  if (VALSTACK[VALSTACKSIZE-2].val<0.0) CODEMSG("invalid index");
                  else
                     {
                     idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE-2].val+0.5);
                     checkarray(ref,idx);
                     if (idx>=GLBVAR[ref].size) CODEMSG("invalid index");
                     else ((unsigned char *)GLBVAR[ref].array)[idx]=(int)floor(VALSTACK[VALSTACKSIZE-1].val+0.5);
                     }
                  VALSTACKSIZE-=2;
                  }
               }
            else if (LOCVAR[LOCVARSIZE-1-ival].refloc>=0)
               {
               refloc=LOCVAR[LOCVARSIZE-1-ival].refloc;
               if (LOCVAR[refloc].item!=ITEM_ARRAY_BYTE) CODEMSG("invalid operation");
               else
                  {
                  if (VALSTACK[VALSTACKSIZE-2].val<0.0) CODEMSG("invalid index");
                  else
                     {
                     idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE-2].val+0.5);
                     checkarrayloc(refloc,idx);
                     if (idx>=LOCVAR[refloc].size) CODEMSG("invalid index");
                     else ((unsigned char *)LOCVAR[refloc].array)[idx]=(int)floor(VALSTACK[VALSTACKSIZE-1].val+0.5);
                     }
                  VALSTACKSIZE-=2;
                  }
               }
            else CODEMSG("invalid reference");
         else CODEMSG("invalid operation");
         break;
      case CODE_INC_REF_IDX:
         if (VALSTACKSIZE<1) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else if (GLBVAR[ival].item==ITEM_REF_FLOAT)
            if (GLBVAR[ival].ref>=0)
               {
               ref=GLBVAR[ival].ref;
               if (GLBVAR[ref].item!=ITEM_ARRAY_FLOAT) CODEMSG("invalid operation");
               else
                  if (VALSTACK[--VALSTACKSIZE].val<0.0) CODEMSG("invalid index");
                  else
                     {
                     idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE].val+0.5);
                     checkarray(ref,idx);
                     if (idx>=GLBVAR[ref].size) CODEMSG("invalid index");
                     else ((float *)GLBVAR[ref].array)[idx]++;
                     }
               }
            else if (GLBVAR[ival].refloc>=0)
               {
               refloc=GLBVAR[ival].refloc;
               if (LOCVAR[refloc].item!=ITEM_ARRAY_FLOAT) CODEMSG("invalid operation");
               else
                  if (VALSTACK[--VALSTACKSIZE].val<0.0) CODEMSG("invalid index");
                  else
                     {
                     idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE].val+0.5);
                     checkarrayloc(refloc,idx);
                     if (idx>=LOCVAR[refloc].size) CODEMSG("invalid index");
                     else ((float *)LOCVAR[refloc].array)[idx]++;
                     }
               }
            else CODEMSG("invalid reference");
         else if (GLBVAR[ival].item==ITEM_REF_BYTE)
            if (GLBVAR[ival].ref>=0)
               {
               ref=GLBVAR[ival].ref;
               if (GLBVAR[ref].item!=ITEM_ARRAY_BYTE) CODEMSG("invalid operation");
               else
                  if (VALSTACK[--VALSTACKSIZE].val<0.0) CODEMSG("invalid index");
                  else
                     {
                     idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE].val+0.5);
                     checkarray(ref,idx);
                     if (idx>=GLBVAR[ref].size) CODEMSG("invalid index");
                     else ((unsigned char *)GLBVAR[ref].array)[idx]++;
                     }
               }
            else if (GLBVAR[ival].refloc>=0)
               {
               refloc=GLBVAR[ival].refloc;
               if (LOCVAR[refloc].item!=ITEM_ARRAY_BYTE) CODEMSG("invalid operation");
               else
                  if (VALSTACK[--VALSTACKSIZE].val<0.0) CODEMSG("invalid index");
                  else
                     {
                     idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE].val+0.5);
                     checkarrayloc(refloc,idx);
                     if (idx>=LOCVAR[refloc].size) CODEMSG("invalid index");
                     else ((unsigned char *)LOCVAR[refloc].array)[idx]++;
                     }
               }
            else CODEMSG("invalid reference");
         else CODEMSG("invalid operation");
         break;
      case CODE_INC_REF_LOC_IDX:
         if (VALSTACKSIZE<1) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else if (LOCVAR[LOCVARSIZE-1-ival].item==ITEM_REF_FLOAT)
            if (LOCVAR[LOCVARSIZE-1-ival].ref>=0)
               {
               ref=LOCVAR[LOCVARSIZE-1-ival].ref;
               if (GLBVAR[ref].item!=ITEM_ARRAY_FLOAT) CODEMSG("invalid operation");
               else
                  if (VALSTACK[--VALSTACKSIZE].val<0.0) CODEMSG("invalid index");
                  else
                     {
                     idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE].val+0.5);
                     checkarray(ref,idx);
                     if (idx>=GLBVAR[ref].size) CODEMSG("invalid index");
                     else ((float *)GLBVAR[ref].array)[idx]++;
                     }
               }
            else if (LOCVAR[LOCVARSIZE-1-ival].refloc>=0)
               {
               refloc=LOCVAR[LOCVARSIZE-1-ival].refloc;
               if (LOCVAR[refloc].item!=ITEM_ARRAY_FLOAT) CODEMSG("invalid operation");
               else
                  if (VALSTACK[--VALSTACKSIZE].val<0.0) CODEMSG("invalid index");
                  else
                     {
                     idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE].val+0.5);
                     checkarrayloc(refloc,idx);
                     if (idx>=LOCVAR[refloc].size) CODEMSG("invalid index");
                     else ((float *)LOCVAR[refloc].array)[idx]++;
                     }
               }
            else CODEMSG("invalid reference");
         else if (LOCVAR[LOCVARSIZE-1-ival].item==ITEM_REF_BYTE)
            if (LOCVAR[LOCVARSIZE-1-ival].ref>=0)
               {
               ref=LOCVAR[LOCVARSIZE-1-ival].ref;
               if (GLBVAR[ref].item!=ITEM_ARRAY_BYTE) CODEMSG("invalid operation");
               else
                  if (VALSTACK[--VALSTACKSIZE].val<0.0) CODEMSG("invalid index");
                  else
                     {
                     idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE].val+0.5);
                     checkarray(ref,idx);
                     if (idx>=GLBVAR[ref].size) CODEMSG("invalid index");
                     else ((unsigned char *)GLBVAR[ref].array)[idx]++;
                     }
               }
            else if (LOCVAR[LOCVARSIZE-1-ival].refloc>=0)
               {
               refloc=LOCVAR[LOCVARSIZE-1-ival].refloc;
               if (LOCVAR[refloc].item!=ITEM_ARRAY_BYTE) CODEMSG("invalid operation");
               else
                  if (VALSTACK[--VALSTACKSIZE].val<0.0) CODEMSG("invalid index");
                  else
                     {
                     idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE].val+0.5);
                     checkarrayloc(refloc,idx);
                     if (idx>=LOCVAR[refloc].size) CODEMSG("invalid index");
                     else ((unsigned char *)LOCVAR[refloc].array)[idx]++;
                     }
               }
            else CODEMSG("invalid reference");
         else CODEMSG("invalid operation");
         break;
      case CODE_DEC_REF_IDX:
         if (VALSTACKSIZE<1) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else if (GLBVAR[ival].item==ITEM_REF_FLOAT)
            if (GLBVAR[ival].ref>=0)
               {
               ref=GLBVAR[ival].ref;
               if (GLBVAR[ref].item!=ITEM_ARRAY_FLOAT) CODEMSG("invalid operation");
               else
                  if (VALSTACK[--VALSTACKSIZE].val<0.0) CODEMSG("invalid index");
                  else
                     {
                     idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE].val+0.5);
                     checkarray(ref,idx);
                     if (idx>=GLBVAR[ref].size) CODEMSG("invalid index");
                     else ((float *)GLBVAR[ref].array)[idx]--;
                     }
               }
            else if (GLBVAR[ival].refloc>=0)
               {
               refloc=GLBVAR[ival].refloc;
               if (LOCVAR[refloc].item!=ITEM_ARRAY_FLOAT) CODEMSG("invalid operation");
               else
                  if (VALSTACK[--VALSTACKSIZE].val<0.0) CODEMSG("invalid index");
                  else
                     {
                     idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE].val+0.5);
                     checkarrayloc(refloc,idx);
                     if (idx>=LOCVAR[refloc].size) CODEMSG("invalid index");
                     else ((float *)LOCVAR[refloc].array)[idx]--;
                     }
               }
            else CODEMSG("invalid reference");
         else if (GLBVAR[ival].item==ITEM_REF_BYTE)
            if (GLBVAR[ival].ref>=0)
               {
               ref=GLBVAR[ival].ref;
               if (GLBVAR[ref].item!=ITEM_ARRAY_BYTE) CODEMSG("invalid operation");
               else
                  if (VALSTACK[--VALSTACKSIZE].val<0.0) CODEMSG("invalid index");
                  else
                     {
                     idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE].val+0.5);
                     checkarray(ref,idx);
                     if (idx>=GLBVAR[ref].size) CODEMSG("invalid index");
                     else ((unsigned char *)GLBVAR[ref].array)[idx]--;
                     }
               }
            else if (GLBVAR[ival].refloc>=0)
               {
               refloc=GLBVAR[ival].refloc;
               if (LOCVAR[refloc].item!=ITEM_ARRAY_BYTE) CODEMSG("invalid operation");
               else
                  if (VALSTACK[--VALSTACKSIZE].val<0.0) CODEMSG("invalid index");
                  else
                     {
                     idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE].val+0.5);
                     checkarrayloc(refloc,idx);
                     if (idx>=LOCVAR[refloc].size) CODEMSG("invalid index");
                     else ((unsigned char *)LOCVAR[refloc].array)[idx]--;
                     }
               }
            else CODEMSG("invalid reference");
         else CODEMSG("invalid operation");
         break;
      case CODE_DEC_REF_LOC_IDX:
         if (VALSTACKSIZE<1) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else if (LOCVAR[LOCVARSIZE-1-ival].item==ITEM_REF_FLOAT)
            if (LOCVAR[LOCVARSIZE-1-ival].ref>=0)
               {
               ref=LOCVAR[LOCVARSIZE-1-ival].ref;
               if (GLBVAR[ref].item!=ITEM_ARRAY_FLOAT) CODEMSG("invalid operation");
               else
                  if (VALSTACK[--VALSTACKSIZE].val<0.0) CODEMSG("invalid index");
                  else
                     {
                     idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE].val+0.5);
                     checkarray(ref,idx);
                     if (idx>=GLBVAR[ref].size) CODEMSG("invalid index");
                     else ((float *)GLBVAR[ref].array)[idx]--;
                     }
               }
            else if (LOCVAR[LOCVARSIZE-1-ival].refloc>=0)
               {
               refloc=LOCVAR[LOCVARSIZE-1-ival].refloc;
               if (LOCVAR[refloc].item!=ITEM_ARRAY_FLOAT) CODEMSG("invalid operation");
               else
                  if (VALSTACK[--VALSTACKSIZE].val<0.0) CODEMSG("invalid index");
                  else
                     {
                     idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE].val+0.5);
                     checkarrayloc(refloc,idx);
                     if (idx>=LOCVAR[refloc].size) CODEMSG("invalid index");
                     else ((float *)LOCVAR[refloc].array)[idx]--;
                     }
               }
            else CODEMSG("invalid reference");
         else if (LOCVAR[LOCVARSIZE-1-ival].item==ITEM_REF_BYTE)
            if (LOCVAR[LOCVARSIZE-1-ival].ref>=0)
               {
               ref=LOCVAR[LOCVARSIZE-1-ival].ref;
               if (GLBVAR[ref].item!=ITEM_ARRAY_BYTE) CODEMSG("invalid operation");
               else
                  if (VALSTACK[--VALSTACKSIZE].val<0.0) CODEMSG("invalid index");
                  else
                     {
                     idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE].val+0.5);
                     checkarray(ref,idx);
                     if (idx>=GLBVAR[ref].size) CODEMSG("invalid index");
                     else ((unsigned char *)GLBVAR[ref].array)[idx]--;
                     }
               }
            else if (LOCVAR[LOCVARSIZE-1-ival].refloc>=0)
               {
               refloc=LOCVAR[LOCVARSIZE-1-ival].refloc;
               if (LOCVAR[refloc].item!=ITEM_ARRAY_BYTE) CODEMSG("invalid operation");
               else
                  if (VALSTACK[--VALSTACKSIZE].val<0.0) CODEMSG("invalid index");
                  else
                     {
                     idx=(unsigned int)floor(VALSTACK[VALSTACKSIZE].val+0.5);
                     checkarrayloc(refloc,idx);
                     if (idx>=LOCVAR[refloc].size) CODEMSG("invalid index");
                     else ((unsigned char *)LOCVAR[refloc].array)[idx]--;
                     }
               }
            else CODEMSG("invalid reference");
         else CODEMSG("invalid operation");
         break;
      case CODE_JMP:
         RETSTACK[RETSTACKSIZE-1]=ival;
         break;
      case CODE_JIF:
         if (VALSTACKSIZE<1) CODEMSG("value stack underrun");
         else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT) CODEMSG("invalid operation");
         else if (VALSTACK[--VALSTACKSIZE].val==0.0f) RETSTACK[RETSTACKSIZE-1]=ival;
         break;
      case CODE_JSR:
         RETSTACKSIZE++;
         allocate_stacks();
         RETSTACK[RETSTACKSIZE-1]=ival;
         break;
      case CODE_RSR:
         if (RETSTACKSIZE>1) RETSTACKSIZE--;
         else CODEMSG("return stack underrun");
         break;
      case CODE_WARN:
         fprintf(stderr,"warning at line %d!\n",ival);
         break;
      default:
         CODEMSG("unknown instruction");
         break;
      }

   if (LUNADEBUG)
      {
      if (VALSTACKSIZE>0) printf(" post=%f\n",VALSTACK[VALSTACKSIZE-1].val);
      else printf(" post=none\n");
      }
   }

void lunacode::pushvalue(float val)
   {execmd(CODE_PUSH,0,val);}

float lunacode::popvalue()
   {
   float val=0.0f;

   if (VALSTACKSIZE<1) CODEMSG("value stack underrun");
   else if (VALSTACK[VALSTACKSIZE-1].item!=ITEM_FLOAT) CODEMSG("invalid operation");
   else val=VALSTACK[--VALSTACKSIZE].val;

   if (LUNADEBUG)
      {
      printf("cmd=POP fval=%f",val);

      if (VALSTACKSIZE>0) printf(" post=%f\n",VALSTACK[VALSTACKSIZE-1].val);
      else printf(" post=none\n");
      }

   return(val);
   }

void lunacode::print()
   {
   int addr;

   int code;
   int mode;
   int ival;
   float fval;

   addr=0;

   while (addr<CODESIZE)
      {
      printf("%d: ",addr);

      code=CODE_NOP;
      mode=MODE_NONE;
      ival=0;
      fval=0.0f;

      addr=getcode(addr,&code,&mode,&ival,&fval);
      printcode(code);

      if (mode==MODE_NONE) printf("()\n");
      else if (mode==MODE_FLOAT) printf("(%g)\n",fval);
      else printf("(%d)\n",ival);
      }
   }

void lunacode::printcode(int code)
   {
   switch (code)
      {
      case CODE_NOP: printf("NOP"); break;
      case CODE_ADD: printf("ADD"); break;
      case CODE_SUB: printf("SUB"); break;
      case CODE_MUL: printf("MUL"); break;
      case CODE_DIV: printf("DIV"); break;
      case CODE_MOD: printf("MOD"); break;
      case CODE_EQ: printf("EQ"); break;
      case CODE_NEQ: printf("NEQ"); break;
      case CODE_LT: printf("LT"); break;
      case CODE_GT: printf("GT"); break;
      case CODE_LE: printf("LE"); break;
      case CODE_GE: printf("GE"); break;
      case CODE_AND: printf("AND"); break;
      case CODE_OR: printf("OR"); break;
      case CODE_NOT: printf("NOT"); break;
      case CODE_MIN: printf("MIN"); break;
      case CODE_MAX: printf("MAX"); break;
      case CODE_ABS: printf("ABS"); break;
      case CODE_SQR: printf("SQR"); break;
      case CODE_SQRT: printf("SQRT"); break;
      case CODE_EXP: printf("EXP"); break;
      case CODE_LOG: printf("LOG"); break;
      case CODE_POW: printf("POW"); break;
      case CODE_SIN: printf("SIN"); break;
      case CODE_COS: printf("COS"); break;
      case CODE_TAN: printf("TAN"); break;
      case CODE_ATAN: printf("ATAN"); break;
      case CODE_ATAN2: printf("ATAN2"); break;
      case CODE_PUSH: printf("PUSH"); break;
      case CODE_RESERVE_VAR: printf("RESERVE_VAR"); break;
      case CODE_RESERVE_VAR_LOC: printf("RESERVE_VAR_LOC"); break;
      case CODE_REMOVE_VAR_LOC: printf("REMOVE_VAR_LOC"); break;
      case CODE_PUSH_VAR: printf("PUSH_VAR"); break;
      case CODE_PUSH_VAR_LOC: printf("PUSH_VAR_LOC"); break;
      case CODE_POP_VAR: printf("POP_VAR"); break;
      case CODE_POP_VAR_STAT: printf("POP_VAR_STAT"); break;
      case CODE_POP_VAR_LOC: printf("POP_VAR_LOC"); break;
      case CODE_INC_VAR: printf("INC_VAR"); break;
      case CODE_INC_VAR_LOC: printf("INC_VAR_LOC"); break;
      case CODE_DEC_VAR: printf("DEC_VAR"); break;
      case CODE_DEC_VAR_LOC: printf("DEC_VAR_LOC"); break;
      case CODE_ALLOC_ARRAY: printf("ALLOC_ARRAY"); break;
      case CODE_ALLOC_ARRAY_LOC: printf("ALLOC_ARRAY_LOC"); break;
      case CODE_ALLOC_ARRAY_BYT: printf("ALLOC_ARRAY_BYT"); break;
      case CODE_ALLOC_ARRAY_BYT_LOC: printf("ALLOC_ARRAY_BYT_LOC"); break;
      case CODE_INIT_ARRAY: printf("INIT_ARRAY"); break;
      case CODE_INIT_ARRAY_LOC: printf("INIT_ARRAY_LOC"); break;
      case CODE_INIT_ARRAY_BYT: printf("INIT_ARRAY_BYT"); break;
      case CODE_INIT_ARRAY_BYT_LOC: printf("INIT_ARRAY_BYT_LOC"); break;
      case CODE_FREE_ARRAY_LOC: printf("FREE_ARRAY_LOC"); break;
      case CODE_SIZE_ARRAY: printf("SIZE_ARRAY"); break;
      case CODE_SIZE_ARRAY_LOC: printf("SIZE_ARRAY_LOC"); break;
      case CODE_INIT_REF: printf("INIT_REF"); break;
      case CODE_INIT_REF_LOC: printf("INIT_REF_LOC"); break;
      case CODE_INIT_REF_BYT: printf("INIT_REF_BYT"); break;
      case CODE_INIT_REF_BYT_LOC: printf("INIT_REF_BYT_LOC"); break;
      case CODE_SIZE_REF: printf("SIZE_REF"); break;
      case CODE_SIZE_REF_LOC: printf("SIZE_REF_LOC"); break;
      case CODE_PUSH_ARRAY: printf("PUSH_ARRAY"); break;
      case CODE_PUSH_ARRAY_LOC: printf("PUSH_ARRAY_LOC"); break;
      case CODE_PUSH_ARRAY_IDX: printf("PUSH_ARRAY_IDX"); break;
      case CODE_PUSH_ARRAY_LOC_IDX: printf("PUSH_ARRAY_LOC_IDX"); break;
      case CODE_POP_ARRAY_IDX: printf("POP_ARRAY_IDX"); break;
      case CODE_POP_ARRAY_LOC_IDX: printf("POP_ARRAY_LOC_IDX"); break;
      case CODE_INC_ARRAY_IDX: printf("INC_ARRAY_IDX"); break;
      case CODE_INC_ARRAY_LOC_IDX: printf("INC_ARRAY_LOC_IDX"); break;
      case CODE_DEC_ARRAY_IDX: printf("DEC_ARRAY_IDX"); break;
      case CODE_DEC_ARRAY_LOC_IDX: printf("DEC_ARRAY_LOC_IDX"); break;
      case CODE_PUSH_REF: printf("PUSH_REF"); break;
      case CODE_PUSH_REF_LOC: printf("PUSH_REF_LOC"); break;
      case CODE_PUSH_REF_IDX: printf("PUSH_REF_IDX"); break;
      case CODE_PUSH_REF_LOC_IDX: printf("PUSH_REF_LOC_IDX"); break;
      case CODE_POP_REF: printf("POP_REF"); break;
      case CODE_POP_REF_LOC: printf("POP_REF_LOC"); break;
      case CODE_POP_REF_IDX: printf("POP_REF_IDX"); break;
      case CODE_POP_REF_LOC_IDX: printf("POP_REF_LOC_IDX"); break;
      case CODE_INC_REF_IDX: printf("INC_REF_IDX"); break;
      case CODE_INC_REF_LOC_IDX: printf("INC_REF_LOC_IDX"); break;
      case CODE_DEC_REF_IDX: printf("DEC_REF_IDX"); break;
      case CODE_DEC_REF_LOC_IDX: printf("DEC_REF_LOC_IDX"); break;
      case CODE_JMP: printf("JMP"); break;
      case CODE_JIF: printf("JIF"); break;
      case CODE_JSR: printf("JSR"); break;
      case CODE_RSR: printf("RSR"); break;
      case CODE_WARN: printf("WARN"); break;
      }
   }

void lunacode::CODEMSG(const char *msg)
   {fprintf(stderr,"code error: %s\n",msg);}
