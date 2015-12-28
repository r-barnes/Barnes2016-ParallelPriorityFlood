Acquire data from `http://e4ftl01.cr.usgs.gov/SRTM/SRTMGL1.003/2000.02.11/` with
e.g.

    wget -r --no-parent --continue --no-clobber http://e4ftl01.cr.usgs.gov/SRTM/SRTMGL1.003/2000.02.11/

Information on the dataset is available from:

    https://lpdaac.usgs.gov/dataset_discovery/measures/measures_products_table/srtmgl1

For visualization:

    cat srtm_global.layout  | sed 's/[SN]..[EW]....hgt/X/g' | sed 's/  */ /g' | less -S

To relocate files:

    find . -iname "*hgt" | xargs -n 1 -P 20 -I {} mv {} ./

To resample to 3x higher resolution run `srtm_resample.sh`