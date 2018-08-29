#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <chrono>
#include "solver.h"
#include "debug.h"

namespace ms {

#ifndef NDEBUG
	#define assert_each_trim() do {\
		for(region_set::iterator ri = regions.begin(); ri != regions.end(); ++ri) {\
			assert_trim(*ri);\
		}\
	}while(0)
	#define assert_norepeat() do {\
		for(region_set::iterator ri = regions.begin(); std::next(ri, 1) != regions.end(); ++ri) {\
			for(region_set::iterator rj = std::next(ri, 1); rj != regions.end(); ++rj) {\
				assert((*ri) != *rj);\
			}\
		}\
	}while(0)
#else
	#define assert_each_trim()
	#define assert_norepeat()
#endif


	/**
	 * Copies grid, all other members default initialize
	 **/
	solver::solver(const grid& start) : g(start, grid::FULL_COPY), regions(g.height(), g.width()) {	}

	/**
	 * Initializes the internal grid with the given parameters
	 **/
	solver::solver(unsigned int height, unsigned int width, unsigned int bombs) : g(height,width,bombs), regions(height, width) { }

	/**
	 * Find the areas around each number where there could be bombs.
	 * Such places must fit the following criteria:
	 *   - on the grid
	 *   - not not flagged or opened (so are hidden or questionmarked)
	 *   - adjacent to the given square
	 *
	 * Determine in each case how many bombs there are.
	 * This will be exact, so `region.min == region.max` for all regions generated.
	 *
	 * Store the results in `this.regions`
	 * 
	 * Guaranteed to create trim regions, not cause two regions of the same area to be added, and not cause empty regions to be added.
	 * 
	 * Complexity \f$O(N)\f$ where \f$N\f$ is the number of cells in the grid
	 **/
	int solver::find_base_regions() {
		region remaining;

		for (unsigned int r = 0; r < g.height(); ++r) {
			for (unsigned int c = 0; c < g.width(); ++c) {
				remaining.add_cell(rc_coord(r,c));
			}
		}
		remaining.set_count(g.bombs());

		for (unsigned int r = 0; r < g.height(); ++r) {
			for (unsigned int c = 0; c < g.width(); ++c) {
				grid::cell gotten = g.get(r,c);
				if(gotten == grid::ms_flag) {
					remaining.remove_bomb(rc_coord(r,c));
				} else if(gotten <= 8 && gotten >= 0) {
					remaining.remove_safe(rc_coord(r,c));
					region reg;
					int num_flags = 0;

					for (int rr = -1; rr <= 1; ++rr)
						for (int cc = -1; cc <= 1; ++cc)
							if (!(rr == 0 && cc == 0) && g.iscontained(r + rr, c + cc)) {
								switch (g.get(r + rr, c + cc)) {
								case grid::ms_hidden:
								case grid::ms_question:
									reg.add_cell(rc_coord(r + rr, c + cc));
									break;
								case grid::ms_flag:
									++num_flags;
									break;
								default:
									break;
								}

							}
					if(!reg.empty()) {
						if(gotten < num_flags)
							throw std::runtime_error("number of flags surrounding the cell exceeds the number of the cell");
						reg.set_count(gotten - num_flags);
						regions.add(reg);
					}
				}
			}
		}
		//this check is added to prevent massive slowdowns from adding remaining when it intersects around 100 cells -> 2000 cells
		//the choice of 10 is somewhat arbitrary, TODO get a better system for deciding when to add remaining
		if(remaining.size() < 10)
			regions.add(remaining);

		return 0;
	}


	/**
	 * Find all regions that can be deduced from the existing regions.
	 * 
	 * Calculate the intersection of each pair of regions
	 *   - if empty, discard it
	 *   - if nonempty, add the intersection and subraction to `this.aux_regions`
	 * 
	 * Returns the number iterations, (zero if nothing to do)
	 * 
	 * If `lazy == true` it stops as soon as a cells can be added to the queue
	 * 
	 * Complexity of the inner loop \f$\O(N^2)\f$..
	 * Outer loop's complexity is nontrivial, runs until it has found aux regions (often just one iteration).
	 **/
	int solver::find_aux_regions(bool lazy) {
		debug_printf("find_aux_regions [%s]...", lazy?"lazy":"nonlazy");
		
		int iterations = 0;

		// using clock = std::chrono::high_resolution_clock;
		// clock::duration accum1 = std::chrono::seconds(0), accum2 = std::chrono::seconds(0);

		const region_set::subset_type& regions_added = regions.get_modified_regions();

		while (!regions_added.empty()) { //loops as long as something was added
			debug2_printf("[%zu/%zu]", regions_added.size(), regions.size());

			if(lazy && fill_queue())
				break;

			std::vector<region> region_queue;

			for(auto ri = regions_added.begin(); ri != regions_added.end(); ++ri) {
				debug2_printf(".");
				region_set::subset_type overlaps = regions.regions_intersecting(**ri);
				
				for(auto rj = overlaps.begin(); rj != overlaps.end(); ++rj) {
					if(*rj == *ri)
						continue;
					region_queue.push_back((*ri)->intersect(**rj));
					region_queue.push_back((*ri)->subtract(**rj));
					region_queue.push_back((*rj)->subtract(**ri));
				}
			}
			debug2_printf("*");
			regions.reset_modified_regions();
			for(region& to_add : region_queue) {
				regions.add(to_add);
			}
			++iterations;
		}
		debug2_printf("\n");
		debug_printf("done\n");
		return iterations;
	}


	int solver::find_regions() {
		find_base_regions();
		find_aux_regions(true);
		if(!(safe_queue.empty() && bomb_queue.empty()))
			return 0;

		//find_chains();
		//find_leftover();
		
		return 1;
	}


	/**
	 * To be called when a safe cell is opened.
	 * Removes `cell` from all regions, so it can be forgotten about and free up memory 
	 * and simplify future calculations
	 * 
	 * returns nonzero if the safe cell is found, otherwise returns 0.
	 **/
	int solver::remove_safe(rc_coord cell) {
		int removed = regions.remove_safe(cell);	

		for(std::deque<rc_coord>::iterator ri = bomb_queue.begin(); ri != bomb_queue.end();) {
			if(*ri == cell) {
				ri = bomb_queue.erase(ri);
			} else {
				++ri;
			}
		}
		for(std::deque<rc_coord>::iterator ri = safe_queue.begin(); ri != safe_queue.end();) {
			if(*ri == cell) {
				ri = safe_queue.erase(ri);
			} else {
				++ri;
			}
		}
		
		return removed;
	}

	/**
	 * to be called when a bomb cell is opened
	 * removes it from all regions, so it can be forgotten about and free up memory
	 * and simplify future calculations
	 * 
	 * returns nonzero if the bomb cell is found, otherwise returns 0
	 **/
	int solver::remove_bomb(rc_coord cell) {
		int removed = regions.remove_bomb(cell);

		for(std::deque<rc_coord>::iterator ri = bomb_queue.begin(); ri != bomb_queue.end();) {
			if(*ri == cell) {
				ri = bomb_queue.erase(ri);
			} else {
				++ri;
			}
		}
		for(std::deque<rc_coord>::iterator ri = safe_queue.begin(); ri != safe_queue.end();) {
			if(*ri == cell) {
				ri = safe_queue.erase(ri);
			} else {
				++ri;
			}
		}
		
		return removed;
	}

	int solver::find_leftover() {
		return 0;
	}

	/**
	 * Forces solver to flag a cell, and treat it as a bomb for all future calculations. 
	 * This may result in errors and exceptions further on if it is incorrect.
	 * 
	 * Returns zero if the cell is successfully flagged, nonzero otherwise.
	 **/
	int solver::manual_flag(rc_coord arg) {
		return apply_flag(arg);
	}

	/**
	 * Forces solver to open a cell.
	 * 
	 * Returns the number of cells opened, or -1 on error
	 **/
	int solver::manual_open(rc_coord arg) {
		return apply_open(arg);
	}

	/**
	 * Flags a cell, and treat it as a bomb for all future calculations.
	 * 
	 * Returns zero if the cell is successfully flagged
	 **/
	int solver::apply_flag(rc_coord arg) {
		g.set_flag(arg.row, arg.col, grid::ms_flag);

		if(g.get(arg.row, arg.col) == grid::ms_flag) {
			remove_bomb(arg);
			return 0;
		} else {
			return 1;
		}
	}

	/**
	 * Opens a cell and removes all instances of the cell from all regions
	 * 
	 * Returns the number of cells opened, or -1 on error
	 **/
	int solver::apply_open(rc_coord arg) {
		switch(g.open(arg.row, arg.col)) {
		case -1:
			throw std::invalid_argument("Could not open the cell");
		case 0:
			return 0;
		case 1:
			remove_safe(arg);
			return 1;
		default: //multiple cells opened, remove all open cells
			int ret = 0;
			debug2_printf("<<");
			for(unsigned int r = 0; r < g.height(); ++r) {
				for(unsigned int c = 0; c < g.width(); ++c) {
					grid::cell curcell = g.get(r,c);
					if((curcell >= grid::ms_0 && curcell <= grid::ms_8) || curcell == grid::ms_non_bomb) {
						ret += !!remove_safe(rc_coord{r,c});
					}
				}
			}
			debug2_printf(">>\n");
			return ret;	
		}
	}

	/**
	 * Finds all regions that guarantee that the contained cells are
	 * safe or bombs and adds them to the queues if they are not already.
	 **/
	int solver::fill_queue() {
		int num_added = 0;
		for(const region& check : regions) {
			if(check.size() == check.min()) { //implies size == max == min for all valid state regions
				for(rc_coord bomb : check) {
					num_added += add_to_bomb_queue(bomb);
				}
			} else if (check.max() == 0) {
				for(rc_coord safe : check) {
					num_added += add_to_safe_queue(safe);
				}				
			}
		}
		return num_added;
	}

	/**
	 * Adds the input cell to the back of the bomb queue if it is not already present.
	 * 
	 * Returns 1 if `to_add` is added, 0 if not.
	 **/
	int solver::add_to_bomb_queue(rc_coord to_add) {
		for(const rc_coord& check : bomb_queue) {
			if(to_add == check)
				return 0;
		}
		bomb_queue.push_back(to_add);
		return 1;
	}

	/**
	 * Adds the input cell to the back of the safe queue if it is not already present.
	 * 
	 * Returns 1 if `to_add` is added, 0 if not.
	 **/
	int solver::add_to_safe_queue(rc_coord to_add) {
		for(const rc_coord& check : safe_queue) {
			if(to_add == check)
				return 0;
		}
		safe_queue.push_back(to_add);
		return 1;
	}


	/**
	 * Runs until the next step is not guaranteed to succeed.
	 * 
	 * Returns the number of steps taken.
	 **/
	int solver::solve_certain() {
		int ret = 0;
		while(step_certain() != BAD_RC_COORD)
			++ret;
		return ret;
	}

	/**
	 * Opens/flags a cell if it is certain it will be correct.
	 * 
	 * Returns the cell opened, or `BAD_RC_COORD` if none is opened
	 **/
	rc_coord solver::step_certain() {
		if(g.gamestate() != grid::RUNNING) {
			return BAD_RC_COORD;
		}

		if(bomb_queue.empty() && safe_queue.empty()) {
			fill_queue();
			if(bomb_queue.empty() && safe_queue.empty()) {
				find_regions();
				fill_queue();
			}
		}

		if(!bomb_queue.empty()) {
			rc_coord ret = bomb_queue.front();
			int err = apply_flag(ret);
			(void) err; //suppress unused warnings
			debug_printf("bomb_queue:\n");
			for(rc_coord bomb : bomb_queue) {
				debug_printf("    ");
				debug_print_rc_coord(bomb);
				debug_printf("\n");
			}
			debug_printf("safe_queue:\n");
			for(rc_coord safe : safe_queue) {
				debug_printf("    ");
				debug_print_rc_coord(safe);
				debug_printf("\n");
			}
			assert(err == 0);
			return ret;
		} else if(!safe_queue.empty()) {
			rc_coord ret = safe_queue.front();
			if(g.get(ret.row,ret.col)!=grid::ms_hidden) {
				//TODO find bug where this code was reached
				debug_printf("cell:%d\nrow:%u\ncol:%u\n",g.get(ret.row,ret.col),ret.row,ret.col);
				throw std::logic_error("Attempting to open a non-hidden cell");
			}
			int open_status = apply_open(ret);//removes ret
			if(!(open_status > 0)) {
				debug_printf("cells opened: %hhu\nrow:%u\ncol:%u\n",open_status,ret.row,ret.col);
				throw std::logic_error("Opened the wrong number of cells");
			}
			debug_printf("safe_queue:\n");
			for(rc_coord safe : safe_queue) {
				debug_printf("    ");
				debug_print_rc_coord(safe);
				debug_printf("\n");
			}
			return ret;
		} else {
			return BAD_RC_COORD;
		}
	}

	std::mt19937 solver::rng(time(NULL));

	/**
	 * Runs until win or loss.
	 * 
	 * Returns gamestate
	 **/
	int solver::solve() {
		int ret = 0;
		while(step() != BAD_RC_COORD)
			++ret;
		return ret;
	}

	/**
	 * Opens/flags a cell as long a game is running or ready to start running.
	 * Opens cells that are certain first before attempting to open a new cell.
	 * 
	 * Returns the cell opened, or BAD_RC_COORD if none is opened
	 **/
	rc_coord solver::step() {
		if(g.gamestate() != grid::RUNNING && g.gamestate() != grid::NEW) {
			return BAD_RC_COORD;
		}

		rc_coord ret = step_certain();
		if(ret != BAD_RC_COORD) {
			debug2_printf("certain[%u,%u]\n",ret.row,ret.col);
			return ret;
		}

		std::vector<rc_coord> best_locs;
		float best_prob = 2; //higher than any real probability could be
		float default_prob = (float) g.bombs() / (g.width() * g.height());

		for(unsigned row = 0; row < g.height(); ++row) {
			for(unsigned col = 0; col < g.width(); ++col) {
				if(g.get(row,col) == grid::ms_hidden || g.get(row,col) == grid::ms_question) {
					const region_set::subset_type& regions_at = regions.regions_intersecting(rc_coord(row, col));
					float probability = regions_at.empty() ? default_prob : 0;
					for(region_set::iterator reg : regions_at) {
						probability = std::max((reg->min() + reg->max()) / (2.f * reg->size()), probability); //pick the worst probability
					}
					if(fabs(probability - best_prob) < .001) { //close enough in probability
						best_locs.push_back(rc_coord(row,col));
					} else if(probability < best_prob) {
						best_locs.clear();
						best_locs.push_back(rc_coord(row,col));
						best_prob = probability;
					}
				}
			}
		}


		if(!best_locs.empty()) {
			std::uniform_int_distribution<> uid(0, best_locs.size() - 1);
			ret = best_locs[uid(rng)];
			apply_open(ret);
			debug2_printf("guess[%u,%u]\n",ret.row,ret.col);
			return ret;
		}
		return BAD_RC_COORD;
		
	}




}

