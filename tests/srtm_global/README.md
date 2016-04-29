Shuttle Radar Topography Mission (SRTM) DEM
===========================================
This is a 30m DEM covering the 80% of Earth's land area.

Stats
-----
Cells:       12,967,201  (per tile)
Dimensions:  3601 x 3601 (per tile)
Tiles:       14297
Total cells: 185,392,072,697
Orientation: Flip tiles vertically and horizontally



Overview
--------



Acquisition
-----------
Acquire data from 

    http://e4ftl01.cr.usgs.gov/SRTM/SRTMGL1.003/2000.02.11/

using, e.g., 

    lftp http://e4ftl01.cr.usgs.gov/SRTM/SRTMGL1.003/2000.02.11/
    mirror -P 4 .



Web Resources
-------------
 * https://lpdaac.usgs.gov/dataset_discovery/measures/measures_products_table/srtmgl1




To relocate files:

    find . -iname "*hgt" | xargs -n 1 -P 20 -I {} mv {} ./

To resample to 3x higher resolution run `srtm_resample.job`