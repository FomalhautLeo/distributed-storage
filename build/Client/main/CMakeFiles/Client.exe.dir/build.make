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
include Client/main/CMakeFiles/Client.exe.dir/depend.make

# Include the progress variables for this target.
include Client/main/CMakeFiles/Client.exe.dir/progress.make

# Include the compile flags for this target's objects.
include Client/main/CMakeFiles/Client.exe.dir/flags.make

Client/main/CMakeFiles/Client.exe.dir/Client.cc.o: Client/main/CMakeFiles/Client.exe.dir/flags.make
Client/main/CMakeFiles/Client.exe.dir/Client.cc.o: ../Client/main/Client.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jace/git/simple-distributed-storage-system/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object Client/main/CMakeFiles/Client.exe.dir/Client.cc.o"
	cd /home/jace/git/simple-distributed-storage-system/build/Client/main && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Client.exe.dir/Client.cc.o -c /home/jace/git/simple-distributed-storage-system/Client/main/Client.cc

Client/main/CMakeFiles/Client.exe.dir/Client.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Client.exe.dir/Client.cc.i"
	cd /home/jace/git/simple-distributed-storage-system/build/Client/main && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/jace/git/simple-distributed-storage-system/Client/main/Client.cc > CMakeFiles/Client.exe.dir/Client.cc.i

Client/main/CMakeFiles/Client.exe.dir/Client.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Client.exe.dir/Client.cc.s"
	cd /home/jace/git/simple-distributed-storage-system/build/Client/main && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/jace/git/simple-distributed-storage-system/Client/main/Client.cc -o CMakeFiles/Client.exe.dir/Client.cc.s

# Object files for target Client.exe
Client_exe_OBJECTS = \
"CMakeFiles/Client.exe.dir/Client.cc.o"

# External object files for target Client.exe
Client_exe_EXTERNAL_OBJECTS =

Client/main/Client.exe: Client/main/CMakeFiles/Client.exe.dir/Client.cc.o
Client/main/Client.exe: Client/main/CMakeFiles/Client.exe.dir/build.make
Client/main/Client.exe: Client/libClient.so
Client/main/Client.exe: Config/libConfig.so
Client/main/Client.exe: Client/main/CMakeFiles/Client.exe.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/jace/git/simple-distributed-storage-system/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable Client.exe"
	cd /home/jace/git/simple-distributed-storage-system/build/Client/main && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Client.exe.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
Client/main/CMakeFiles/Client.exe.dir/build: Client/main/Client.exe

.PHONY : Client/main/CMakeFiles/Client.exe.dir/build

Client/main/CMakeFiles/Client.exe.dir/clean:
	cd /home/jace/git/simple-distributed-storage-system/build/Client/main && $(CMAKE_COMMAND) -P CMakeFiles/Client.exe.dir/cmake_clean.cmake
.PHONY : Client/main/CMakeFiles/Client.exe.dir/clean

Client/main/CMakeFiles/Client.exe.dir/depend:
	cd /home/jace/git/simple-distributed-storage-system/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/jace/git/simple-distributed-storage-system /home/jace/git/simple-distributed-storage-system/Client/main /home/jace/git/simple-distributed-storage-system/build /home/jace/git/simple-distributed-storage-system/build/Client/main /home/jace/git/simple-distributed-storage-system/build/Client/main/CMakeFiles/Client.exe.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : Client/main/CMakeFiles/Client.exe.dir/depend
