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
./configure --prefix="$dir_gsl"
make -j
make check -j
make install -j

# Clean up
cd ..
rm -rf gsl-"$version"*
