/* 
 * $Id: tip.pov,v 1.3 2004/02/01 09:39:56 petterik Exp $
 *
 * Original code was written by Paul D. Bourke
 * http://astronomy.swin.edu.au/~pbourke/povray/df3/
 */

#include "metals.inc"

#declare RADIUS = 0.3;
#declare NX = 32;
#declare NY = 32;
#declare NZ = 95;
#declare DD = <NX,NY,NZ>;
#declare CC = DD / 2;
#declare VP = <120*cos(2*pi*clock),120*sin(2*pi*clock),0>;

global_settings { 
	ambient_light <1,1,1> 
	assumed_gamma 1
}

camera {
   location VP
   up y
   right x
   angle 60
   sky <0,0,1>
   look_at <0,0,0>
}

light_source {
   VP + <0,0,NZ/2>
   color rgb <1,1,1>
   media_interaction on
   media_attenuation on
   shadowless
}


/*
#declare theinterior = interior {
   media {
      intervals 100
		ratio 0.5
      samples 4,4
		method 2
      emission <1,1,1> / 10
      absorption <1,1,1> / 30
      scattering { 1, <0,0,0> }
      confidence 0.999
      variance 1/1000
      density {
         density_file df3 "tip.df3" 
			interpolate 1 
            color_map {
                [0.0 rgb <0,0,0>]
                [1.0 rgb <1,1,1>]
            }
      }
   }
}
*/

box {
   <0,0,0>, <1,1,1>
   pigment { rgbf 1 }
   /*interior { theinterior }*/
   hollow
	translate <-0.5,-0.5,-0.5>
	scale DD
}

#declare bbox = texture {
   pigment { rgb <0.5,0.5,0.5> }
   finish { F_MetalB }
}

/* Corners of box */
union {
	sphere { <0,0,0>, RADIUS texture {bbox} }
	sphere { <0,NY,0>, RADIUS texture {bbox} }
	sphere { <NX,NY,0>, RADIUS texture {bbox} }
	sphere { <NX,0,0>, RADIUS texture {bbox} }
	sphere { <0,0,NZ>, RADIUS texture {bbox} }
	sphere { <0,NY,NZ>, RADIUS texture {bbox} }
	sphere { <NX,NY,NZ>, RADIUS texture {bbox} }
	sphere { <NX,0,NZ>, RADIUS texture {bbox} }
	translate -CC
}

/* Main border of box */
union {
	cylinder { <0,0,0>, <NX,0,0>, RADIUS texture {bbox} }
	cylinder { <0,0,0>, <0,NY,0>, RADIUS texture {bbox} }
	cylinder { <0,0,0>, <0,0,NZ>, RADIUS texture {bbox} }
	cylinder { <NX,NY,NZ>, <0,NY,NZ>, RADIUS texture {bbox} }
	cylinder { <NX,NY,NZ>, <NX,0,NZ>, RADIUS texture {bbox} }
	cylinder { <NX,NY,NZ>, <NX,NY,0>, RADIUS texture {bbox} }
	cylinder { <0,0,NZ>, <NX,0,NZ>, RADIUS texture {bbox} }
	cylinder { <0,0,NZ>, <0,NY,NZ>, RADIUS texture {bbox} }
	cylinder { <NX,NY,0>, <NX,0,0>, RADIUS texture {bbox} }
	cylinder { <NX,NY,0>, <0,NY,0>, RADIUS texture {bbox} }
	cylinder { <0,NY,0>, <0,NY,NZ>, RADIUS texture {bbox} }
	cylinder { <NX,0,0>, <NX,0,NZ>, RADIUS texture {bbox} }
	translate -CC
}

/* file tip.inc is created by xyz2df3.tcl */
#include "tip.inc"

/*
cylinder { 
	<15,15,0>, <15,15,96>, 10
 	scale <1,1,1> 
	rotate<0,0,0>
	translate -CC
	texture {
		pigment {rgb <1.0, 0.5, 0.5> filter 0.9}
		finish {diffuse 0.1 ambient 0.1 phong 0.1}
	}
}
*/
