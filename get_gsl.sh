#!/bin/bash

# Arguments passed by makefile: root and target directories and GSL version
dir_root="$1"
dir_gsl="$2"
version="$3"
echo "Downloading and installing GSL" "$version" "to" "$dir_gsl"

# Exit if any command fails
set -e
# Echo commands
set -o xtrace

# Download and extract
cd "$dir_root"
if ! [ -d gsl-"$version" ]; then
    if ! [ -f gsl-"$version".tar.gz ]; then
        wget ftp://ftp.gnu.org/gnu/gsl/gsl-"$version".tar.gz
    fi
    tar -zxvf gsl-"$version".tar.gz
fi
mkdir -p "$dir_gsl"

# Compile
cd gsl-"$version"
# If this is called via make on a mac, the next line will fail unless run as
# a superuser, because ./configure is a separately-compiled C program.
# To solve this, either run `sudo make`, or cut and paste the following lines,
# starting with cd articpy; ./configure --prefix="../gsl"
./configure --prefix="$dir_gsl"
make -j
make check -j
make install -j

# Clean up
cd ..
rm -rf gsl-"$version"*
#rm -rf build
