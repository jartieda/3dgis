# Install script for directory: /home/jatrigueros/programas/osggis_googlecode/3dgis/src

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr/local")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "Debug")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "1")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  INCLUDE("/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/cmake_install.cmake")
  INCLUDE("/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGISProjects/cmake_install.cmake")
  INCLUDE("/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osggis_build/cmake_install.cmake")
  INCLUDE("/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osggis_makelayer/cmake_install.cmake")
  INCLUDE("/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osggis_simple/cmake_install.cmake")
  INCLUDE("/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osggis_viewer/cmake_install.cmake")
  INCLUDE("/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osggis_mapper/cmake_install.cmake")
  INCLUDE("/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osggis_drape/cmake_install.cmake")

ENDIF(NOT CMAKE_INSTALL_LOCAL_ONLY)

