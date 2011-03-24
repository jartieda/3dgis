// (c) by Stefan Roettger

#ifndef MINIBASE_H
#define MINIBASE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <float.h>
#include <math.h>

namespace minibase {

extern void (*minierrorhandler)(const char *file,int line,int fatal);

#define WARNMSG() minierrormsg(__FILE__,__LINE__)
#define ERRORMSG() minierrormsg(__FILE__,__LINE__,1)

inline void minierrormsg(const char *file,int line,int fatal=0)
   {
   if (minierrorhandler==0)
      {
      if (fatal==0) fprintf(stderr,"warning");
      else fprintf(stderr,"fatal error");
      fprintf(stderr," in <%s> at line %d!\n",file,line);
      }
   else minierrorhandler(file,line,fatal);
   if (fatal!=0) exit(EXIT_FAILURE);
   }

#ifndef NULL
#define NULL (0)
#endif

#define BOOLINT char

#ifndef TRUE
#define TRUE (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

#ifndef PI
#define PI (3.14159265358979323846)
#endif

#ifndef RAD
#define RAD (PI/180.0)
#endif

#ifndef MAXFLOAT
#define MAXFLOAT (FLT_MAX)
#endif

#undef ffloor
#define ffloor(x) floor((double)(x))
#undef fceil
#define fceil(x) ceil((double)(x))
#define ftrc(x) (int)ffloor(x)
#define FTRC(x) (int)floor(x)

inline double fABS(const float x) {return((x<0.0)?-x:x);}
inline double FABS(const double x) {return((x<0.0)?-x:x);}
#define fabs(x) fABS(x)

inline int min(const int a,const int b) {return((a<b)?a:b);}
inline double fMIN(const float a,const float b) {return((a<b)?a:b);}
inline double FMIN(const double a,const double b) {return((a<b)?a:b);}
#define fmin(a,b) fMIN(a,b)

inline int max(const int a,const int b) {return((a>b)?a:b);}
inline double fMAX(const float a,const float b) {return((a>b)?a:b);}
inline double FMAX(const double a,const double b) {return((a>b)?a:b);}
#define fmax(a,b) fMAX(a,b)

inline int sqr(const int x) {return(x*x);}
inline double fsqr(const float x) {return(x*x);}
inline double FSQR(const double x) {return(x*x);}

#undef fsqrt
#define fsqrt(x) sqrt((double)(x))

#undef fsin
#define fsin(x) sin((double)(x))
#undef fcos
#define fcos(x) cos((double)(x))
#undef ftan
#define ftan(x) tan((double)(x))

#undef fasin
#define fasin(x) asin((double)(x))
#undef facos
#define facos(x) acos((double)(x))
#undef fatan
#define fatan(x) atan((double)(x))
#undef fatan2
#define fatan2(y,x) atan2((double)(y),(double)(x))

#undef fexp
#define fexp(x) exp((double)(x))
#undef flog
#define flog(x) log((double)(x))
#undef fpow
#define fpow(x,y) pow((double)(x),(double)(y))

inline float fround2(const float v,const unsigned int n=6) {return(ffloor(v*fpow(10.0f,(int)n)+0.5f)/fpow(10.0f,(int)n));}
inline double round2(const double v,const unsigned int n=6) {return(floor(v*pow(10.0,(int)n)+0.5)/pow(10.0,(int)n));}

#ifdef _MSC_VER
#define strdup _strdup
#define snprintf _snprintf
#endif

inline char *strcct(const char *str1,const char *str2)
   {
   char *str;

   if (str1==NULL && str2==NULL) return(NULL);

   if (str1==NULL) return(strdup(str2));
   if (str2==NULL) return(strdup(str1));

   if ((str=(char *)malloc(strlen(str1)+strlen(str2)+1))==NULL) ERRORMSG();

   memcpy(str,str1,strlen(str1));
   memcpy(str+strlen(str1),str2,strlen(str2)+1);

   return(str);
   }

}

using namespace minibase;

#endif
