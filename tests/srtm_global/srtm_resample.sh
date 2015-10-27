#!/bin/bash  
#SBATCH --job-name="srtm_global_resample"  
#SBATCH --output="srtm_global_resample.%j.%N.out"  
#SBATCH --partition=compute  
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=24  
#SBATCH --export=ALL  
#SBATCH -t 08:00:00

#Deflation times and sizes of N44W094 by algorithm
#
#    Algorithm  Time  Size
#    Deflate     20s   46M
#    LZW         16s  101M
#    LZMA       388s   46M
#    None         4s  223M
#
#Therefore, we use the Deflate algorithm for its balance of speed and efficacy

cd /home/rbarnes1/scratch/srtm_global

find . -iname "*hgt" | xargs -t -n 1 -P 24 -I {} gdalwarp -co 'COMPRESS=DEFLATE' -ts 10803 10803 -ot Int16 {} {}.tif