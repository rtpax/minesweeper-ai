Minesweeper AI

A minesweeper game and an AI for solving it.

Note that for difficult (high bomb) board the odds of success with perfect play can by quite low,
and even for specific easy (low bomb) boards it can be below 50% (though higher on average)

general overview of how it works:
1:  Create sets of cells with known numbers of bombs, and by analyzing the overlapping of
    these sets determine the cells guaranteed to be bombs and nonbombs. Pick the guarantees 
    and repeat until none.
2:  Create a union of all these cells and determine the minimum and maximum values. The 
    order of unions can affect the given minimum and maximum values as it stands, so it is 
    important to pick a good order, the best order if feasable. If any guarantees arise, pick 
    them and go to the start
3:  Determine if how many of the leftover cells are bombs (min and max usually, in special cases
    exact) if any guarantees arise, pick them and go to the start
4:  Play out from cells that show a lot of promise (little decided from the fact that it's not a 
    bomb, but lots decided by the specific number. rationale being that numbers that guarantee a 
    lot just by not being a bomb are more likely to be a bomb. the truthfulness of this is to be
    investigated). After picking the best, go to the start.


current state:
region.cpp and grid.cpp are working completely

solver.cpp is on it's way

the rest are only prototypes