#!/bin/tcsh -f

set option=$1
set file=$2

if ("$option" == "") then
   echo "usage: tabify -u|t|U|T [<file>]"
   echo " replaces all tabs at the begin of each line with 4-space (-u) or 8-space (-U) and the other way round (-t|T)"
   echo " additionally removes trailing white space, all spaces before a tab, and gets rid of all ctrl-m"
   echo " whithout a specified file argument all .cpp and .h files are treated accordingly"
   exit
endif

set tabify=0

if ("$option" == "-u") set tabify=1
if ("$option" == "-t") set tabify=2

if ("$option" == "-U") set tabify=3
if ("$option" == "-T") set tabify=4

if ($tabify == 0) exit 1

set cr='\0x0a'
set lf='\0x0d'

set tab='	'

set rmctrlm="s/$cr$lf"'$'"/$lf/"
if ($HOSTTYPE == "i386-cygwin") set rmctrlm=""

set rmspc="s/[ $tab]*"'$'"//"
set rmspctab="s/ *$tab/$tab/g"

if ($file == "") then
   if ($tabify == 1) then
      find . \( -name \*.cpp -o -name \*.h \) -exec tabify.sh -t {} \; -exec tabify.sh -u {} \;
   endif
   if ($tabify == 2) then
      find . \( -name \*.cpp -o -name \*.h \) -exec tabify.sh -u {} \; -exec tabify.sh -t {} \;
   endif
   if ($tabify == 3) then
      find . \( -name \*.cpp -o -name \*.h \) -exec tabify.sh -T {} \; -exec tabify.sh -U {} \;
   endif
   if ($tabify == 4) then
      find . \( -name \*.cpp -o -name \*.h \) -exec tabify.sh -U {} \; -exec tabify.sh -T {} \;
   endif
else
   if ($tabify == 1) then
      echo untabifying $file
      sed -e "$rmctrlm" -e "$rmspc" -e "$rmspctab" -e ":l\
                                                       s/^\( *\)$tab/\1    /\
                                                       t l" <$file >$file.tmp
   endif
   if ($tabify == 2) then
      echo tabifying $file
      sed -e "$rmctrlm" -e "$rmspc" -e ":l\
                                        s/^\($tab*\)    /\1$tab/\
                                        t l" <$file >$file.tmp
   endif
   if ($tabify == 3) then
      echo untabifying $file
      sed -e "$rmctrlm" -e "$rmspc" -e "$rmspctab" -e ":l\
                                                       s/^\( *\)$tab/\1        /\
                                                       t l" <$file >$file.tmp
   endif
   if ($tabify == 4) then
      echo tabifying $file
      sed -e "$rmctrlm" -e "$rmspc" -e ":l\
                                        s/^\($tab*\)        /\1$tab/\
                                        t l" <$file >$file.tmp
   endif
   mv -f $file.tmp $file
endif
