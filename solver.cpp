#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <chrono>
#include "solver.h"
#include "debug.h"

namespace ms {

#ifndef NDEBUG
	#define assert_each_trim() do {\
		for(regions_iter ri = regions.begin(); ri != regions.end(); ++ri) {\
			assert_trim(*ri);\
		}\
	}while(0)
	#define assert_norepeat() do {\
		for(regions_iter ri = regions.begin(); std::next(ri, 1) != regions.end(); ++ri) {\
			for(regions_iter rj = std::next(ri, 1); rj != regions.end(); ++rj) {\
				assert((*ri) != *rj);\
			}\
		}\
	}while(0)
#else
	#define assert_each_trim()
	#define assert_norepeat()
#endif


	int solver::init_cell_keys() {
		cell_keys = new key_type*[g.height()];
		for(unsigned i = 0; i < g.height(); ++i) {
			cell_keys[i] = new key_type[g.width()]();
		}
		return 0;
	}

	/**
	 * Copies grid, all other members default initialize
	 **/
	solver::solver(const grid& start) : g(start, grid::FULL_COPY) {
		init_cell_keys();
	}

	/**
	 * Initializes the internal grid with the given parameters
	 **/
	solver::solver(unsigned int height, unsigned int width, unsigned int bombs) : g(height,width,bombs) {
		init_cell_keys();
	}

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
				remaining.addcell(rc_coord(r,c));
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
									reg.addcell(rc_coord(r + rr, c + cc));
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
						add_region(reg);
					}
				}
			}
		}
		add_region(remaining);

		return 0;
	}


	/**
	 * Find all regions that can be deduced from the existing regions.
	 * 
	 * Calculate the intersection of each pair of regions
	 *   - if empty, discard it
	 *   - if nonempty, add the intersection and subraction to `this.aux_regions`
	 * 
	 * Returns the number of regions added (note that it does not include modified regions, only added)
	 * 
	 * If `lazy == true` it stops as soon as a cells can be added to the queue
	 * 
	 * Complexity of the inner loop \f$\O(N^2)\f$..
	 * Outer loop's complexity is nontrivial, runs until it has found aux regions (often just one iteration).
	 **/
	int solver::find_aux_regions(bool lazy) {
		debug_printf("find_aux_regions [%s]...", lazy?"lazy":"nonlazy");
		trim_regions();
		int size = regions.size();

		// using clock = std::chrono::high_resolution_clock;
		// clock::duration accum1 = std::chrono::seconds(0), accum2 = std::chrono::seconds(0);

		std::vector<regions_iter> regions_added;
		for(regions_iter ri = regions.begin(); ri != regions.end(); ++ri)
			regions_added.push_back(ri);

		while (!regions_added.empty()) { //loops as long as something was added
			std::cout << "[" << regions_added.size() << "/" << regions.size() << "]";
			std::cout.flush();
			if(lazy && fill_queue())
				break;

			std::vector<region> region_queue;
			for(std::vector<regions_iter>::iterator ri = regions_added.begin(); ri != regions_added.end(); ++ri) {
				std::cout << ".";
				std::cout.flush();
				key_type overlaps;
				// clock::time_point A = clock::now();
				for(rc_coord cell : **ri) {
					for(regions_iter over : cell_keys[cell.row][cell.col]) {
						key_iter it = overlaps.find(over);
						if(it == overlaps.end()) {
							overlaps.insert(over);
						}
					}
				}
				// clock::time_point B = clock::now();
				for(key_iter rj = overlaps.begin(); rj != overlaps.end(); ++rj) {
					if(*rj == *ri)
						continue;
					region_queue.push_back((*ri)->intersect(**rj));
					region_queue.push_back((*ri)->subtract(**rj));
					region_queue.push_back((*rj)->subtract(**ri));
				}
				// clock::time_point C = clock::now();
				// accum1 += B - A;
				// accum2 += C - B;
				// std::cout << "[" << accum1.count() << ":" << accum2.count() << "]\n";
			}
			std::cout << "*";
			std::cout.flush();
			regions_added.clear();
			for(region& to_add : region_queue) {
				auto add_info = add_region(to_add);
				if(add_info.second) {
					//this check might take more time than it's worth
					if(std::find(regions_added.begin(), regions_added.end(), add_info.first) == regions_added.end())
						regions_added.push_back(add_info.first);
				}
			}

		}
		std::cout << "\n";
		assert_each_trim();
		assert_norepeat();

		debug_printf("done\n");
		return regions.size() - size;
	}

	/**
	 * Removes all empty regions and regions that give no information about min max beyong their size (min = 0, max = size).
	 * 
	 * Returns the number of regions removed
	 * 
	 * complexity \f$O(N)\f$
	 **/
	int solver::trim_regions() {
		int removed = 0;

		for (regions_iter ri = regions.begin(); ri != regions.end();) {
			if(!ri->is_helpful()) {
				ri = remove_region(ri);
				++removed;
			} else {
				++ri;
			}
		}

		return removed;
	}

	/**
	 * Adds a region to the list of regions, merging if a region already exists covering the same area.
	 * Skips regions that offer no information. If a new region is added, add it to the appropriate cell_keys
	 * 
	 * Returns `regions.end()` if region not added, otherwise returns an iterator to the region added/merged. 
	 *
	 * Complexity \f$O(N)\f$
	 **/
	std::pair<solver::regions_iter, bool> solver::add_region(const region& to_add) {
		typedef std::pair<solver::regions_iter, bool> ret_type;
		debug_printf("adding region ");
		debug_print_region(to_add);
		debug_printf("...\n");
		if(!to_add.is_helpful()) {
			debug_printf("skipping because unhelpful\n");
			return ret_type(regions.end(), false);
		}

		regions_iter added;
		bool did_add = false;
		regions_iter similar = regions.lower_bound(to_add);
		if(similar == regions.end() || !similar->samearea(to_add)) {
			auto added_info = regions.insert(to_add);
			assert(added_info.second);
			added = added_info.first;
			did_add = true;
			for(rc_coord cell : to_add) {
				cell_keys[cell.row][cell.col].insert(added);
			}
		} else {
			did_add = similar->min() < to_add.min() || similar->max() > to_add.max();
			similar->order_preserve_merge_to(to_add);
			added = similar;
		}

		debug_printf("region added\n");
		return ret_type(added, did_add);
	}

	/**
	 * Removes the specified region from the list of regions and all associated keys.
	 * 
	 * Complexity \f$O(log(N) + M)\f$ where \f$M\f$ is the size of the region and 
	 * \f$N\f$ is the size of `regions`
	 **/
	solver::regions_iter solver::remove_region(regions_iter to_remove) {
		for(rc_coord cell : *to_remove) {
			key_type& key = cell_keys[cell.row][cell.col];
			key_iter remove_it = std::find(key.begin(), key.end(), to_remove);
			assert(remove_it != key.end());
			key.erase(remove_it);
		}
		return regions.erase(to_remove);
	}





	int solver::find_regions() {
		trim_regions();
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
	 * Removes `cell` from all regions, so it can be forgotten about and free up memory and 
	 * simplify future calculations. Guaranteed to not create any un-trim regions
	 * 
	 * returns nonzero if the safe cell is found, otherwise returns 0.
	 **/
	int solver::remove_safe_from_all_regions(rc_coord cell) {
		key_type& key = cell_keys[cell.row][cell.col];
		int found = 0;

		while(!key.empty()) {
			key_iter removing = key.begin();
			region unsafe = **removing;
			int err = unsafe.remove_safe(cell);
			assert(err != 2 && "Attempted to remove safe from region with no safe spaces");
			size_t old_size = key.size();
			remove_region(*removing); //should remove "removing" from key
			assert(old_size > key.size());
			add_region(unsafe);
			++found;
			(void) err, (void) old_size; //suppress unused warnings
		}

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
		
		return found;
	}

	/**
	 * to be called when a bomb cell is opened
	 * removes it from all regions, so it can be forgotten about and free up memory
	 * skips regions that do not have the cell
	 * throws an error if a region has the cell, but removal is impossible (has no bomb)
	 * 
	 * all empty cells are removed
	 * 
	 * returns nonzero if the bomb cell is found, otherwise returns 0
	 **/
	int solver::remove_bomb_from_all_regions(rc_coord cell) {
		key_type& key = cell_keys[cell.row][cell.col];
		int found = 0;

		while(!key.empty()) {
			key_iter removing = key.begin();
			region unbomb = **removing;
			int err = unbomb.remove_bomb(cell);
			if(err == 2)
				assert(err != 2 && "Attempted to remove bomb from region with no bombs");
			size_t old_size = key.size();
			remove_region(*removing); //should remove "removing" from key
			assert(old_size > key.size());
			add_region(unbomb);
			++found;
			(void) err, (void) old_size; //suppress unused warnings
		}


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
		
		return found;
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
			remove_bomb_from_all_regions(arg);
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
			remove_safe_from_all_regions(arg);
			return 1;
		default: //multiple cells opened, remove all open cells
			int ret = 0;
			for(unsigned int r = 0; r < g.height(); ++r) {
				for(unsigned int c = 0; c < g.width(); ++c) {
					grid::cell curcell = g.get(r,c);
					if((curcell >= grid::ms_0 && curcell <= grid::ms_8) || curcell == grid::ms_non_bomb) {
						ret += remove_safe_from_all_regions(rc_coord{r,c});
					}
				}
			}
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
	 * Runs until the next step may fail.
	 * 
	 * Returns the number of steps taken.
	 **/
	int solver::solve_certain() {
		int ret = 0;
		while(step_certain() != rc_coord{0xffff,0xffff})
			++ret;
		return ret;
	}

	/**
	 * Opens/flags a cell if it is certain it will be correct.
	 * 
	 * Returns the cell opened, or {0xffff,0xffff} if none is opened
	 **/
	rc_coord solver::step_certain() {
		if(g.gamestate() != grid::RUNNING) {
			return rc_coord{ 0xffff,0xffff };
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
			return rc_coord{ 0xffff,0xffff };
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
		while(step() != rc_coord{0xffff,0xffff})
			++ret;
		return ret;
	}

	/**
	 * Opens/flags a cell as long a game is running or ready to start running.
	 * Opens cells that are certain first before attempting to open a new cell.
	 * 
	 * Returns the cell opened, or {0xffff,0xffff} if none is opened
	 **/
	rc_coord solver::step() {
		if(g.gamestate() != grid::RUNNING && g.gamestate() != grid::NEW) {
			return rc_coord{ 0xffff,0xffff };
		}

		rc_coord ret = step_certain();
		if(ret != rc_coord{ 0xffff,0xffff })
			return ret;

		std::vector<rc_coord> choices;
		for(unsigned row = 0; row < g.height(); ++row) {
			for(unsigned col = 0; col < g.width(); ++col) {
				if(g.get(row,col) == grid::ms_hidden || g.get(row,col) == grid::ms_question) {
					choices.push_back(rc_coord{row,col});
				}
			}
		}

		if(choices.size() > 0) {
			int open_index = rng() % choices.size();
			apply_open(choices[open_index]);
			return choices[open_index];
		} else {
			return rc_coord{ 0xffff,0xffff };
		}
	}




}

