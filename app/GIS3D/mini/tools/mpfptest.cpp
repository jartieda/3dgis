#include "minibase.h"

#include "minimpfp.h"
#include "minitime.h"

int main(int argc,char *argv[])
   {
   if (argc!=1)
      {
      printf("usage: %s\n",argv[0]);
      exit(1);
      }

   double time;

   long long int i,j;

   minimf test1,test2,test3;
   double error1,error2,error3,error4,error5;

   // report constants:

   printf("bits=%u\n",minimf::getbits());
   printf("limit=%g\n",minimf::getlimit());

   // add speed test:

   time=minigettime();

   const long long int addmax1=100;
   const long long int addmax2=100;

   for (i=-addmax1-1; i<addmax1; i++)
      for (j=-addmax2-1; j<addmax2; j++)
         {
         test1=minimf(0.1*i);
         test2=minimf(0.1*j);

         test1.add(test2,test3);

         error1=FABS(test3.get()-0.1*i-0.1*j);

         if (error1>1E-3) printf("%g+%g!=%g\n",0.1*i,0.1*j,test3.get());
         }

   time=minigettime()-time;

   printf("%gk adds/s\n",(double)4*addmax1*addmax2/time/1E3);

   // mul speed test:

   time=minigettime();

   const long long int mulmax1=100;
   const long long int mulmax2=100;

   for (i=-mulmax1-1; i<mulmax1; i++)
      for (j=-mulmax2-1; j<mulmax2; j++)
         {
         test1=minimf(0.1*i);
         test2=minimf(0.1*j);

         test1.mul(test2,test3);

         error2=FABS(test3.get()-0.1*i*0.1*j);

         if (error2>1E-3) printf("%g*%g!=%g\n",0.1*i,0.1*j,test3.get());
         }

   time=minigettime()-time;

   printf("%gk muls/s\n",(double)4*mulmax1*mulmax2/time/1E3);

   // div speed test:

   time=minigettime();

   const long long int divmax1=100;
   const long long int divmax2=100;

   for (i=-divmax1-1; i<divmax1; i++)
      for (j=-divmax2-1; j<divmax2+1; j++)
         if (j!=0)
            {
            test1=minimf(0.1*i);
            test2=minimf(0.1*j);

            test1.div(test2,test3);

            error3=FABS(test3.get()-(double)i/j);

            if (error3>1E-3) printf("%g/%g!=%g\n",0.1*i,0.1*j,test3.get());
            }

   time=minigettime()-time;

   printf("%gk divs/s\n",(double)4*divmax1*divmax2/time/1E3);

   // sqrt speed test:

   time=minigettime();

   const long long int sqrtmax=10000;

   for (i=0; i<sqrtmax; i++)
      {
      test1=minimf(0.1*i);

      test2=test1.sqroot();

      error4=FABS(test2.get()-sqrt(0.1*i));

      if (error4>1E-3) printf("sqrt(%g)!=%g\n",0.1*i,test2.get());
      }

   time=minigettime()-time;

   printf("%gk sqrts/s\n",(double)sqrtmax/time/1E3);

   // 1/sqrt speed test:

   time=minigettime();

   const long long int invsqrtmax=10000;

   for (i=1; i<invsqrtmax+1; i++)
      {
      test1=minimf(0.1*i);

      test2=test1.invsqroot();

      error5=FABS(test2.get()-1.0/sqrt(0.1*i));

      if (error5>1E-3) printf("1/sqrt(%g)!=%g\n",0.1*i,test2.get());
      }

   time=minigettime()-time;

   printf("%gk invsqrts/s\n",(double)invsqrtmax/time/1E3);

   // precision test:

   minimf c(271.3);

   minimf x=c*c*c*c;
   minimf y=(x-1)*(x+1);
   minimf z=x*x-y;

   z=(z-1).abs();

   printf("precision test #1: %g(%d)\n",z.get(),z.getmsbit());

   minimf inv=1/c;
   minimf mlt=inv*c;

   mlt=(mlt-1).abs();

   printf("precision test #2: %g(%d)\n",mlt.get(),mlt.getmsbit());

   // validity test:

   printf("validity test: %d\n",!minimf::isvalid());

   return(0);
   }
