#ifndef GRID_H
#define GRID_H

namespace ms {

	enum  cell : char {
		ms_0 = 0, ms_1 = 1, ms_2 = 2, ms_3 = 3, ms_4 = 4,
		ms_5 = 5, ms_6 = 6, ms_7 = 7, ms_8 = 8, ms_bomb = 9,
		ms_hidden = 10, ms_question = 11, ms_flag = 12, ms_error = -1
	};

	enum gamestate {
		RUNNING, WON, LOST, NEW
	};



	class grid {
	protected:
		int init(unsigned int row, unsigned int col);
		int updategamestate();
		int open__(int row, int col);
		unsigned int _height, _width, _bombs;
		char ** _grid;
		char ** _visgrid;
		gamestate _gs;
	public:
		grid();
		grid(unsigned int height, unsigned int width, unsigned int bombs);

		unsigned int width() const { return _width; }
		unsigned int height() const { return _height; }
		unsigned int bombs() const { return _bombs; }
		gamestate gamestate() const { return _gs; }
		bool iscontained(int row, int col) const;

		cell get(unsigned int row, unsigned int col) const { if (iscontained(row, col)) return (cell)_visgrid[row][col]; else return ms_error; }
		cell peak(unsigned int row, unsigned int col) const { if (iscontained(row, col)) return (cell)_grid[row][col]; else return ms_error; }//for cheating (aka debugging)

		int flag(unsigned int row, unsigned int col);//toggles flag status none=>flagged=>question=>none //returns 0 on success, 1 on failure (out of bounds or not initialized)
		int open(unsigned int row, unsigned int col);//returns number of cells openned (1 if hidden and nonzero, X if a number surrounded by the correct number of bombs or zero, 0 otherwise, -1 on error (out of bounds or not initialized))

		void reset();

		~grid();

	};


}


#endif