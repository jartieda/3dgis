// (c) by Stefan Roettger

#ifndef LUNACODE_H
#define LUNACODE_H

#include "minibase.h"

class lunacode
   {
   public:

   enum
      {
      CODE_NOP=0,
      CODE_ADD=1,
      CODE_SUB=2,
      CODE_MUL=3,
      CODE_DIV=4,
      CODE_MOD=5,
      CODE_EQ=6,
      CODE_NEQ=7,
      CODE_LT=8,
      CODE_GT=9,
      CODE_LE=10,
      CODE_GE=11,
      CODE_AND=12,
      CODE_OR=13,
      CODE_NOT=14,
      CODE_MIN=15,
      CODE_MAX=16,
      CODE_ABS=17,
      CODE_SQR=18,
      CODE_SQRT=19,
      CODE_EXP=20,
      CODE_LOG=21,
      CODE_POW=22,
      CODE_SIN=23,
      CODE_COS=24,
      CODE_TAN=25,
      CODE_ATAN=26,
      CODE_ATAN2=27,
      CODE_PUSH=28,
      CODE_RESERVE_VAR=29,
      CODE_RESERVE_VAR_LOC=30,
      CODE_REMOVE_VAR_LOC=31,
      CODE_PUSH_VAR=32,
      CODE_PUSH_VAR_LOC=33,
      CODE_POP_VAR=34,
      CODE_POP_VAR_STAT=35,
      CODE_POP_VAR_LOC=36,
      CODE_INC_VAR=37,
      CODE_INC_VAR_LOC=38,
      CODE_DEC_VAR=39,
      CODE_DEC_VAR_LOC=40,
      CODE_ALLOC_ARRAY=41,
      CODE_ALLOC_ARRAY_LOC=42,
      CODE_ALLOC_ARRAY_BYT=43,
      CODE_ALLOC_ARRAY_BYT_LOC=44,
      CODE_INIT_ARRAY=45,
      CODE_INIT_ARRAY_LOC=46,
      CODE_INIT_ARRAY_BYT=47,
      CODE_INIT_ARRAY_BYT_LOC=48,
      CODE_FREE_ARRAY_LOC=49,
      CODE_SIZE_ARRAY=50,
      CODE_SIZE_ARRAY_LOC=51,
      CODE_INIT_REF=52,
      CODE_INIT_REF_LOC=53,
      CODE_INIT_REF_BYT=54,
      CODE_INIT_REF_BYT_LOC=55,
      CODE_SIZE_REF=56,
      CODE_SIZE_REF_LOC=57,
      CODE_PUSH_ARRAY=58,
      CODE_PUSH_ARRAY_LOC=59,
      CODE_PUSH_ARRAY_IDX=60,
      CODE_PUSH_ARRAY_LOC_IDX=61,
      CODE_POP_ARRAY_IDX=62,
      CODE_POP_ARRAY_LOC_IDX=63,
      CODE_INC_ARRAY_IDX=64,
      CODE_INC_ARRAY_LOC_IDX=65,
      CODE_DEC_ARRAY_IDX=66,
      CODE_DEC_ARRAY_LOC_IDX=67,
      CODE_PUSH_REF=68,
      CODE_PUSH_REF_LOC=69,
      CODE_PUSH_REF_IDX=70,
      CODE_PUSH_REF_LOC_IDX=71,
      CODE_POP_REF=72,
      CODE_POP_REF_LOC=73,
      CODE_POP_REF_IDX=74,
      CODE_POP_REF_LOC_IDX=75,
      CODE_INC_REF_IDX=76,
      CODE_INC_REF_LOC_IDX=77,
      CODE_DEC_REF_IDX=78,
      CODE_DEC_REF_LOC_IDX=79,
      CODE_JMP=80,
      CODE_JIF=81,
      CODE_JSR=82,
      CODE_RSR=83,
      CODE_WARN=84
      };

   enum
      {
      MODE_NONE=0,
      MODE_BYTE=1,
      MODE_SHORT=2,
      MODE_INT=3,
      MODE_FLOAT=4,
      MODE_ANY=5
      };

   enum
      {
      ITEM_NONE=0,
      ITEM_FLOAT=1,
      ITEM_ARRAY_FLOAT=2,
      ITEM_ARRAY_BYTE=3,
      ITEM_REF_FLOAT=4,
      ITEM_REF_BYTE=5,
      };

   //! default constructor
   lunacode();

   //! destructor
   ~lunacode();

   void addcode(int code=CODE_NOP,int mode=MODE_NONE,int ival=0,float fval=0.0f);
   void addcodeat(int addr,int code=CODE_NOP,int mode=MODE_NONE,int ival=0,float fval=0.0f);

   int getaddr();

   void init(BOOLINT init_static=TRUE);
   void execute();
   void pushvalue(float val);
   float popvalue();

   void print();
   void printcode(int code);

   void setdebug(BOOLINT on) {LUNADEBUG=on;}

   protected:

   unsigned char *CODE;
   int CODESIZE,CODEMAX;

   struct LUNA_ITEM
      {
      int item;
      double val;

      void *array;
      unsigned int maxsize,size;

      unsigned int timeloc;

      int ref,refloc;
      };

   int *RETSTACK;
   int RETSTACKSIZE,RETSTACKMAX;

   LUNA_ITEM *VALSTACK;
   int VALSTACKSIZE,VALSTACKMAX;

   LUNA_ITEM *GLBVAR,*LOCVAR;

   int GLBVARSIZE,GLBVARMAX;
   int LOCVARSIZE,LOCVARMAX;

   void addbyte(int addr,int val);
   int getcode(int addr,int *code,int *mode,int *ival,float *fval);

   void allocate_stacks();
   void allocate_vars();

   void execmd(int code,int ival=0,float fval=0.0f);

   void checkarray(int ref,unsigned int idx);
   void checkarrayloc(int refloc,unsigned int idx);

   void *mallocarray(int item,unsigned int size);
   void *reallocarray(void *array,int item,unsigned int size,unsigned int orig);

   private:

   BOOLINT LUNADEBUG;

   inline double mod(double a,double b);

   void CODEMSG(const char *msg);
   };

#endif
