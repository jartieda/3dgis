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
CMAKE_SOURCE_DIR = /home/jatrigueros/programas/RALine/osggis

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/jatrigueros/programas/RALine/osggis

# Include any dependencies generated for this target.
include src/osggis_build/CMakeFiles/app_osggis_build.dir/depend.make

# Include the progress variables for this target.
include src/osggis_build/CMakeFiles/app_osggis_build.dir/progress.make

# Include the compile flags for this target's objects.
include src/osggis_build/CMakeFiles/app_osggis_build.dir/flags.make

src/osggis_build/CMakeFiles/app_osggis_build.dir/osggis_build.o: src/osggis_build/CMakeFiles/app_osggis_build.dir/flags.make
src/osggis_build/CMakeFiles/app_osggis_build.dir/osggis_build.o: src/osggis_build/osggis_build.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/jatrigueros/programas/RALine/osggis/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object src/osggis_build/CMakeFiles/app_osggis_build.dir/osggis_build.o"
	cd /home/jatrigueros/programas/RALine/osggis/src/osggis_build && /usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/app_osggis_build.dir/osggis_build.o -c /home/jatrigueros/programas/RALine/osggis/src/osggis_build/osggis_build.cpp

src/osggis_build/CMakeFiles/app_osggis_build.dir/osggis_build.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/app_osggis_build.dir/osggis_build.i"
	cd /home/jatrigueros/programas/RALine/osggis/src/osggis_build && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/jatrigueros/programas/RALine/osggis/src/osggis_build/osggis_build.cpp > CMakeFiles/app_osggis_build.dir/osggis_build.i

src/osggis_build/CMakeFiles/app_osggis_build.dir/osggis_build.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/app_osggis_build.dir/osggis_build.s"
	cd /home/jatrigueros/programas/RALine/osggis/src/osggis_build && /usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/jatrigueros/programas/RALine/osggis/src/osggis_build/osggis_build.cpp -o CMakeFiles/app_osggis_build.dir/osggis_build.s

src/osggis_build/CMakeFiles/app_osggis_build.dir/osggis_build.o.requires:
.PHONY : src/osggis_build/CMakeFiles/app_osggis_build.dir/osggis_build.o.requires

src/osggis_build/CMakeFiles/app_osggis_build.dir/osggis_build.o.provides: src/osggis_build/CMakeFiles/app_osggis_build.dir/osggis_build.o.requires
	$(MAKE) -f src/osggis_build/CMakeFiles/app_osggis_build.dir/build.make src/osggis_build/CMakeFiles/app_osggis_build.dir/osggis_build.o.provides.build
.PHONY : src/osggis_build/CMakeFiles/app_osggis_build.dir/osggis_build.o.provides

src/osggis_build/CMakeFiles/app_osggis_build.dir/osggis_build.o.provides.build: src/osggis_build/CMakeFiles/app_osggis_build.dir/osggis_build.o
.PHONY : src/osggis_build/CMakeFiles/app_osggis_build.dir/osggis_build.o.provides.build

# Object files for target app_osggis_build
app_osggis_build_OBJECTS = \
"CMakeFiles/app_osggis_build.dir/osggis_build.o"

# External object files for target app_osggis_build
app_osggis_build_EXTERNAL_OBJECTS =

bin/osggis_buildd: src/osggis_build/CMakeFiles/app_osggis_build.dir/osggis_build.o
bin/osggis_buildd: src/osggis_build/CMakeFiles/app_osggis_build.dir/build.make
bin/osggis_buildd: src/osggis_build/CMakeFiles/app_osggis_build.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable ../../bin/osggis_buildd"
	cd /home/jatrigueros/programas/RALine/osggis/src/osggis_build && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/app_osggis_build.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/osggis_build/CMakeFiles/app_osggis_build.dir/build: bin/osggis_buildd
.PHONY : src/osggis_build/CMakeFiles/app_osggis_build.dir/build

src/osggis_build/CMakeFiles/app_osggis_build.dir/requires: src/osggis_build/CMakeFiles/app_osggis_build.dir/osggis_build.o.requires
.PHONY : src/osggis_build/CMakeFiles/app_osggis_build.dir/requires

src/osggis_build/CMakeFiles/app_osggis_build.dir/clean:
	cd /home/jatrigueros/programas/RALine/osggis/src/osggis_build && $(CMAKE_COMMAND) -P CMakeFiles/app_osggis_build.dir/cmake_clean.cmake
.PHONY : src/osggis_build/CMakeFiles/app_osggis_build.dir/clean

src/osggis_build/CMakeFiles/app_osggis_build.dir/depend:
	cd /home/jatrigueros/programas/RALine/osggis && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/jatrigueros/programas/RALine/osggis /home/jatrigueros/programas/RALine/osggis/src/osggis_build /home/jatrigueros/programas/RALine/osggis /home/jatrigueros/programas/RALine/osggis/src/osggis_build /home/jatrigueros/programas/RALine/osggis/src/osggis_build/CMakeFiles/app_osggis_build.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/osggis_build/CMakeFiles/app_osggis_build.dir/depend

