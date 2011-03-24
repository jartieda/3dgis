SHELL	= sh
COMPILER = g++
MODS	= mini miniOGL\
	  miniv3f miniv3d miniv4f miniv4d\
	  minimath minimpfp minitime miniio minihsv miniutm\
	  ministub minitile miniload\
	  minilayer miniterrain miniearth\
	  minicache minishader miniwarp miniray\
	  ministrip minipoint minitext minisky miniglobe\
	  minitree minibrick minilod\
	  minigeom minimesh miniproj\
	  pnmbase pnmsample\
	  database datafill\
	  datacloud datacache datagrid\
	  datacalc dataparse\
	  lunascan lunaparse lunacode

VMODS	= viewerbase\
	  threadbase curlbase\
	  jpegbase pngbase squishbase greycbase\
	  convbase imgbase

SRCS	= $(MODS:=.cpp)
OBJS	= $(MODS:=.o)

VSRCS	= $(VMODS:=.cpp)
VOBJS	= $(VMODS:=.o)

AR	= ar -crs

CP	= cp -rfp
LN	= ln -fs
RM	= rm -rf

ZIP	= zip -qr9

.SUFFIXES: .cpp

.cpp.o:
	$(COMPILER) -I.. -I/usr/include -I/c/MinGW/include/GL -DNOOGL $(OPTS) -g -c $*.cpp



all:	lib libsfx example viewer tools

lib:	$(OBJS)
	@-$(AR) libMini.a $(OBJS)

libsfx:	$(VOBJS)
	@-$(AR) libMiniSFX.a $(VOBJS)

example: lib
	$(COMPILER) $(OPTS) -o example -I.. -I/usr/include example.cpp -L. -L/usr/lib -lMini $(LINK) -lGL -lGLU -lm

stubtest: lib
	$(COMPILER) $(OPTS) -o stubtest -I.. stubtest.cpp -L. -lMini -lm

viewer:	lib libsfx
	$(COMPILER) $(OPTS) -o viewer -I.. viewer.cpp -L. -lMiniSFX -lMini $(LINK) -lGL -lGLU -lpthread -lcurl -ljpeg -lpng -lm

depend:
	@-$(MAKEDEPEND) $(SRCS) >Makefile.d 2>/dev/null

vdepend: depend
	@-$(MAKEDEPEND) $(VSRCS) >>Makefile.d 2>/dev/null

install:
	@-echo "copying libraries to $(INSTALL)/lib"
	@-if [ ! -e $(INSTALL)/lib ]; then mkdir -p $(INSTALL)/lib; fi
	@-$(CP) libMini*.a $(INSTALL)/lib
	@-echo "copying headers to $(INSTALL)/include/mini"
	@-if [ ! -e $(INSTALL)/include/mini ]; then mkdir -p $(INSTALL)/include/mini; fi
	@-$(CP) *.h $(INSTALL)/include/mini
	@-echo "copying data to $(DATA)/mini"
	@-if [ ! -e $(DATA)/mini ]; then mkdir -p $(DATA)/mini; fi
	@-$(CP) data/* $(DATA)/mini

tools:	lib libsfx
	@-(cd tools; make all)

clean:
	@-$(RM) $(OBJS) $(VOBJS) ii_files
	@-(cd tools; make clean)

tidy:	clean
	@-$(RM) *~ Makefile.d Debug* Release* *.ncb *.opt *.plg
	@-$(RM) libMini*.a example stubtest viewer
	@-(cd tools; make tidy)

zip:	tidy
	@-(cd ..; $(ZIP) mini/MINI.zip mini -x \*/.\?\?\* -x \*CVS/\*)

sinclude Makefile.d
