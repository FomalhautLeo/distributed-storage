# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
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

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/jace/git/simple-distributed-storage-system

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/jace/git/simple-distributed-storage-system/build

# Include any dependencies generated for this target.
include CacheServer/main/CMakeFiles/CacheServer.exe.dir/depend.make

# Include the progress variables for this target.
include CacheServer/main/CMakeFiles/CacheServer.exe.dir/progress.make

# Include the compile flags for this target's objects.
include CacheServer/main/CMakeFiles/CacheServer.exe.dir/flags.make

CacheServer/main/CMakeFiles/CacheServer.exe.dir/CacheServer.cc.o: CacheServer/main/CMakeFiles/CacheServer.exe.dir/flags.make
CacheServer/main/CMakeFiles/CacheServer.exe.dir/CacheServer.cc.o: ../CacheServer/main/CacheServer.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jace/git/simple-distributed-storage-system/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CacheServer/main/CMakeFiles/CacheServer.exe.dir/CacheServer.cc.o"
	cd /home/jace/git/simple-distributed-storage-system/build/CacheServer/main && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/CacheServer.exe.dir/CacheServer.cc.o -c /home/jace/git/simple-distributed-storage-system/CacheServer/main/CacheServer.cc

CacheServer/main/CMakeFiles/CacheServer.exe.dir/CacheServer.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/CacheServer.exe.dir/CacheServer.cc.i"
	cd /home/jace/git/simple-distributed-storage-system/build/CacheServer/main && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/jace/git/simple-distributed-storage-system/CacheServer/main/CacheServer.cc > CMakeFiles/CacheServer.exe.dir/CacheServer.cc.i

CacheServer/main/CMakeFiles/CacheServer.exe.dir/CacheServer.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/CacheServer.exe.dir/CacheServer.cc.s"
	cd /home/jace/git/simple-distributed-storage-system/build/CacheServer/main && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/jace/git/simple-distributed-storage-system/CacheServer/main/CacheServer.cc -o CMakeFiles/CacheServer.exe.dir/CacheServer.cc.s

# Object files for target CacheServer.exe
CacheServer_exe_OBJECTS = \
"CMakeFiles/CacheServer.exe.dir/CacheServer.cc.o"

# External object files for target CacheServer.exe
CacheServer_exe_EXTERNAL_OBJECTS =

CacheServer/main/CacheServer.exe: CacheServer/main/CMakeFiles/CacheServer.exe.dir/CacheServer.cc.o
CacheServer/main/CacheServer.exe: CacheServer/main/CMakeFiles/CacheServer.exe.dir/build.make
CacheServer/main/CacheServer.exe: CacheServer/libCacheServer.so
CacheServer/main/CacheServer.exe: Config/libConfig.so
CacheServer/main/CacheServer.exe: CacheServer/main/CMakeFiles/CacheServer.exe.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/jace/git/simple-distributed-storage-system/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable CacheServer.exe"
	cd /home/jace/git/simple-distributed-storage-system/build/CacheServer/main && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/CacheServer.exe.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CacheServer/main/CMakeFiles/CacheServer.exe.dir/build: CacheServer/main/CacheServer.exe

.PHONY : CacheServer/main/CMakeFiles/CacheServer.exe.dir/build

CacheServer/main/CMakeFiles/CacheServer.exe.dir/clean:
	cd /home/jace/git/simple-distributed-storage-system/build/CacheServer/main && $(CMAKE_COMMAND) -P CMakeFiles/CacheServer.exe.dir/cmake_clean.cmake
.PHONY : CacheServer/main/CMakeFiles/CacheServer.exe.dir/clean

CacheServer/main/CMakeFiles/CacheServer.exe.dir/depend:
	cd /home/jace/git/simple-distributed-storage-system/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/jace/git/simple-distributed-storage-system /home/jace/git/simple-distributed-storage-system/CacheServer/main /home/jace/git/simple-distributed-storage-system/build /home/jace/git/simple-distributed-storage-system/build/CacheServer/main /home/jace/git/simple-distributed-storage-system/build/CacheServer/main/CMakeFiles/CacheServer.exe.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CacheServer/main/CMakeFiles/CacheServer.exe.dir/depend
