#!/bin/sh

echo -n 'static char const rcsid[] = "$Revision: 1.03-r' > version.h
svnversion -n . >> version.h
echo -n ' $";' >> version.h
echo >> version.h
