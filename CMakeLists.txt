# The CMakeLists for the whole project.
cmake_minimum_required(VERSION 3.12)
message(STATUS "cmake version ${CMAKE_VERSION}")

# Parameters of Install Directory
include(GNUInstallDirs)

# Project and Build Options
project(CacheServer LANGUAGES CXX)

option(DEBUG "Turn debug options on" OFF)

# Compiler and Linker Settings
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -pthread")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-undefined,error")

if(DEBUG)
    set(CMAKE_VERBOSE_MAKEFILE ON)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -ggdb3")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif()

# Build Target CacheServer
add_subdirectory(CacheServer)
# Build Target Client
add_subdirectory(Client)
# Build Target Config
add_subdirectory(Config)
# Build Target Master
add_subdirectory(Master)

# install path
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR})

#Generate and install configure files
include(cmake/StorageCreateConfig.cmake)