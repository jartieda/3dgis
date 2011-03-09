# Install script for directory: /home/jatrigueros/programas/RALine/osggis/src/osgGIS

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr")
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
  IF(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libosgGISd.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libosgGISd.so")
    FILE(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libosgGISd.so"
         RPATH "")
  ENDIF()
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/jatrigueros/programas/RALine/osggis/lib/libosgGISd.so")
  IF(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libosgGISd.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libosgGISd.so")
    IF(CMAKE_INSTALL_DO_STRIP)
      EXECUTE_PROCESS(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libosgGISd.so")
    ENDIF(CMAKE_INSTALL_DO_STRIP)
  ENDIF()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgGIS" TYPE FILE FILES
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/AlignFilter"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/Attribute"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/AttributedNode"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/AttributeIndex"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/AutoResetBlock"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/BufferFilter"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/BuildGeomFilter"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/BuildLabelsFilter"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/BuildNodesFilter"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/ChangeShapeTypeFilter"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/ClampFilter"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/CollectionFilter"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/CollectionFilterState"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/Common"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/ConvexHullFilter"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/CombineLinesFilter"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/CropFilter"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/DecimateFilter"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/DefaultFeatureStoreFactory"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/DefaultRasterStoreFactory"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/DensifyFilter"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/ElevationGrid"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/ElevationResource"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/Ellipsoid"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/Export"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/ExtrudeGeomFilter"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/FadeHelper"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/Feature"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/FeatureCursor"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/FeatureFilter"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/FeatureFilterState"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/FeatureLayer"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/FeatureLayerCompiler"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/FeatureLayerResource"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/FeatureStore"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/FeatureStoreCompiler"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/FeatureStoreFactory"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/Filter"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/FilterGraph"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/FilterState"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/FilterEnv"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/Fragment"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/FragmentFilter"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/FragmentFilterState"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/GDAL_RasterStore"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/GeocentricSpatialReference"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/GeoExtent"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/GeometryCleaner"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/GeoPoint"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/GeoShape"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/GriddedLayerCompiler"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/GroupFilter"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/ImageUtils"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/LayerCompiler"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/LineSegmentIntersector2"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/LocalizeNodesFilter"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/Lua_ScriptEngine"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/Lua_ScriptEngine_tolua"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/ModelResource"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/NodeFilter"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/NodeFilterState"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/Notify"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/OGR_Feature"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/OGR_FeatureStore"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/OGR_SpatialReference"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/OGR_SpatialReferenceFactory"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/OGR_Utils"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/OptimizerHints"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/PagedLayerCompiler"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/PathResource"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/Property"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/RandomGroupingFilter"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/RasterResource"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/RasterStore"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/RasterStoreFactory"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/RecalculateExtentFilter"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/Registry"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/RemoveHolesFilter"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/Report"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/Resource"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/ResourceCache"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/ResourceLibrary"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/ResourcePackager"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/RTree"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/RTreeSpatialIndex"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/Script"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/ScriptEngine"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/SelectFilter"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/Session"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/SkinResource"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/SimpleFeature"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/SimpleLayerCompiler"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/SimpleSpatialIndex"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/SmartReadCallback"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/SpatialIndex"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/SpatialReference"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/SpatialReferenceBase"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/SpatialReferenceFactory"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/SRSResource"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/StubSpatialIndex"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/SubstituteModelFilter"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/Tags"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/Task"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/TaskManager"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/TerrainUtils"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/TransformFilter"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/Units"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/Utils"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/WriteFeaturesFilter"
    "/home/jatrigueros/programas/RALine/osggis/src/osgGIS/WriteTextFilter"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

