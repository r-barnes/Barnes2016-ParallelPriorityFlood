These directories contain information useful in acquiring and performing the
tests described in the manuscript.


File size statistics can be generated with:

    find . -type f -printf '%s\n' | awk '{s+=$0} END {printf "Count: %u\nTotal size: %.2f\nAverage size: %.2f\n", NR, s, s/NR}'