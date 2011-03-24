#ifndef MINISORT_H
#define MINISORT_H

#include "minibase.h"

namespace minisort {

// Shellsort as proposed by Robert Sedgewick in "Algorithms"
template <class Item>
void shellsort(Item *a[],const int n)
   {
   int i,j,h;

   Item *v;

   for (h=1; h<=(n-1)/9; h=3*h+1);

   while (h>0)
      {
      for (i=h; i<n; i++)
         {
         j=i;
         v=a[i];
         while (j>=h && (*v)<(*a[j-h]))
            {
            a[j]=a[j-h];
            j-=h;
            }
         a[j]=v;
         }
      h/=3;
      }
   }

}

using namespace minisort;

#endif
