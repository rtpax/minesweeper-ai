Minesweeper AI

A minesweeper game and an AI for solving it.

Note that for difficult (high bomb) board the odds of success with perfect play can by quite low,
and even for specific easy (low bomb) boards it can be below 50% (though higher on average)

general overview of how it works:
1.  Create sets of cells with known numbers of bombs (`region`s in the code), and by analyzing the 
    overlapping of the regions, create more regions that have a range of bombs they may containe. 
    Repeat the process until it is known for certain the location of a bomb or safe cell.
2.  If there are known cells from [1], open these cells and go back to [1].
3.  Determine how likely each cell is to be bomb, and how much closer to winning opening the cell
    is likely to bring the game. (This is vague, implementaion details to be decided)
4.  Open the cell that has the best chance of winning and go back to [1]



This application uses boost C++ library and CATCH C++ testing framework
