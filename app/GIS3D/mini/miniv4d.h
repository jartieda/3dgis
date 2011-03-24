// (c) by Stefan Roettger

#ifndef MINIV4D_H
#define MINIV4D_H

#include <iostream>

#include "minibase.h"

class miniv4f;
class miniv3d;
class miniv3f;

class miniv4d
   {
   public:

   //! default constructor
   miniv4d() {}

   //! copy constructor
   miniv4d(const miniv4d &v) {x=v.x; y=v.y; z=v.z; w=v.w;}

   //! constructors
   miniv4d(const double v) {x=y=z=v; w=0.0;}
   miniv4d(const double v,const double vw) {x=y=z=v; w=vw;}
   miniv4d(const double vx,const double vy,const double vz) {x=vx; y=vy; z=vz; w=0.0;}
   miniv4d(const double vx,const double vy,const double vz,const double vw) {x=vx; y=vy; z=vz; w=vw;}

   //! array constructors
   miniv4d(const double ptr[4]) {x=ptr[0]; y=ptr[1]; z=ptr[2]; w=ptr[3];}
   miniv4d(const float ptr[4]) {x=(double)ptr[0]; y=(double)ptr[1]; z=(double)ptr[2]; w=(double)ptr[3];}
   miniv4d(const int ptr[4]) {x=ptr[0]; y=ptr[1]; z=ptr[2]; w=ptr[3];}
   miniv4d(const short int ptr[4]) {x=ptr[0]; y=ptr[1]; z=ptr[2]; w=ptr[3];}
   miniv4d(const unsigned short int ptr[4]) {x=ptr[0]; y=ptr[1]; z=ptr[2]; w=ptr[3];}
   miniv4d(const unsigned char ptr[4]) {x=ptr[0]; y=ptr[1]; z=ptr[2]; w=ptr[3];}

   //! conversion constructors
   miniv4d(const miniv4f &v);
   miniv4d(const miniv3d &v);
   miniv4d(const miniv3d &v,const double vw);
   miniv4d(const miniv3f &v);
   miniv4d(const miniv3f &v,const float vw);

   //! destructor
   ~miniv4d() {}

   void setcoords(const double vx,const double vy,const double vz,const double vw=0.0) {x=vx; y=vy; z=vz; w=vw;}

   double getlength() const {return(fsqrt(x*x+y*y+z*z+w*w));}
   double getlength2() const {return(x*x+y*y+z*z+w*w);}

   inline miniv4d& operator += (const miniv4d &v);
   inline miniv4d& operator -= (const miniv4d &v);

   inline miniv4d& operator *= (const double c);
   inline miniv4d& operator /= (const double c);

   double normalize()
      {
      double length=getlength();
      if (length>0.0) *this/=length;
      return(length);
      }

   double x,y,z,w;
   };

inline miniv4d& miniv4d::operator += (const miniv4d &v)
   {
   x+=v.x; y+=v.y; z+=v.z; w+=v.w;
   return(*this);
   }

inline miniv4d& miniv4d::operator -= (const miniv4d &v)
   {
   x-=v.x; y-=v.y; z-=v.z; w-=v.w;
   return(*this);
   }

inline miniv4d& miniv4d::operator *= (const double c)
   {
   x*=c; y*=c; z*=c; w*=c;
   return(*this);
   }

inline miniv4d& miniv4d::operator /= (const double c)
   {
   double rc;
   if (c!=0.0) rc=1.0/c;
   else rc=1.0;
   x*=rc; y*=rc; z*=rc; w*=rc;
   return(*this);
   }

inline miniv4d operator + (const miniv4d &a,const miniv4d &b);
inline miniv4d operator - (const miniv4d &a,const miniv4d &b);
inline miniv4d operator - (const miniv4d &v);

inline miniv4d operator * (const double a,const miniv4d &b);
inline miniv4d operator * (const miniv4d &a,const double b);
inline miniv4d operator / (const miniv4d &a,const double b);

inline double operator * (const miniv4d &a,const miniv4d &b);

inline int operator == (const miniv4d &a,const miniv4d &b);
inline int operator != (const miniv4d &a,const miniv4d &b);

inline int operator < (const miniv4d &a,const miniv4d &b);
inline int operator > (const miniv4d &a,const miniv4d &b);

inline int operator <= (const miniv4d &a,const miniv4d &b);
inline int operator >= (const miniv4d &a,const miniv4d &b);

inline std::ostream& operator << (std::ostream &out,const miniv4d &v);

inline miniv4d operator + (const miniv4d &a,const miniv4d &b)
   {return(miniv4d(a.x+b.x,a.y+b.y,a.z+b.z,a.w+b.w));}

inline miniv4d operator - (const miniv4d &a,const miniv4d &b)
   {return(miniv4d(a.x-b.x,a.y-b.y,a.z-b.z,a.w-b.w));}

inline miniv4d operator - (const miniv4d &v)
   {return(miniv4d(-v.x,-v.y,-v.z,-v.w));}

inline miniv4d operator * (const double a,const miniv4d &b)
   {return(miniv4d(a*b.x,a*b.y,a*b.z,a*b.w));}

inline miniv4d operator * (const miniv4d &a,const double b)
   {return(miniv4d(a.x*b,a.y*b,a.z*b,a.w*b));}

inline miniv4d operator / (const miniv4d &a,const double b)
   {return(miniv4d(a.x/b,a.y/b,a.z/b,a.w/b));}

inline double operator * (const miniv4d &a,const miniv4d &b)
   {return(a.x*b.x+a.y*b.y+a.z*b.z+a.w*b.w);}

inline int operator == (const miniv4d &a,const miniv4d &b)
   {return(a.x==b.x && a.y==b.y && a.z==b.z && a.w==b.w);}

inline int operator != (const miniv4d &a,const miniv4d &b)
   {return(a.x!=b.x || a.y!=b.y || a.z!=b.z || a.w!=b.w);}

inline int operator < (const miniv4d &a,const miniv4d &b)
   {return(a.x*a.x+a.y*a.y+a.z*a.z+a.w*a.w<b.x*b.x+b.y*b.y+b.z*b.z+b.w*b.w);}

inline int operator > (const miniv4d &a,const miniv4d &b)
   {return(a.x*a.x+a.y*a.y+a.z*a.z+a.w*a.w>b.x*b.x+b.y*b.y+b.z*b.z+b.w*b.w);}

inline int operator <= (const miniv4d &a,const miniv4d &b)
   {return(a.x*a.x+a.y*a.y+a.z*a.z+a.w*a.w<=b.x*b.x+b.y*b.y+b.z*b.z+b.w*b.w);}

inline int operator >= (const miniv4d &a,const miniv4d &b)
   {return(a.x*a.x+a.y*a.y+a.z*a.z+a.w*a.w>=b.x*b.x+b.y*b.y+b.z*b.z+b.w*b.w);}

inline std::ostream& operator << (std::ostream &out,const miniv4d &v)
   {return(out << '(' << round2(v.x) << ',' << round2(v.y) << ',' << round2(v.z) << ',' << round2(v.w) << ')');}

#endif
