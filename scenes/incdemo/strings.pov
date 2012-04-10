//	Persistence of Vision Raytracer Scene Description File
//	File: strings.pov
//	Author: Chris Huff
//	Description: A demo of the string macros in strings.inc.
//	This file will not render an image, check the debug messages
//	for the results.
//
// -f
//
//*******************************************

#version 3.6;
global_settings {assumed_gamma 1.0}

#include "strings.inc"

#debug concat("\n",CRGBFTStr(color rgbft <0.2, 0.3, 0.4, 0.5, 0.6>, 5, 5),"\n")
#debug concat(CRGBStr(color rgb <0.2, 0.3, 0.4>, 5, 5),"\n")

#debug concat(Triangle_Str(<0, 0, 0>, <1, 0, 0>, <0.5, 1, 0>),"\n")
#debug concat(Smooth_Triangle_Str(<0, 0, 0>, <-0.2,-0.1,-1>, <1, 0, 0>, <0.2,-0.1,-1>, <0.5, 1, 0>, <0, 0.2,-1>),"\n\n")

//*******************************************

