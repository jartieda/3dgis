// (c) by Stefan Roettger

#include "minibase.h"

#include "minimath.h"

namespace minimath {

// greatest common divisor
unsigned int gcd(unsigned int a,unsigned int b)
   {
   unsigned int c;

   while (b>0)
      {
      c=a%b;
      a=b;
      b=c;
      }

   return(a);
   }

// lowest common multiple
unsigned int lcm(unsigned int a,unsigned int b)
   {return(a*b/gcd(a,b));}

// matrix copying:

void cpy_mtx(miniv3d cpy[3],miniv3d mtx[3]) {cpy[0]=mtx[0]; cpy[1]=mtx[1]; cpy[2]=mtx[2];}
void cpy_mtx(miniv4d cpy[3],miniv3d mtx[3]) {cpy[0]=mtx[0]; cpy[1]=mtx[1]; cpy[2]=mtx[2];}
void cpy_mtx(miniv4d cpy[3],miniv4d mtx[3]) {cpy[0]=mtx[0]; cpy[1]=mtx[1]; cpy[2]=mtx[2];}
void cpy_mtx(miniv3d cpy[3],miniv4d mtx[3]) {cpy[0]=mtx[0]; cpy[1]=mtx[1]; cpy[2]=mtx[2];}

void cpy_mtx(miniv3d cpy[3],const miniv3d mtx[3]) {cpy[0]=mtx[0]; cpy[1]=mtx[1]; cpy[2]=mtx[2];}
void cpy_mtx(miniv4d cpy[3],const miniv3d mtx[3]) {cpy[0]=mtx[0]; cpy[1]=mtx[1]; cpy[2]=mtx[2];}
void cpy_mtx(miniv4d cpy[3],const miniv4d mtx[3]) {cpy[0]=mtx[0]; cpy[1]=mtx[1]; cpy[2]=mtx[2];}
void cpy_mtx(miniv3d cpy[3],const miniv4d mtx[3]) {cpy[0]=mtx[0]; cpy[1]=mtx[1]; cpy[2]=mtx[2];}

// multiply two 4x3 matrices
void mlt_mtx(miniv4d mtx[3],const miniv4d mtx1[3],const miniv4d mtx2[3])
   {
   int i;

   miniv4d m[3];

   for (i=0; i<3; i++)
      {
      // fourth row is assumed to be (0,0,0,1)
      m[i].x=mtx1[i].x*mtx2[0].x+mtx1[i].y*mtx2[1].x+mtx1[i].z*mtx2[2].x;
      m[i].y=mtx1[i].x*mtx2[0].y+mtx1[i].y*mtx2[1].y+mtx1[i].z*mtx2[2].y;
      m[i].z=mtx1[i].x*mtx2[0].z+mtx1[i].y*mtx2[1].z+mtx1[i].z*mtx2[2].z;
      m[i].w=mtx1[i].x*mtx2[0].w+mtx1[i].y*mtx2[1].w+mtx1[i].z*mtx2[2].w+mtx1[i].w;
      }

   cpy_mtx(mtx,m);
   }

// multiply three 4x3 matrices
void mlt_mtx(miniv4d mtx[3],const miniv4d mtx1[3],const miniv4d mtx2[3],const miniv4d mtx3[3])
   {
   miniv4d m[3];

   mlt_mtx(m,mtx2,mtx3);
   mlt_mtx(mtx,mtx1,m);
   }

// multiply four 4x3 matrices
void mlt_mtx(miniv4d mtx[3],const miniv4d mtx1[3],const miniv4d mtx2[3],const miniv4d mtx3[3],const miniv4d mtx4[3])
   {
   miniv4d m[3];

   mlt_mtx(m,mtx3,mtx4);
   mlt_mtx(mtx,mtx1,mtx2,m);
   }

// multiply five 4x3 matrices
void mlt_mtx(miniv4d mtx[3],const miniv4d mtx1[3],const miniv4d mtx2[3],const miniv4d mtx3[3],const miniv4d mtx4[3],const miniv4d mtx5[3])
   {
   miniv4d m[3];

   mlt_mtx(m,mtx3,mtx4,mtx5);
   mlt_mtx(mtx,mtx1,mtx2,m);
   }

// multiply six 4x3 matrices
void mlt_mtx(miniv4d mtx[3],const miniv4d mtx1[3],const miniv4d mtx2[3],const miniv4d mtx3[3],const miniv4d mtx4[3],const miniv4d mtx5[3],const miniv4d mtx6[3])
   {
   miniv4d m[3];

   mlt_mtx(m,mtx4,mtx5,mtx6);
   mlt_mtx(mtx,mtx1,mtx2,mtx3,m);
   }

// multiply seven 4x3 matrices
void mlt_mtx(miniv4d mtx[3],const miniv4d mtx1[3],const miniv4d mtx2[3],const miniv4d mtx3[3],const miniv4d mtx4[3],const miniv4d mtx5[3],const miniv4d mtx6[3],const miniv4d mtx7[3])
   {
   miniv4d m[3];

   mlt_mtx(m,mtx4,mtx5,mtx6,mtx7);
   mlt_mtx(mtx,mtx1,mtx2,mtx3,m);
   }

// multiply eight 4x3 matrices
void mlt_mtx(miniv4d mtx[3],const miniv4d mtx1[3],const miniv4d mtx2[3],const miniv4d mtx3[3],const miniv4d mtx4[3],const miniv4d mtx5[3],const miniv4d mtx6[3],const miniv4d mtx7[3],const miniv4d mtx8[3])
   {
   miniv4d m[3];

   mlt_mtx(m,mtx5,mtx6,mtx7,mtx8);
   mlt_mtx(mtx,mtx1,mtx2,mtx3,mtx4,m);
   }

// calculate determinant of 3x3 matrix
double det_mtx(const miniv3d mtx[3])
   {
   return(mtx[0].x*(mtx[2].z*mtx[1].y-mtx[1].z*mtx[2].y)+
          mtx[0].y*(mtx[1].z*mtx[2].x-mtx[2].z*mtx[1].x)+
          mtx[0].z*(mtx[2].y*mtx[1].x-mtx[1].y*mtx[2].x));
   }

// invert a 3x3 matrix
void inv_mtx(miniv3d inv[3],const miniv3d mtx[3])
   {
   double det;
   miniv3d m[3];

   // calculate determinant
   det=det_mtx(mtx);

   // check determinant
   if (det==0.0) inv[0]=inv[1]=inv[2]=miniv3d(0.0);
   else
      {
      det=1.0/det;

      cpy_mtx(m,mtx);

      inv[0].x=det*(m[2].z*m[1].y-m[1].z*m[2].y);
      inv[1].x=det*(m[1].z*m[2].x-m[2].z*m[1].x);
      inv[2].x=det*(m[2].y*m[1].x-m[1].y*m[2].x);
      inv[0].y=det*(m[0].z*m[2].y-m[2].z*m[0].y);
      inv[1].y=det*(m[2].z*m[0].x-m[0].z*m[2].x);
      inv[2].y=det*(m[0].y*m[2].x-m[2].y*m[0].x);
      inv[0].z=det*(m[1].z*m[0].y-m[0].z*m[1].y);
      inv[1].z=det*(m[0].z*m[1].x-m[1].z*m[0].x);
      inv[2].z=det*(m[1].y*m[0].x-m[0].y*m[1].x);
      }
   }

// invert a 4x3 matrix
void inv_mtx(miniv4d inv[3],const miniv4d mtx[3])
   {
   miniv3d m[3];
   miniv4d m1[3],m2[3];

   // extract 3x3 sub-matrix
   cpy_mtx(m,mtx);

   // it is sufficient to invert the 3x3 sub-matrix
   inv_mtx(m,m);

   // decompose 4x3 inversion into matrix #1 = inverted 3x3 sub-matrix
   cpy_mtx(m1,m);

   // decompose 4x3 inversion into matrix #2 = inverted 1x3 sub-matrix = negated vector
   m2[0]=miniv4d(1.0,0.0,0.0,-mtx[0].w);
   m2[1]=miniv4d(0.0,1.0,0.0,-mtx[1].w);
   m2[2]=miniv4d(0.0,0.0,1.0,-mtx[2].w);

   // compose the inverted matrix
   mlt_mtx(inv,m1,m2);
   }

// transpose a 3x3 matrix
void tra_mtx(miniv3d tra[3],const miniv3d mtx[3])
   {
   miniv3d m[3];

   // compute transposition
   m[0]=miniv3d(mtx[0].x,mtx[1].x,mtx[2].x);
   m[1]=miniv3d(mtx[0].y,mtx[1].y,mtx[2].y);
   m[2]=miniv3d(mtx[0].z,mtx[1].z,mtx[2].z);

   cpy_mtx(tra,m);
   }

// transpose a 4x3 matrix
void tra_mtx(miniv4d tra[3],const miniv4d mtx[3])
   {
   miniv3d m[3];

   // compute transposition
   m[0]=miniv3d(mtx[0].x,mtx[1].x,mtx[2].x);
   m[1]=miniv3d(mtx[0].y,mtx[1].y,mtx[2].y);
   m[2]=miniv3d(mtx[0].z,mtx[1].z,mtx[2].z);

   cpy_mtx(tra,m);
   }

}
