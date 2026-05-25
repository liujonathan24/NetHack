# Install script for directory: /scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/util

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "RelWithDebInfo")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/home/jl0796/nethackdir.nle/makedefs" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/jl0796/nethackdir.nle/makedefs")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/home/jl0796/nethackdir.nle/makedefs"
         RPATH "")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/jl0796/nethackdir.nle/makedefs")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/jl0796/nethackdir.nle" TYPE EXECUTABLE MESSAGE_LAZY FILES "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/makedefs")
  if(EXISTS "$ENV{DESTDIR}/home/jl0796/nethackdir.nle/makedefs" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/jl0796/nethackdir.nle/makedefs")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/home/jl0796/nethackdir.nle/makedefs")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/home/jl0796/nethackdir.nle/dgn_comp" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/jl0796/nethackdir.nle/dgn_comp")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/home/jl0796/nethackdir.nle/dgn_comp"
         RPATH "")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/jl0796/nethackdir.nle/dgn_comp")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/jl0796/nethackdir.nle" TYPE EXECUTABLE MESSAGE_LAZY FILES "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/dgn_comp")
  if(EXISTS "$ENV{DESTDIR}/home/jl0796/nethackdir.nle/dgn_comp" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/jl0796/nethackdir.nle/dgn_comp")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/home/jl0796/nethackdir.nle/dgn_comp")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/home/jl0796/nethackdir.nle/lev_comp" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/jl0796/nethackdir.nle/lev_comp")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/home/jl0796/nethackdir.nle/lev_comp"
         RPATH "")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/jl0796/nethackdir.nle/lev_comp")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/jl0796/nethackdir.nle" TYPE EXECUTABLE MESSAGE_LAZY FILES "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/lev_comp")
  if(EXISTS "$ENV{DESTDIR}/home/jl0796/nethackdir.nle/lev_comp" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/jl0796/nethackdir.nle/lev_comp")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/home/jl0796/nethackdir.nle/lev_comp")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/home/jl0796/nethackdir.nle/dlb" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/jl0796/nethackdir.nle/dlb")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/home/jl0796/nethackdir.nle/dlb"
         RPATH "")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/jl0796/nethackdir.nle/dlb")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/jl0796/nethackdir.nle" TYPE EXECUTABLE MESSAGE_LAZY FILES "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/dlb")
  if(EXISTS "$ENV{DESTDIR}/home/jl0796/nethackdir.nle/dlb" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/jl0796/nethackdir.nle/dlb")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/home/jl0796/nethackdir.nle/dlb")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/home/jl0796/nethackdir.nle/recover" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/jl0796/nethackdir.nle/recover")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/home/jl0796/nethackdir.nle/recover"
         RPATH "")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/jl0796/nethackdir.nle/recover")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/jl0796/nethackdir.nle" TYPE EXECUTABLE MESSAGE_LAZY FILES "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/util/recover")
  if(EXISTS "$ENV{DESTDIR}/home/jl0796/nethackdir.nle/recover" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/jl0796/nethackdir.nle/recover")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/home/jl0796/nethackdir.nle/recover")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/jl0796/nethackdir.nle/man/makedefs.6;/home/jl0796/nethackdir.nle/man/dgn_comp.6;/home/jl0796/nethackdir.nle/man/lev_comp.6;/home/jl0796/nethackdir.nle/man/dlb.6;/home/jl0796/nethackdir.nle/man/recover.6;/home/jl0796/nethackdir.nle/man/nethack.6")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/jl0796/nethackdir.nle/man" TYPE FILE MESSAGE_LAZY FILES
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/doc/makedefs.6"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/doc/dgn_comp.6"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/doc/lev_comp.6"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/doc/dlb.6"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/doc/recover.6"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/doc/nethack.6"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/jl0796/nethackdir.nle/doc/makedefs.txt;/home/jl0796/nethackdir.nle/doc/dgn_comp.txt;/home/jl0796/nethackdir.nle/doc/lev_comp.txt;/home/jl0796/nethackdir.nle/doc/dlb.txt;/home/jl0796/nethackdir.nle/doc/recover.txt;/home/jl0796/nethackdir.nle/doc/nethack.txt")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/jl0796/nethackdir.nle/doc" TYPE FILE MESSAGE_LAZY FILES
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/doc/makedefs.txt"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/doc/dgn_comp.txt"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/doc/lev_comp.txt"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/doc/dlb.txt"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/doc/recover.txt"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/doc/nethack.txt"
    )
endif()

