# Install script for directory: /home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGISProjects

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

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  IF(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libosgGISProjectsd.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libosgGISProjectsd.so")
    FILE(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libosgGISProjectsd.so"
         RPATH "")
  ENDIF()
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/jatrigueros/programas/osggis_googlecode/3dgis/lib/libosgGISProjectsd.so")
  IF(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libosgGISProjectsd.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libosgGISProjectsd.so")
    IF(CMAKE_INSTALL_DO_STRIP)
      EXECUTE_PROCESS(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libosgGISProjectsd.so")
    ENDIF(CMAKE_INSTALL_DO_STRIP)
  ENDIF()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgGISProjects" TYPE FILE FILES
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGISProjects/Build"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGISProjects/Builder"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGISProjects/BuildLayer"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGISProjects/BuildLayerSlice"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGISProjects/BuildTarget"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGISProjects/Cell"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGISProjects/CellCompiler"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGISProjects/Common"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGISProjects/Document"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGISProjects/Export"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGISProjects/GriddedMapLayerCompiler"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGISProjects/MapLayer"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGISProjects/MapLayerLevelOfDetail"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGISProjects/MapLayerCompiler"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGISProjects/Project"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGISProjects/QuadKey"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGISProjects/QuadTreeMapLayerCompiler"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGISProjects/RuntimeMap"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGISProjects/RuntimeMapLayer"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGISProjects/Serializer"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGISProjects/SimpleMapLayerCompiler"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGISProjects/Source"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGISProjects/Terrain"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGISProjects/XmlDocument"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGISProjects/XmlDOM"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGISProjects/XmlSerializer"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

