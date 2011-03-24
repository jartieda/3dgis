// (c) by Stefan Roettger

#include "minibase.h"

#include "minishader.h"

unsigned char *minishader::VISBATHYMAP=NULL;
int minishader::VISBATHYWIDTH=0,minishader::VISBATHYHEIGHT=0,minishader::VISBATHYCOMPS=0;
int minishader::VISBATHYMOD=0;

unsigned char *minishader::NPRBATHYMAP=NULL;
int minishader::NPRBATHYWIDTH=0,minishader::NPRBATHYHEIGHT=0,minishader::NPRBATHYCOMPS=0;
int minishader::NPRBATHYMOD=0;

// enable vertex and pixel shader for VIS purposes
void minishader::setVISshader(minicache *cache,
                              float scale,float exaggeration,
                              float fogstart,float fogend,
                              float fogdensity,
                              float fogcolor[3],
                              float bathystart,float bathyend,
                              float contours,
                              float sealevel,float seabottom,
                              float seacolor[3],
                              float seatrans,float bottomtrans,
                              float bottomcolor[3],
                              float seamodulate)
   {
   float fog_a,fog_b,fog_c;
   float bathy_a,bathy_b,bathy_c;
   float cnt_a,cnt_b,cnt_c,cnt_d;
   float sea_a,sea_b;

   // fragment program for the terrain
   static const char *fragprog1A="!!ARBfp1.0 \n\
      PARAM c0=program.env[0]; \n\
      PARAM c1=program.env[1]; \n\
      PARAM c2=program.env[2]; \n\
      PARAM c3=program.env[3]; \n\
      PARAM c4=program.env[4]; \n\
      PARAM a=program.env[8]; \n\
      PARAM t=program.env[9]; \n\
      PARAM l=program.env[10]; \n\
      PARAM p=program.env[11]; \n\
      TEMP col,colb,nrm,vtx,len,fog; \n\
      ### fetch texture color \n\
      TEX col,fragment.texcoord[0],texture[0],2D; \n\
      MAD col,col,a.x,a.b; \n\
      ### fade-out at sea bottom \n\
      SUB vtx.y,fragment.texcoord[0].z,c0.x; \n\
      MUL_SAT vtx.w,-vtx.y,c0.y; \n\
      SUB vtx.w,c0.w,vtx.w; \n\
      MAD vtx.w,vtx.w,-vtx.w,c0.w; \n\
      MUL vtx.w,vtx.w,c1.w; \n\
      LRP colb.xyz,vtx.w,c1,col; \n\
      CMP col.xyz,vtx.y,colb,col; \n\
      ### modulate with contours \n\
      MUL vtx.y,fragment.texcoord[0].z,-c2.x; \n\
      FRC vtx.y,vtx.y; \n\
      MAD vtx.y,vtx.y,c2.z,-c2.w; \n\
      ABS vtx.y,vtx.y; \n\
      SUB vtx.y,c0.w,vtx.y; \n\
      MUL_SAT vtx.y,vtx.y,c2.y; \n\
      MUL col.xyz,col,vtx.y; \n\
      ### modulate with fragment color \n\
      MUL col,col,fragment.color; \n\
      ### modulate with directional light \n\
      MOV nrm,fragment.texcoord[1]; \n\
      DP3 len.x,nrm,nrm; \n\
      RSQ len.x,len.x; \n\
      MUL nrm,nrm,len.x; \n\
      DP3_SAT nrm.z,nrm,l; \n\
      MAD nrm.z,nrm.z,p.x,p.y; \n\
      MUL_SAT col.xyz,col,nrm.z; \n\
      ### modulate with spherical fog \n\
      MOV fog.x,fragment.fogcoord.x; \n\
      MAD_SAT fog.x,fog.x,c3.x,c3.y; \n\
      POW fog.x,fog.x,c3.z; \n\
      LRP col.xyz,fog.x,c4,col; \n\
      ### write resulting color \n\
      MOV result.color,col; \n\
      END \n";

   // fragment program for the terrain with color mapping before the fade-out
   static const char *fragprog1B="!!ARBfp1.0 \n\
      PARAM c0=program.env[0]; \n\
      PARAM c1=program.env[1]; \n\
      PARAM c2=program.env[2]; \n\
      PARAM c3=program.env[3]; \n\
      PARAM c4=program.env[4]; \n\
      PARAM c5=program.env[5]; \n\
      PARAM a=program.env[8]; \n\
      PARAM t=program.env[9]; \n\
      PARAM l=program.env[10]; \n\
      PARAM p=program.env[11]; \n\
      TEMP col,colt,colb,nrm,vtx,len,fog; \n\
      ### fetch texture color \n\
      TEX col,fragment.texcoord[0],texture[0],2D; \n\
      MAD col,col,a.x,a.b; \n\
      ### blend with color map \n\
      SUB vtx.z,fragment.texcoord[0].z,c5.x; \n\
      MUL vtx.z,-vtx.z,c5.y; \n\
      MOV vtx.y,c5.z; \n\
      MAD vtx.x,vtx.z,t.x,t.y; \n\
      TEX colt,vtx,texture[1],2D; \n\
      LRP colt.xyz,colt.w,colt,c5.w; \n\
      MUL colt.xyz,colt,col; \n\
      CMP col.xyz,-vtx.z,colt,col; \n\
      ### fade-out at sea bottom \n\
      SUB vtx.y,fragment.texcoord[0].z,c0.x; \n\
      MUL_SAT vtx.w,-vtx.y,c0.y; \n\
      SUB vtx.w,c0.w,vtx.w; \n\
      MAD vtx.w,vtx.w,-vtx.w,c0.w; \n\
      MUL vtx.w,vtx.w,c1.w; \n\
      LRP colb.xyz,vtx.w,c1,col; \n\
      CMP col.xyz,vtx.y,colb,col; \n\
      ### modulate with contours \n\
      MUL vtx.y,fragment.texcoord[0].z,-c2.x; \n\
      FRC vtx.y,vtx.y; \n\
      MAD vtx.y,vtx.y,c2.z,-c2.w; \n\
      ABS vtx.y,vtx.y; \n\
      SUB vtx.y,c0.w,vtx.y; \n\
      MUL_SAT vtx.y,vtx.y,c2.y; \n\
      MUL col.xyz,col,vtx.y; \n\
      ### modulate with fragment color \n\
      MUL col,col,fragment.color; \n\
      ### modulate with directional light \n\
      MOV nrm,fragment.texcoord[1]; \n\
      DP3 len.x,nrm,nrm; \n\
      RSQ len.x,len.x; \n\
      MUL nrm,nrm,len.x; \n\
      DP3_SAT nrm.z,nrm,l; \n\
      MAD nrm.z,nrm.z,p.x,p.y; \n\
      MUL_SAT col.xyz,col,nrm.z; \n\
      ### modulate with spherical fog \n\
      MOV fog.x,fragment.fogcoord.x; \n\
      MAD_SAT fog.x,fog.x,c3.x,c3.y; \n\
      POW fog.x,fog.x,c3.z; \n\
      LRP col.xyz,fog.x,c4,col; \n\
      ### write resulting color \n\
      MOV result.color,col; \n\
      END \n";

   // fragment program for the terrain with color mapping after the fade-out
   static const char *fragprog1C="!!ARBfp1.0 \n\
      PARAM c0=program.env[0]; \n\
      PARAM c1=program.env[1]; \n\
      PARAM c2=program.env[2]; \n\
      PARAM c3=program.env[3]; \n\
      PARAM c4=program.env[4]; \n\
      PARAM c5=program.env[5]; \n\
      PARAM a=program.env[8]; \n\
      PARAM t=program.env[9]; \n\
      PARAM l=program.env[10]; \n\
      PARAM p=program.env[11]; \n\
      TEMP col,colt,colb,nrm,vtx,len,fog; \n\
      ### fetch texture color \n\
      TEX col,fragment.texcoord[0],texture[0],2D; \n\
      MAD col,col,a.x,a.b; \n\
      ### fade-out at sea bottom \n\
      SUB vtx.y,fragment.texcoord[0].z,c0.x; \n\
      MUL_SAT vtx.w,-vtx.y,c0.y; \n\
      SUB vtx.w,c0.w,vtx.w; \n\
      MAD vtx.w,vtx.w,-vtx.w,c0.w; \n\
      MUL vtx.w,vtx.w,c1.w; \n\
      LRP colb.xyz,vtx.w,c1,col; \n\
      CMP col.xyz,vtx.y,colb,col; \n\
      ### blend with color map \n\
      SUB vtx.z,fragment.texcoord[0].z,c5.x; \n\
      MUL vtx.z,-vtx.z,c5.y; \n\
      MOV vtx.y,c5.z; \n\
      MAD vtx.x,vtx.z,t.x,t.y; \n\
      TEX colt,vtx,texture[1],2D; \n\
      LRP colt.xyz,colt.w,colt,c5.w; \n\
      MUL colt.xyz,colt,col; \n\
      CMP col.xyz,-vtx.z,colt,col; \n\
      ### modulate with contours \n\
      MUL vtx.y,fragment.texcoord[0].z,-c2.x; \n\
      FRC vtx.y,vtx.y; \n\
      MAD vtx.y,vtx.y,c2.z,-c2.w; \n\
      ABS vtx.y,vtx.y; \n\
      SUB vtx.y,c0.w,vtx.y; \n\
      MUL_SAT vtx.y,vtx.y,c2.y; \n\
      MUL col.xyz,col,vtx.y; \n\
      ### modulate with fragment color \n\
      MUL col,col,fragment.color; \n\
      ### modulate with directional light \n\
      MOV nrm,fragment.texcoord[1]; \n\
      DP3 len.x,nrm,nrm; \n\
      RSQ len.x,len.x; \n\
      MUL nrm,nrm,len.x; \n\
      DP3_SAT nrm.z,nrm,l; \n\
      MAD nrm.z,nrm.z,p.x,p.y; \n\
      MUL_SAT col.xyz,col,nrm.z; \n\
      ### modulate with spherical fog \n\
      MOV fog.x,fragment.fogcoord.x; \n\
      MAD_SAT fog.x,fog.x,c3.x,c3.y; \n\
      POW fog.x,fog.x,c3.z; \n\
      LRP col.xyz,fog.x,c4,col; \n\
      ### write resulting color \n\
      MOV result.color,col; \n\
      END \n";

   // fragment program for the sea surface
   static const char *fragprog2="!!ARBfp1.0 \n\
      PARAM c0=program.env[0]; \n\
      PARAM c1=program.env[1]; \n\
      PARAM c2=program.env[2]; \n\
      PARAM c3=program.env[3]; \n\
      PARAM c4=program.env[4]; \n\
      PARAM a=program.env[8]; \n\
      PARAM t=program.env[9]; \n\
      PARAM l=program.env[10]; \n\
      PARAM p=program.env[11]; \n\
      TEMP col,tex,nrm,len,fog; \n\
      ### fetch fragment color \n\
      MOV col,fragment.color; \n\
      ### modulate with texture color \n\
      TEX tex,fragment.texcoord[0],texture[0],2D; \n\
      MAD tex,tex,a.x,a.b; \n\
      LRP col.xyz,c0.x,tex,col; \n\
      ### modulate with directional light \n\
      MOV nrm,fragment.texcoord[1]; \n\
      DP3 len.x,nrm,nrm; \n\
      RSQ len.x,len.x; \n\
      MUL nrm,nrm,len.x; \n\
      DP3_SAT nrm.z,nrm,l; \n\
      MAD nrm.z,nrm.z,p.x,p.y; \n\
      MUL_SAT col.xyz,col,nrm.z; \n\
      ### modulate with spherical fog \n\
      MOV fog.x,fragment.fogcoord.x; \n\
      MAD_SAT fog.x,fog.x,c3.x,c3.y; \n\
      POW fog.x,fog.x,c3.z; \n\
      LRP col.xyz,fog.x,c4,col; \n\
      ### write resulting color \n\
      MOV result.color,col; \n\
      END \n";

   // set primary sea color
   cache->setseacolor(seacolor[0],seacolor[1],seacolor[2],seatrans);

   // use default vertex shader plugin
   cache->setvtxshader();
   cache->usevtxshader(1);

   // calculate the fog parameters
   if (fogstart<fogend)
      {
      fog_a=fsqr(scale/fogend);
      fog_b=0.0f;
      fog_c=log(fmin(0.5f*fogdensity,1.0f))/(2.0f*log(fogstart/fogend/2.0f+0.5f));
      }
   else
      {
      fog_a=0.0f;
      fog_b=0.0f;
      fog_c=1.0f;
      }

   // calculate the bathymetry parameters
   if (exaggeration*(bathystart-bathyend)!=0.0f)
      {
      bathy_a=bathystart*exaggeration/scale;
      bathy_b=scale/(exaggeration*(bathystart-bathyend));
      bathy_c=0.5f;
      }
   else
      {
      bathy_a=0.0f;
      bathy_b=0.0f;
      bathy_c=0.0f;
      }

   // calculate the contour parameters
   if (exaggeration*contours!=0.0f)
      {
      cnt_a=scale/(exaggeration*fabs(contours));
      cnt_b=3.0f;
      cnt_c=2.0f;
      cnt_d=1.0f;
      }
   else
      {
      cnt_a=0.0f;
      cnt_b=1.0f;
      cnt_c=0.0f;
      cnt_d=0.0f;
      }

   // calculate the sea parameters
   if (sealevel!=-MAXFLOAT && exaggeration*seabottom!=0.0f)
      {
      sea_a=sealevel*exaggeration/scale;
      sea_b=scale/(exaggeration*fabs(seabottom));
      }
   else
      {
      sea_a=0.0f;
      sea_b=0.0f;
      }

   // use pixel shader plugin
   cache->setpixshader((bathystart==bathyend || VISBATHYMAP==NULL)?fragprog1A:(seabottom<0.0f)?fragprog1B:fragprog1C);
   cache->setpixshaderparams(sea_a,sea_b,0.0f,1.0f);
   cache->setpixshaderparams(bottomcolor[0],bottomcolor[1],bottomcolor[2],bottomtrans,1);
   cache->setpixshaderparams(cnt_a,cnt_b,cnt_c,cnt_d,2);
   cache->setpixshaderparams(fog_a,fog_b,fog_c,0.0f,3);
   cache->setpixshaderparams(fogcolor[0],fogcolor[1],fogcolor[2],0.0f,4);
   cache->setpixshaderparams(bathy_a,bathy_b,bathy_c,1.0f,5);
   cache->usepixshader(1);
   cache->setseashader(fragprog2);
   cache->setseashaderparams(seamodulate,0.0f,0.0f,0.0f);
   cache->setseashaderparams(fog_a,fog_b,fog_c,0.0f,3);
   cache->setseashaderparams(fogcolor[0],fogcolor[1],fogcolor[2],0.0f,4);
   cache->useseashader(1);

   // set bathymetry color map
   if (VISBATHYMAP!=NULL && VISBATHYMOD!=0)
      {
      cache->setpixshadertex(VISBATHYMAP,VISBATHYWIDTH,VISBATHYHEIGHT,VISBATHYCOMPS);

      VISBATHYMOD=0;
      NPRBATHYMOD=1;
      }
   }

// set bathymetry color map for VIS shader
void minishader::setVISbathymap(unsigned char *bathymap,
                                int bathywidth,int bathyheight,int bathycomps)
   {
   VISBATHYMAP=bathymap;

   VISBATHYWIDTH=bathywidth;
   VISBATHYHEIGHT=bathyheight;
   VISBATHYCOMPS=bathycomps;

   VISBATHYMOD=1;
   }

// enable vertex and pixel shader for NPR purposes
void minishader::setNPRshader(minicache *cache,
                              float scale,float exaggeration,
                              float fogstart,float fogend,
                              float fogdensity,
                              float fogcolor[3],
                              float bathystart,float bathyend,
                              float contours,
                              float fadefactor,
                              float sealevel,
                              float seacolor[3],float seatrans,
                              float seagray)
   {
   float fog_a,fog_b,fog_c;
   float bathy_a,bathy_b,bathy_c;
   float cnt_a,cnt_b,cnt_c,cnt_d;
   float npr_a,npr_b,npr_c,npr_d;

   // fragment program for the terrain
   static const char *fragprog1A="!!ARBfp1.0 \n\
      PARAM c0=program.env[0]; \n\
      PARAM c1=program.env[1]; \n\
      PARAM c2=program.env[2]; \n\
      PARAM c3=program.env[3]; \n\
      PARAM c4=program.env[4]; \n\
      PARAM a=program.env[8]; \n\
      PARAM t=program.env[9]; \n\
      PARAM l=program.env[10]; \n\
      PARAM p=program.env[11]; \n\
      TEMP col,nrm,vtx,len,fog; \n\
      ### fetch texture color \n\
      TEX col,fragment.texcoord[0],texture[0],2D; \n\
      MAD col,col,a.x,a.b; \n\
      ### fade texture \n\
      DP3 col.xyz,col,c0; \n\
      ADD col.xyz,col,c0.w; \n\
      ### replace bathymetry \n\
      SUB vtx.y,fragment.texcoord[0].z,c1.w; \n\
      CMP col.xyz,vtx.y,c1,col; \n\
      ### modulate with contours \n\
      MUL vtx.y,fragment.texcoord[0].z,-c2.x; \n\
      FRC vtx.y,vtx.y; \n\
      MAD vtx.y,vtx.y,c2.z,-c2.w; \n\
      ABS vtx.y,vtx.y; \n\
      SUB vtx.y,c4.w,vtx.y; \n\
      MUL_SAT vtx.y,vtx.y,c2.y; \n\
      MUL col.xyz,col,vtx.y; \n\
      ### modulate with fragment color \n\
      MUL col,col,fragment.color; \n\
      ### modulate with directional light \n\
      MOV nrm,fragment.texcoord[1]; \n\
      DP3 len.x,nrm,nrm; \n\
      RSQ len.x,len.x; \n\
      MUL nrm,nrm,len.x; \n\
      DP3_SAT nrm.z,nrm,l; \n\
      MAD nrm.z,nrm.z,p.x,p.y; \n\
      MUL_SAT col.xyz,col,nrm.z; \n\
      ### modulate with spherical fog \n\
      MOV fog.x,fragment.fogcoord.x; \n\
      MAD_SAT fog.x,fog.x,c3.x,c3.y; \n\
      POW fog.x,fog.x,c3.z; \n\
      LRP col.xyz,fog.x,c4,col; \n\
      ### write resulting color \n\
      MOV result.color,col; \n\
      END \n";

   // fragment program for the terrain with color mapping
   static const char *fragprog1B="!!ARBfp1.0 \n\
      PARAM c0=program.env[0]; \n\
      PARAM c1=program.env[1]; \n\
      PARAM c2=program.env[2]; \n\
      PARAM c3=program.env[3]; \n\
      PARAM c4=program.env[4]; \n\
      PARAM c5=program.env[5]; \n\
      PARAM a=program.env[8]; \n\
      PARAM t=program.env[9]; \n\
      PARAM l=program.env[10]; \n\
      PARAM p=program.env[11]; \n\
      TEMP col,colt,nrm,vtx,len,fog; \n\
      ### fetch texture color \n\
      TEX col,fragment.texcoord[0],texture[0],2D; \n\
      MAD col,col,a.x,a.b; \n\
      ### fade texture \n\
      DP3 col.xyz,col,c0; \n\
      ADD col.xyz,col,c0.w; \n\
      ### replace bathymetry \n\
      SUB vtx.y,fragment.texcoord[0].z,c1.w; \n\
      CMP col.xyz,vtx.y,c1,col; \n\
      ### blend with color map \n\
      SUB vtx.z,fragment.texcoord[0].z,c5.x; \n\
      MUL vtx.z,-vtx.z,c5.y; \n\
      MOV vtx.y,c5.z; \n\
      MAD vtx.x,vtx.z,t.x,t.y; \n\
      TEX colt,vtx,texture[1],2D; \n\
      LRP colt.xyz,colt.w,colt,c5.w; \n\
      MUL colt.xyz,colt,col; \n\
      CMP col.xyz,-vtx.z,colt,col; \n\
      ### modulate with contours \n\
      MUL vtx.y,fragment.texcoord[0].z,-c2.x; \n\
      FRC vtx.y,vtx.y; \n\
      MAD vtx.y,vtx.y,c2.z,-c2.w; \n\
      ABS vtx.y,vtx.y; \n\
      SUB vtx.y,c4.w,vtx.y; \n\
      MUL_SAT vtx.y,vtx.y,c2.y; \n\
      MUL col.xyz,col,vtx.y; \n\
      ### modulate with fragment color \n\
      MUL col,col,fragment.color; \n\
      ### modulate with directional light \n\
      MOV nrm,fragment.texcoord[1]; \n\
      DP3 len.x,nrm,nrm; \n\
      RSQ len.x,len.x; \n\
      MUL nrm,nrm,len.x; \n\
      DP3_SAT nrm.z,nrm,l; \n\
      MAD nrm.z,nrm.z,p.x,p.y; \n\
      MUL_SAT col.xyz,col,nrm.z; \n\
      ### modulate with spherical fog \n\
      MOV fog.x,fragment.fogcoord.x; \n\
      MAD_SAT fog.x,fog.x,c3.x,c3.y; \n\
      POW fog.x,fog.x,c3.z; \n\
      LRP col.xyz,fog.x,c4,col; \n\
      ### write resulting color \n\
      MOV result.color,col; \n\
      END \n";

   // fragment program for the sea surface
   static const char *fragprog2="!!ARBfp1.0 \n\
      PARAM c0=program.env[0]; \n\
      PARAM c1=program.env[1]; \n\
      PARAM c2=program.env[2]; \n\
      PARAM c3=program.env[3]; \n\
      PARAM c4=program.env[4]; \n\
      PARAM a=program.env[8]; \n\
      PARAM t=program.env[9]; \n\
      PARAM l=program.env[10]; \n\
      PARAM p=program.env[11]; \n\
      TEMP col,tex,nrm,len,fog; \n\
      ### fetch fragment color \n\
      MOV col,fragment.color; \n\
      ### modulate with texture color \n\
      TEX tex,fragment.texcoord[0],texture[0],2D; \n\
      MAD tex,tex,a.x,a.b; \n\
      LRP col.xyz,c0.x,tex,col; \n\
      ### modulate with directional light \n\
      MOV nrm,fragment.texcoord[1]; \n\
      DP3 len.x,nrm,nrm; \n\
      RSQ len.x,len.x; \n\
      MUL nrm,nrm,len.x; \n\
      DP3_SAT nrm.z,nrm,l; \n\
      MAD nrm.z,nrm.z,p.x,p.y; \n\
      MUL_SAT col.xyz,col,nrm.z; \n\
      ### modulate with spherical fog \n\
      MOV fog.x,fragment.fogcoord.x; \n\
      MAD_SAT fog.x,fog.x,c3.x,c3.y; \n\
      POW fog.x,fog.x,c3.z; \n\
      LRP col.xyz,fog.x,c4,col; \n\
      ### write resulting color \n\
      MOV result.color,col; \n\
      END \n";

   // set primary sea color
   cache->setseacolor(seacolor[0],seacolor[1],seacolor[2],seatrans);

   // use default vertex shader plugin
   cache->setvtxshader();
   cache->usevtxshader(1);

   // calculate the fog parameters
   if (fogstart<fogend)
      {
      fog_a=fsqr(scale/fogend);
      fog_b=0.0f;
      fog_c=log(fmin(0.5f*fogdensity,1.0f))/(2.0f*log(fogstart/fogend/2.0f+0.5f));
      }
   else
      {
      fog_a=0.0f;
      fog_b=0.0f;
      fog_c=1.0f;
      }

   // calculate the bathymetry parameters
   if (exaggeration*(bathystart-bathyend)!=0.0f)
      {
      bathy_a=bathystart*exaggeration/scale;
      bathy_b=scale/(exaggeration*(bathystart-bathyend));
      bathy_c=0.5f;
      }
   else
      {
      bathy_a=0.0f;
      bathy_b=0.0f;
      bathy_c=0.0f;
      }

   // calculate the contour parameters
   if (exaggeration*contours!=0.0f)
      {
      cnt_a=scale/(exaggeration*fabs(contours));
      cnt_b=5.0f;
      cnt_c=2.0f;
      cnt_d=1.0f;
      }
   else
      {
      cnt_a=0.0f;
      cnt_b=1.0f;
      cnt_c=0.0f;
      cnt_d=0.0f;
      }

   // calculate the NPR parameters
   if (fadefactor<0.0f)
      {
      npr_a=0.3f*(1.0f+fadefactor);
      npr_b=0.6f*(1.0f+fadefactor);
      npr_c=0.1f*(1.0f+fadefactor);
      npr_d=0.0f;
      }
   else
      {
      npr_a=0.3f*(1.0f-fadefactor);
      npr_b=0.6f*(1.0f-fadefactor);
      npr_c=0.1f*(1.0f-fadefactor);
      npr_d=fadefactor;
      }

   // use pixel shader plugin
   cache->setpixshader((bathystart==bathyend || NPRBATHYMAP==NULL)?fragprog1A:fragprog1B);
   cache->setpixshaderparams(npr_a,npr_b,npr_c,npr_d);
   cache->setpixshaderparams(seagray,seagray,seagray,sealevel/scale,1);
   cache->setpixshaderparams(cnt_a,cnt_b,cnt_c,cnt_d,2);
   cache->setpixshaderparams(fog_a,fog_b,fog_c,0.0f,3);
   cache->setpixshaderparams(fogcolor[0],fogcolor[1],fogcolor[2],1.0f,4);
   cache->setpixshaderparams(bathy_a,bathy_b,bathy_c,1.0f,5);
   cache->usepixshader(1);
   cache->setseashader(fragprog2);
   cache->setseashaderparams(0.0f,0.0f,0.0f,0.0f);
   cache->setseashaderparams(fog_a,fog_b,fog_c,0.0f,3);
   cache->setseashaderparams(fogcolor[0],fogcolor[1],fogcolor[2],0.0f,4);
   cache->useseashader(1);

   // set bathymetry color map
   if (NPRBATHYMAP!=NULL && NPRBATHYMOD!=0)
      {
      cache->setpixshadertex(NPRBATHYMAP,NPRBATHYWIDTH,NPRBATHYHEIGHT,NPRBATHYCOMPS);

      NPRBATHYMOD=0;
      VISBATHYMOD=1;
      }
   }

// set bathymetry color map for NPR shader
void minishader::setNPRbathymap(unsigned char *bathymap,
                                int bathywidth,int bathyheight,int bathycomps)
   {
   NPRBATHYMAP=bathymap;

   NPRBATHYWIDTH=bathywidth;
   NPRBATHYHEIGHT=bathyheight;
   NPRBATHYCOMPS=bathycomps;

   NPRBATHYMOD=1;
   }

// disable vertex and pixel shaders
void minishader::unsetshaders(minicache *cache)
   {
   cache->usevtxshader(0);
   cache->usepixshader(0);
   cache->useseashader(0);
   }
