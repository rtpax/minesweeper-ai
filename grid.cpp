#include "grid.h"
#include <vector>
#include <time.h>
#include "rc_coord.h"
#include <cassert>


namespace ms {

	std::mt19937 grid::rng(time(NULL));
	
	/**
	 * Returns true if the given location is contained in the grid. Returns false otherwise.
	 **/
	bool grid::iscontained(int row, int col) const {
		return (0 <= row && row < (signed int)_height) && (0 <= col && col < (signed int)_width);
	}

	/**
	 * Returns the number of adjacent cells that are the given underlying value (bomb by default).
	 **/
	int grid::count_neighbor(int row, int col, cell value) {
		int count = 0;

		for (int rr = -1; rr <= 1; ++rr) {
			for (int cc = -1; cc <= 1; ++cc) {
				if (!(rr == 0 && cc == 0) && iscontained(row + rr, col + cc)) {
					if (_grid[row + rr][col + cc] == value) {
						++count;
					}
				}
			}
		}

		return count;
	}

	/**
	 * Returns the number of adjacent cells that are the given visible value (flag by default)
	 **/
	int grid::count_vis_neighbor(int row, int col, cell value) {
		int count = 0;

		for (int rr = -1; rr <= 1; ++rr) {
			for (int cc = -1; cc <= 1; ++cc) {
				if (!(rr == 0 && cc == 0) && iscontained(row + rr, col + cc)) {
					if (_visgrid[row + rr][col + cc] == value) {
						++count;
					}
				}
			}
		}

		return count;
	}

	/**
	 * Initializes the state of the grid such that it has the correct number of bombs 
	 * and the input location is not a bomb. The input location is opened. If there
	 * are too few spaces to place bombs, init will place as many as it can (it will
	 * never place on the input location).
	 **/
	std::unordered_set<rc_coord, rc_coord_hash> grid::init(unsigned int row, unsigned int col) {		
		std::vector<rc_coord> nonbombs;

		unopened_cells.clear();
		for (unsigned int r = 0; r < _height; ++r) {
			for (unsigned int c = 0; c < _width; ++c) {
				_visgrid[r][c] = ms_hidden;
				_grid[r][c] = ms_0;
				unopened_cells.insert(rc_coord{ r, c });
				if (!(r == row && c == col)) { //first click is always not a bomb
					nonbombs.push_back(rc_coord{ r, c });
				}
			}
		}

		for (unsigned int b = 0; b < _bombs && !nonbombs.empty(); ++b) {
			int index = rng() % nonbombs.size();
			_grid[nonbombs[index].row][nonbombs[index].col] = ms_bomb;
			std::swap(nonbombs[index], nonbombs.back());
			nonbombs.pop_back();
		}

		for (unsigned int r = 0; r < _height; ++r) {
			for (unsigned int c = 0; c < _width; ++c) {
				if (_grid[r][c] != ms_bomb)
					_grid[r][c] = (cell) count_neighbor(r, c, ms_bomb);
			}
		}

		_gs = RUNNING;

		return open(row, col);
	}


	/**
	 * initializes the grid with the correct height, width, and bombs. Allocates
	 * space for the hidden and visible data (2 * height * width bytes of data). 
	 **/
	int grid::allocate__(unsigned int height, unsigned int width, unsigned int bombs) {
		_width = width > 0 ? width : 1;
		_height = height > 0 ? height : 1;
		_bombs = bombs;
		_grid = new cell*[_height];
		_visgrid = new cell*[_height];
		for (unsigned int h = 0; h < _height; ++h) {
			_grid[h] = new cell[_width]();
			_visgrid[h] = new cell[_width];
			std::fill_n(_visgrid[h],_width,ms_hidden);
		}

		return 0;
	}

	grid::grid(unsigned int height, unsigned int width, cell ** arr) {
		allocate__(height, width, 0);
		_gs = RUNNING;
		flag_count = 0;

		for(unsigned int r = 0; r < _height; ++r) {
			for(unsigned int c = 0; c < _width; ++c) {
				unopened_cells.insert(rc_coord(r,c));
				if(arr[r][c] == ms_bomb) {
					_grid[r][c] = ms_bomb;
					++_bombs;
				}
			}
		}

		for (unsigned int r = 0; r < _height; ++r) {
			for (unsigned int c = 0; c < _width; ++c) {
				if (_grid[r][c] != ms_bomb)
					_grid[r][c] = (cell) count_neighbor(r, c, ms_bomb);
			}
		}
	}

	grid::grid(unsigned int height, unsigned int width, unsigned int bombs) {
		allocate__(height,width,bombs);
		_gs = NEW;
		flag_count = 0;
	}

	grid::grid(const grid& copy, copy_type gct) {

		allocate__(copy._height,copy._width,copy._bombs);

		switch(gct) {
		case FULL_COPY:
			for(unsigned int r = 0; r < _height; ++r) {
				for(unsigned int c = 0; c < _width; ++c) {
					_visgrid[r][c] = copy._visgrid[r][c];
					_grid[r][c] = copy._grid[r][c];
				}
			}
			unopened_cells = copy.unopened_cells;
			_gs = copy._gs;
			flag_count = copy.flag_count;
			break;
		case HIDDEN_COPY:
			unopened_cells.clear();
			for(unsigned int r = 0; r < _height; ++r) {
				for(unsigned int c = 0; c < _width; ++c) {
					_visgrid[r][c] = ms_hidden;
					_grid[r][c] = copy._grid[r][c];
					unopened_cells.insert(rc_coord(r,c));
				}
			}
			_gs = RUNNING;
			flag_count = 0;
			break;
		case SURFACE_COPY:
			for(unsigned int r = 0; r < _height; ++r) {
				for(unsigned int c = 0; c < _width; ++c) {
					_visgrid[r][c] = copy._visgrid[r][c];
					_grid[r][c] = grid::ms_error;
				}
			}	
			unopened_cells = copy.unopened_cells;
			_gs = copy._gs;	
			flag_count = copy.flag_count;
			break;
		case PARAM_COPY:
			_gs = NEW;
			break;
		}
	}


	grid::~grid() {
		for (unsigned int h = 0; h < _height; ++h) {
			delete[] _grid[h];
			delete[] _visgrid[h];
		}

		delete[] _grid;
		delete[] _visgrid;
	}

	/**
	 * Toggles flag status none=>flagged=>question=>none.
	 * 
	 * Returns 0 on success, nonzero on failure (out of bounds or not initialized or already opened).
	 **/
	int grid::flag(unsigned int row, unsigned int col) {
		if (_gs != RUNNING || !iscontained(row, col))
			return 1;

		switch (_visgrid[row][col]) {
		case ms_hidden:
			++flag_count;
			_visgrid[row][col] = ms_flag;
			return 0;
		case ms_flag:
			--flag_count;
			_visgrid[row][col] = ms_question;
			return 0;
		case ms_question:
			_visgrid[row][col] = ms_hidden;
			return 0;
		default:
			return 1;
		}
	}

	/**
	 * Sets the flag status of a cell.
	 * 
	 * Viable inputs for `flag` are: grid::cell::ms_hidden,ms_flag,ms_question
	 * 
	 * Returns 0 on success, 1 on failure (out of bounds or not initialized or already opened).
	 **/
	int grid::set_flag(unsigned int row, unsigned int col, cell flag) {
		if (_gs != RUNNING || !iscontained(row, col) ||
				(flag != ms_flag && flag != ms_hidden && flag != ms_question))
			return 1;
		
		switch (_visgrid[row][col]) {
		case ms_hidden:
		case ms_question:
			if(flag == ms_flag)
				++flag_count;//increment flag if becoming a flag
			_visgrid[row][col] = flag;
			return 0;
		case ms_flag:
			if(flag != ms_flag)
				--flag_count;
			_visgrid[row][col] = flag;
			return 0;
		default:
			return 1;
		}
	}

	std::unordered_set<rc_coord, rc_coord_hash> grid::open__(int row, int col) {
		std::unordered_set<rc_coord, rc_coord_hash> all_opened = {};
		std::unordered_set<rc_coord, rc_coord_hash> to_open = { rc_coord(row,col) };
		std::unordered_set<rc_coord, rc_coord_hash> next_open = {};

		while(!to_open.empty()) {
			for(rc_coord opening  : to_open) {
				cell vis = _visgrid[opening.row][opening.col];

				if(count_vis_neighbor(opening.row, opening.col) == vis) { // implies 0 <= vis <= 8
					for(int rr = -1; rr <= 1; ++rr) {
						for(int cc = -1; cc <= 1; ++cc) {
							if (!(rr == 0 && cc == 0) && iscontained(opening.row + rr, opening.col + cc))
								if(_visgrid[opening.row + rr][opening.col + cc] == ms_hidden || _visgrid[opening.row + rr][opening.col + cc] == ms_question)
									next_open.insert(rc_coord(opening.row + rr, opening.col + cc));
						}
					}
				} else if (vis == ms_hidden || vis == ms_question) {
					_visgrid[opening.row][opening.col] = _grid[opening.row][opening.col];
					if(_visgrid[opening.row][opening.col] == ms_0)
						next_open.insert(opening);
					else if(_visgrid[opening.row][opening.col] == ms_bomb)
						_gs = LOST;
					all_opened.insert(opening);
					unopened_cells.erase(unopened_cells.find(opening));
				} else if (vis != ms_flag && (vis > ms_8 || vis < ms_0)) {
					throw grid_error("Could not open cell " + rc_coord(row,col).to_string());
				}
			}
			to_open = std::move(next_open);
			next_open.clear();
		}
		return all_opened;
	}


	/**
	 * Returns an unordered set of cells opened
	 * 
	 * throws an error if the cell could not be opened
	 **/
	std::unordered_set<rc_coord, rc_coord_hash> grid::open(unsigned int row, unsigned int col) {
		std::unordered_set<rc_coord, rc_coord_hash> ret;
		if(_gs == WON || _gs == LOST)
			return ret;
		if (!iscontained(row, col))
			throw grid_error("attempted to open cell " + rc_coord(row, col).to_string() + "not contained in grid");
		else if (_gs == NEW) {
			ret = init(row, col);
		} else
			ret = open__((signed int)row, (signed int)col);

		update_if_won();
		if(_gs == WON) {
			_gs = RUNNING;//temporarily allow set_flag
			for(rc_coord cell : unopened_cells) {
				if(_grid[cell.row][cell.col] == ms_bomb)
					set_flag(cell.row,cell.col,ms_flag);
			}
			_gs = WON;
		} else if (_gs == LOST) {
			for(rc_coord cell : unopened_cells) {
				if(_grid[cell.row][cell.col] == ms_bomb && _visgrid[cell.row][cell.col] != ms_flag)
					_visgrid[cell.row][cell.col] = ms_unopened_bomb;
			}			
		}

		return ret;
	}

	/**
	 * Checks if the game has been won (only bombs hidden, no bombs open).
	 *   - returns 1 if the gamestate is updated to WON 
	 *   - returns 0 otherwise
	 **/
	int grid::update_if_won() {
		if (_gs != RUNNING)
			return 0;
		bool only_bombs_unopened = true;
		for(rc_coord cell : unopened_cells) {
			if(_grid[cell.row][cell.col] != ms_bomb) {
				assert(_visgrid[cell.row][cell.col] != _grid[cell.row][cell.col]);
				only_bombs_unopened = false;
				break;
			}
		}
		if (only_bombs_unopened) {
			_gs = WON;
			return 1;
		}
		return 0;
	}

	/**
	 * reinitializes the grid, all cells are hidden, gamestate is new
	 * 
	 * (bombs are not placed until first cell opened)
	 **/
	void grid::reset() {
		_gs = NEW;
		for (unsigned int r = 0; r < _height; ++r) {
			for (unsigned int c = 0; c < _width; ++c) {
				_visgrid[r][c] = ms_hidden;
			}
		}
		flag_count = 0;
	}

	/**
	 * resets all flags to ms_hidden
	 **/
	void grid::clear_all_flags() {
		for(rc_coord cell : unopened_cells) {
			_visgrid[cell.row][cell.col] = ms_hidden;
		}
		flag_count = 0;
	}

}