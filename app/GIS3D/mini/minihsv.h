// (c) by Stefan Roettger

#ifndef MINIHSV_H
#define MINIHSV_H

namespace minihsv {

void rgb2hsv(float r,float g,float b,float hsv[3]);
void hsv2rgb(float hue,float sat,float val,float rgb[3]);

}

using namespace minihsv;

#endif
