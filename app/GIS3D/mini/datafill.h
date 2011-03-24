// (c) by Stefan Roettger

#ifndef DATAFILL_H
#define DATAFILL_H

#include "database.h"

class datafill: public databuf
   {
   public:

   //! default constructor
   datafill(): databuf() {}

   //! copy constructor
   datafill(const databuf &buf): databuf(buf) {}

   //! fill-in no-data values by region growing
   unsigned int fillin_by_regiongrowing(int radius_stop=5,int radius_start=2);

   protected:

   unsigned int fillin(int radius);
   };

#endif
