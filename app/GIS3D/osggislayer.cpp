#include "osggislayer.h"

int
die( const std::string& msg )
{
        osgGIS::notify( osg::FATAL ) << "ERROR: " << msg << std::endl;
        return -1;
}

osggisLayer::osggisLayer()
{

}

//void osggisLayer::compile()
osg::ref_ptr<osg::Node> osggisLayer::compile()
{
    // Compile the feature layer into a scene graph.
    osgGIS::SimpleLayerCompiler compiler( graph.get() );

    layer_node = compiler.compile( layer.get() );

    return layer_node;

}
osg::ref_ptr<osg::Node> osggisLayer::addLayer(std::string filename)
{
    shpfilename=filename.c_str();

    osg::Vec4f color(1,1,1,1);

    osgGIS::Registry* registry = osgGIS::Registry::instance();

    layer = registry->createFeatureLayer( filename  );

   /* // Creamos una nueva Skin para los edificios
    osg::Image* texture = osgDB::readImageFile( "C:\Programas\BBDD\rabd_v2\rabd_v2\debug\skin.jpeg" );
    if(!wall)
    {
    std::cout << "Error: Couldn't find texture!" << std::endl;
    }
    osg::ref_ptr<osgGIS::SkinResource> skin = new osgGIS::SkinResource(texture);
    skin->setTextureWidthMeters(10);
    skin->repeats_vertically(false);*/

    if ( !layer.valid() )
        return NULL ;//die( "Failed to create feature layer." );

    // The FilterGraph is a series of filters that will transform the GIS
    // feature data into a scene graph.
    graph = new osgGIS::FilterGraph();

    // Transform Filter
    osg::ref_ptr<osgGIS::TransformFilter> tf = new osgGIS::TransformFilter();
    //tf->setUseTerrainSRS(true);
    tf->setLocalize(true);
    graph->appendFilter(tf.get());

    osg::ref_ptr<osgGIS::BuildGeomFilter> gf = new osgGIS::BuildGeomFilter();
    graph->appendFilter(gf.get());

   // Polygon to point Filter
   /*osg::ref_ptr<osgGIS::PolytopointFilter> ptp = new osgGIS::PolytopointFilter();
   ptp->setNumberOfPoints(30);
   graph->appendFilter(ptp.get());*/

// Clamp Filter over the TerrainOSG
  // osg::ref_ptr<osgGIS::ClampFilter2> cf2 = new osgGIS::ClampFilter2();
  // cf2->setTerrain(terreno);
  // graph->appendFilter(cf2.get());

//SubstitudeFilter: nodes are replaced by other models at feature locations
 /*   osg::ref_ptr<osgGIS::SubstituteModelFilter> sf = new osgGIS::SubstituteModelFilter();
    sf->setModelPathScript(new osgGIS::Script("'C:/Programas/BBDD/rabd_v2/rabd_v2/debug/cow.osg'"));
    graph->appendFilter(sf.get());*/
    osg::ref_ptr<osgGIS::BuildGeomFilter> nf = new osgGIS::BuildGeomFilter();
    graph->appendFilter(nf.get());

/*// Extrude
    //Script::Script* skin = new Script::Script("skin","lua",'');

    osg::ref_ptr<osgGIS::ExtrudeGeomFilter> ef = new osgGIS::ExtrudeGeomFilter();
    ef->setHeightScript(new osgGIS::Script("feature:getAttribute('Elevation'):asDouble()"));
    ef->setWallSkinScript(new osgGIS::Script("'C:/Programas/BBDD/rabd_v2/rabd_v2/debug/skin.jpg'"));
    graph->appendFilter(ef.get());*/

//Si no hay un SubstitudeFilter , hay que poner un BuildGeomFilter
    //osg::ref_ptr<osgGIS::BuildGeomFilter> bg = new osgGIS::BuildGeomFilter();
    //graph->appendFilter(bg.get());


// Collection Filter
    osg::ref_ptr<osgGIS::CollectFilter> cf = new osgGIS::CollectFilter();
    graph->appendFilter(cf.get());

// Build Nodes
    osg::ref_ptr<osgGIS::BuildNodesFilter> bn = new osgGIS::BuildNodesFilter();
    graph->appendFilter( bn.get());

    osgGIS::SimpleLayerCompiler compiler(graph.get());

    layer_node = compiler.compile( layer.get() );

    return layer_node;
}
