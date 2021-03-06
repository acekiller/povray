// Persistence Of Vision raytracer sample file.
// Crackle pattern example
//
// -w320 -h240
// -w800 -h600 +a0.3

#version 3.6;
global_settings {assumed_gamma 2.2}

#include "colors.inc"
#include "shapes.inc"
#include "textures.inc"

camera {
    location -z*3.5
    direction z
    up y
    right x*1.3333
    look_at 0
}

light_source { <-200.0, 200.0, -800.0> colour White }

sphere { <-1.0 0.0 0.0> 1
    pigment {
        crackle
        colour_map {
            [0.05 colour rgb<0, 0, 0> ]
            [0.08 colour rgb<0, 1, 1> ]
            [0.10 colour rgb<0, 1, 1> ]
            [1.00 colour rgb<0, 0, 1> ]
        }
    scale 0.3
    }
    finish { Shiny }
}

sphere { <1.0 0.0 0.0> 1
    pigment {
        crackle
        turbulence 0.5
        colour_map {
            [0.05 colour rgb<0, 0, 0> ]
            [0.08 colour rgb<0, 1, 1> ]
            [0.10 colour rgb<0, 1, 1> ]
            [1.00 colour rgb<0, 0, 1> ]
        }
    scale 0.3
    }
    finish { Shiny }
}

