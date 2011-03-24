// (c) by Stefan Roettger

#ifndef PNMSAMPLE_H
#define PNMSAMPLE_H

namespace pnmsample {

// resample a collection of heterogeneous grids
// the first grid defines the resampling extent
// each grid is assumed to have a corner centric data representation
void resample(int num, // number of heterogeneous grids
              char **grid, // filenames of heterogeneous grids
              int tiles=8, // number of tiles along the largest edge
              int down=0, // number of downsampling levels
              int maxsize=4000, // maximum size of a tile
              char *basepath=0, // base path to the generated files
              float *centerx=0,float *centery=0, // positions of map centers
              float *extentx=0,float *extenty=0, // map extents
              float *scaling=0, // vertical scaling of elevations
              int missing=-9999, // missing elevation value
              int *cols=0,int *rows=0, // output columns and rows
              float *outparams=0, // array of 5 geometrical output parameters
              int *outmaxsize=0, // output maximum size of resampled tiles
              float offx=0.0f,float offy=0.0f, // manual offset
              int pyramid=4); // generate LOD pyramid

// generate the normal maps of a grid collection
void normalize(int num, // number of heterogeneous grids
               char **grid, // filenames of heterogeneous grids
               char *basepath=0, // base path to the generated files
               float *centerx=0,float *centery=0, // positions of map centers
               float *extentx=0,float *extenty=0, // map extents
               float *scaling=0, // vertical scaling of elevations
               int missing=-9999, // missing elevation value
               int border=0); // generate normals at border

// configuring
void configure_supersampling(int supersampling=1); // specify texture supersampling
void configure_downsampling(int downsampling=3); // specify texture downsampling
void configure_startupfile(int startupfile=0); // enable ouput of startup file
void configure_tilesetpath(char *tilesetpath); // define tile set path
void configure_stdprefix(char *stdprefix); // standard prefix of tileset file
void configure_rgbprefix(char *rgbprefix); // rgb prefix of tileset file
void configure_tilesetname(char *tilesetname); // name of tileset file
void configure_startupname(char *startupname); // name of startup file

// calculate an RGB normal map from a DEM
// Nx=2R-1, Nz=2G-1, Ny=sqrt(1-Nx*Nx-Nz*Nz), B=!missing
unsigned char *normalizemap(unsigned char *hfield, // height field
                            int width,int height,int components, // grid size
                            float extentx,float extenty, // map extent
                            float scaling, // vertical scaling of elevations
                            int missing=-9999, // missing elevation value
                            int border=0); // generate normals at border

// calculate an RG normal map from a DEM and put elevation into B
// Nx=2R-1, Nz=2G-1, Ny=sqrt(1-Nx*Nx-Nz*Nz), B=(elevation/snowline)^2
unsigned char *normalmap(unsigned char *hfield, // height field
                         int size,int components, // grid size
                         int width,int height, // target size
                         float extentx,float extenty, // map extent
                         float scaling, // vertical scaling of elevations
                         float sealevel=0.0f, // reference elevation of sea level
                         float snowline=3000.0f); // elevation of snow line

// calculate a texture map from a DEM tile
void texturemap(char *heightfile,
                char *texturefile,
                int twidth,int theight,
                void (*shader)(float nx,float ny,float nz,float elev,float *rgb),
                float sealevel=0.0f,float snowline=3000.0f);

// merge normalized RGB image #2 into gray-scale image #1
void mergeRGBimages(unsigned char *image1,int width1,int height1,
                    unsigned char *image2,int width2,int height2);

}

using namespace pnmsample;

#endif
