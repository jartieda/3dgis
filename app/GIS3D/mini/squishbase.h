// (c) by Stefan Roettger

#ifndef SQUISHBASE_H
#define SQUISHBASE_H

namespace squishbase {

// we strive to compress as fast as possible by default
enum {SQUISHMODE_FAST,SQUISHMODE_GOOD,SQUISHMODE_SLOW};

void compressS3TC(int isrgbadata,unsigned char *rawdata,unsigned int bytes,
                  unsigned char **s3tcdata,unsigned int *s3tcbytes,int width,int height,
                  int mode=SQUISHMODE_FAST);

void decompressS3TC(int isrgbadata,unsigned char *s3tcdata,unsigned int bytes,
                    unsigned char **rawdata,unsigned int *rawbytes,int width,int height);

}

using namespace squishbase;

#endif
