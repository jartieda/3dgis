// (c) by Stefan Roettger

#ifndef MINIIO_H
#define MINIIO_H

namespace miniio {

//! check a file
int checkfile(const char *filename);

//! write a RAW file
void writefile(const char *filename,unsigned char *data,unsigned int bytes);

//! read a RAW file
unsigned char *readfile(const char *filename,unsigned int *bytes);

}

using namespace miniio;

#endif
