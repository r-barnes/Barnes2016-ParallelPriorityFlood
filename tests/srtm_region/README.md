Acquire data from `http://dds.cr.usgs.gov/srtm/version2_1/SRTM1/` with, e.g.,

    wget -r --no-parent --continue --no-clobber http://dds.cr.usgs.gov/srtm/version2_1/SRTM1/

    wget -r --no-parent --continue --no-clobber http://e4ftl01.cr.usgs.gov/SRTM/SRTMGL1.003/2000.02.11/

https://lpdaac.usgs.gov/dataset_discovery/measures/measures_products_table/srtmgl1

http://e4ftl01.cr.usgs.gov/SRTM/SRTMGL1.003/2000.02.11/

cd dds.cr.usgs.gov/srtm/version2_1/SRTM1/
find . -iname "*zip" | xargs -n 1 -P 20 unzip