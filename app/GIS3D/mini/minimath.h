// (c) by Stefan Roettger

#ifndef MINIMATH_H
#define MINIMATH_H

#include "miniv3d.h"
#include "miniv4d.h"

namespace minimath {

// basic mathematical operations:

unsigned int gcd(unsigned int a,unsigned int b); // greatest common divisor
unsigned int lcm(unsigned int a,unsigned int b); // lowest common multiple

// matrix operations:

void cpy_mtx(miniv3d cpy[3],miniv3d mtx[3]);
void cpy_mtx(miniv4d cpy[3],miniv3d mtx[3]);
void cpy_mtx(miniv4d cpy[3],miniv4d mtx[3]);
void cpy_mtx(miniv3d cpy[3],miniv4d mtx[3]);

void cpy_mtx(miniv3d cpy[3],const miniv3d mtx[3]);
void cpy_mtx(miniv4d cpy[3],const miniv3d mtx[3]);
void cpy_mtx(miniv4d cpy[3],const miniv4d mtx[3]);
void cpy_mtx(miniv3d cpy[3],const miniv4d mtx[3]);

void mlt_mtx(miniv4d mtx[3],const miniv4d mtx1[3],const miniv4d mtx2[3]);
void mlt_mtx(miniv4d mtx[3],const miniv4d mtx1[3],const miniv4d mtx2[3],const miniv4d mtx3[3]);
void mlt_mtx(miniv4d mtx[3],const miniv4d mtx1[3],const miniv4d mtx2[3],const miniv4d mtx3[3],const miniv4d mtx4[3]);
void mlt_mtx(miniv4d mtx[3],const miniv4d mtx1[3],const miniv4d mtx2[3],const miniv4d mtx3[3],const miniv4d mtx4[3],const miniv4d mtx5[3]);
void mlt_mtx(miniv4d mtx[3],const miniv4d mtx1[3],const miniv4d mtx2[3],const miniv4d mtx3[3],const miniv4d mtx4[3],const miniv4d mtx5[3],const miniv4d mtx6[3]);
void mlt_mtx(miniv4d mtx[3],const miniv4d mtx1[3],const miniv4d mtx2[3],const miniv4d mtx3[3],const miniv4d mtx4[3],const miniv4d mtx5[3],const miniv4d mtx6[3],const miniv4d mtx7[3]);
void mlt_mtx(miniv4d mtx[3],const miniv4d mtx1[3],const miniv4d mtx2[3],const miniv4d mtx3[3],const miniv4d mtx4[3],const miniv4d mtx5[3],const miniv4d mtx6[3],const miniv4d mtx7[3],const miniv4d mtx8[3]);

double det_mtx(const miniv3d mtx[3]);

void inv_mtx(miniv3d inv[3],const miniv3d mtx[3]);
void inv_mtx(miniv4d inv[3],const miniv4d mtx[3]);

void tra_mtx(miniv3d tra[3],const miniv3d mtx[3]);
void tra_mtx(miniv4d tra[3],const miniv4d mtx[3]);

}

using namespace minimath;

#endif
