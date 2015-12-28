Acquire data from `http://dds.cr.usgs.gov/srtm/version2_1/SRTM1/` with, e.g.,

    wget -r --no-parent --continue --no-clobber http://dds.cr.usgs.gov/srtm/version2_1/SRTM1/

To unzip the files use:

    cd dds.cr.usgs.gov/srtm/version2_1/SRTM1/
    find . -iname "*zip" | xargs -n 1 -P 20 unzip

To get the small test region in `nasa_srtm3_small.layout` use the following:

    mkdir small_test
    wget -O small_test/N40W088.hgt.zip http://dds.cr.usgs.gov/srtm/version2_1/SRTM1/Region_03/N40W088.hgt.zip
    wget -O small_test/N40W089.hgt.zip http://dds.cr.usgs.gov/srtm/version2_1/SRTM1/Region_03/N40W089.hgt.zip
    wget -O small_test/N40W090.hgt.zip http://dds.cr.usgs.gov/srtm/version2_1/SRTM1/Region_03/N40W090.hgt.zip
    wget -O small_test/N41W088.hgt.zip http://dds.cr.usgs.gov/srtm/version2_1/SRTM1/Region_03/N41W088.hgt.zip
    wget -O small_test/N41W089.hgt.zip http://dds.cr.usgs.gov/srtm/version2_1/SRTM1/Region_03/N41W089.hgt.zip
    wget -O small_test/N41W090.hgt.zip http://dds.cr.usgs.gov/srtm/version2_1/SRTM1/Region_03/N41W090.hgt.zip
    wget -O small_test/N42W088.hgt.zip http://dds.cr.usgs.gov/srtm/version2_1/SRTM1/Region_03/N42W088.hgt.zip
    wget -O small_test/N42W089.hgt.zip http://dds.cr.usgs.gov/srtm/version2_1/SRTM1/Region_03/N42W089.hgt.zip
    wget -O small_test/N42W090.hgt.zip http://dds.cr.usgs.gov/srtm/version2_1/SRTM1/Region_03/N42W090.hgt.zip

    cd small_test
    find *zip | xargs -n 1 unzip

    rm -f *zip

    gdal_merge.py -o srtm_merged.tif -of GTiff -ot Int16 -n -32768 -a_nodata -32768 *hgt

    cp ../nasa_srtm3_small.layout ./

    cd ..