// (c) by Stefan Roettger

#ifndef MINITEXT_H
#define MINITEXT_H

namespace minitext {

void drawline(float x1,float y1,float x2,float y2,
              float hue,float sat,float val,float alpha);

void drawlineRGBA(float x1,float y1,float x2,float y2,
                  float r,float g,float b,float alpha);

void drawquad(float x,float y,float width,float height,
              float hue,float sat,float val,float alpha);

void drawquadRGBA(float x,float y,float width,float height,
                  float r,float g,float b,float alpha);

void drawframe(float x,float y,float width,float height,
               float hue,float sat,float val,float alpha);

void drawframeRGBA(float x,float y,float width,float height,
                   float r,float g,float b,float alpha);

void drawstring(float width,
                float hue,float sat,float val,float alpha,const char *str,
                float backval=0.0f,float backalpha=0.0f);

// configuring
void configure_zfight(float zscale=0.95f); // shift string towards the viewer to avoid Z-fighting with background

}

#endif
