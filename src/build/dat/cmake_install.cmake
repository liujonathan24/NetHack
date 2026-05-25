# Install script for directory: /scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat

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
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/jl0796/nethackdir.nle/nhdat;/home/jl0796/nethackdir.nle/perm;/home/jl0796/nethackdir.nle/record;/home/jl0796/nethackdir.nle/logfile;/home/jl0796/nethackdir.nle/xlogfile")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/jl0796/nethackdir.nle" TYPE FILE MESSAGE_LAZY FILES
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/dat/nhdat"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/dat/perm"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/dat/record"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/dat/logfile"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/build/dat/xlogfile"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/jl0796/nethackdir.nle/dat/help;/home/jl0796/nethackdir.nle/dat/hh;/home/jl0796/nethackdir.nle/dat/cmdhelp;/home/jl0796/nethackdir.nle/dat/keyhelp;/home/jl0796/nethackdir.nle/dat/history;/home/jl0796/nethackdir.nle/dat/opthelp;/home/jl0796/nethackdir.nle/dat/wizhelp;/home/jl0796/nethackdir.nle/dat/bigroom.des;/home/jl0796/nethackdir.nle/dat/castle.des;/home/jl0796/nethackdir.nle/dat/endgame.des;/home/jl0796/nethackdir.nle/dat/gehennom.des;/home/jl0796/nethackdir.nle/dat/knox.des;/home/jl0796/nethackdir.nle/dat/medusa.des;/home/jl0796/nethackdir.nle/dat/mines.des;/home/jl0796/nethackdir.nle/dat/oracle.des;/home/jl0796/nethackdir.nle/dat/sokoban.des;/home/jl0796/nethackdir.nle/dat/tower.des;/home/jl0796/nethackdir.nle/dat/yendor.des;/home/jl0796/nethackdir.nle/dat/Arch.des;/home/jl0796/nethackdir.nle/dat/Barb.des;/home/jl0796/nethackdir.nle/dat/Caveman.des;/home/jl0796/nethackdir.nle/dat/Healer.des;/home/jl0796/nethackdir.nle/dat/Knight.des;/home/jl0796/nethackdir.nle/dat/Monk.des;/home/jl0796/nethackdir.nle/dat/Priest.des;/home/jl0796/nethackdir.nle/dat/Ranger.des;/home/jl0796/nethackdir.nle/dat/Rogue.des;/home/jl0796/nethackdir.nle/dat/Samurai.des;/home/jl0796/nethackdir.nle/dat/Tourist.des;/home/jl0796/nethackdir.nle/dat/Valkyrie.des;/home/jl0796/nethackdir.nle/dat/Wizard.des;/home/jl0796/nethackdir.nle/dat/bogusmon.txt;/home/jl0796/nethackdir.nle/dat/data.base;/home/jl0796/nethackdir.nle/dat/dungeon.def;/home/jl0796/nethackdir.nle/dat/engrave.txt;/home/jl0796/nethackdir.nle/dat/epitaph.txt;/home/jl0796/nethackdir.nle/dat/oracles.txt;/home/jl0796/nethackdir.nle/dat/quest.txt;/home/jl0796/nethackdir.nle/dat/rumors.fal;/home/jl0796/nethackdir.nle/dat/rumors.tru;/home/jl0796/nethackdir.nle/dat/tribute;/home/jl0796/nethackdir.nle/dat/license;/home/jl0796/nethackdir.nle/dat/symbols")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/jl0796/nethackdir.nle/dat" TYPE FILE MESSAGE_LAZY FILES
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/help"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/hh"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/cmdhelp"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/keyhelp"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/history"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/opthelp"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/wizhelp"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/bigroom.des"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/castle.des"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/endgame.des"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/gehennom.des"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/knox.des"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/medusa.des"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/mines.des"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/oracle.des"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/sokoban.des"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/tower.des"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/yendor.des"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/Arch.des"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/Barb.des"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/Caveman.des"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/Healer.des"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/Knight.des"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/Monk.des"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/Priest.des"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/Ranger.des"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/Rogue.des"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/Samurai.des"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/Tourist.des"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/Valkyrie.des"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/Wizard.des"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/bogusmon.txt"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/data.base"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/dungeon.def"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/engrave.txt"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/epitaph.txt"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/oracles.txt"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/quest.txt"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/rumors.fal"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/rumors.tru"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/tribute"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/license"
    "/scratch/gpfs/ZHUANGL/jl0796/PufferLib/vendor/nle/src/dat/symbols"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/jl0796/nethackdir.nle/save/")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/jl0796/nethackdir.nle/save" TYPE DIRECTORY MESSAGE_LAZY FILES "")
endif()

