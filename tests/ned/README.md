After unpacking the NED tiles, grab only the files which begin with `imgn` and
end with `img`.

View map with

    cat ned_tiles.layout | sed 's/..imgn..........img/X/g' | sed 's/  */ /g' | less -S