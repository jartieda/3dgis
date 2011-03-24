// (c) by Stefan Roettger

#include "miniv4d.h"

#include "miniv4f.h"
#include "miniv3d.h"
#include "miniv3f.h"

// conversion constructors
miniv4d::miniv4d(const miniv4f &v) {x=(double)v.x; y=(double)v.y; z=(double)v.z; w=(double)v.w;}
miniv4d::miniv4d(const miniv3d &v) {x=v.x; y=v.y; z=v.z; w=0.0;}
miniv4d::miniv4d(const miniv3d &v,const double vw) {x=v.x; y=v.y; z=v.z; w=vw;}
miniv4d::miniv4d(const miniv3f &v) {x=(double)v.x; y=(double)v.y; z=(double)v.z; w=0.0;}
miniv4d::miniv4d(const miniv3f &v,const float vw) {x=(double)v.x; y=(double)v.y; z=(double)v.z; w=(double)vw;}
