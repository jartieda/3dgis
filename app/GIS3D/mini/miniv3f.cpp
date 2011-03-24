// (c) by Stefan Roettger

#include "miniv3f.h"

#include "miniv3d.h"
#include "miniv4f.h"
#include "miniv4d.h"

// conversion constructors
miniv3f::miniv3f(const miniv3d &v) {x=(float)v.x; y=(float)v.y; z=(float)v.z;}
miniv3f::miniv3f(const miniv4f &v) {x=v.x; y=v.y; z=v.z;}
miniv3f::miniv3f(const miniv4d &v) {x=(float)v.x; y=(float)v.y; z=(float)v.z;}
