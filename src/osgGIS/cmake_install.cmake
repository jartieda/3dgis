# Install script for directory: /home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS

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
  IF(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libosgGISd.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libosgGISd.so")
    FILE(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libosgGISd.so"
         RPATH "")
  ENDIF()
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/jatrigueros/programas/osggis_googlecode/3dgis/lib/libosgGISd.so")
  IF(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libosgGISd.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libosgGISd.so")
    IF(CMAKE_INSTALL_DO_STRIP)
      EXECUTE_PROCESS(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libosgGISd.so")
    ENDIF(CMAKE_INSTALL_DO_STRIP)
  ENDIF()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgGIS" TYPE FILE FILES
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/AlignFilter"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/Attribute"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/AttributedNode"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/AttributeIndex"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/AutoResetBlock"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/BufferFilter"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/BuildGeomFilter"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/BuildLabelsFilter"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/BuildNodesFilter"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/ChangeShapeTypeFilter"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/ClampFilter"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/CollectionFilter"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/CollectionFilterState"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/Common"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/ConvexHullFilter"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/CombineLinesFilter"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/CropFilter"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/DecimateFilter"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/DefaultFeatureStoreFactory"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/DefaultRasterStoreFactory"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/DensifyFilter"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/ElevationGrid"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/ElevationResource"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/Ellipsoid"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/Export"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/ExtrudeGeomFilter"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/FadeHelper"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/Feature"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/FeatureCursor"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/FeatureFilter"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/FeatureFilterState"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/FeatureLayer"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/FeatureLayerCompiler"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/FeatureLayerResource"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/FeatureStore"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/FeatureStoreCompiler"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/FeatureStoreFactory"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/Filter"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/FilterGraph"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/FilterState"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/FilterEnv"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/Fragment"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/FragmentFilter"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/FragmentFilterState"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/GDAL_RasterStore"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/GeocentricSpatialReference"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/GeoExtent"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/GeometryCleaner"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/GeoPoint"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/GeoShape"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/GriddedLayerCompiler"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/GroupFilter"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/ImageUtils"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/LayerCompiler"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/LineSegmentIntersector2"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/LocalizeNodesFilter"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/Lua_ScriptEngine"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/Lua_ScriptEngine_tolua"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/ModelResource"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/NodeFilter"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/NodeFilterState"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/Notify"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/OGR_Feature"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/OGR_FeatureStore"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/OGR_SpatialReference"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/OGR_SpatialReferenceFactory"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/OGR_Utils"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/OptimizerHints"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/PagedLayerCompiler"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/PathResource"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/Property"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/RandomGroupingFilter"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/RasterResource"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/RasterStore"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/RasterStoreFactory"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/RecalculateExtentFilter"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/Registry"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/RemoveHolesFilter"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/Report"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/Resource"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/ResourceCache"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/ResourceLibrary"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/ResourcePackager"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/RTree"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/RTreeSpatialIndex"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/Script"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/ScriptEngine"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/SelectFilter"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/Session"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/SkinResource"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/SimpleFeature"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/SimpleLayerCompiler"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/SimpleSpatialIndex"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/SmartReadCallback"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/SpatialIndex"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/SpatialReference"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/SpatialReferenceBase"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/SpatialReferenceFactory"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/SRSResource"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/StubSpatialIndex"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/SubstituteModelFilter"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/Tags"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/Task"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/TaskManager"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/TerrainUtils"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/TransformFilter"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/Units"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/Utils"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/WriteFeaturesFilter"
    "/home/jatrigueros/programas/osggis_googlecode/3dgis/src/osgGIS/WriteTextFilter"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

