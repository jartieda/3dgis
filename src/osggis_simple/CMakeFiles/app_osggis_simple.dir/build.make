# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canoncical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = /usr/bin/cmake-gui

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/jatrigueros/programas/osggis_googlecode/3dgis

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/jatrigueros/programas/osggis_googlecode/3dgis

# Include any dependencies generated for this target.
include src/osggis_simple/CMakeFiles/app_osggis_simple.dir/depend.make

# Include the progress variables for this target.
include src/osggis_simple/CMakeFiles/app_osggis_simple.dir/progress.make

# Include the compile flags for this target's objects.
include src/osggis_simple/CMakeFiles/app_osggis_simple.dir/flags.make

src/osggis_simple/CMakeFiles/app_osggis_simple.dir/osggis_simple.o: src/osggis_simple/CMakeFiles/app_osggis_simple.dir/flags.make
src/osggis_simple/CMakeFiles/app_osggis_simple.dir/osggis_simple.o: src/osggis_simple/osggis_simple.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/jatrigueros/programas/osggis_googlecode/3dgis/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object src/osggis_simple/CMakeFiles/app_osggis_simple.dir/osggis_simple.o"
	cd /home/jatrigueros/programas/osggis_googlecode/3dgis/src/osggis_simple && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/app_osggis_simple.dir/osggis_simple.o -c /home/jatrigueros/programas/osggis_googlecode/3dgis/src/osggis_simple/osggis_simple.cpp

src/osggis_simple/CMakeFiles/app_osggis_simple.dir/osggis_simple.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/app_osggis_simple.dir/osggis_simple.i"
	cd /home/jatrigueros/programas/osggis_googlecode/3dgis/src/osggis_simple && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/jatrigueros/programas/osggis_googlecode/3dgis/src/osggis_simple/osggis_simple.cpp > CMakeFiles/app_osggis_simple.dir/osggis_simple.i

src/osggis_simple/CMakeFiles/app_osggis_simple.dir/osggis_simple.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/app_osggis_simple.dir/osggis_simple.s"
	cd /home/jatrigueros/programas/osggis_googlecode/3dgis/src/osggis_simple && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/jatrigueros/programas/osggis_googlecode/3dgis/src/osggis_simple/osggis_simple.cpp -o CMakeFiles/app_osggis_simple.dir/osggis_simple.s

src/osggis_simple/CMakeFiles/app_osggis_simple.dir/osggis_simple.o.requires:
.PHONY : src/osggis_simple/CMakeFiles/app_osggis_simple.dir/osggis_simple.o.requires

src/osggis_simple/CMakeFiles/app_osggis_simple.dir/osggis_simple.o.provides: src/osggis_simple/CMakeFiles/app_osggis_simple.dir/osggis_simple.o.requires
	$(MAKE) -f src/osggis_simple/CMakeFiles/app_osggis_simple.dir/build.make src/osggis_simple/CMakeFiles/app_osggis_simple.dir/osggis_simple.o.provides.build
.PHONY : src/osggis_simple/CMakeFiles/app_osggis_simple.dir/osggis_simple.o.provides

src/osggis_simple/CMakeFiles/app_osggis_simple.dir/osggis_simple.o.provides.build: src/osggis_simple/CMakeFiles/app_osggis_simple.dir/osggis_simple.o
.PHONY : src/osggis_simple/CMakeFiles/app_osggis_simple.dir/osggis_simple.o.provides.build

# Object files for target app_osggis_simple
app_osggis_simple_OBJECTS = \
"CMakeFiles/app_osggis_simple.dir/osggis_simple.o"

# External object files for target app_osggis_simple
app_osggis_simple_EXTERNAL_OBJECTS =

bin/osggis_simpled: src/osggis_simple/CMakeFiles/app_osggis_simple.dir/osggis_simple.o
bin/osggis_simpled: src/osggis_simple/CMakeFiles/app_osggis_simple.dir/build.make
bin/osggis_simpled: src/osggis_simple/CMakeFiles/app_osggis_simple.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable ../../bin/osggis_simpled"
	cd /home/jatrigueros/programas/osggis_googlecode/3dgis/src/osggis_simple && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/app_osggis_simple.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/osggis_simple/CMakeFiles/app_osggis_simple.dir/build: bin/osggis_simpled
.PHONY : src/osggis_simple/CMakeFiles/app_osggis_simple.dir/build

src/osggis_simple/CMakeFiles/app_osggis_simple.dir/requires: src/osggis_simple/CMakeFiles/app_osggis_simple.dir/osggis_simple.o.requires
.PHONY : src/osggis_simple/CMakeFiles/app_osggis_simple.dir/requires

src/osggis_simple/CMakeFiles/app_osggis_simple.dir/clean:
	cd /home/jatrigueros/programas/osggis_googlecode/3dgis/src/osggis_simple && $(CMAKE_COMMAND) -P CMakeFiles/app_osggis_simple.dir/cmake_clean.cmake
.PHONY : src/osggis_simple/CMakeFiles/app_osggis_simple.dir/clean

src/osggis_simple/CMakeFiles/app_osggis_simple.dir/depend:
	cd /home/jatrigueros/programas/osggis_googlecode/3dgis && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/jatrigueros/programas/osggis_googlecode/3dgis /home/jatrigueros/programas/osggis_googlecode/3dgis/src/osggis_simple /home/jatrigueros/programas/osggis_googlecode/3dgis /home/jatrigueros/programas/osggis_googlecode/3dgis/src/osggis_simple /home/jatrigueros/programas/osggis_googlecode/3dgis/src/osggis_simple/CMakeFiles/app_osggis_simple.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/osggis_simple/CMakeFiles/app_osggis_simple.dir/depend

