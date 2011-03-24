SHELL	= sh

PRGS	= db2db eval\
	  mpfptest\
	  test

SRCS	= $(PRGS:=.cpp)
OBJS	= $(PRGS:=.o)

RM	= rm -rf

.SUFFIXES: .cpp

.cpp.o:
	$(COMPILER) $(OPTS) -c -I.. $*.cpp
	$(COMPILER) $(OPTS) -o $* $*.o -L.. -lMiniSFX -lMini $(LINK) -ljpeg -lpng -lGL -lGLU -lm

trap:

all:	$(OBJS)

clean:
	@-$(RM) $(OBJS) ii_files

tidy:	clean
	@-$(RM) *~
	@-$(RM) $(PRGS)
