//	Persistence of Vision Raytracer Version 3.5 Scene Description File
//	File: media5.pov
//	Author: Chris Young
//	Description:
// This scene shows the use of a density_map.
//
//*******************************************

#include "colors.inc"

camera {location  <0, 0, -100>}

plane { z, 50
   pigment {checker Yellow, White scale 20}
   finish {ambient 0.2  diffuse 0.8}
   hollow
}

// Declare 2 similar density.  Only the color map differs.

#declare Density1=
  density {
    spherical
    ramp_wave
    color_map {
      [0.0 color rgb <0.0, 0.0, 0.0>]
      [0.2 color rgb <1.0, 0.3, 0.1>]
      [1.0 color rgb <1.0, 1.0, 0.1>]
    }
  }

#declare Density2=
  density {
    boxed
    ramp_wave
    color_map {
      [0.0 color rgb <0.0, 0.0, 0.0>]
      [0.2 color rgb <0.1, 1.0, 0.3>]
      [1.0 color rgb <0.1, 1.0, 1.0>]
    }
  }

sphere { 0, 2
  pigment { color rgbf<1, 1, 1, 1> }
  finish { ambient 0 diffuse 0 }
  interior {
    media {
      emission 0.05
      intervals 1
      samples 5
      method 3
/*      intervals 5
      samples 1, 10
      confidence 0.9999
      variance 1/1000*/
      density {
        gradient x
        density_map {
          [0.5 Density1]
          [0.5 Density2]
        }
      }
    }
  }
  scale 24
  hollow
}

light_source {
  <500, 600, -500>
  color White
}

