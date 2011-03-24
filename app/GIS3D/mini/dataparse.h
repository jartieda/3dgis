// (c) by Stefan Roettger

#ifndef DATAPARSE_H
#define DATAPARSE_H

#include "lunaparse.h"

class dataparse
   {
   public:

   //! default constructor
   dataparse();

   //! destructor
   ~dataparse();

   //! set the code to be parsed and executed
   void setcode(char *code,int bytes);

   //! push one value onto the computation stack
   void pushvalue(float v);

   //! pop one value from the computation stack
   float popvalue();

   //! reset the interpreter
   void init();

   //! execute the previously parsed program
   void execute();

   protected:

   lunaparse *parser;

   private:
   };

#endif
