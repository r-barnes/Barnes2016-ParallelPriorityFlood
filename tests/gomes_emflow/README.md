This is a clone of commit 0ca9e0ef0433cbd6097106385fdc99ab83b42e0d of
"https://github.com/guipenaufv/EMFlow", only the relevant variables listed below
have been changed.

Run `make` to compile the program, the appropriate memory (2GB) and tile size
(400x400) variables have already been set.

gdal_merge.py
gdal_translate -srcwin 0 0 40000 40000 input.raster output.raster



EMFlow
======
Compute the drainage network on digital elevation models stored in external memory.

Compilation
-----------
To compile, run **make** or use:

    g++ -O3 -o EMFlow hydrogEmFillFlow.cpp lz4.c

Running
-------
To run, need set up the amount of memory available (line 77) and tiled block size (line 76) on code hydrogEmFillFlow.cpp and need set up the amount of memory available (line 44) and tiled block size (line 43) on code flow.cpp.

    ./EMFlow nrows terrain_name.hgt

About the Script
----------------
* .creat folder tiles to store temporary tiles, the tiles folder is necessary for the program operation
* .creat folder tempos to store the running time
* .Run the program for several instances and stores the runtime

Output
------
As a result it is given a file named flow.hgt containing the accumulated flow with 2 bytes.

Bibliography
------------
This program is described by the publication

Thiago L. Gomes, Salles V. G. Magalhães, Marcus V. A. Andrade, W. Randolph Franklin, and Guilherme C. Pena. 2012. Computing the drainage network on huge grid terrains. In Proceedings of the 1st ACM SIGSPATIAL International Workshop on Analytics for Big Geospatial Data (BigSpatial '12). ACM, New York, NY, USA, 53-60. DOI=10.1145/2447481.2447488 http://doi.acm.org/10.1145/2447481.2447488 ([Download](http://dx.doi.org/10.1145/2447481.2447488))
