#ifndef OSGGISLAYER_H
#define OSGGISLAYER_H

#include <string>
#include <QString>


#include <osgGIS/Registry>
#include <osgGIS/SimpleLayerCompiler>
#include <osgGIS/FilterGraph>
#include <osgGIS/BuildGeomFilter>
#include <osgGIS/BuildNodesFilter>
#include <osgGIS/SubstituteModelFilter>
#include <osgGIS/ClampFilter>
#include <osgGIS/TransformFilter>
#include <osgGIS/ExtrudeGeomFilter>
#include <osgGIS/BufferFilter>
#include <osgGIS/FadeHelper>
#include <osgGIS/ScriptEngine>
#include <iostream>


class osggisLayer
{
public:


    osggisLayer();
    osg::ref_ptr<osg::Node> addLayer(std::string filename);
    osg::ref_ptr<osg::Node> compile();

    osg::ref_ptr<osgGIS::FilterGraph> graph ;
    osg::ref_ptr<osg::Node> layer_node;
    osg::ref_ptr<osg::Node> old_node;
    osg::ref_ptr<osgGIS::FeatureLayer> layer;
    QString shpfilename;
};

#endif // OSGGISLAYER_H
