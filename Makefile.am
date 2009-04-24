## Process this file with automake to produce Makefile.in
#
#   Makefile.am
#
#  from Persistence of Vision Raytracer
#  Copyright 1995,1999 Persistence of Vision Team
#---------------------------------------------------------------------------
#  NOTICE: This source code file is provided so that users may experiment
#  with enhancements to POV-Ray and to port the software to platforms other
#  than those supported by the POV-Ray Team.  There are strict rules under
#  which you are permitted to use this file.  The rules are in the file
#  named POVLEGAL.DOC which should be distributed with this file.
#  If POVLEGAL.DOC is not available or for more info please contact the POV-Ray
#  Team Coordinator by email to team-coord@povray.org or visit us on the web at
#  http://www.povray.org. The latest version of POV-Ray may be found at this 
#  site.
#
# This program is based on the popular DKB raytracer version 2.12.
# DKBTrace was originally written by David K. Buck.
# DKBTrace Ver 2.0-2.12 were written by David K. Buck & Aaron A. Collins.
#
#
##############################################################################

povlibdir = @datadir@/povray-3.5
povray.ini: povray.in
	sed 's!the C:.POVRAY3 drive and!${povlibdir}!' povray.in > povray.ini
	echo >> povray.ini
	echo ";; Search path for #include source files or command line ini files not" >> povray.ini
	echo ";; found in the current directory.  New directories are added to the" >> povray.ini
	echo ";; search path, up to a maximum of 25." >> povray.ini
	echo >> povray.ini
	echo "Library_Path=${povlibdir}" >> povray.ini
	echo "Library_Path=${povlibdir}/include" >> povray.ini
	echo >> povray.ini
	echo ";; File output type control." >> povray.ini
	echo ";;     T    Uncompressed Targa-24" >> povray.ini
	echo ";;     C    Compressed Targa-24" >> povray.ini
	echo ";;     P    UNIX PPM" >> povray.ini
	echo ";;     N    PNG (8-bits per colour RGB)" >> povray.ini
	echo ";;     Nc   PNG (c'' bit per colour RGB where 5 <= c <= 16)" >> povray.ini
	echo >> povray.ini
	echo "Output_to_File=true" >> povray.ini
	echo "Output_File_Type=N8             ;; (+/-Ftype)" >> povray.ini
SUBDIRS = doc include ini scenes scripts src tests
docdir = ${datadir}/doc/@PACKAGE@-@VERSION@
doc_DATA = povlegal.doc povwhere.txt gamma.gif gamma.gif.txt README README.unix 
povconfdir = @sysconfdir@
sysconf_DATA = povray.conf povray.ini
TEST = tests/test.sh
EXTRA_DIST = doc include scenes scripts tests povlegal.doc povwhere.txt gamma.gif gamma.gif.txt README README.unix povray.in povray.ini povray.conf
POVINI = @datadir@/povray-3.5
clean-generic:
	rm -f povray.ini
