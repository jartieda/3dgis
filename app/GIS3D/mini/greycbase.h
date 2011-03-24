// (c) by Stefan Roettger

#ifndef GREYCBASE_H
#define GREYCBASE_H

namespace greycbase {

void denoiseGREYCimage(unsigned char *image,int width,int height,int components,
                       float p=0.8f,float a=0.4f);

}

using namespace greycbase;

#endif
