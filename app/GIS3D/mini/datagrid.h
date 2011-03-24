// (c) by Stefan Roettger

#ifndef DATAGRID_H
#define DATAGRID_H

#include "minibase.h"

#include "miniv3d.h"
#include "miniv4d.h"

#include "minidyna.h"
#include "minimesh.h"
#include "miniwarp.h"

#include "database.h"

class datagrid
   {
   public:

   //! default constructor
   datagrid();

   //! destructor
   virtual ~datagrid();

   //! set coordinate system
   void setcrs(const minicoord::MINICOORD crs=minicoord::MINICOORD_ECEF);

   //! create data brick id
   unsigned int create(const unsigned int slot, // data slot
                       const BOOLINT flip=FALSE); // decomposition flip

   //! load data
   void load(const unsigned int id, // data brick id
             const databuf &buf); // data buffer

   //! remove data
   void remove(const unsigned int id); // data brick id

   //! move data
   void move(const unsigned int id, // data brick id
             const float swx,const float swy, // SW corner of data brick
             const float nwx,const float nwy, // NW corner of data brick
             const float nex,const float ney, // NE corner of data brick
             const float sex,const float sey, // SE corner of data brick
             const float h0,const float dh,  // base elevation and height of data brick
             const float t0,const float dt); // time frame start and exposure time

   //! clear all data bricks
   void clear();

   //! check if any valid bricks are present
   BOOLINT isclear();

   //! apply matrix (pre of producing a tetrahedral mesh)
   void applymtx(const miniv4d mtx[3]);

   //! specify matrix (post of producing a tetrahedral mesh)
   void specmtx(const miniv4d mtx[3]);

   //! construct tetrahedral mesh from all data bricks
   void construct();

   //! preprocess tetrahedral mesh one step at a time
   BOOLINT preprocess();

   //! trigger pushing the mesh for a particular time step
   void trigger(const double time);

   //! trigger pushing the mesh for a particular time step and eye point
   void trigger(const double time,
                const minicoord &eye,const miniv3d &dir,
                const float nearp,const float farp,const float fovy,const float aspect,
                const double maxradius=MAXFLOAT);

   //! push the mesh for a particular time step
   virtual void push(const minimesh &mesh,const double time);

   //! push the mesh for a particular time step and eye point
   virtual void push(const minimesh &mesh,
                     const double time,
                     const minicoord &eye,const miniv3d &dir,
                     const float nearp,const float farp,const float fovy,const float aspect);

   protected:

   minicoord::MINICOORD CRS;

   minidyna<BOOLINT> FLAG;
   minidyna<unsigned int> SLOT;
   minidyna<BOOLINT> FLIP;

   minidyna<databuf> DATA;

   miniv4d MTXPRE[3],MTXPOST[3];
   BOOLINT IDPRE,IDPOST;

   miniv4d INVTRAPRE[3],INVTRAPOST[3];

   private:

   BOOLINT INVALID;

   minimesh MESH;
   BOOLINT DONE;

   unsigned int PHASE,STEP;

   minibsptree BSPT1,BSPT2;
   BOOLINT CONSTRUCTED;

   minimesh UNSORTED;
   minimesh SORTED;

   miniv4d ID[3];

   BOOLINT decompose();
   void decompose(unsigned int idx);
   };

#endif
