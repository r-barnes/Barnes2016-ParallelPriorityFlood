After unpacking the NED tiles, grab only the files which begin with `imgn` and
end with `img`.

Acquire data from:

    ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/Elevation/13/IMG/

View map with

    cat ned_tiles.layout | sed 's/..imgn..........img/X/g' | sed 's/  */ /g' | less -S