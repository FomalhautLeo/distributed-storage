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
include Config/CMakeFiles/Config.dir/depend.make

# Include the progress variables for this target.
include Config/CMakeFiles/Config.dir/progress.make

# Include the compile flags for this target's objects.
include Config/CMakeFiles/Config.dir/flags.make

Config/CMakeFiles/Config.dir/src/Logger.cc.o: Config/CMakeFiles/Config.dir/flags.make
Config/CMakeFiles/Config.dir/src/Logger.cc.o: ../Config/src/Logger.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jace/git/simple-distributed-storage-system/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object Config/CMakeFiles/Config.dir/src/Logger.cc.o"
	cd /home/jace/git/simple-distributed-storage-system/build/Config && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Config.dir/src/Logger.cc.o -c /home/jace/git/simple-distributed-storage-system/Config/src/Logger.cc

Config/CMakeFiles/Config.dir/src/Logger.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Config.dir/src/Logger.cc.i"
	cd /home/jace/git/simple-distributed-storage-system/build/Config && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/jace/git/simple-distributed-storage-system/Config/src/Logger.cc > CMakeFiles/Config.dir/src/Logger.cc.i

Config/CMakeFiles/Config.dir/src/Logger.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Config.dir/src/Logger.cc.s"
	cd /home/jace/git/simple-distributed-storage-system/build/Config && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/jace/git/simple-distributed-storage-system/Config/src/Logger.cc -o CMakeFiles/Config.dir/src/Logger.cc.s

Config/CMakeFiles/Config.dir/src/MessageType.cc.o: Config/CMakeFiles/Config.dir/flags.make
Config/CMakeFiles/Config.dir/src/MessageType.cc.o: ../Config/src/MessageType.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jace/git/simple-distributed-storage-system/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object Config/CMakeFiles/Config.dir/src/MessageType.cc.o"
	cd /home/jace/git/simple-distributed-storage-system/build/Config && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Config.dir/src/MessageType.cc.o -c /home/jace/git/simple-distributed-storage-system/Config/src/MessageType.cc

Config/CMakeFiles/Config.dir/src/MessageType.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Config.dir/src/MessageType.cc.i"
	cd /home/jace/git/simple-distributed-storage-system/build/Config && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/jace/git/simple-distributed-storage-system/Config/src/MessageType.cc > CMakeFiles/Config.dir/src/MessageType.cc.i

Config/CMakeFiles/Config.dir/src/MessageType.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Config.dir/src/MessageType.cc.s"
	cd /home/jace/git/simple-distributed-storage-system/build/Config && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/jace/git/simple-distributed-storage-system/Config/src/MessageType.cc -o CMakeFiles/Config.dir/src/MessageType.cc.s

Config/CMakeFiles/Config.dir/src/NetworkInfo.cc.o: Config/CMakeFiles/Config.dir/flags.make
Config/CMakeFiles/Config.dir/src/NetworkInfo.cc.o: ../Config/src/NetworkInfo.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jace/git/simple-distributed-storage-system/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object Config/CMakeFiles/Config.dir/src/NetworkInfo.cc.o"
	cd /home/jace/git/simple-distributed-storage-system/build/Config && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Config.dir/src/NetworkInfo.cc.o -c /home/jace/git/simple-distributed-storage-system/Config/src/NetworkInfo.cc

Config/CMakeFiles/Config.dir/src/NetworkInfo.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Config.dir/src/NetworkInfo.cc.i"
	cd /home/jace/git/simple-distributed-storage-system/build/Config && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/jace/git/simple-distributed-storage-system/Config/src/NetworkInfo.cc > CMakeFiles/Config.dir/src/NetworkInfo.cc.i

Config/CMakeFiles/Config.dir/src/NetworkInfo.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Config.dir/src/NetworkInfo.cc.s"
	cd /home/jace/git/simple-distributed-storage-system/build/Config && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/jace/git/simple-distributed-storage-system/Config/src/NetworkInfo.cc -o CMakeFiles/Config.dir/src/NetworkInfo.cc.s

# Object files for target Config
Config_OBJECTS = \
"CMakeFiles/Config.dir/src/Logger.cc.o" \
"CMakeFiles/Config.dir/src/MessageType.cc.o" \
"CMakeFiles/Config.dir/src/NetworkInfo.cc.o"

# External object files for target Config
Config_EXTERNAL_OBJECTS =

Config/libConfig.so: Config/CMakeFiles/Config.dir/src/Logger.cc.o
Config/libConfig.so: Config/CMakeFiles/Config.dir/src/MessageType.cc.o
Config/libConfig.so: Config/CMakeFiles/Config.dir/src/NetworkInfo.cc.o
Config/libConfig.so: Config/CMakeFiles/Config.dir/build.make
Config/libConfig.so: Config/CMakeFiles/Config.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/jace/git/simple-distributed-storage-system/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX shared library libConfig.so"
	cd /home/jace/git/simple-distributed-storage-system/build/Config && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Config.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
Config/CMakeFiles/Config.dir/build: Config/libConfig.so

.PHONY : Config/CMakeFiles/Config.dir/build

Config/CMakeFiles/Config.dir/clean:
	cd /home/jace/git/simple-distributed-storage-system/build/Config && $(CMAKE_COMMAND) -P CMakeFiles/Config.dir/cmake_clean.cmake
.PHONY : Config/CMakeFiles/Config.dir/clean

Config/CMakeFiles/Config.dir/depend:
	cd /home/jace/git/simple-distributed-storage-system/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/jace/git/simple-distributed-storage-system /home/jace/git/simple-distributed-storage-system/Config /home/jace/git/simple-distributed-storage-system/build /home/jace/git/simple-distributed-storage-system/build/Config /home/jace/git/simple-distributed-storage-system/build/Config/CMakeFiles/Config.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : Config/CMakeFiles/Config.dir/depend
