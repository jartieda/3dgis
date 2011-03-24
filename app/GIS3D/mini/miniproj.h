// (c) by Stefan Roettger

#ifndef MINIPROJ_H
#define MINIPROJ_H

#include "minibase.h"

#include "miniv3d.h"

class miniproj
   {
   public:

   //! default constructor
   miniproj();

   //! destructor
   ~miniproj();

   //! initialize projection state
   void initproj(float emi,float rho);

   //! project a tetrahedron
   void proj(const miniv3d &v1,const double c1,
             const miniv3d &v2,const double c2,
             const miniv3d &v3,const double c3,
             const miniv3d &v4,const double c4,
             const miniv3d &col,
             const miniv3d &eye);

   //! clip&project a tetrahedron
   void clip(const miniv3d &v1,const double c1,
             const miniv3d &v2,const double c2,
             const miniv3d &v3,const double c3,
             const miniv3d &v4,const double c4,
             const miniv3d &col,
             const miniv3d &eye,
             const miniv3d &dir,
             const double nearp);

   //! de-initialize projection state
   void exitproj();

   private:

   float EMI,RHO;

   inline BOOLINT isfront(const miniv3d &p,const miniv3d &v1,const miniv3d &v2,const miniv3d &v3,const miniv3d &e);
   inline double intersect(const miniv3d &p,const miniv3d &d,const miniv3d &o,const miniv3d &d1,const miniv3d &d2,miniv3d &m);

   inline void proj3tri(const miniv3d &v1,const double c1,
                        const miniv3d &v2,const double c2,
                        const miniv3d &v3,const double c3,
                        const miniv3d &v4,const double c4,
                        const miniv3d &eye);

   inline void proj4tri(const miniv3d &v1,const double c1,
                        const miniv3d &v2,const double c2,
                        const miniv3d &v3,const double c3,
                        const miniv3d &v4,const double c4,
                        const miniv3d &eye);

   void projpri(const miniv3d &v1,const double c1,
                const miniv3d &v2,const double c2,
                const miniv3d &v3,const double c3,
                const miniv3d &v4,const double c4,
                const miniv3d &v5,const double c5,
                const miniv3d &v6,const double c6,
                const miniv3d &col,
                const miniv3d &eye);

   void clip1A(const miniv3d &v1,const double c1,const double d1,
               const miniv3d &v2,const double c2,const double d2,
               const miniv3d &v3,const double c3,const double d3,
               const miniv3d &v4,const double c4,const double d4,
               const miniv3d &col,
               const miniv3d &eye);

   void clip1B(const miniv3d &v1,const double c1,const double d1,
               const miniv3d &v2,const double c2,const double d2,
               const miniv3d &v3,const double c3,const double d3,
               const miniv3d &v4,const double c4,const double d4,
               const miniv3d &col,
               const miniv3d &eye);

   void clip2(const miniv3d &v1,const double c1,const double d1,
              const miniv3d &v2,const double c2,const double d2,
              const miniv3d &v3,const double c3,const double d3,
              const miniv3d &v4,const double c4,const double d4,
              const miniv3d &col,
              const miniv3d &eye);

   // vertex and fragment programs:

   char *VTXPROG,*FRGPROG;
   int VTXPROGID,FRGPROGID;

   void enablevtxshader();
   void disablevtxshader();

   void enablepixshader();
   void disablepixshader();

   void setupprogs();
   void deleteprogs();

   // OpenGL extensions:

   int GLSETUP;

   int GLEXT_MT;
   int GLEXT_VP;
   int GLEXT_FP;

   void initglsetup();
   void initglexts();

   // Windows OpenGL extensions:

   int WGLSETUP;

#ifndef NOOGL

#ifdef _WIN32

#ifdef GL_ARB_multitexture
   PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;
   PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB;
#endif

#if defined(GL_ARB_vertex_program) && defined(GL_ARB_fragment_program)
   PFNGLGENPROGRAMSARBPROC glGenProgramsARB;
   PFNGLBINDPROGRAMARBPROC glBindProgramARB;
   PFNGLPROGRAMSTRINGARBPROC glProgramStringARB;
   PFNGLPROGRAMENVPARAMETER4FARBPROC glProgramEnvParameter4fARB;
   PFNGLDELETEPROGRAMSARBPROC glDeleteProgramsARB;
#endif

#endif

#endif

   void initwglprocs();
   };

#endif
