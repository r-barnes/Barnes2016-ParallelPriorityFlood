Shuttle Radar Topography Mission (SRTM) Regional DEMs
=====================================================

Stats
-----
Cells:       12,967,201  (per tile)
Dimensions:  3601 x 3601 (per tile)
Tiles:       152 - 169
Total cells: 1,971,014,552 - 2,191,456,969
Orientation: Flip tiles vertically and horizontally



Acquisition
-----------
Acquire data from
    
    http://dds.cr.usgs.gov/srtm/version2_1/SRTM1/

using, e.g.,

    wget -r --no-parent --continue --no-clobber http://dds.cr.usgs.gov/srtm/version2_1/SRTM1/

To unzip the files use, for example:

    cd dds.cr.usgs.gov/srtm/version2_1/SRTM1/
    find . -iname "*zip" | xargs -n 1 -P 20 unzip



Testing
-------
I have created a layout file for a small nine-tile subset of Region 3 which can
be used for correctness testing of the algorithm. This layout file is called
`nasa_srtm3_small.layout`.

To set up the test use the following code to acquire and unpack the data files.

    #Make directory for data files
    mkdir small_test

    #Download data files from the interwebs
    wget -O small_test/N40W088.hgt.zip http://dds.cr.usgs.gov/srtm/version2_1/SRTM1/Region_03/N40W088.hgt.zip
    wget -O small_test/N40W089.hgt.zip http://dds.cr.usgs.gov/srtm/version2_1/SRTM1/Region_03/N40W089.hgt.zip
    wget -O small_test/N40W090.hgt.zip http://dds.cr.usgs.gov/srtm/version2_1/SRTM1/Region_03/N40W090.hgt.zip
    wget -O small_test/N41W088.hgt.zip http://dds.cr.usgs.gov/srtm/version2_1/SRTM1/Region_03/N41W088.hgt.zip
    wget -O small_test/N41W089.hgt.zip http://dds.cr.usgs.gov/srtm/version2_1/SRTM1/Region_03/N41W089.hgt.zip
    wget -O small_test/N41W090.hgt.zip http://dds.cr.usgs.gov/srtm/version2_1/SRTM1/Region_03/N41W090.hgt.zip
    wget -O small_test/N42W088.hgt.zip http://dds.cr.usgs.gov/srtm/version2_1/SRTM1/Region_03/N42W088.hgt.zip
    wget -O small_test/N42W089.hgt.zip http://dds.cr.usgs.gov/srtm/version2_1/SRTM1/Region_03/N42W089.hgt.zip
    wget -O small_test/N42W090.hgt.zip http://dds.cr.usgs.gov/srtm/version2_1/SRTM1/Region_03/N42W090.hgt.zip

    #Enter the directory
    cd small_test

    #Unpack the data files
    find *zip | xargs -n 1 unzip

    #Remove the ZIP files since they've been unpacked
    rm -f *zip

    #Copy the layout file into this directory
    cp ../nasa_srtm3_small.layout ./

    #Go back to the parent directory
    cd ..