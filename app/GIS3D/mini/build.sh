#!/bin/sh -f

#usage: build.sh [rule {option}]

# available rules:
# no rule   -> same as mini
# mini      -> same as lib
# lib       -> build main library
# libsfx    -> build viewer library
# stub      -> build without OpenGL
# example   -> build example
# stubtest  -> build stubtest
# viewer    -> build viewer
# tools     -> build tools
# all       -> build everything
# deps      -> make main dependencies
# vdeps     -> make viewer dependencies
# install   -> make install
# clean     -> remove object files
# tidy      -> clean up all temporary files

# available options:
# noviewer  -> build without viewer
# useopenth -> build with openthreads
# nosquish  -> build without squish
# usegreyc  -> build with greyc
# wall      -> build with all warnings

# additional include directories
set INCLUDE="-I/usr/local/include"

# additional link directories
set LINK="-L/usr/local/lib"

# default installation directory
set INSTALL="/usr/local"

# default data directory
set DATA="/usr/share"

set qcwd=$cwd:as/ /\ /

set rule=$1
#if ($rule == "") set rule="mini"
#if ($rule == "mini") set rule="lib"

set option0=""
set option1=$2
set option2=$3
set option3=$4
set option4=$5
set option5=$6

foreach option ("noviewer" "useopenth" "nosquish" "usegreyc" "wall")
   if ($rule == $option) then
      set rule="lib"
      set option0=$option
   endif
end

set incl=""
if ("$INCLUDE" != "") set incl=" $INCLUDE"

set link=""
if ("$LINK" != "") set link=" $LINK"

set stub=""
if ($rule == "stub") then
   set rule="lib"
   set stub=" -DNOOGL"
endif

set depend="depend"
if ($rule == "vdeps") then
   set rule="deps"
   set depend="vdepend"
endif

if ($rule == "install") then
   if ("$option1" != "") set INSTALL="$option1"
   if ("$option2" != "") set DATA="$option2"
   if ("$INSTALL" == "/usr/local" && "$USER" != "root") then
      echo "installation requires super-user priviledges"
   endif
   exec make INSTALL="$INSTALL" DATA="$DATA" install
endif

set defs=""
unset noviewer
unset useopenth
set usesquish
unset usegreyc
foreach option ("$option0" "$option1" "$option2" "$option3" "$option4" "$option5")
   if ("$option" == "noviewer") then
      set defs="$defs -DNOVIEWER"
      set noviewer
   endif
   if ("$option" == "useopenth") then
      set defs="$defs -DUSEOPENTH"
      set useopenth
   endif
   if ("$option" == "nosquish") then
      set defs="$defs -DNOSQUISH"
      unset usesquish
   endif
   if ("$option" == "usegreyc") then
      set defs="$defs -DUSEGREYC"
      set usegreyc
   endif
   if ("$option" == "wall") then
      set defs="$defs -W -Wall"
   endif
end

if (-e ../deps/freeglut) then
   set incl="$incl -I$qcwd/../deps/freeglut/include"
   set link="$link -L$qcwd/../deps/freeglut"
endif

if ($?useopenth && ! $?noviewer) then
   if (-e ../deps/openthreads) then
      set incl="$incl -I$qcwd/../deps/openthreads/include"
      set link="$link -L$qcwd/../deps/openthreads/lib"
   endif
   set link="$link -lOpenThreads"
endif

if ($?usesquish) then
   if (-e ../deps/squish) then
      set incl="$incl -I$qcwd/../deps/squish"
      set link="$link -L$qcwd/../deps/squish"
   endif
   set link="$link -lsquish"
endif

if ($?usegreyc) then
   if (-e ../deps/greycstoration) then
      set incl="$incl -I$qcwd/../deps/greycstoration"
   endif
endif

if ($rule == "deps") then
   if ($HOSTTYPE == "iris4d") make MAKEDEPEND="CC -M$incl$stub$defs" $depend
   if ($HOSTTYPE == "i386") make MAKEDEPEND="c++ -M -I/usr/X11R6/include$incl$stub$defs" $depend
   if ($HOSTTYPE == "i386-linux") make MAKEDEPEND="c++ -M -I/usr/X11R6/include$incl$stub$defs" $depend
   if ($HOSTTYPE == "i386-cygwin") make MAKEDEPEND="c++ -M -I/usr/X11R6/include$incl$stub$defs" $depend
   if ($HOSTTYPE == "i486") make MAKEDEPEND="c++ -M -I/usr/X11R6/include$incl$stub$defs" $depend
   if ($HOSTTYPE == "i486-linux") make MAKEDEPEND="c++ -M -I/usr/X11R6/include$incl$stub$defs" $depend
   if ($HOSTTYPE == "i586") make MAKEDEPEND="c++ -M -I/usr/X11R6/include$incl$stub$defs" $depend
   if ($HOSTTYPE == "i586-linux") make MAKEDEPEND="c++ -M -I/usr/X11R6/include$incl$stub$defs" $depend
   if ($HOSTTYPE == "i686") make MAKEDEPEND="c++ -M -I/usr/X11R6/include$incl$stub$defs" $depend
   if ($HOSTTYPE == "i686-linux") make MAKEDEPEND="c++ -M -I/usr/X11R6/include$incl$stub$defs" $depend
   if ($HOSTTYPE == "x86_64-linux") make MAKEDEPEND="c++ -M -m64 -I/usr/X11R6/include$incl$stub$defs" $depend
   if ($HOSTTYPE == "powerpc") make MAKEDEPEND="c++ -M -I/usr/X11R6/include$incl$stub$defs" $depend
   if ($HOSTTYPE == "powermac") make MAKEDEPEND="c++ -M$incl$stub$defs" $depend
   if ($HOSTTYPE == "intel-pc") make MAKEDEPEND="c++ -M$incl$stub$defs" $depend
else
   if ($HOSTTYPE == "iris4d") make COMPILER="CC" OPTS="-O3 -mips3 -OPT:Olimit=0 -Wl,-woff84$incl$stub$defs" LINK="-lglut -lX11 -lXm -lXt -lXmu$link" $rule
   if ($HOSTTYPE == "i386") make COMPILER="c++" OPTS="-O3 -I/usr/X11R6/include$incl$stub$defs" LINK="-lglut -lGLU -L/usr/X11R6/lib -lX11$link" $rule
   if ($HOSTTYPE == "i386-linux") make COMPILER="c++" OPTS="-O3 -I/usr/X11R6/include$incl$stub$defs" LINK="-lglut -lGLU -L/usr/X11R6/lib -lX11$link" $rule
   if ($HOSTTYPE == "i386-cygwin") make COMPILER="c++" OPTS="-O3 -I/usr/X11R6/include$incl$stub$defs" LINK="-lglut -lGLU -L/usr/X11R6/lib -lX11$link" $rule
   if ($HOSTTYPE == "i486") make COMPILER="c++" OPTS="-O3 -I/usr/X11R6/include$incl$stub$defs" LINK="-lglut -lGLU -L/usr/X11R6/lib -lX11$link" $rule
   if ($HOSTTYPE == "i486-linux") make COMPILER="c++" OPTS="-O3 -I/usr/X11R6/include$incl$stub$defs" LINK="-lglut -lGLU -L/usr/X11R6/lib -lX11$link" $rule
   if ($HOSTTYPE == "i586") make COMPILER="c++" OPTS="-O3 -I/usr/X11R6/include$incl$stub$defs" LINK="-lglut -lGLU -L/usr/X11R6/lib -lX11$link" $rule
   if ($HOSTTYPE == "i586-linux") make COMPILER="c++" OPTS="-O3 -I/usr/X11R6/include$incl$stub$defs" LINK="-lglut -lGLU -L/usr/X11R6/lib -lX11$link" $rule
   if ($HOSTTYPE == "i686") make COMPILER="c++" OPTS="-O3 -I/usr/X11R6/include$incl$stub$defs" LINK="-lglut -lGLU -L/usr/X11R6/lib -lX11$link" $rule
   if ($HOSTTYPE == "i686-linux") make COMPILER="c++" OPTS="-O3 -I/usr/X11R6/include$incl$stub$defs" LINK="-lglut -lGLU -L/usr/X11R6/lib -lX11$link" $rule
   if ($HOSTTYPE == "x86_64-linux") make COMPILER="c++" OPTS="-m64 -O3 -I/usr/X11R6/include$incl$stub$defs" LINK="-lglut -lGLU -L/usr/X11R6/lib64 -lX11$link" $rule
   if ($HOSTTYPE == "powerpc") make COMPILER="c++" OPTS="-O3 -I/usr/X11R6/include$incl$stub$defs" LINK="-lglut -lGLU -L/usr/X11R6/lib -lX11$link" $rule
   if ($HOSTTYPE == "powermac") make COMPILER="c++" OPTS="-O3$incl$stub$defs" LINK="-Wl,-w -L/System/Library/Frameworks/OpenGL.framework/Libraries -framework OpenGL -framework GLUT -lobjc$link" $rule
   if ($HOSTTYPE == "intel-pc") make COMPILER="c++" OPTS="-O3$incl$stub$defs" LINK="-Wl,-w -L/System/Library/Frameworks/OpenGL.framework/Libraries -framework OpenGL -framework GLUT -lobjc$link" $rule
endif
