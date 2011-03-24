// (c) by Stefan Roettger

#ifndef MINIV3F_H
#define MINIV3F_H

#include <iostream>

#include "minibase.h"

class miniv3d;
class miniv4f;
class miniv4d;

class miniv3f
   {
   public:

   //! default constructor
   miniv3f() {}

   //! copy constructor
   miniv3f(const miniv3f &v) {x=v.x; y=v.y; z=v.z;}

   //! constructors
   miniv3f(const float v) {x=y=z=v;}
   miniv3f(const float vx,const float vy,const float vz) {x=vx; y=vy; z=vz;}

   //! array constructors
   miniv3f(const float ptr[3]) {x=ptr[0]; y=ptr[1]; z=ptr[2];}
   miniv3f(const double ptr[3]) {x=(float)ptr[0]; y=(float)ptr[1]; z=(float)ptr[2];}
   miniv3f(const int ptr[3]) {x=ptr[0]; y=ptr[1]; z=ptr[2];}
   miniv3f(const short int ptr[3]) {x=ptr[0]; y=ptr[1]; z=ptr[2];}
   miniv3f(const unsigned short int ptr[3]) {x=ptr[0]; y=ptr[1]; z=ptr[2];}
   miniv3f(const unsigned char ptr[3]) {x=ptr[0]; y=ptr[1]; z=ptr[2];}

   //! conversion constructors
   miniv3f(const miniv3d &v);
   miniv3f(const miniv4f &v);
   miniv3f(const miniv4d &v);

   //! destructor
   ~miniv3f() {}

   void setcoords(const float vx,const float vy,const float vz) {x=vx; y=vy; z=vz;}

   float getlength() const {return(fsqrt(x*x+y*y+z*z));}
   float getlength2() const {return(x*x+y*y+z*z);}

   inline miniv3f& operator += (const miniv3f &v);
   inline miniv3f& operator -= (const miniv3f &v);

   inline miniv3f& operator *= (const float c);
   inline miniv3f& operator /= (const float c);

   float normalize()
      {
      float length=getlength();
      if (length>0.0f) *this/=length;
      return(length);
      }

   float x,y,z;
   };

inline miniv3f& miniv3f::operator += (const miniv3f &v)
   {
   x+=v.x; y+=v.y; z+=v.z;
   return(*this);
   }

inline miniv3f& miniv3f::operator -= (const miniv3f &v)
   {
   x-=v.x; y-=v.y; z-=v.z;
   return(*this);
   }

inline miniv3f& miniv3f::operator *= (const float c)
   {
   x*=c; y*=c; z*=c;
   return(*this);
   }

inline miniv3f& miniv3f::operator /= (const float c)
   {
   float rc;
   if (c!=0.0f) rc=1.0f/c;
   else rc=1.0f;
   x*=rc; y*=rc; z*=rc;
   return(*this);
   }

inline miniv3f operator + (const miniv3f &a,const miniv3f &b);
inline miniv3f operator - (const miniv3f &a,const miniv3f &b);
inline miniv3f operator - (const miniv3f &v);

inline miniv3f operator * (const float a,const miniv3f &b);
inline miniv3f operator * (const miniv3f &a,const float b);
inline miniv3f operator / (const miniv3f &a,const float b);

inline float operator * (const miniv3f &a,const miniv3f &b);
inline miniv3f operator / (const miniv3f &a,const miniv3f &b);

inline int operator == (const miniv3f &a,const miniv3f &b);
inline int operator != (const miniv3f &a,const miniv3f &b);

inline int operator < (const miniv3f &a,const miniv3f &b);
inline int operator > (const miniv3f &a,const miniv3f &b);

inline int operator <= (const miniv3f &a,const miniv3f &b);
inline int operator >= (const miniv3f &a,const miniv3f &b);

inline std::ostream& operator << (std::ostream &out,const miniv3f &v);

inline miniv3f operator + (const miniv3f &a,const miniv3f &b)
   {return(miniv3f(a.x+b.x,a.y+b.y,a.z+b.z));}

inline miniv3f operator - (const miniv3f &a,const miniv3f &b)
   {return(miniv3f(a.x-b.x,a.y-b.y,a.z-b.z));}

inline miniv3f operator - (const miniv3f &v)
   {return(miniv3f(-v.x,-v.y,-v.z));}

inline miniv3f operator * (const float a,const miniv3f &b)
   {return(miniv3f(a*b.x,a*b.y,a*b.z));}

inline miniv3f operator * (const miniv3f &a,const float b)
   {return(miniv3f(a.x*b,a.y*b,a.z*b));}

inline miniv3f operator / (const miniv3f &a,const float b)
   {return(miniv3f(a.x/b,a.y/b,a.z/b));}

inline float operator * (const miniv3f &a,const miniv3f &b)
   {return(a.x*b.x+a.y*b.y+a.z*b.z);}

// (0,0,-1)/(-1,0,0)=(0,1,0)
inline miniv3f operator / (const miniv3f &a,const miniv3f &b)
   {return(miniv3f(a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x));}

inline int operator == (const miniv3f &a,const miniv3f &b)
   {return(a.x==b.x && a.y==b.y && a.z==b.z);}

inline int operator != (const miniv3f &a,const miniv3f &b)
   {return(a.x!=b.x || a.y!=b.y || a.z!=b.z);}

inline int operator < (const miniv3f &a,const miniv3f &b)
   {return(a.x*a.x+a.y*a.y+a.z*a.z<b.x*b.x+b.y*b.y+b.z*b.z);}

inline int operator > (const miniv3f &a,const miniv3f &b)
   {return(a.x*a.x+a.y*a.y+a.z*a.z>b.x*b.x+b.y*b.y+b.z*b.z);}

inline int operator <= (const miniv3f &a,const miniv3f &b)
   {return(a.x*a.x+a.y*a.y+a.z*a.z<=b.x*b.x+b.y*b.y+b.z*b.z);}

inline int operator >= (const miniv3f &a,const miniv3f &b)
   {return(a.x*a.x+a.y*a.y+a.z*a.z>=b.x*b.x+b.y*b.y+b.z*b.z);}

inline std::ostream& operator << (std::ostream &out,const miniv3f &v)
   {return(out << '(' << fround2(v.x) << ',' << fround2(v.y) << ',' << fround2(v.z) << ')');}

#endif
