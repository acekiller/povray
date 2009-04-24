@echo off
rem
rem
if not "%1" == "" goto NotEmpty
echo This batch file copies all files for compiling POV-Ray with Borland C++
echo from a working directory you specify into this BORLAND directory,
echo as well as the IBMPC and SOURCE directories
echo For example: "GETBC C:\POVWORK" copies files from 
echo               the C:\POVWORK directory
goto Done
:NotEmpty
if not exist %1\nul echo Directory %1 doesn't exist!
if not exist %1\nul goto Done
rem
echo if not exist %%1 echo File %%1 not found > tcopy.bat
echo if not exist %%1 goto Over >> tcopy.bat
echo copy %%1 %%2 >> tcopy.bat
echo :Over >> tcopy.bat
rem
call tcopy %1\IBMBC.MAK  ibmbc.mak
call tcopy %1\IBMBC.LNK  ibmbc.lnk
call tcopy %1\IBM.C      ..\ibm.c
call tcopy %1\CONFIG.H   ..\ibmconf.h
call tcopy %1\AI.LIB     ..\tiga
call tcopy %1\EXTEND.H   ..\tiga
call tcopy %1\TIGA.H     ..\tiga
call tcopy %1\TIGADEFS.H ..\tiga
call tcopy %1\GIFDECOD.C ..\..\..\source
call tcopy %1\OBJECTS.C  ..\..\..\source
call tcopy %1\CSG.C      ..\..\..\source
call tcopy %1\BLOB.C     ..\..\..\source
call tcopy %1\BEZIER.C   ..\..\..\source
call tcopy %1\HFIELD.C   ..\..\..\source
call tcopy %1\LIGHTING.C ..\..\..\source
call tcopy %1\TEXTURE.C  ..\..\..\source
call tcopy %1\MATRICES.C ..\..\..\source
call tcopy %1\IFF.C      ..\..\..\source
call tcopy %1\PIGMENT.C  ..\..\..\source
call tcopy %1\EXPRESS.C  ..\..\..\source
call tcopy %1\BOUND.C    ..\..\..\source
call tcopy %1\SPHERES.C  ..\..\..\source
call tcopy %1\RENDER.C   ..\..\..\source
call tcopy %1\POINT.C    ..\..\..\source
call tcopy %1\PARSE.C    ..\..\..\source
call tcopy %1\QUADRICS.C ..\..\..\source
call tcopy %1\POLY.C     ..\..\..\source
call tcopy %1\CONES.C    ..\..\..\source
call tcopy %1\CAMERA.C   ..\..\..\source
call tcopy %1\COLOUR.C   ..\..\..\source
call tcopy %1\POVRAY.C   ..\..\..\source
call tcopy %1\RAY.C      ..\..\..\source
call tcopy %1\DISCS.C    ..\..\..\source
call tcopy %1\TARGA.C    ..\..\..\source
call tcopy %1\TRIANGLE.C ..\..\..\source
call tcopy %1\DUMP.C     ..\..\..\source
call tcopy %1\PLANES.C   ..\..\..\source
call tcopy %1\GIF.C      ..\..\..\source
call tcopy %1\RAW.C      ..\..\..\source
call tcopy %1\VECT.C     ..\..\..\source
call tcopy %1\TXTTEST.C  ..\..\..\source
call tcopy %1\TOKENIZE.C ..\..\..\source
call tcopy %1\IMAGE.C    ..\..\..\source
call tcopy %1\NORMAL.C   ..\..\..\source
call tcopy %1\BOXES.C    ..\..\..\source
call tcopy %1\POVPROTO.H ..\..\..\source
call tcopy %1\TEXTURE.H  ..\..\..\source
call tcopy %1\VECTOR.H   ..\..\..\source
call tcopy %1\PARSE.H    ..\..\..\source
call tcopy %1\FRAME.H    ..\..\..\source
:done
del tcopy.bat
