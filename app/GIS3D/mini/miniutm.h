// (c) by Stefan Roettger

#ifndef MINIUTM_H
#define MINIUTM_H

class miniutm
   {
   public:

   static double EARTH_radius; // radius of the earth

   static double WGS84_r_major; // WGS84 semi-major axis
   static double WGS84_r_minor; // WGS84 semi-minor axis
   static double WGS84_f;       // WGS84 flattening
   static double WGS84_e2;      // WGS84 eccentricity squared
   static double WGS84_ed2;     // WGS84 eccentricity derived

   //! transform Lat/Lon to UTM
   static void LL2UTM(double lat,double lon, // geographic input coordinates in arc-seconds (WGS84 datum)
                      int zone,int datum, // UTM zone and datum of output coordinates
                      double *x,double *y); // output UTM coordinates (Easting and Northing)

   static void LL2UTM(double lat,double lon,
                      int zone,int datum,
                      float *x,float *y);

   //! transform UTM to Lat/Lon
   static void UTM2LL(double x,double y, // input UTM coordinates (Easting and Northing)
                      int zone,int datum, // UTM zone and datum of input coordinates
                      double *lat,double *lon); // geographic output coordinates in arc-seconds (WGS84 datum)

   static void UTM2LL(double x,double y,
                      int zone,int datum,
                      float *lat,float *lon);

   //! transform Lat/Lon/H to ECEF
   static void LLH2ECEF(double lat,double lon,double h, // geographic input coordinates in arc-seconds (WGS84 datum)
                        double xyz[3]); // output ECEF coordinates

   static void LLH2ECEF(double lat,double lon,double h,
                        float xyz[3]);

   //! transform ECEF to Lat/Lon/H
   static void ECEF2LLH(double xyz[3], // input ECEF coordinates
                        double *lat,double *lon,double *h); // geographic output coordinates in arc-seconds (WGS84 datum)

   static void ECEF2LLH(float xyz[3],
                        float *lat,float *lon,float *h);

   //! 1 arc-second equals about 30 meters
   static void arcsec2meter(double lat,double *as2m);
   static void arcsec2meter(double lat,float *as2m);

   protected:

   // UTM conversion functions
   static void initUTM(int zone);
   static void calcLL2UTM(double lat,double lon,double *x,double *y);
   static void calcUTM2LL(double x,double y,double *lat,double *lon);

   // ECEF conversion functions
   static void calcLLH2ECEF(double lat,double lon,double h,double xyz[3]);
   static void calcECEF2LLH(double xyz[3],double *lat,double *lon,double *h);

   // Molodensky transformation between two datums
   static void molodensky(int src,int dst,double *lat,double *lon);

   // Molodensky transformation based on ellipsoid change
   static void molodensky(double *lat,double *lon,double *h, // transformed coordinates
                          double r_maj,double f,             // semi-major axis and flattening
                          double dr_maj,double df,           // ellipsoid change
                          double dx,double dy,double dz);    // origin change

   private:

   static int act_datum;
   static int act_zone;

   static double r_major,r_minor;
   static double o_dx,o_dy,o_dz;

   static double scale_factor;

   static double lon_center;
   static double e0,e1,e2,e3;
   static double e,es,esp;
   static double false_northing;
   static double false_easting;

   static void choose_ellipsoid(double r_maj,double r_min);
   static void choose_ellipsoid_flat(double r_maj,double f);

   static void choose_datum(int ellipsoid,
                            double dx,double dy,double dz);

   static void choose_datum(int datum);
   };

namespace minilon {

// longitude arithmetic:

//! add longitudes
inline float LONADD(float a,float b=0.0f)
   {
   float lon=a+b;
   while (lon<0) lon+=360*60*60;
   while (lon>360*60*60) lon-=360*60*60;
   return(lon);
   }

//! subtract longitudes
inline float LONSUB(float a,float b=0.0f)
   {
   float diff=a-b;
   while (diff<-180*60*60) diff+=360*60*60;
   while (diff>180*60*60) diff-=360*60*60;
   return(diff);
   }

//! return leftmost longitude
inline float LONLEFT(float a,float b)
   {
   if (LONSUB(a,b)<0.0f) return(a);
   else return(b);
   }

//! return rightmost longitude
inline float LONRIGHT(float a,float b)
   {
   if (LONSUB(a,b)>0.0f) return(a);
   else return(b);
   }

//! linear interpolation of longitudes
inline float LONLERP(float a,float b,float lerp=0.5f)
   {
   float diff=LONADD(b,-a);
   return(LONSUB(a,-lerp*diff));
   }

//! average longitudes
inline float LONMEAN(float a,float b,float weight=0.5f)
   {
   if (LONSUB(a,b)<0.0f) return(LONLERP(a,b,weight));
   else return(LONLERP(b,a,1.0f-weight));
   }

}

using namespace minilon;

#endif
