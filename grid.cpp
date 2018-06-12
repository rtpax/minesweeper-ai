#include "grid.h"
#include <random>
#include <time.h>
#include <vector>

namespace ms {


	bool grid::iscontained(int row, int col) const {
		return (0 <= row && row < (signed int)_height) && (0 <= col && col < (signed int)_width);
	}

	struct rc_coord { unsigned int row, col; };

	int grid::init(unsigned int row, unsigned int col) {
		std::vector<rc_coord> nonbombs(_height*_width);

		int index = 0;
		for (unsigned int r = 0; r < _height; ++r) {
			for (unsigned int c = 0; c < _width; ++c) {
				_visgrid[r][c] = ms_hidden;
				_grid[r][c] = 0;
				if (!(r == row && c == col)) { //first click is always not a bomb
					nonbombs[index].row = r;
					nonbombs[index].col = c;
					++index;
				}
			}
		}

		std::mt19937 rng;
		rng.seed(time(NULL));

		for (unsigned int b = 0; b < _bombs; ++b) {
			index = rng() % nonbombs.size();
			_grid[nonbombs[index].row][nonbombs[index].col] = ms_bomb;
			nonbombs.erase(nonbombs.begin() + index);
		}

		//count all bombs adjacent to each nonbomb square
		for (int r = 0; r < _height; ++r) {
			for (int c = 0; c < _width; ++c) {
				if (_grid[r][c] != ms_bomb)
					for (int rr = -1; rr <= 1; ++rr)
						for (int cc = -1; cc <= 1; ++cc)
							if (!(rr == 0 && cc == 0) && iscontained(r + rr, c + cc))
								if (_grid[r + rr][c + cc] == ms_bomb)
									++(_grid[r][c]);

			}
		}

		_gs = RUNNING;

		return open(row, col);
	}

	grid::grid(unsigned int height, unsigned int width, unsigned int bombs) {
		_width = width > 0 ? width : 1;
		_height = height > 0 ? height : 1;
		_bombs = bombs;
		_grid = new char*[_height];
		_visgrid = new char*[_height];
		for (int h = 0; h < _height; ++h) {
			_grid[h] = new char[_width];
			_visgrid[h] = new char[_width];
		}
		_gs = NEW;
	}


	grid::~grid() {
		for (int h = 0; h < _height; ++h) {
			delete[] _grid[h];
			delete[] _visgrid[h];
		}

		delete[] _grid;
		delete[] _visgrid;
	}

	int grid::flag(unsigned int row, unsigned int col) {
		if (_gs != RUNNING || !iscontained(row, col))
			return 1;

		switch (_visgrid[row][col]) {
		case ms_hidden:
			_visgrid[row][col] = ms_flag;
			break;
		case ms_flag:
			_visgrid[row][col] = ms_question;
			break;
		case ms_question:
			_visgrid[row][col] = ms_hidden;
			break;
		default:
			break;
		}

		return 0;
	}


	int grid::open__(int row, int col) {
		if (0 <= _visgrid[row][col] && _visgrid[row][col] <= 8) {
			int flagcount = 0;
			for (int rr = -1; rr <= 1; ++rr) {
				for (int cc = -1; cc <= 1; ++cc) {
					if (!(rr == 0 && cc == 0) && iscontained(row + rr, col + cc))
						if (_visgrid[row + rr][row + cc] == ms_flag)
							++flagcount;
				}
			}
			if (flagcount == _visgrid[row][col]) {
				int sum = 0;
				for (int rr = -1; rr <= 1; ++rr) {
					for (int cc = -1; cc <= 1; ++cc) {
						if (!(rr == 0 && cc == 0) && iscontained(row + rr, col + cc))
							if (_visgrid[row + rr][col + cc] == ms_hidden || _visgrid[row + rr][col + cc] == ms_question)
								sum += open__(row + rr, col + cc);
					}
				}
				return sum;
			}
			else {
				return 0;
			}
		}
		else if (_visgrid[row][col] == ms_hidden || _visgrid[row][col] == ms_question) {
			_visgrid[row][col] = _grid[row][col];
			if (_visgrid[row][col] == 0) {
				return open__(row, col);
			}
			return 1;
		}
		else if (_visgrid[row][col] == ms_flag) {
			return 0;
		}
		else {
			return -1;
		}
	}

	int grid::open(unsigned int row, unsigned int col) {
		int ret = 0;
		if (!iscontained(row, col))
			return -1;
		else if (_gs == NEW)
			ret = init(row, col);
		else if (_gs != RUNNING || !iscontained(row, col))
			ret = -1;
		else
			ret = open__((signed int)row, (signed int)col);

		updategamestate();

		return ret;
	}

	int grid::updategamestate() {
		if (_gs != RUNNING)
			return 0;
		bool haswon = 1;
		for (int r = 0; r < _height; ++r) {
			for (int c = 0; c < _width; ++c) {
				if (_visgrid[r][c] == ms_bomb) {
					_gs = LOST;
					return 1;
				}
				if (_grid[r][c] != ms_bomb && _visgrid[r][c] != _grid[r][c]) {
					haswon = 0;
				}
			}
		}
		if (haswon) {
			_gs = WON;
			return 1;
		}
		return 0;
	}

	void grid::reset() {
		_gs = NEW;
		for (int r = 0; r < _height; ++r) {
			for (int c = 0; c < _width; ++c) {
				_visgrid[r][c] = ms_hidden;
			}
		}
	}


}