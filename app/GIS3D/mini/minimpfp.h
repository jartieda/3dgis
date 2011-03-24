// (c) by Stefan Roettger

#ifndef MINIMPFP_H
#define MINIMPFP_H

#include "iostream"

#include "minibase.h"

// multi-precision fixed-point arithmetic:

// 32 bit base precision
class minimpfp_base
   {
   public:

   //! default constructor
   minimpfp_base() {}

   //! constructor
   minimpfp_base(const unsigned int m,const unsigned int f)
      {V=(((unsigned long long int )m)<<32)|(unsigned long long int)f;}

   //! constructor
   minimpfp_base(const double v)
      {set(v);}

   //! destructor
   ~minimpfp_base() {}

   unsigned long long int V;

   static BOOLINT MINIMPFP_DIVBYZERO;
   static BOOLINT MINIMPFP_OVERFLOW;

   static const unsigned int getbits() {return(64);}
   static const double getlimit() {return((double)(1ll<<32));}

   static const minimpfp_base zero() {return(minimpfp_base(0,0));}
   static const minimpfp_base one() {return(minimpfp_base(1,0));}

   static const minimpfp_base min() {return(minimpfp_base(0,1));}
   static const minimpfp_base max() {return(minimpfp_base(0xFFFFFFFF,0xFFFFFFFF));}

   const unsigned int getmag() const {return(V>>32);}
   const unsigned int getfrc() const {return(V);}

   void set(const double v) {V=(unsigned long long int)floor(v*(1ll<<32)+0.5);}
   double get() const {return(V*(1.0/(double)(1ll<<32)));}

   BOOLINT iszero() const {return(V==0);}
   BOOLINT isnotzero() const {return(V!=0);}

   BOOLINT isone() const {return(V==(1ll<<32));}
   BOOLINT isnotone() const {return(V!=(1ll<<32));}

   BOOLINT ismin() const {return(V==1);}
   BOOLINT isnotmin() const {return(V!=1);}

   BOOLINT ismax() const {return(V==0xFFFFFFFFFFFFFFFFll);}
   BOOLINT isnotmax() const {return(V!=0xFFFFFFFFFFFFFFFFll);}

   BOOLINT isequal(const minimpfp_base &value) const {return(value.V==V);}
   BOOLINT isnotequal(const minimpfp_base &value) const {return(value.V!=V);}

   minimpfp_base left() const {return(minimpfp_base(getfrc(),0));}
   minimpfp_base right() const {return(minimpfp_base(0,getmag()));}

   void nrm() {/*nop*/}
   void cpm() {V=~V+1;}

   BOOLINT add2(const minimpfp_base &value,minimpfp_base &result) const
      {
      unsigned long long int v=V;
      result.V=V+value.V;
      return(result.V<v);
      }

   BOOLINT sub2(const minimpfp_base &value,minimpfp_base &result) const
      {
      unsigned int long long v=V;
      result.V=V-value.V;
      return(result.V>v);
      }

   minimpfp_base mul2(const minimpfp_base &value,minimpfp_base &result) const
      {
      unsigned long long int r1,r2;
      unsigned long long int m1,m2,f1,f2;
      unsigned long long int mv1,mv2,mv3,mv4;
      m1=(V>>32);
      m2=(value.V>>32);
      f1=(V&0xFFFFFFFFll);
      f2=(value.V&0xFFFFFFFFll);
      mv1=f1*f2;
      mv2=f1*m2;
      mv3=m1*f2;
      mv4=m1*m2;
      r1=(mv1>>32)|(mv4<<32);
      mv4>>=32;
      r2=r1+mv2;
      if (r2<r1) mv4++;
      r1=r2+mv3;
      if (r1<r2) mv4++;
      result.V=r1;
      return(minimpfp_base((unsigned int)mv4,(unsigned int)mv1));
      }

   minimpfp_base left2(const unsigned int bits,minimpfp_base &result) const
      {
      unsigned long long int lv1,lv2;
      lv1=V<<bits;
      lv2=V>>(64-bits);
      result.V=lv1;
      return(minimpfp_base(lv2>>32,lv2));
      }

   minimpfp_base right2(const unsigned int bits,minimpfp_base &result) const
      {
      unsigned long long int lv1,lv2;
      lv1=V>>bits;
      lv2=V<<(64-bits);
      result.V=lv1;
      return(minimpfp_base(lv2>>32,lv2));
      }

   minimpfp_base inv2() const
      {
      unsigned long long int iv;
      if (V==0) {MINIMPFP_DIVBYZERO=TRUE; return(max());}
      else iv=0xFFFFFFFFFFFFFFFFll/V;
      return(minimpfp_base(iv>>32,iv));
      }

   unsigned int getmsbit() const
      {
      static const unsigned int table[16]={0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4};

      if (V&0xFFFFFFFF00000000ll)
         if (V&0xFFFF000000000000ll)
            if (V&0xFF00000000000000ll)
               if (V&0xF000000000000000ll) return(table[V>>60]+60);
               else return(table[V>>56]+56);
            else
               if (V&0x00F0000000000000ll) return(table[V>>52]+52);
               else return(table[V>>48]+48);
         else
            if (V&0x0000FF0000000000ll)
               if (V&0x0000F00000000000ll) return(table[V>>44]+44);
               else return(table[V>>40]+40);
            else
               if (V&0x000000F000000000ll) return(table[V>>36]+36);
               else return(table[V>>32]+32);
      else
         if (V&0x00000000FFFF0000ll)
            if (V&0x00000000FF000000ll)
               if (V&0x00000000F0000000ll) return(table[V>>28]+28);
               else return(table[V>>24]+24);
            else
               if (V&0x0000000000F00000ll) return(table[V>>20]+20);
               else return(table[V>>16]+16);
         else
            if (V&0x000000000000FF00ll)
               if (V&0x000000000000F000ll) return(table[V>>12]+12);
               else return(table[V>>8]+8);
            else
               if (V&0x00000000000000F0ll) return(table[V>>4]+4);
               else return(table[V]);
      }

   void print() const
      {printf("%08X%08X",(unsigned int)(V>>32),(unsigned int)V);}
   };

// double template precision
template <class N>
class minimpfp
   {
   public:

   //! default constructor
   minimpfp() {}

   //! constructor
   minimpfp(const BOOLINT s,const N &m,const N &f)
      {
      S=s;
      M=m;
      F=f;
      }

   //! constructor
   minimpfp(const N &m,const N &f)
      {
      S=TRUE;
      M=m;
      F=f;
      }

   //! constructor
   minimpfp(const double v) {set(v);}

   //! destructor
   ~minimpfp() {}

   static const unsigned int getbits() {return(2*N::getbits());}
   static const double getlimit() {return(N::getlimit()*N::getlimit());}

   static const minimpfp zero() {return(minimpfp(N::zero(),N::zero()));}
   static const minimpfp one() {return(minimpfp(N::min(),N::zero()));}

   static const minimpfp min() {return(minimpfp(N::zero(),N::min()));}
   static const minimpfp max() {return(minimpfp(N::max(),N::max()));}

   minimpfp minval() const {return(minimpfp(S,N::zero(),N::min()));}
   minimpfp maxval() const {return(minimpfp(S,N::max(),N::max()));}

   BOOLINT getsgn() const {return(S);}
   N getmag() const {return(M);}
   N getfrc() const {return(F);}

   void set(const double v)
      {
      static const double limit=getlimit();
      static const double limit2=N::getlimit();

      double av;

      S=(v>=0.0);
      av=S?v:-v;

      if (av>=limit) M=F=N::max();
      else
         {
         M=N(floor(av)*(1.0/limit2));
         F=N((av-floor(av))*limit2);
         }
      }

   double get() const
      {
      static const double limit=N::getlimit();

      double v;

      v=M.get()*limit;
      v+=F.get()*(1.0/limit);

      return(S?v:-v);
      }

   BOOLINT iszero() const
      {
      if (M.iszero())
         if (F.iszero()) return(TRUE);

      return(FALSE);
      }

   BOOLINT isnotzero() const
      {
      if (M.isnotzero()) return(TRUE);
      if (F.isnotzero()) return(TRUE);

      return(FALSE);
      }

   BOOLINT isone() const
      {
      if (M.ismin())
         if (F.iszero()) return(TRUE);

      return(FALSE);
      }

   BOOLINT isnotone() const
      {
      if (M.isnotmin()) return(TRUE);
      if (F.isnotzero()) return(TRUE);

      return(FALSE);
      }

   BOOLINT ismin() const
      {
      if (M.iszero())
         if (F.ismin()) return(TRUE);

      return(FALSE);
      }

   BOOLINT isnotmin() const
      {
      if (M.isnotzero()) return(TRUE);
      if (F.isnotmin()) return(TRUE);

      return(FALSE);
      }

   BOOLINT ismax() const
      {
      if (M.ismax())
         if (F.ismax()) return(TRUE);

      return(FALSE);
      }

   BOOLINT isnotmax() const
      {
      if (M.isnotmax()) return(TRUE);
      if (F.isnotmax()) return(TRUE);

      return(FALSE);
      }

   BOOLINT isequal(const minimpfp &value) const
      {
      if (M.isequal(value.getmag()))
         if (F.isequal(value.getfrc()))
            if (S==value.getsgn()) return(TRUE);
            else
               if (M.iszero())
                  if (F.iszero()) return(TRUE);

      return(FALSE);
      }

   BOOLINT isnotequal(const minimpfp &value) const
      {
      if (M.isnotequal(value.getmag())) return(TRUE);
      if (F.isnotequal(value.getfrc())) return(TRUE);
      if (S==value.getsgn()) return(FALSE);

      if (M.isnotzero()) return(TRUE);
      if (F.isnotzero()) return(TRUE);

      return(FALSE);
      }

   minimpfp left() const {return(minimpfp(F,N::zero()));}
   minimpfp right() const {return(minimpfp(N::zero(),M));}

   minimpfp neg() const {return(minimpfp(!S,M,F));}
   minimpfp abs() const {return(minimpfp(M,F));}

   void nrm()
      {
      if (!S)
         {
         add2(max(),*this);
         add2(min(),*this);
         }
      }

   void cpm() {S=!S;}

   void add(const minimpfp &value,minimpfp &result) const
      {
      if (S^value.getsgn()) add2(value,result);
      else if (add2(value,result))
         {
         minimpfp_base::MINIMPFP_OVERFLOW=TRUE;
         result=result.maxval();
         }
      }

   void add(const minimpfp &value) {add(value,*this);}

   // recursive addition
   BOOLINT add2(const minimpfp &value,minimpfp &result) const
      {
      BOOLINT sign;
      N result1,result2;
      BOOLINT overflow1,overflow2;

      if (S)
         if (value.getsgn())
            // add two positive numbers
            {
            overflow1=F.add2(value.getfrc(),result1);
            overflow2=M.add2(value.getmag(),result2);

            if (overflow1) overflow1=result2.add2(N::min(),result2);

            result=minimpfp(result2,result1);
            }
         else
            // subtract second from first number
            {
            sign=TRUE;

            overflow1=M.sub2(value.getmag(),result1);

            if (overflow1)
               {
               sign=FALSE;
               result1.cpm();

               overflow2=value.getfrc().sub2(F,result2);
               }
            else overflow2=F.sub2(value.getfrc(),result2);

            if (overflow2)
               {
               result2.nrm();
               overflow2=result1.sub2(N::min(),result1);

               if (overflow2)
                  {
                  sign=FALSE;
                  result1=N::zero();
                  result2.cpm();
                  result2.nrm();
                  }
               }

            result=minimpfp(sign,result1,result2);
            }
      else
         if (value.getsgn())
            // subtract first from second number
            {
            sign=TRUE;

            overflow1=value.getmag().sub2(M,result1);

            if (overflow1)
               {
               sign=FALSE;
               result1.cpm();

               overflow2=F.sub2(value.getfrc(),result2);
               }
            else overflow2=value.getfrc().sub2(F,result2);

            if (overflow2)
               {
               result2.nrm();
               overflow2=result1.sub2(N::min(),result1);

               if (overflow2)
                  {
                  sign=FALSE;
                  result1=N::zero();
                  result2.cpm();
                  result2.nrm();
                  }
               }

            result=minimpfp(sign,result1,result2);
            }
         else
            // add two negative numbers
            {
            overflow1=value.getfrc().add2(F,result1);
            overflow2=value.getmag().add2(M,result2);

            if (overflow1) overflow1=result2.add2(N::min(),result2);

            result=minimpfp(FALSE,result2,result1);
            }

      return(overflow1 || overflow2);
      }

   void sub(const minimpfp &value,minimpfp &result) const
      {add(value.neg(),result);}

   void sub(const minimpfp &value)
      {sub(value.neg(),*this);}

   BOOLINT sub2(const minimpfp &value,minimpfp &result) const
      {return(add2(value.neg(),result));}

   BOOLINT sml(const minimpfp &value) const
      {
      minimpfp result;

      if (S)
         if (value.getsgn())
            {
            sub2(value,result);

            if (!result.getsgn())
               if (result.isnotzero()) return(TRUE);
            }
         else
            if (isnotzero() || value.isnotzero()) return(FALSE);
            else return(TRUE);
      else
         if (value.getsgn())
            if (isnotzero() || value.isnotzero()) return(TRUE);
            else return(FALSE);
         else
            {
            sub2(value,result);

            if (result.getsgn())
               if (result.isnotzero()) return(TRUE);
            }

      return(FALSE);
      }

   BOOLINT grt(const minimpfp &value) const
      {
      minimpfp result;

      if (S)
         if (value.getsgn())
            {
            sub2(value,result);

            if (result.getsgn())
               if (result.isnotzero()) return(TRUE);
            }
         else
            if (isnotzero() || value.isnotzero()) return(TRUE);
            else return(FALSE);
      else
         if (value.getsgn())
            if (isnotzero() || value.isnotzero()) return(FALSE);
            else return(TRUE);
         else
            {
            sub2(value,result);

            if (!result.getsgn())
               if (result.isnotzero()) return(TRUE);
            }

      return(FALSE);
      }

   minimpfp min(const minimpfp &value) {return((sml(value))?*this:value);}
   minimpfp max(const minimpfp &value) {return((grt(value))?*this:value);}

   void mul(const minimpfp &value,minimpfp &result) const
      {
      if (mul2(value,result).getmag().isnotzero())
         {
         minimpfp_base::MINIMPFP_OVERFLOW=TRUE;
         result=result.maxval();
         }
      }

   // recursive multiplication
   minimpfp mul2(const minimpfp &value,minimpfp &result) const
      {
      BOOLINT sign;
      N result1,result2,result3,result4;
      N overflow1,overflow2,overflow3,overflow4;
      minimpfp overflow;

      // multiply sign
      sign=!(S^value.getsgn());

      // calculate sub-terms
      overflow1=F.mul2(value.getfrc(),result1);
      overflow2=F.mul2(value.getmag(),result2);
      overflow3=M.mul2(value.getfrc(),result3);
      overflow4=M.mul2(value.getmag(),result4);

      // sum up sub-terms without overflow
      result=minimpfp(sign,N(result4.getfrc(),overflow4.getfrc()),N(overflow1.getmag(),result1.getmag()));
      overflow=minimpfp(N(overflow4.getmag(),result4.getmag()),N(result1.getfrc(),overflow1.getfrc()));

      // sum up sub-terms with overflow
      if (result.add2(minimpfp(sign,N(overflow2.getmag(),result2.getmag()),N(result2.getfrc(),overflow2.getfrc())),result)) overflow.add2(one(),overflow);
      if (result.add2(minimpfp(sign,N(overflow3.getmag(),result3.getmag()),N(result3.getfrc(),overflow3.getfrc())),result)) overflow.add2(one(),overflow);

      // return overflow
      return(overflow);
      }

   // recursive left bit shift
   minimpfp left2(const unsigned int bits,minimpfp &result) const
      {
      N result1,result2;
      N overflow1,overflow2;

      if (bits==0)
         // no shift
         {
         result=*this;
         return(zero());
         }
      else if (bits<=N::getbits())
         // shift no more than half word size
         {
         overflow1=F.left2(bits,result1);
         overflow2=M.left2(bits,result2);

         result2.add2(overflow1,result2);

         result=minimpfp(S,result2,result1);

         return(minimpfp(N::zero(),overflow2));
         }
      else
         // shift more than half word size
         {
         overflow1=F.left2(bits-N::getbits(),result1);
         overflow2=M.left2(bits-N::getbits(),result2);

         result2.add2(overflow1,result2);

         result=minimpfp(S,result1,N::zero());

         return(minimpfp(overflow2,result2));
         }
      }

   // recursive right bit shift
   minimpfp right2(const unsigned int bits,minimpfp &result) const
      {
      N result1,result2;
      N overflow1,overflow2;

      if (bits==0)
         // no shift
         {
         result=*this;
         return(zero());
         }
      else if (bits<=N::getbits())
         // shift no more than half word size
         {
         overflow1=M.right2(bits,result1);
         overflow2=F.right2(bits,result2);

         result2.add2(overflow1,result2);

         result=minimpfp(S,result1,result2);

         return(minimpfp(overflow2,N::zero()));
         }
      else
         // shift more than half word size
         {
         overflow1=M.right2(bits-N::getbits(),result1);
         overflow2=F.right2(bits-N::getbits(),result2);

         result2.add2(overflow1,result2);

         result=minimpfp(S,N::zero(),result1);

         return(minimpfp(result2,overflow2));
         }
      }

   unsigned int getmsbit() const
      {
      unsigned int bit;

      bit=M.getmsbit();
      if (bit!=0) return(bit+N::getbits());
      else return(F.getmsbit());
      }

   void div(const minimpfp &value,minimpfp &result) const
      {
      if (div2(value,result).getmag().isnotzero())
         {
         minimpfp_base::MINIMPFP_OVERFLOW=TRUE;
         result=result.maxval();
         }
      }

   minimpfp div2(const minimpfp &value,minimpfp &result) const
      {return(mul2(value.inv(),result));}

   // recursive inversion (reciprocal value)
   minimpfp inv() const
      {
      unsigned int bit;
      minimpfp result;

      // check for division by zero
      if (iszero())
         {
         minimpfp_base::MINIMPFP_DIVBYZERO=TRUE;
         return(maxval());
         }

      // check for overflow
      if (ismin())
         {
         minimpfp_base::MINIMPFP_OVERFLOW=TRUE;
         return(maxval());
         }

      // get most significant bit
      bit=getmsbit();

      // shift most significant bit behind binary point
      if (bit>N::getbits()) right2(bit-N::getbits(),result);
      else left2(N::getbits()-bit,result);

      // compute inverse
      if (result.getsgn()) result=result.inv2();
      else result=result.neg().inv2().neg();

      // shift inverse in place
      if (bit>N::getbits()) result.right2(bit-N::getbits(),result);
      else result.left2(N::getbits()-bit,result);

      // return shifted inverse
      return(result);
      }

   // Newton-Raphson iteration with x_n+1=x_n*(2-v*x_n)
   // assumes that the value v to be inverted is in the range [0.5-1[
   // starting value is derived recursively from an inversion with half precision
   // due to quadratic convergence one iteration round suffices per recursion level
   minimpfp inv2() const
      {
      static const minimpfp two(2.0);

      N x2;
      minimpfp x,y;

      // compute starting value
      x2=F.right().inv2();
      x=minimpfp(x2.right(),x2.left());

      // one round of Newton-Raphson iteration
      mul2(x,y);
      two.sub2(y,y);
      x.mul2(y,x);

      // return inverted value
      return(x);
      }

   // Newton-Raphson iteration with x_n+1=1/2*(v/x_n+x_n)
   // starting value is approximated from floating-point root
   minimpfp sqroot() const
      {
      minimpfp r,r2,e,e2,e3;

      if (!S) return(zero());
      if (iszero()) return(zero());

      r.set(sqrt(get()));
      e=max();

      do
         {
         e2=e;
         div(r,r2);
         r2.sub(r,r2);
         r2.right2(1,e);
         r.add(e,r);
         e.left2(1,e3);
         }
      while (e3.abs().sml(e2.abs()));

      return(r);
      }

   // Newton-Raphson iteration with x_n+1=(3/2-1/2*v*x_n^2)*x_n
   // starting value is approximated from inverse floating-point root
   minimpfp invsqroot() const
      {
      static const minimpfp c1(0.5);
      static const minimpfp c2(1.5);

      minimpfp r,r2,e,e2,e3;

      if (!S) return(zero());
      if (iszero()) return(maxval());

      r.set(1.0/sqrt(get()));
      e=max();

      do
         {
         e2=e;
         mul(r,r2);
         r2.mul(r,r2);
         r2.mul(c1,r2);
         c2.sub(r2,r2);
         r2.mul(r,r2);
         r2.sub(r,e);
         r=r2;
         e.left2(1,e3);
         }
      while (e3.abs().sml(e2.abs()));

      return(r);
      }

   void print() const
      {
      if (!S) printf("-");
      printf("(");
      M.print();
      printf(",");
      F.print();
      printf(")");
      }

   static BOOLINT isvalid()
      {
      return(minimpfp_base::MINIMPFP_OVERFLOW ||
             minimpfp_base::MINIMPFP_DIVBYZERO);
      }

   private:

   BOOLINT S;
   N M,F;
   };

typedef minimpfp<minimpfp_base> minimpfp1; // 128 bit precision
typedef minimpfp<minimpfp1> minimpfp2;     // 256 bit precision
typedef minimpfp<minimpfp2> minimpfp4;     // 512 bit precision
typedef minimpfp<minimpfp4> minimpfp8;     // 1024 bit precision

typedef minimpfp2 minimf;

// multi-precision floating point operators:

inline minimf operator + (const minimf &a,const minimf &b)
   {
   minimf result;
   a.add(b,result);
   return(result);
   }

inline minimf operator + (const double a,const minimf &b)
   {
   minimf result;
   minimf(a).add(b,result);
   return(result);
   }

inline minimf operator + (const minimf &a,const double b)
   {
   minimf result;
   a.add(minimf(b),result);
   return(result);
   }

inline minimf operator - (const minimf &a,const minimf &b)
   {
   minimf result;
   a.sub(b,result);
   return(result);
   }

inline minimf operator - (const double a,const minimf &b)
   {
   minimf result;
   minimf(a).sub(b,result);
   return(result);
   }

inline minimf operator - (const minimf &a,const double b)
   {
   minimf result;
   a.sub(minimf(b),result);
   return(result);
   }

inline minimf operator - (const minimf &v)
   {return(v.neg());}

inline minimf operator * (const minimf &a,const minimf &b)
   {
   minimf result;
   a.mul(b,result);
   return(result);
   }

inline minimf operator * (const double a,const minimf &b)
   {
   minimf result;
   minimf(a).mul(b,result);
   return(result);
   }

inline minimf operator * (const minimf &a,const double b)
   {
   minimf result;
   a.mul(minimf(b),result);
   return(result);
   }

inline minimf operator / (const minimf &a,const minimf &b)
   {
   minimf result;
   a.div(b,result);
   return(result);
   }

inline minimf operator / (const double a,const minimf &b)
   {
   minimf result;
   minimf(a).div(b,result);
   return(result);
   }

inline minimf operator / (const minimf &a,const double b)
   {
   minimf result;
   a.div(minimf(b),result);
   return(result);
   }

inline int operator == (const minimf &a,const minimf &b)
   {return(a.isequal(b));}

inline int operator == (const minimf &a,const double b)
   {return(a.isequal(minimf(b)));}

inline int operator != (const minimf &a,const minimf &b)
   {return(a.isnotequal(b));}

inline int operator != (const minimf &a,const double b)
   {return(a.isnotequal(minimf(b)));}

inline int operator < (const minimf &a,const minimf &b)
   {return(a.sml(b));}

inline int operator < (const minimf &a,const double b)
   {return(a.sml(minimf(b)));}

inline int operator > (const minimf &a,const minimf &b)
   {return(a.grt(b));}

inline int operator > (const minimf &a,const double b)
   {return(a.grt(minimf(b)));}

inline int operator <= (const minimf &a,const minimf &b)
   {return(!a.grt(b));}

inline int operator <= (const minimf &a,const double b)
   {return(!a.grt(minimf(b)));}

inline int operator >= (const minimf &a,const minimf &b)
   {return(!a.sml(b));}

inline int operator >= (const minimf &a,const double b)
   {return(!a.sml(minimf(b)));}

inline std::ostream& operator << (std::ostream &out,const minimf &mf)
   {return(out << mf.get());}

#endif
