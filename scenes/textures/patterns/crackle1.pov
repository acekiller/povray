// Persistence Of Vision raytracer sample file.
// Crackle pattern example
//
// -w320 -h240
// -w800 -h600 +a0.3

#version 3.6;

global_settings {
  assumed_gamma 1.0
}

#include "colors.inc"

#declare T1=
 texture{
   pigment{
     crackle color_map{[0.0 Black][1.0 White]}
     scale 0.24
   }
 }

#declare T2=
 texture{
   pigment{White}
   normal{
     crackle 0.6
     scale 0.24
   }
   finish{phong 0.8 phong_size 200}
 }

#include "pignorm.inc"
