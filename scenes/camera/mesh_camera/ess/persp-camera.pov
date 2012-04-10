// Persistence Of Vision Raytracer sample file.
// Demo for extreme super-sampling using the mesh_camera.
// See the README.txt file for more information.

/*****************************************************************************************
 * $File: //depot/povray/smp/distribution/scenes/camera/mesh_camera/ess/persp-camera.pov $
 * $Revision: #1 $
 * $Change: 5407 $
 * $DateTime: 2011/02/21 15:25:00 $
 * $Author: jholsenback $
 ****************************************************************************************/
#version 3.7;

global_settings { assumed_gamma 1.0 }

#include "colors.inc"
#include "stones.inc"

camera {
	   location <0, 0, -3>
	   look_at <0, 0, 0>
}

light_source {
			 <2, 4, -3>
			 color White
}

sphere {
	   <0, 0, 0>, 1
	   texture {
	   		   T_Stone25
			   scale 4
	   }
}

#local i = 0;
#while(i < 10)
		 #local j = 0;
		 #while(j < 10)
		 		  sphere {
		 	   	  		 <1+i/10, j/10, 0>, .001
						 texture {
						 		 pigment {color White*2}
			   			 }
         		  }
		 		  #local j = j + 1;
		 #end
		 #local i = i + 1;
#end
