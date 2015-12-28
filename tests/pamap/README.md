Running with `@offloadall` on `pan_tiles_small.job` had a Producer Overall time
of 6.5891s for the 9 tiles.

Since `pan_tiles.layout` specifies 6666 tiles, the approximate completion time
is 6666/9*6.5891=4880.326s. Adding a 50% margin of safety gives 7320.4901s (2hr
2min). This ran out of time on the second part of the algorithm at approximately
block 1500, so things are taking longer than expected. Perhaps there is I/O
contention? Set time for 4hr 30min.

Note: Tiles must be flipped vertically to fit together in the algorithm.



%NOTE: PAMAP North and South tiles cannot be mixed together!

%ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/Elevation/
%NED data are available nationally and in Puerto Rico at resolutions of 1
%arc-second (about 30 meters) and 1/3 arc-second (about 10 meters), and in
%limited areas at 1/9 arc-second (about 3 meters). In most of Alaska, only lower
%resolution source data are available. As a result, most NED data for Alaska are
%at 2-arc-second (about 60 meters) grid spacing. Part of Alaska is available at
%the 1- and 1/3-arc-second resolution.

%National map 10m resolution has ~80,804,643,000 cells

%ftp://rockyftp.cr.usgs.gov/vdelivery/Datasets/Staged/Elevation/13/IMG/
%http://nationalmap.gov/3DEP/3dep_prodmetadata.html

%Details: http://www.dcnr.state.pa.us/topogeo/pamap/lidar/index.htm

%Data units are in feet, and all data use the NAD83 horizontal datum, GRS80
%ellipsoid, NAVD88 vertical datum, and GEOID03 National Geodetic Survey.

%The lidar-derived data products were clipped to the PAMAP tiles and are
%organized into datasets by collection year. The resulting digital data files
%have names that start with a concatenation of the first four digits of the
%State Plane northing and easting that defines the northwest corner of the tile
%covered by the data. This number is followed by the state identifier “PA” and
%the State Plane zone “N” or “S.” The counties flown each year and the
%associated State Plane zones are illustrated on the map below. There is about
%one tile of overlap between the two zones.

%lftp pamap.pasda.psu.edu
%mirror -c -P 4 DEM
