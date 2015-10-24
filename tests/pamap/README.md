Running with `@offloadall` on `pan_tiles_small.job` had a Producer Overall time
of 6.5891s for the 9 tiles.

Since `pan_tiles.layout` specifies 6666 tiles, the approximate completion time
is 6666/9*6.5891=4880.326s. Adding a 50% margin of safety gives 7320.4901s (2hr
2min). This ran out of time on the second part of the algorithm at approximately
block 1500, so things are taking longer than expected. Perhaps there is I/O
contention? Set time for 4hr 30min.

Note: Tiles must be flipped vertically to fit together in the algorithm.