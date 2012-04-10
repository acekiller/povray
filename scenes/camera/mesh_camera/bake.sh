#!/bin/bash

#!/bin/bash

# Persistence Of Vision Raytracer sample file.

# bake all the meshes for the scene baking_demo.pov
# and repair the seams for each one
# Jaime Vives Piqueres, Jan. 2011  <jaime@ignorancia.org>

# /***************************************************************************
# * $File: //depot/povray/smp/distribution/scenes/camera/mesh_camera/bake.sh $
# * $Revision: #2 $
# * $Change: 5377 $
# * $DateTime: 2011/01/09 19:56:00 $
# * $Author: jholsenback $
# ***************************************************************************/

# vase 1
povray +Ibaking_demo +w512 +h512 +a +UA +K1 +Oim_vase1_baked.png +FN -p Declare=use_baking=1
povray +Ibaking_repair_seams +w512 +h512 +a +UA +K1 +Oim_vase1_baked.png +FN -p

# vase 2
povray +Ibaking_demo +w512 +h512 +a +UA +K2 +Oim_vase2_baked.png +FN -p Declare=use_baking=1
povray +Ibaking_repair_seams +w512 +h512 +a +UA +K2 +Oim_vase2_baked.png +FN -p

# room
povray +Ibaking_demo +w1024 +h1024 +a +UA +K3 +Oim_room_baked.png +FN -p Declare=use_baking=1
povray +Ibaking_repair_seams +w1024 +h1024 +a +UA +K3 +Oim_room_baked.png +FN -p
