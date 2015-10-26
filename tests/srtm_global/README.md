For visualization:

    cat srtm_global.layout  | sed 's/[SN]..[EW]....hgt/X/g' | sed 's/  */ /g' | less -S

To relocate files:

    find . -iname "*hgt" | xargs -n 1 -P 20 -I {} mv {} ./