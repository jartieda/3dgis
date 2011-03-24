// (c) by Stefan Roettger

#include "miniv4f.h"

#include "miniv4d.h"
#include "miniv3f.h"
#include "miniv3d.h"

// conversion constructors
miniv4f::miniv4f(const miniv4d &v) {x=(float)v.x; y=(float)v.y; z=(float)v.z; w=(float)v.w;}
miniv4f::miniv4f(const miniv3f &v) {x=v.x; y=v.y; z=v.z; w=0.0f;}
miniv4f::miniv4f(const miniv3f &v,const float vw) {x=v.x; y=v.y; z=v.z; w=vw;}
miniv4f::miniv4f(const miniv3d &v) {x=(float)v.x; y=(float)v.y; z=(float)v.z; w=0.0f;}
miniv4f::miniv4f(const miniv3d &v,const double vw) {x=(float)v.x; y=(float)v.y; z=(float)v.z; w=(float)vw;}
