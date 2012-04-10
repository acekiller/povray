// Persistence Of Vision raytracer sample file.
//
// -w320 -h240
// -w800 -h600 +a0.3

#version 3.6;
global_settings {assumed_gamma 2.2}

#include "colors.inc"
#include "skies.inc"

camera {
    location <0, 1, -100>
    up y
    right x*1.33
    direction z
    look_at <0 20 0>
    angle 57
}

light_source { <100, 100, -50> White }

sky_sphere { S_Cloud5 }
plane { y, 0 pigment { color red 0.3 green 0.75 blue 0.5} }

