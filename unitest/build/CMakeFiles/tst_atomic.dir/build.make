# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.20

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /home/james/cmake-3.20.6-linux-x86_64/bin/cmake

# The command to remove a file.
RM = /home/james/cmake-3.20.6-linux-x86_64/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/james/work/rte_demo/unitest

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/james/work/rte_demo/unitest/build

# Include any dependencies generated for this target.
include CMakeFiles/tst_atomic.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/tst_atomic.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/tst_atomic.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/tst_atomic.dir/flags.make

CMakeFiles/tst_atomic.dir/tst_atomic.cc.o: CMakeFiles/tst_atomic.dir/flags.make
CMakeFiles/tst_atomic.dir/tst_atomic.cc.o: ../tst_atomic.cc
CMakeFiles/tst_atomic.dir/tst_atomic.cc.o: CMakeFiles/tst_atomic.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/james/work/rte_demo/unitest/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/tst_atomic.dir/tst_atomic.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/tst_atomic.dir/tst_atomic.cc.o -MF CMakeFiles/tst_atomic.dir/tst_atomic.cc.o.d -o CMakeFiles/tst_atomic.dir/tst_atomic.cc.o -c /home/james/work/rte_demo/unitest/tst_atomic.cc

CMakeFiles/tst_atomic.dir/tst_atomic.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tst_atomic.dir/tst_atomic.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/james/work/rte_demo/unitest/tst_atomic.cc > CMakeFiles/tst_atomic.dir/tst_atomic.cc.i

CMakeFiles/tst_atomic.dir/tst_atomic.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tst_atomic.dir/tst_atomic.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/james/work/rte_demo/unitest/tst_atomic.cc -o CMakeFiles/tst_atomic.dir/tst_atomic.cc.s

# Object files for target tst_atomic
tst_atomic_OBJECTS = \
"CMakeFiles/tst_atomic.dir/tst_atomic.cc.o"

# External object files for target tst_atomic
tst_atomic_EXTERNAL_OBJECTS =

tst_atomic: CMakeFiles/tst_atomic.dir/tst_atomic.cc.o
tst_atomic: CMakeFiles/tst_atomic.dir/build.make
tst_atomic: CMakeFiles/tst_atomic.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/james/work/rte_demo/unitest/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable tst_atomic"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/tst_atomic.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/tst_atomic.dir/build: tst_atomic
.PHONY : CMakeFiles/tst_atomic.dir/build

CMakeFiles/tst_atomic.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/tst_atomic.dir/cmake_clean.cmake
.PHONY : CMakeFiles/tst_atomic.dir/clean

CMakeFiles/tst_atomic.dir/depend:
	cd /home/james/work/rte_demo/unitest/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/james/work/rte_demo/unitest /home/james/work/rte_demo/unitest /home/james/work/rte_demo/unitest/build /home/james/work/rte_demo/unitest/build /home/james/work/rte_demo/unitest/build/CMakeFiles/tst_atomic.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/tst_atomic.dir/depend

