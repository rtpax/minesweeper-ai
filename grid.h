#ifndef MS_GRID_H
#define MS_GRID_H

#include <random>


namespace ms {
	/**
	 * Contains data and methods required to play a minesweeper game.
	 **/
	class grid {
	public:

		/**
		 * `cell`s hold the values of individual cells of grids.
		 **/
		enum  cell : char {
			/**0 bombs surround this cell and this cell is not a bomb**/
			ms_0 = 0, 
			/**1 bombs surround this cell and this cell is not a bomb**/
			ms_1 = 1, 
			/**2 bombs surround this cell and this cell is not a bomb**/
			ms_2 = 2, 
			/**3 bombs surround this cell and this cell is not a bomb**/
			ms_3 = 3, 
			/**4 bombs surround this cell and this cell is not a bomb**/
			ms_4 = 4, 
			/**5 bombs surround this cell and this cell is not a bomb**/
			ms_5 = 5, 
			/**6 bombs surround this cell and this cell is not a bomb**/
			ms_6 = 6, 
			/**7 bombs surround this cell and this cell is not a bomb**/
			ms_7 = 7, 
			/**8 bombs surround this cell and this cell is not a bomb**/
			ms_8 = 8, 
			/**This cell is a bomb**/
			ms_bomb = 9, 
			/**The contents of this cell are hidden from the user**/
			ms_hidden = 10, 
			/**The contents of this cell are hidden from the user. The users may mark a cell as ms_question, but it has no designated meaning**/
			ms_question = 11, 
			/**The contents of this cell are hidden from the user, but the user has marked it as a bomb. It cannot be opened until unmarked. **/
			ms_flag = 12, 
			/**Not a bomb, but specific number not known. Only used by ms::spingrid.**/
			ms_non_bomb = 13, 
			/**This indicates some sort of error has occured. See specific function details for more info.**/
			ms_error = -1 
		};

		/**
		 * `gamestate` indicates the progress of a grid in its game
		 **/
		enum gamestate {
			/**No bombs are open, some but not all non-bomb cells are open**/ 
			RUNNING,
			/**No bombs are open, all-nonbomb cells are open**/
			WON, 
			/**A bomb is open**/
			LOST, 
			/**No cells are open**/
			NEW 
		};

		/**
		 * Used for passing into `grid`'s copy constructor. Indicates how it should be copied.
		 **/
		enum copy_type {
			/**Only copy the visible contents of the grid. All underlying values are ms_error. **/
			SURFACE_COPY, 
			/**Make an exact copy of the input grid.**/
			FULL_COPY, 
			/**Copy the contents of a grid, but make all visible cells hidden. Reset the gamestate to NEW. **/
			HIDDEN_COPY, 
			/**Copy the `width`, `height`, and `bombs` parameters and create a new grid.**/
			PARAM_COPY 
		};


	private:
		int init(unsigned int row, unsigned int col);
		int updategamestate();
		int open__(int row, int col);
		int allocate__(unsigned int row, unsigned int col, unsigned int bombs);
		int count_neighbor(int row, int col, cell value);
		/**Checks the contents of `_grid` with bounds checking**/
		cell peek(unsigned int row, unsigned int col) const {
			if (iscontained(row, col)) return (cell)_grid[row][col]; else return ms_error; 
		}
		unsigned int _height, _width, _bombs;
		char ** _grid;
		char ** _visgrid;
		gamestate _gs;
		static std::mt19937 rng;
	public:
		grid(unsigned int height, unsigned int width, unsigned int bombs);
		grid(unsigned int height, unsigned int width, cell ** arr);
		grid(const grid& copy, copy_type gct);

		unsigned int width() const { return _width; }
		unsigned int height() const { return _height; }
		unsigned int bombs() const { return _bombs; }
		gamestate gamestate() const { return _gs; }
		bool iscontained(int row, int col) const;

		/**Returns the visible contents of a cell. Return `ms_error` if the specified cell is not contained in the grid.**/
		cell get(unsigned int row, unsigned int col) const { 
			if (iscontained(row, col)) return (cell)_visgrid[row][col]; else return ms_error; 
		}
		
		int flag(unsigned int row, unsigned int col);
		int set_flag(unsigned int row, unsigned int col, cell flag);
		int open(unsigned int row, unsigned int col);
		
		void reset();

		~grid();




	};
}


#endif