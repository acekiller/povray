// Persistence Of Vision raytracer version 3.5 sample file.
//
// -w320 -h240
// -w800 -h600 +a0.3

// By Alexander Enzmann
/* sample quartic scene file written by Alexander Enzmann */
#version  3.6;
global_settings { 
  assumed_gamma 1.0
}

#include "shapes.inc"
#include "colors.inc"
#include "textures.inc"

/* Cubic curve - can make a nice teardrop by cleverly adding a clipping plane */
intersection {
   quartic {
      < 0.0,  0.0,  0.0,  -0.5, 0.0,  0.0,  0.0,  0.0,  0.0, -0.5,
        0.0,  0.0,  0.0,   0.0, 0.0,  0.0,  0.0,  0.0,  0.0,  0.0,
        0.0,  0.0,  0.0,   0.0, 0.0,  1.0,  0.0,  0.0,  0.0,  0.0,
        0.0,  0.0,  1.0,   0.0, 0.0 >

      texture {
         pigment { Red }
         finish {
            phong 1.0
            diffuse 0.8
            ambient 0.2
         }
      }
   }

   sphere {
      <0, 0, 0>, 2
      texture { pigment { Clear } }
   }

   bounded_by { sphere { <0, 0, 0>, 2.2 } }
   rotate 20*y
}

camera {
   location  <0.0, 0.0, -5.0>
   direction <0.0, 0.0,  1.0>
   up        <0.0, 1.0,  0.0>
   right     <4/3, 0.0,  0.0>
}

light_source { <200, 30, -300> colour White }

light_source { <-200, 30, -300> colour White }
