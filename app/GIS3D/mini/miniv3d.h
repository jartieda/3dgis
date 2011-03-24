// (c) by Stefan Roettger

#ifndef MINIV3D_H
#define MINIV3D_H

#include <iostream>

#include "minibase.h"

class miniv3f;
class miniv4d;
class miniv4f;

class miniv3d
   {
   public:

   //! default constructor
   miniv3d() {}

   //! copy constructor
   miniv3d(const miniv3d &v) {x=v.x; y=v.y; z=v.z;}

   //! constructors
   miniv3d(const double v) {x=y=z=v;}
   miniv3d(const double vx,const double vy,const double vz) {x=vx; y=vy; z=vz;}

   //! array constructors
   miniv3d(const double ptr[3]) {x=ptr[0]; y=ptr[1]; z=ptr[2];}
   miniv3d(const float ptr[3]) {x=(double)ptr[0]; y=(double)ptr[1]; z=(double)ptr[2];}
   miniv3d(const int ptr[3]) {x=ptr[0]; y=ptr[1]; z=ptr[2];}
   miniv3d(const short int ptr[3]) {x=ptr[0]; y=ptr[1]; z=ptr[2];}
   miniv3d(const unsigned short int ptr[3]) {x=ptr[0]; y=ptr[1]; z=ptr[2];}
   miniv3d(const unsigned char ptr[3]) {x=ptr[0]; y=ptr[1]; z=ptr[2];}

   //! conversion constructors
   miniv3d(const miniv3f &v);
   miniv3d(const miniv4d &v);
   miniv3d(const miniv4f &v);

   //! destructor
   ~miniv3d() {}

   void setcoords(const double vx,const double vy,const double vz) {x=vx; y=vy; z=vz;}

   double getlength() const {return(fsqrt(x*x+y*y+z*z));}
   double getlength2() const {return(x*x+y*y+z*z);}

   inline miniv3d& operator += (const miniv3d &v);
   inline miniv3d& operator -= (const miniv3d &v);

   inline miniv3d& operator *= (const double c);
   inline miniv3d& operator /= (const double c);

   double normalize()
      {
      double length=getlength();
      if (length>0.0) *this/=length;
      return(length);
      }

   double x,y,z;
   };

inline miniv3d& miniv3d::operator += (const miniv3d &v)
   {
   x+=v.x; y+=v.y; z+=v.z;
   return(*this);
   }

inline miniv3d& miniv3d::operator -= (const miniv3d &v)
   {
   x-=v.x; y-=v.y; z-=v.z;
   return(*this);
   }

inline miniv3d& miniv3d::operator *= (const double c)
   {
   x*=c; y*=c; z*=c;
   return(*this);
   }

inline miniv3d& miniv3d::operator /= (const double c)
   {
   double rc;
   if (c!=0.0) rc=1.0/c;
   else rc=1.0;
   x*=rc; y*=rc; z*=rc;
   return(*this);
   }

inline miniv3d operator + (const miniv3d &a,const miniv3d &b);
inline miniv3d operator - (const miniv3d &a,const miniv3d &b);
inline miniv3d operator - (const miniv3d &v);

inline miniv3d operator * (const double a,const miniv3d &b);
inline miniv3d operator * (const miniv3d &a,const double b);
inline miniv3d operator / (const miniv3d &a,const double b);

inline double operator * (const miniv3d &a,const miniv3d &b);
inline miniv3d operator / (const miniv3d &a,const miniv3d &b);

inline int operator == (const miniv3d &a,const miniv3d &b);
inline int operator != (const miniv3d &a,const miniv3d &b);

inline int operator < (const miniv3d &a,const miniv3d &b);
inline int operator > (const miniv3d &a,const miniv3d &b);

inline int operator <= (const miniv3d &a,const miniv3d &b);
inline int operator >= (const miniv3d &a,const miniv3d &b);

inline std::ostream& operator << (std::ostream &out,const miniv3d &v);

inline miniv3d operator + (const miniv3d &a,const miniv3d &b)
   {return(miniv3d(a.x+b.x,a.y+b.y,a.z+b.z));}

inline miniv3d operator - (const miniv3d &a,const miniv3d &b)
   {return(miniv3d(a.x-b.x,a.y-b.y,a.z-b.z));}

inline miniv3d operator - (const miniv3d &v)
   {return(miniv3d(-v.x,-v.y,-v.z));}

inline miniv3d operator * (const double a,const miniv3d &b)
   {return(miniv3d(a*b.x,a*b.y,a*b.z));}

inline miniv3d operator * (const miniv3d &a,const double b)
   {return(miniv3d(a.x*b,a.y*b,a.z*b));}

inline miniv3d operator / (const miniv3d &a,const double b)
   {return(miniv3d(a.x/b,a.y/b,a.z/b));}

inline double operator * (const miniv3d &a,const miniv3d &b)
   {return(a.x*b.x+a.y*b.y+a.z*b.z);}

// (0,0,-1)/(-1,0,0)=(0,1,0)
inline miniv3d operator / (const miniv3d &a,const miniv3d &b)
   {return(miniv3d(a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x));}

inline int operator == (const miniv3d &a,const miniv3d &b)
   {return(a.x==b.x && a.y==b.y && a.z==b.z);}

inline int operator != (const miniv3d &a,const miniv3d &b)
   {return(a.x!=b.x || a.y!=b.y || a.z!=b.z);}

inline int operator < (const miniv3d &a,const miniv3d &b)
   {return(a.x*a.x+a.y*a.y+a.z*a.z<b.x*b.x+b.y*b.y+b.z*b.z);}

inline int operator > (const miniv3d &a,const miniv3d &b)
   {return(a.x*a.x+a.y*a.y+a.z*a.z>b.x*b.x+b.y*b.y+b.z*b.z);}

inline int operator <= (const miniv3d &a,const miniv3d &b)
   {return(a.x*a.x+a.y*a.y+a.z*a.z<=b.x*b.x+b.y*b.y+b.z*b.z);}

inline int operator >= (const miniv3d &a,const miniv3d &b)
   {return(a.x*a.x+a.y*a.y+a.z*a.z>=b.x*b.x+b.y*b.y+b.z*b.z);}

inline std::ostream& operator << (std::ostream &out,const miniv3d &v)
   {return(out << '(' << round2(v.x) << ',' << round2(v.y) << ',' << round2(v.z) << ')');}

#endif
