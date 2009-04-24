#!/bin/sh

echo Testing
    if [ ! -f $HOME/.povray.conf ]
	then echo "[File I/O Security]" > $HOME/.povray.conf
	echo "none" >> $HOME/.povray.conf
    fi
../src/povray -f -h400 -w600 -ipvdemo.pov ../ini/povray.ini && echo "POV-Ray has build successfully"
rm -f $HOME/.povray.conf
