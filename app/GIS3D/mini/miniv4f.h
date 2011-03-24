// (c) by Stefan Roettger

#ifndef MINIV4F_H
#define MINIV4F_H

#include <iostream>

#include "minibase.h"

class miniv4d;
class miniv3f;
class miniv3d;

class miniv4f
   {
   public:

   //! default constructor
   miniv4f() {}

   //! copy constructor
   miniv4f(const miniv4f &v) {x=v.x; y=v.y; z=v.z; w=v.w;}

   //! constructors
   miniv4f(const float v) {x=y=z=v; w=0.0f;}
   miniv4f(const float v,const float vw) {x=y=z=v; w=vw;}
   miniv4f(const float vx,const float vy,const float vz) {x=vx; y=vy; z=vz; w=0.0f;}
   miniv4f(const float vx,const float vy,const float vz,const float vw) {x=vx; y=vy; z=vz; w=vw;}

   //! array constructors
   miniv4f(const float ptr[4]) {x=ptr[0]; y=ptr[1]; z=ptr[2]; w=ptr[3];}
   miniv4f(const double ptr[4]) {x=(float)ptr[0]; y=(float)ptr[1]; z=(float)ptr[2]; w=(float)ptr[3];}
   miniv4f(const int ptr[4]) {x=ptr[0]; y=ptr[1]; z=ptr[2]; w=ptr[3];}
   miniv4f(const short int ptr[4]) {x=ptr[0]; y=ptr[1]; z=ptr[2]; w=ptr[3];}
   miniv4f(const unsigned short int ptr[4]) {x=ptr[0]; y=ptr[1]; z=ptr[2]; w=ptr[3];}
   miniv4f(const unsigned char ptr[4]) {x=ptr[0]; y=ptr[1]; z=ptr[2]; w=ptr[3];}

   //! conversion constructors
   miniv4f(const miniv4d &v);
   miniv4f(const miniv3f &v);
   miniv4f(const miniv3f &v,const float vw);
   miniv4f(const miniv3d &v);
   miniv4f(const miniv3d &v,const double vw);

   //! destructor
   ~miniv4f() {}

   void setcoords(const float vx,const float vy,const float vz,const float vw=0.0f) {x=vx; y=vy; z=vz; w=vw;}

   float getlength() const {return(fsqrt(x*x+y*y+z*z+w*w));}
   float getlength2() const {return(x*x+y*y+z*z+w*w);}

   inline miniv4f& operator += (const miniv4f &v);
   inline miniv4f& operator -= (const miniv4f &v);

   inline miniv4f& operator *= (const float c);
   inline miniv4f& operator /= (const float c);

   float normalize()
      {
      float length=getlength();
      if (length>0.0f) *this/=length;
      return(length);
      }

   float x,y,z,w;
   };

inline miniv4f& miniv4f::operator += (const miniv4f &v)
   {
   x+=v.x; y+=v.y; z+=v.z; w+=v.w;
   return(*this);
   }

inline miniv4f& miniv4f::operator -= (const miniv4f &v)
   {
   x-=v.x; y-=v.y; z-=v.z; w-=v.w;
   return(*this);
   }

inline miniv4f& miniv4f::operator *= (const float c)
   {
   x*=c; y*=c; z*=c; w*=c;
   return(*this);
   }

inline miniv4f& miniv4f::operator /= (const float c)
   {
   float rc;
   if (c!=0.0f) rc=1.0f/c;
   else rc=1.0f;
   x*=rc; y*=rc; z*=rc; w*=rc;
   return(*this);
   }

inline miniv4f operator + (const miniv4f &a,const miniv4f &b);
inline miniv4f operator - (const miniv4f &a,const miniv4f &b);
inline miniv4f operator - (const miniv4f &v);

inline miniv4f operator * (const float a,const miniv4f &b);
inline miniv4f operator * (const miniv4f &a,const float b);
inline miniv4f operator / (const miniv4f &a,const float b);

inline float operator * (const miniv4f &a,const miniv4f &b);

inline int operator == (const miniv4f &a,const miniv4f &b);
inline int operator != (const miniv4f &a,const miniv4f &b);

inline int operator < (const miniv4f &a,const miniv4f &b);
inline int operator > (const miniv4f &a,const miniv4f &b);

inline int operator <= (const miniv4f &a,const miniv4f &b);
inline int operator >= (const miniv4f &a,const miniv4f &b);

inline std::ostream& operator << (std::ostream &out,const miniv4f &v);

inline miniv4f operator + (const miniv4f &a,const miniv4f &b)
   {return(miniv4f(a.x+b.x,a.y+b.y,a.z+b.z,a.w+b.w));}

inline miniv4f operator - (const miniv4f &a,const miniv4f &b)
   {return(miniv4f(a.x-b.x,a.y-b.y,a.z-b.z,a.w-b.w));}

inline miniv4f operator - (const miniv4f &v)
   {return(miniv4f(-v.x,-v.y,-v.z,-v.w));}

inline miniv4f operator * (const float a,const miniv4f &b)
   {return(miniv4f(a*b.x,a*b.y,a*b.z,a*b.w));}

inline miniv4f operator * (const miniv4f &a,const float b)
   {return(miniv4f(a.x*b,a.y*b,a.z*b,a.w*b));}

inline miniv4f operator / (const miniv4f &a,const float b)
   {return(miniv4f(a.x/b,a.y/b,a.z/b,a.w/b));}

inline float operator * (const miniv4f &a,const miniv4f &b)
   {return(a.x*b.x+a.y*b.y+a.z*b.z+a.w*b.w);}

inline int operator == (const miniv4f &a,const miniv4f &b)
   {return(a.x==b.x && a.y==b.y && a.z==b.z && a.w==b.w);}

inline int operator != (const miniv4f &a,const miniv4f &b)
   {return(a.x!=b.x || a.y!=b.y || a.z!=b.z || a.w!=b.w);}

inline int operator < (const miniv4f &a,const miniv4f &b)
   {return(a.x*a.x+a.y*a.y+a.z*a.z+a.w*a.w<b.x*b.x+b.y*b.y+b.z*b.z+b.w*b.w);}

inline int operator > (const miniv4f &a,const miniv4f &b)
   {return(a.x*a.x+a.y*a.y+a.z*a.z+a.w*a.w>b.x*b.x+b.y*b.y+b.z*b.z+b.w*b.w);}

inline int operator <= (const miniv4f &a,const miniv4f &b)
   {return(a.x*a.x+a.y*a.y+a.z*a.z+a.w*a.w<=b.x*b.x+b.y*b.y+b.z*b.z+b.w*b.w);}

inline int operator >= (const miniv4f &a,const miniv4f &b)
   {return(a.x*a.x+a.y*a.y+a.z*a.z+a.w*a.w>=b.x*b.x+b.y*b.y+b.z*b.z+b.w*b.w);}

inline std::ostream& operator << (std::ostream &out,const miniv4f &v)
   {return(out << '(' << fround2(v.x) << ',' << fround2(v.y) << ',' << fround2(v.z) << ',' << fround2(v.w) << ')');}

#endif
