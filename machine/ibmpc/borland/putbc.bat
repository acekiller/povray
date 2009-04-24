@echo off
rem
rem
if not "%1" == "" goto NotEmpty
echo This batch file copies all files for compiling POV-Ray with Borland C++
echo to a working directory you specify.
echo For example: "PUTBC C:\POVWORK" copies files to the C:\POVWORK directory.
goto Done
:NotEmpty
if not exist %1\nul echo Directory %1 doesn't exist!
if not exist %1\nul goto Done
rem
rem
echo on
copy ibmbc*.* %1
copy ..\ibm.c %1
copy ..\ibmconf.h %1\config.h
copy ..\tiga\*.* %1
copy ..\..\..\source\*.c %1
copy ..\..\..\source\*.h %1
:done
